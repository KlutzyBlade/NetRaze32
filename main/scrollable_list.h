#ifndef SCROLLABLE_LIST_H
#define SCROLLABLE_LIST_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char items[50][64];
    int count;
    int scroll_offset;
    int max_visible;
} scrollable_list_t;

void scrollable_list_init(scrollable_list_t* list, int max_visible);
void scrollable_list_add(scrollable_list_t* list, const char* item);
void scrollable_list_draw(scrollable_list_t* list, int start_y);
bool scrollable_list_handle_touch(scrollable_list_t* list, int touch_y);
void scrollable_list_clear(scrollable_list_t* list);

#endif // SCROLLABLE_LIST_H