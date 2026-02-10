#include "packet_capture.h"
#include "led_alerts.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_spiffs.h"
#include <stdio.h>
#include <string.h>

static const char* TAG = "PKT_CAP";
static packet_capture_t capture = {0};

esp_err_t packet_capture_init(void) {
    memset(&capture, 0, sizeof(packet_capture_t));
    
    // Initialize SPIFFS
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };
    
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init SPIFFS: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "Packet capture initialized with SPIFFS");
    return ESP_OK;
}

esp_err_t packet_capture_start(const char* filename) {
    if (capture.active) {
        ESP_LOGW(TAG, "Capture already active");
        return ESP_ERR_INVALID_STATE;
    }
    
    // Create full path in SPIFFS
    char filepath[80];
    snprintf(filepath, sizeof(filepath), "/spiffs/%s", filename);
    
    // Open file for writing
    FILE* f = fopen(filepath, "wb");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open file: %s", filepath);
        return ESP_FAIL;
    }
    
    // Write PCAP file header
    pcap_file_header_t pcap_header = {
        .magic_number = 0xa1b2c3d4,
        .version_major = 2,
        .version_minor = 4,
        .thiszone = 0,
        .sigfigs = 0,
        .snaplen = 65535,
        .network = 105  // IEEE 802.11
    };
    
    if (fwrite(&pcap_header, sizeof(pcap_header), 1, f) != 1) {
        ESP_LOGE(TAG, "Failed to write PCAP header");
        fclose(f);
        return ESP_FAIL;
    }
    
    capture.file_handle = f;
    capture.active = true;
    capture.packet_count = 0;
    capture.bytes_captured = 0;
    strncpy(capture.filename, filename, sizeof(capture.filename) - 1);
    
    led_alert_success();
    ESP_LOGI(TAG, "Packet capture started: %s", filepath);
    return ESP_OK;
}

esp_err_t packet_capture_stop(void) {
    if (!capture.active) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (capture.file_handle) {
        fclose((FILE*)capture.file_handle);
        capture.file_handle = NULL;
    }
    
    ESP_LOGI(TAG, "Capture stopped: %lu packets, %lu bytes", 
             (unsigned long)capture.packet_count, (unsigned long)capture.bytes_captured);
    
    if (capture.packet_count > 0) {
        led_alert_capture();
    }
    
    capture.active = false;
    return ESP_OK;
}

bool packet_capture_is_active(void) {
    return capture.active;
}

uint32_t packet_capture_get_count(void) {
    return capture.packet_count;
}

void packet_capture_handler(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (!capture.active || !capture.file_handle) {
        return;
    }
    
    wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    
    // Get timestamp
    int64_t now = esp_timer_get_time();
    uint32_t ts_sec = now / 1000000;
    uint32_t ts_usec = now % 1000000;
    
    // Write packet header
    pcap_packet_header_t pkt_header = {
        .ts_sec = ts_sec,
        .ts_usec = ts_usec,
        .incl_len = pkt->rx_ctrl.sig_len,
        .orig_len = pkt->rx_ctrl.sig_len
    };
    
    FILE* f = (FILE*)capture.file_handle;
    
    if (fwrite(&pkt_header, sizeof(pkt_header), 1, f) == 1) {
        if (fwrite(pkt->payload, pkt->rx_ctrl.sig_len, 1, f) == 1) {
            capture.packet_count++;
            capture.bytes_captured += pkt->rx_ctrl.sig_len;
            
            // Flush every 10 packets
            if (capture.packet_count % 10 == 0) {
                fflush(f);
            }
        }
    }
}
