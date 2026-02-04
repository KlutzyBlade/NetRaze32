#include "find3_scanner.h"
#include "esp_wifi.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cJSON.h"
#include <string.h>

static const char *TAG = "FIND3";
static find3_config_t g_config;
static TaskHandle_t scan_task_handle = NULL;
static bool is_scanning = false;

static esp_err_t http_post_data(const char *url, const char *json_data) {
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 5000,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, json_data, strlen(json_data));
    
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP POST Status = %d", esp_http_client_get_status_code(client));
    } else {
        ESP_LOGE(TAG, "HTTP POST failed: %s", esp_err_to_name(err));
    }
    
    esp_http_client_cleanup(client);
    return err;
}

static void scan_task(void *pvParameters) {
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = true,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
    };
    
    while (is_scanning) {
        esp_wifi_scan_start(&scan_config, true);
        
        uint16_t ap_count = 0;
        esp_wifi_scan_get_ap_num(&ap_count);
        
        if (ap_count > 0) {
            wifi_ap_record_t *ap_list = malloc(sizeof(wifi_ap_record_t) * ap_count);
            if (ap_list) {
                esp_wifi_scan_get_ap_records(&ap_count, ap_list);
                
                cJSON *root = cJSON_CreateObject();
                cJSON_AddStringToObject(root, "d", g_config.device_name);
                cJSON_AddStringToObject(root, "f", g_config.family_name);
                cJSON_AddNumberToObject(root, "t", esp_timer_get_time() / 1000);
                
                if (g_config.learning_mode) {
                    cJSON_AddStringToObject(root, "l", g_config.location);
                }
                
                cJSON *wifi_obj = cJSON_CreateObject();
                for (int i = 0; i < ap_count; i++) {
                    char mac_str[18];
                    snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
                             ap_list[i].bssid[0], ap_list[i].bssid[1], ap_list[i].bssid[2],
                             ap_list[i].bssid[3], ap_list[i].bssid[4], ap_list[i].bssid[5]);
                    cJSON_AddNumberToObject(wifi_obj, mac_str, ap_list[i].rssi);
                }
                cJSON_AddItemToObject(root, "s", cJSON_CreateObject());
                cJSON_AddItemToObject(cJSON_GetObjectItem(root, "s"), "wifi", wifi_obj);
                
                char *json_str = cJSON_PrintUnformatted(root);
                if (json_str) {
                    char url[256];
                    snprintf(url, sizeof(url), "%s/%s", g_config.server_url, 
                             g_config.learning_mode ? "learn" : "track");
                    http_post_data(url, json_str);
                    free(json_str);
                }
                
                cJSON_Delete(root);
                free(ap_list);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(g_config.scan_interval_ms));
    }
    
    scan_task_handle = NULL;
    vTaskDelete(NULL);
}

esp_err_t find3_init(const find3_config_t *config) {
    if (!config) return ESP_ERR_INVALID_ARG;
    memcpy(&g_config, config, sizeof(find3_config_t));
    ESP_LOGI(TAG, "FIND3 initialized: %s", g_config.device_name);
    return ESP_OK;
}

esp_err_t find3_start_scanning(void) {
    if (is_scanning) return ESP_ERR_INVALID_STATE;
    is_scanning = true;
    xTaskCreate(scan_task, "find3_scan", 4096, NULL, 5, &scan_task_handle);
    ESP_LOGI(TAG, "Scanning started");
    return ESP_OK;
}

esp_err_t find3_stop_scanning(void) {
    is_scanning = false;
    if (scan_task_handle) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    ESP_LOGI(TAG, "Scanning stopped");
    return ESP_OK;
}

esp_err_t find3_set_learning_mode(bool enable, const char *location) {
    g_config.learning_mode = enable;
    if (enable && location) {
        strncpy(g_config.location, location, sizeof(g_config.location) - 1);
    }
    ESP_LOGI(TAG, "Learning mode: %s", enable ? "ON" : "OFF");
    return ESP_OK;
}

bool find3_is_scanning(void) {
    return is_scanning;
}
