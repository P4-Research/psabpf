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

#ifndef __PSABPF_PRE_H
#define __PSABPF_PRE_H

#include "psabpf.h"

/*
 * PRE - Clone Sessions
 */
typedef uint32_t psabpf_clone_session_id_t;

struct psabpf_clone_session_entry {
    uint32_t  egress_port;
    uint16_t  instance;
    uint8_t   class_of_service;
    uint8_t   truncate;
    uint16_t  packet_length_bytes;
} __attribute__((aligned(4)));

typedef struct psabpf_clone_session_entry psabpf_clone_session_entry_t;

typedef struct psabpf_clone_session_ctx {
    psabpf_clone_session_id_t id;

    /* For iteration over entries in clone session */
    psabpf_bpf_map_descriptor_t session_map;
    psabpf_clone_session_entry_t current_entry;
    uint32_t current_egress_port;
    uint16_t current_instance;
} psabpf_clone_session_ctx_t;


void psabpf_clone_session_context_init(psabpf_clone_session_ctx_t *ctx);
void psabpf_clone_session_context_free(psabpf_clone_session_ctx_t *ctx);

void psabpf_clone_session_id(psabpf_clone_session_ctx_t *ctx, psabpf_clone_session_id_t id);
psabpf_clone_session_id_t psabpf_clone_session_get_id(psabpf_clone_session_ctx_t *ctx);

int psabpf_clone_session_create(psabpf_context_t *ctx, psabpf_clone_session_ctx_t *session);
bool psabpf_clone_session_exists(psabpf_context_t *ctx, psabpf_clone_session_ctx_t *session);
int psabpf_clone_session_delete(psabpf_context_t *ctx, psabpf_clone_session_ctx_t *session);

void psabpf_clone_session_entry_init(psabpf_clone_session_entry_t *entry);
void psabpf_clone_session_entry_free(psabpf_clone_session_entry_t *entry);

void psabpf_clone_session_entry_port(psabpf_clone_session_entry_t *entry, uint32_t egress_port);
uint32_t psabpf_clone_session_entry_get_port(psabpf_clone_session_entry_t *entry);
void psabpf_clone_session_entry_instance(psabpf_clone_session_entry_t *entry, uint16_t instance);
uint16_t psabpf_clone_session_entry_get_instance(psabpf_clone_session_entry_t *entry);
void psabpf_clone_session_entry_cos(psabpf_clone_session_entry_t *entry, uint8_t class_of_service);
uint8_t psabpf_clone_session_entry_get_cos(psabpf_clone_session_entry_t *entry);
void psabpf_clone_session_entry_truncate_enable(psabpf_clone_session_entry_t *entry, uint16_t packet_length_bytes);
void psabpf_clone_session_entry_truncate_disable(psabpf_clone_session_entry_t *entry);
bool psabpf_clone_session_entry_get_truncate_state(psabpf_clone_session_entry_t *entry);
uint16_t psabpf_clone_session_entry_get_truncate_length(psabpf_clone_session_entry_t *entry);

int psabpf_clone_session_entry_update(psabpf_context_t *ctx, psabpf_clone_session_ctx_t *session, psabpf_clone_session_entry_t *entry);
int psabpf_clone_session_entry_delete(psabpf_context_t *ctx, psabpf_clone_session_ctx_t *session, psabpf_clone_session_entry_t *entry);
int psabpf_clone_session_entry_exists(psabpf_context_t *ctx, psabpf_clone_session_ctx_t *session, psabpf_clone_session_entry_t *entry);
int psabpf_clone_session_entry_get(psabpf_context_t *ctx, psabpf_clone_session_ctx_t *session, psabpf_clone_session_entry_t *entry);

psabpf_clone_session_entry_t *psabpf_clone_session_get_next_entry(psabpf_context_t *ctx, psabpf_clone_session_ctx_t *session);

typedef struct psabpf_clone_session_list {
    psabpf_bpf_map_descriptor_t session_map;
    psabpf_clone_session_id_t current_id;
    psabpf_clone_session_ctx_t current_session;
} psabpf_clone_session_list_t;

int psabpf_clone_session_list_init(psabpf_context_t *ctx, psabpf_clone_session_list_t *list);
void psabpf_clone_session_list_free(psabpf_clone_session_list_t *list);
psabpf_clone_session_ctx_t *psabpf_clone_session_list_get_next_group(psabpf_clone_session_list_t *list);

/*
 * PRE - Multicast Groups
 */
typedef uint32_t psabpf_mcast_grp_id_t;

typedef struct psabpf_mcast_grp_member {
    uint32_t egress_port;
    uint16_t instance;
} psabpf_mcast_grp_member_t;

typedef struct psabpf_mcast_grp_context {
    psabpf_mcast_grp_id_t id;

    /* For iteration over members */
    psabpf_bpf_map_descriptor_t group_map;
    psabpf_mcast_grp_member_t current_member;
    uint32_t current_egress_port;
    uint16_t current_instance;
} psabpf_mcast_grp_ctx_t;

void psabpf_mcast_grp_context_init(psabpf_mcast_grp_ctx_t *group);
void psabpf_mcast_grp_context_free(psabpf_mcast_grp_ctx_t *group);

void psabpf_mcast_grp_id(psabpf_mcast_grp_ctx_t *group, psabpf_mcast_grp_id_t mcast_grp_id);
psabpf_mcast_grp_id_t psabpf_mcast_grp_get_id(psabpf_mcast_grp_ctx_t *group);

int psabpf_mcast_grp_create(psabpf_context_t *ctx, psabpf_mcast_grp_ctx_t *group);
bool psabpf_mcast_grp_exists(psabpf_context_t *ctx, psabpf_mcast_grp_ctx_t *group);
int psabpf_mcast_grp_delete(psabpf_context_t *ctx, psabpf_mcast_grp_ctx_t *group);

void psabpf_mcast_grp_member_init(psabpf_mcast_grp_member_t *member);
void psabpf_mcast_grp_member_free(psabpf_mcast_grp_member_t *member);

void psabpf_mcast_grp_member_port(psabpf_mcast_grp_member_t *member, uint32_t egress_port);
void psabpf_mcast_grp_member_instance(psabpf_mcast_grp_member_t *member, uint16_t instance);

uint32_t psabpf_mcast_grp_member_get_port(psabpf_mcast_grp_member_t *member);
uint16_t psabpf_mcast_grp_member_get_instance(psabpf_mcast_grp_member_t *member);

int psabpf_mcast_grp_member_update(psabpf_context_t *ctx, psabpf_mcast_grp_ctx_t *group, psabpf_mcast_grp_member_t *member);
int psabpf_mcast_grp_member_exists(psabpf_context_t *ctx, psabpf_mcast_grp_ctx_t *group, psabpf_mcast_grp_member_t *member);
int psabpf_mcast_grp_member_delete(psabpf_context_t *ctx, psabpf_mcast_grp_ctx_t *group, psabpf_mcast_grp_member_t *member);

psabpf_mcast_grp_member_t *psabpf_mcast_grp_get_next_member(psabpf_context_t *ctx, psabpf_mcast_grp_ctx_t *group);

typedef struct psabpf_mcast_grp_list {
    psabpf_bpf_map_descriptor_t group_map;
    psabpf_mcast_grp_id_t current_id;
    psabpf_mcast_grp_ctx_t current_group;
} psabpf_mcast_grp_list_t;

int psabpf_mcast_grp_list_init(psabpf_context_t *ctx, psabpf_mcast_grp_list_t *list);
void psabpf_mcast_grp_list_free(psabpf_mcast_grp_list_t *list);
psabpf_mcast_grp_ctx_t *psabpf_mcast_grp_list_get_next_group(psabpf_mcast_grp_list_t *list);

#endif  /* __PSABPF_PRE_H */
