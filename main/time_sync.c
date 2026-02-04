#include "time_sync.h"
#include "esp_sntp.h"
#include "esp_log.h"
#include <time.h>
#include <sys/time.h>
#include <string.h>

static const char* TAG = "TIME_SYNC";
static bool time_synced = false;

static void time_sync_notification_cb(struct timeval *tv) {
    ESP_LOGI(TAG, "Time synchronized");
    time_synced = true;
}

esp_err_t time_sync_init(void) {
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    
    // Set timezone to EST (UTC-5)
    setenv("TZ", "EST5EDT,M3.2.0/2,M11.1.0", 1);
    tzset();
    
    ESP_LOGI(TAG, "Time sync initialized");
    return ESP_OK;
}

void time_sync_start(void) {
    if (!esp_sntp_enabled()) {
        esp_sntp_init();
        ESP_LOGI(TAG, "SNTP started");
    }
}

bool is_time_synced(void) {
    return time_synced;
}

void get_current_time_string(char* time_str, size_t max_len) {
    if (!time_synced) {
        strncpy(time_str, "12:34 PM", max_len - 1);
        time_str[max_len - 1] = '\0';
        return;
    }
    
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    
    strftime(time_str, max_len, "%I:%M %p", &timeinfo);
}