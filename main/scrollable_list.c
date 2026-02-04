#include "scrollable_list.h"
#include "display.h"
#include <string.h>

void scrollable_list_init(scrollable_list_t* list, int max_visible) {
    list->count = 0;
    list->scroll_offset = 0;
    list->max_visible = max_visible;
}

void scrollable_list_add(scrollable_list_t* list, const char* item) {
    if (list->count < 50) {
        strncpy(list->items[list->count], item, 63);
        list->items[list->count][63] = '\0';
        list->count++;
    }
}

void scrollable_list_draw(scrollable_list_t* list, int start_y) {
    // Clear list area
    display_fill_rect(10, start_y, 220, list->max_visible * 15, COLOR_BLACK);
    
    for (int i = 0; i < list->max_visible && (i + list->scroll_offset) < list->count; i++) {
        int item_index = i + list->scroll_offset;
        display_draw_text(10, start_y + i * 15, list->items[item_index], COLOR_GREEN, COLOR_BLACK);
    }
    
    // Draw scroll indicators and instructions
    if (list->count > list->max_visible) {
        char scroll_info[32];
        snprintf(scroll_info, sizeof(scroll_info), "%d-%d of %d", 
                list->scroll_offset + 1, 
                (list->scroll_offset + list->max_visible > list->count) ? list->count : list->scroll_offset + list->max_visible,
                list->count);
        display_draw_text(10, start_y - 20, scroll_info, COLOR_GRAY, COLOR_BLACK);
        
        // Draw scroll arrows
        if (list->scroll_offset > 0) {
            display_draw_text(200, start_y - 20, "^ UP", COLOR_BLUE, COLOR_BLACK);
        }
        if (list->scroll_offset < list->count - list->max_visible) {
            display_draw_text(200, start_y + list->max_visible * 15, "v DOWN", COLOR_BLUE, COLOR_BLACK);
        }
    }
}

bool scrollable_list_handle_touch(scrollable_list_t* list, int touch_y) {
    if (touch_y < 150) { // Upper half - scroll up
        if (list->scroll_offset > 0) {
            list->scroll_offset--;
            return true;
        }
    } else if (touch_y > 150 && touch_y < 270) { // Lower half - scroll down
        if (list->scroll_offset < list->count - list->max_visible) {
            list->scroll_offset++;
            return true;
        }
    }
    return false;
}

void scrollable_list_clear(scrollable_list_t* list) {
    list->count = 0;
    list->scroll_offset = 0;
}