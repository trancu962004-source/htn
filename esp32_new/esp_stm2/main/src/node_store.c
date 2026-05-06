#include "node_store.h"

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "board_config.h"

static const char *TAG = "NODE_STORE";
static node_state_t g_nodes[MAX_NODES];

void node_store_init(void)
{
    memset(g_nodes, 0, sizeof(g_nodes));
}

static int find_node(uint8_t wing_id, uint8_t room_id, uint8_t bed_id,
                     uint8_t patient_id, uint8_t node_type)
{
    for (int i = 0; i < MAX_NODES; i++) {
        if (g_nodes[i].used &&
            g_nodes[i].wing_id == wing_id &&
            g_nodes[i].room_id == room_id &&
            g_nodes[i].bed_id == bed_id &&
            g_nodes[i].patient_id == patient_id &&
            g_nodes[i].node_type == node_type) {
            return i;
        }
    }
    return -1;
}

static int alloc_node(uint8_t wing_id, uint8_t room_id, uint8_t bed_id,
                      uint8_t patient_id, uint8_t node_type)
{
    for (int i = 0; i < MAX_NODES; i++) {
        if (!g_nodes[i].used) {
            memset(&g_nodes[i], 0, sizeof(g_nodes[i]));
            g_nodes[i].used = true;
            g_nodes[i].wing_id = wing_id;
            g_nodes[i].room_id = room_id;
            g_nodes[i].bed_id = bed_id;
            g_nodes[i].patient_id = patient_id;
            g_nodes[i].node_type = node_type;
            g_nodes[i].last_seen_ms = (uint32_t)esp_log_timestamp();
            return i;
        }
    }
    return -1;
}

int node_store_find_or_alloc(uint8_t wing_id, uint8_t room_id, uint8_t bed_id,
                             uint8_t patient_id, uint8_t node_type)
{
    int idx = find_node(wing_id, room_id, bed_id, patient_id, node_type);
    if (idx >= 0) {
        return idx;
    }
    return alloc_node(wing_id, room_id, bed_id, patient_id, node_type);
}

node_state_t *node_store_get(int idx)
{
    if (idx < 0 || idx >= MAX_NODES) {
        return NULL;
    }
    return &g_nodes[idx];
}

void node_monitor_task(void *arg)
{
    (void)arg;

    while (1) {
        uint32_t now = (uint32_t)esp_log_timestamp();

        for (int i = 0; i < MAX_NODES; i++) {
            if (g_nodes[i].used && (now - g_nodes[i].last_seen_ms > NODE_TIMEOUT_MS)) {
                ESP_LOGW(TAG,
                         "TIMEOUT | W=%c R=%u B=%u P=%u TYPE=%u",
                         g_nodes[i].wing_id,
                         g_nodes[i].room_id,
                         g_nodes[i].bed_id,
                         g_nodes[i].patient_id,
                         g_nodes[i].node_type);
                g_nodes[i].used = false;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
