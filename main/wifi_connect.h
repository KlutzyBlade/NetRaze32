#ifndef WIFI_CONNECT_H
#define WIFI_CONNECT_H

#include "esp_err.h"

void wifi_connect_menu(void);
esp_err_t wifi_connect_to_network(const char* ssid, const char* password);

#endif // WIFI_CONNECT_H