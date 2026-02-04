#include "packet_logger.h"
#include "esp_log.h"
#include "esp_system.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

static const char* TAG = "PACKET_LOGGER";
static FILE* log_file = NULL;
static char current_log_filename[64];

bool packet_logger_init(void) {
    // Create logs directory if it doesn't exist
    struct stat st = {0};
    if (stat("/sdcard/logs", &st) == -1) {
        if (mkdir("/sdcard/logs", 0700) != 0) {
            ESP_LOGE(TAG, "Failed to create logs directory");
            return false;
        }
    }
    
    // Generate timestamped filename
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    
    snprintf(current_log_filename, sizeof(current_log_filename), 
             "/sdcard/logs/netRaze_%04d%02d%02d_%02d%02d%02d.log",
             timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
             timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    
    log_file = fopen(current_log_filename, "w");
    if (!log_file) {
        ESP_LOGE(TAG, "Failed to create log file: %s", current_log_filename);
        return false;
    }
    
    // Write header
    fprintf(log_file, "# NetRaze32 Packet Log\n");
    fprintf(log_file, "# Generated: %04d-%02d-%02d %02d:%02d:%02d\n",
            timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
            timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    fprintf(log_file, "# Format: TIMESTAMP,TYPE,DATA\n\n");
    fflush(log_file);
    
    ESP_LOGI(TAG, "Packet logger initialized: %s", current_log_filename);
    return true;
}

void packet_logger_close(void) {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
        ESP_LOGI(TAG, "Packet logger closed");
    }
}

static void write_log_entry(const char* type, const char* data) {
    if (!log_file) return;
    
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    
    fprintf(log_file, "%02d:%02d:%02d,%s,%s\n",
            timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
            type, data);
    fflush(log_file);
}

void packet_log_wifi_scan(const char* ssid, int rssi, const char* security) {
    char data[128];
    snprintf(data, sizeof(data), "SSID=%s,RSSI=%d,SEC=%s", ssid, rssi, security);
    write_log_entry("WIFI_SCAN", data);
}

void packet_log_deauth_attack(const char* target_mac, int count) {
    char data[64];
    snprintf(data, sizeof(data), "TARGET=%s,COUNT=%d", target_mac, count);
    write_log_entry("DEAUTH", data);
}

void packet_log_beacon_spam(const char* ssid, int count) {
    char data[64];
    snprintf(data, sizeof(data), "SSID=%s,COUNT=%d", ssid, count);
    write_log_entry("BEACON_SPAM", data);
}

void packet_log_ble_scan(const char* name, const char* mac, int rssi) {
    char data[128];
    snprintf(data, sizeof(data), "NAME=%s,MAC=%s,RSSI=%d", name, mac, rssi);
    write_log_entry("BLE_SCAN", data);
}

void packet_log_wps_attempt(const char* ssid, uint32_t pin) {
    char data[64];
    snprintf(data, sizeof(data), "SSID=%s,PIN=%08lu", ssid, pin);
    write_log_entry("WPS_ATTEMPT", data);
}

void packet_log_wps_success(const char* ssid, uint32_t pin) {
    char data[64];
    snprintf(data, sizeof(data), "SSID=%s,PIN=%08lu,STATUS=SUCCESS", ssid, pin);
    write_log_entry("WPS_SUCCESS", data);
}

void packet_log_handshake_capture(const char* ssid, const char* client_mac) {
    char data[96];
    snprintf(data, sizeof(data), "SSID=%s,CLIENT=%s", ssid, client_mac);
    write_log_entry("HANDSHAKE", data);
}

void packet_log_custom(const char* type, const char* data) {
    write_log_entry(type, data);
}

const char* packet_logger_get_filename(void) {
    return current_log_filename;
}