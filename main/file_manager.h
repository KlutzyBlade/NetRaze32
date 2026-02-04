#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define MAX_FILENAME_LEN 64
#define MAX_PATH_LEN 512
#define MAX_FILES_PER_DIR 50

typedef enum {
    FILE_TYPE_DIR,
    FILE_TYPE_TXT,
    FILE_TYPE_CSV,
    FILE_TYPE_LOG,
    FILE_TYPE_BIN,
    FILE_TYPE_UNKNOWN
} file_type_t;

typedef struct {
    char name[MAX_FILENAME_LEN];
    char path[MAX_PATH_LEN];
    file_type_t type;
    size_t size;
    bool is_selected;
} file_entry_t;

typedef struct {
    file_entry_t files[MAX_FILES_PER_DIR];
    int file_count;
    int current_index;
    char current_path[MAX_PATH_LEN];
} file_browser_t;

void file_manager_init(void);
void file_manager_show(void);
void file_manager_navigate_up(void);
void file_manager_navigate_down(void);
void file_manager_select_file(void);
void file_manager_delete_file(void);
void file_manager_copy_file(void);
void file_manager_view_file(void);
bool file_manager_handle_touch(int x, int y);

#endif