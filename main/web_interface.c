#include "web_interface.h"
#include "esp_http_server.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "display.h"
#include "touchscreen.h"
#include "wifi_functions.h"
#include "bluetooth_functions.h"

static const char* TAG = "WEB_INTERFACE";
static httpd_handle_t server = NULL;

static const char* html_page = 
"<!DOCTYPE html>"
"<html><head><title>ESP32-DIV Control</title>"
"<style>body{font-family:Arial;margin:20px;background:#1a1a1a;color:#fff;}"
"button{padding:10px 20px;margin:5px;background:#ff6600;color:#fff;border:none;border-radius:5px;cursor:pointer;}"
"button:hover{background:#ff8833;}"
"h1{color:#ff6600;}</style></head>"
"<body><h1>ESP32-DIV Remote Control</h1>"
"<h2>WiFi Attacks</h2>"
"<button onclick=\"fetch('/wifi/scan')\">WiFi Scan</button>"
"<button onclick=\"fetch('/wifi/deauth')\">Deauth Attack</button>"
"<button onclick=\"fetch('/wifi/evil_twin')\">Evil Twin</button>"
"<button onclick=\"fetch('/wifi/karma')\">Karma Attack</button>"
"<button onclick=\"fetch('/wifi/rickroll')\">RickRoll AP</button>"
"<h2>BLE Attacks</h2>"
"<button onclick=\"fetch('/ble/scan')\">BLE Scan</button>"
"<button onclick=\"fetch('/ble/apple_spam')\">Apple Spam</button>"
"<button onclick=\"fetch('/ble/samsung_spam')\">Samsung Spam</button>"
"<button onclick=\"fetch('/ble/swiftpair')\">SwiftPair Spam</button>"
"<button onclick=\"fetch('/ble/beacon_flood')\">Beacon Flood</button>"
"<h2>Status</h2>"
"<div id=\"status\">Ready</div>"
"<script>setInterval(()=>fetch('/status').then(r=>r.text()).then(t=>document.getElementById('status').innerHTML=t),2000);</script>"
"</body></html>";

static esp_err_t root_handler(httpd_req_t *req) {
    httpd_resp_send(req, html_page, strlen(html_page));
    return ESP_OK;
}

static esp_err_t wifi_scan_handler(httpd_req_t *req) {
    wifi_scan_start();
    httpd_resp_send(req, "WiFi scan started", 17);
    return ESP_OK;
}

static esp_err_t wifi_deauth_handler(httpd_req_t *req) {
    wifi_deauth_attack();
    httpd_resp_send(req, "Deauth attack started", 21);
    return ESP_OK;
}

static esp_err_t wifi_evil_twin_handler(httpd_req_t *req) {
    wifi_evil_twin_attack();
    httpd_resp_send(req, "Evil twin started", 17);
    return ESP_OK;
}

static esp_err_t wifi_karma_handler(httpd_req_t *req) {
    wifi_karma_attack();
    httpd_resp_send(req, "Karma attack started", 20);
    return ESP_OK;
}

static esp_err_t wifi_rickroll_handler(httpd_req_t *req) {
    wifi_rickroll_attack();
    httpd_resp_send(req, "RickRoll AP started", 19);
    return ESP_OK;
}

static esp_err_t ble_scan_handler(httpd_req_t *req) {
    ble_scan_start();
    httpd_resp_send(req, "BLE scan started", 16);
    return ESP_OK;
}

static esp_err_t ble_apple_spam_handler(httpd_req_t *req) {
    ble_apple_spam();
    httpd_resp_send(req, "Apple spam started", 18);
    return ESP_OK;
}

static esp_err_t ble_samsung_spam_handler(httpd_req_t *req) {
    ble_samsung_spam();
    httpd_resp_send(req, "Samsung spam started", 20);
    return ESP_OK;
}

static esp_err_t ble_swiftpair_handler(httpd_req_t *req) {
    ble_swiftpair_spam();
    httpd_resp_send(req, "SwiftPair spam started", 22);
    return ESP_OK;
}

static esp_err_t ble_beacon_flood_handler(httpd_req_t *req) {
    ble_beacon_flood();
    httpd_resp_send(req, "Beacon flood started", 20);
    return ESP_OK;
}

static esp_err_t status_handler(httpd_req_t *req) {
    httpd_resp_send(req, "ESP32-DIV Online", 16);
    return ESP_OK;
}

esp_err_t web_interface_init(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t root_uri = {.uri = "/", .method = HTTP_GET, .handler = root_handler};
        httpd_register_uri_handler(server, &root_uri);
        
        httpd_uri_t wifi_scan_uri = {.uri = "/wifi/scan", .method = HTTP_GET, .handler = wifi_scan_handler};
        httpd_register_uri_handler(server, &wifi_scan_uri);
        
        httpd_uri_t wifi_deauth_uri = {.uri = "/wifi/deauth", .method = HTTP_GET, .handler = wifi_deauth_handler};
        httpd_register_uri_handler(server, &wifi_deauth_uri);
        
        httpd_uri_t wifi_evil_twin_uri = {.uri = "/wifi/evil_twin", .method = HTTP_GET, .handler = wifi_evil_twin_handler};
        httpd_register_uri_handler(server, &wifi_evil_twin_uri);
        
        httpd_uri_t wifi_karma_uri = {.uri = "/wifi/karma", .method = HTTP_GET, .handler = wifi_karma_handler};
        httpd_register_uri_handler(server, &wifi_karma_uri);
        
        httpd_uri_t wifi_rickroll_uri = {.uri = "/wifi/rickroll", .method = HTTP_GET, .handler = wifi_rickroll_handler};
        httpd_register_uri_handler(server, &wifi_rickroll_uri);
        
        httpd_uri_t ble_scan_uri = {.uri = "/ble/scan", .method = HTTP_GET, .handler = ble_scan_handler};
        httpd_register_uri_handler(server, &ble_scan_uri);
        
        httpd_uri_t ble_apple_spam_uri = {.uri = "/ble/apple_spam", .method = HTTP_GET, .handler = ble_apple_spam_handler};
        httpd_register_uri_handler(server, &ble_apple_spam_uri);
        
        httpd_uri_t ble_samsung_spam_uri = {.uri = "/ble/samsung_spam", .method = HTTP_GET, .handler = ble_samsung_spam_handler};
        httpd_register_uri_handler(server, &ble_samsung_spam_uri);
        
        httpd_uri_t ble_swiftpair_uri = {.uri = "/ble/swiftpair", .method = HTTP_GET, .handler = ble_swiftpair_handler};
        httpd_register_uri_handler(server, &ble_swiftpair_uri);
        
        httpd_uri_t ble_beacon_flood_uri = {.uri = "/ble/beacon_flood", .method = HTTP_GET, .handler = ble_beacon_flood_handler};
        httpd_register_uri_handler(server, &ble_beacon_flood_uri);
        
        httpd_uri_t status_uri = {.uri = "/status", .method = HTTP_GET, .handler = status_handler};
        httpd_register_uri_handler(server, &status_uri);
        
        ESP_LOGI(TAG, "Web interface started on port 80");
        return ESP_OK;
    }
    
    ESP_LOGE(TAG, "Failed to start web server");
    return ESP_FAIL;
}

void web_interface_start(void) {
    display_fill_screen(COLOR_BLACK);
    display_draw_text(10, 10, "Web Interface", COLOR_WHITE, COLOR_BLACK);
    display_draw_text(10, 40, "Starting AP mode...", COLOR_ORANGE, COLOR_BLACK);
    
    wifi_config_t ap_config = {
        .ap = {
            .ssid = "ESP32-DIV-Control",
            .ssid_len = strlen("ESP32-DIV-Control"),
            .channel = 6,
            .password = "changeme123",
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA2_PSK
        }
    };
    
    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &ap_config);
    esp_wifi_start();
    
    web_interface_init();
    
    display_draw_text(10, 60, "AP: ESP32-DIV-Control", COLOR_GREEN, COLOR_BLACK);
    display_draw_text(10, 80, "Pass: changeme123", COLOR_BLUE, COLOR_BLACK);
    display_draw_text(10, 100, "URL: http://192.168.4.1", COLOR_ORANGE, COLOR_BLACK);
    display_draw_text(10, 140, "Web interface active!", COLOR_GREEN, COLOR_BLACK);
    display_draw_text(10, 280, "Touch to stop", COLOR_GRAY, COLOR_BLACK);
    
    while (!touchscreen_is_touched()) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    web_interface_stop();
}

void web_interface_stop(void) {
    if (server) {
        httpd_stop(server);
        server = NULL;
    }
    
    esp_wifi_stop();
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    
    ESP_LOGI(TAG, "Web interface stopped");
}