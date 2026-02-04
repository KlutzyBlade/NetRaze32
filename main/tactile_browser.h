#ifndef TACTILE_BROWSER_H
#define TACTILE_BROWSER_H

#include "esp_err.h"
#include <stdbool.h>

typedef struct {
    char url[128];
    char title[32];
} bookmark_t;

typedef struct {
    char url[128];
    char title[32];
    uint32_t timestamp;
} history_entry_t;

esp_err_t browser_init(void);
void browser_start(void);
void browser_navigate(const char* url);
void browser_add_bookmark(const char* url, const char* title);
void browser_show_bookmarks(void);
void browser_show_history(void);
void browser_settings(void);

#endif // TACTILE_BROWSER_H