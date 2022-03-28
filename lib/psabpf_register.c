/*
 * Copyright 2022 Orange
 * Copyright 2022 Warsaw University of Technology
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <bpf/bpf.h>

#include <psabpf.h>
#include "common.h"
#include "btf.h"
#include "bpf_defs.h"

void psabpf_register_ctx_init(psabpf_register_context_t *ctx) {
    if (ctx == NULL)
        return;

    memset(ctx, 0, sizeof(psabpf_register_context_t));
}

void psabpf_register_ctx_free(psabpf_register_context_t *ctx) {
    if (ctx == NULL)
        return;

    free_btf(&ctx->btf_metadata);
    close_object_fd(&(ctx->reg.fd));
    free_struct_field_descriptor_set(&ctx->key_fds);
    free_struct_field_descriptor_set(&ctx->value_fds);
}

static int fill_key_btf_info(psabpf_register_context_t *ctx)
{
    uint32_t type_id = psabtf_get_member_type_id_by_name(ctx->btf_metadata.btf, ctx->reg.btf_type_id, "key");
    return parse_struct_type(&ctx->btf_metadata, type_id, ctx->reg.key_size, &ctx->key_fds);
}

static int fill_value_btf_info(psabpf_register_context_t *ctx)
{
    uint32_t type_id = psabtf_get_member_type_id_by_name(ctx->btf_metadata.btf, ctx->reg.btf_type_id, "value");
    return parse_struct_type(&ctx->btf_metadata, type_id, ctx->reg.value_size, &ctx->value_fds);
}

int psabpf_register_ctx_name(psabpf_context_t *psabpf_ctx, psabpf_register_context_t *ctx, const char *name) {
    if (psabpf_ctx == NULL || ctx == NULL || name == NULL)
        return EINVAL;

    /* get the BTF, will not work without it because there is too many possible configurations */
    if (load_btf(psabpf_ctx, &ctx->btf_metadata) != NO_ERROR) {
        fprintf(stderr, "couldn't find a BTF info\n");
        return ENOTSUP;
    }

    int ret = open_bpf_map(psabpf_ctx, name, &ctx->btf_metadata, &ctx->reg);
    if (ret != NO_ERROR) {
        fprintf(stderr, "couldn't open a register %s\n", name);
        return ret;
    }

    if (fill_key_btf_info(ctx) != NO_ERROR && fill_value_btf_info(ctx) != NO_ERROR) {
        fprintf(stderr, "%s: not a Register instance\n", name);
        close_object_fd(&ctx->reg.fd);
        return EOPNOTSUPP;
    }

    return NO_ERROR;
}

void psabpf_register_entry_init(psabpf_register_entry_t *entry) {
    if (entry == NULL)
        return;

    memset(entry, 0, sizeof(psabpf_register_entry_t));
}

void psabpf_register_entry_free(psabpf_register_entry_t *entry) {
    if (entry == NULL)
        return;

    free_struct_field_set(&entry->entry_key);

    if (entry->raw_key != NULL)
        free(entry->raw_key);
    entry->raw_key = NULL;

    if (entry->raw_value != NULL)
        free(entry->raw_value);
    entry->raw_value = NULL;
}

int psabpf_register_entry_set_key(psabpf_register_entry_t *entry, const void *data, size_t data_len) {
    if (entry == NULL)
        return EINVAL;
    if (data == NULL || data_len < 1)
        return ENODATA;

    int ret = struct_field_set_append(&entry->entry_key, data, data_len);
    if (ret != NO_ERROR)
        fprintf(stderr, "couldn't append key to an entry: %s\n", strerror(ret));
    return ret;
}

static void *allocate_key_buffer(psabpf_register_context_t *ctx, psabpf_register_entry_t *entry)
{
    if (entry->raw_key != NULL)
        return entry->raw_key;  /* already allocated */

    entry->raw_key = malloc(ctx->reg.key_size);
    if (entry->raw_key == NULL)
        fprintf(stderr, "not enough memory\n");

    return entry->raw_key;
}

static void *allocate_value_buffer(psabpf_register_context_t *ctx, psabpf_register_entry_t *entry)
{
    if (entry->raw_value != NULL)
        return entry->raw_value;

    entry->raw_value = malloc(ctx->reg.value_size);
    if (entry->raw_value == NULL)
        fprintf(stderr, "not enough memory\n");

    return entry->raw_value;
}

psabpf_struct_field_t * psabpf_register_get_next_field(psabpf_register_context_t *ctx, psabpf_register_entry_t *entry)
{
    if (ctx == NULL || entry == NULL)
        return NULL;

    psabpf_struct_field_descriptor_t *fd;
    fd = get_struct_field_descriptor(&ctx->value_fds, entry->current_field_id);
    if (fd == NULL) {
        entry->current_field_id = 0;
        return NULL;
    }

    entry->current.type = fd->type;
    entry->current.data_len = fd->data_len;
    entry->current.name = fd->name;
    entry->current.data = entry->raw_value + fd->data_offset;

    entry->current_field_id = entry->current_field_id + 1;

    return &entry->current;
}

int psabpf_register_get(psabpf_register_context_t *ctx, psabpf_register_entry_t *entry)
{
    if (allocate_key_buffer(ctx, entry) == NULL)
        return ENOMEM;

    int ret = construct_struct_from_fields(&entry->entry_key, &ctx->key_fds, entry->raw_key, ctx->reg.key_size);
    if (ret != NO_ERROR)
        return ret;

    if (allocate_value_buffer(ctx, entry) == NULL)
        return ENOMEM;

    ret = bpf_map_lookup_elem(ctx->reg.fd, entry->raw_key, entry->raw_value);
    if (ret != 0) {
        ret = errno;
        fprintf(stderr, "failed to read Register entry: %s\n", strerror(ret));
        return ret;
    }

    return NO_ERROR;
}

int psabpf_register_set(psabpf_register_context_t *ctx, psabpf_register_entry_t *entry) {
    // TODO implement
    return NO_ERROR;
}

int psabpf_register_reset(psabpf_register_context_t *ctx, psabpf_register_entry_t *entry) {
    // TODO implement
    return NO_ERROR;
}