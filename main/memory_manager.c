#include "memory_manager.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "MEMORY_MGR";

void memory_manager_init(void) {
    ESP_LOGI(TAG, "Memory manager initialized");
    log_memory_stats();
}

bool check_memory_health(void) {
    size_t free_heap = esp_get_free_heap_size();
    return free_heap > MIN_FREE_HEAP_SIZE;
}

void force_garbage_collection(void) {
    ESP_LOGW(TAG, "Forcing garbage collection");
    // Force heap defragmentation if available
    heap_caps_malloc_extmem_enable(0);
}

size_t get_largest_free_block(void) {
    return heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);
}

void log_memory_stats(void) {
    size_t free_heap = esp_get_free_heap_size();
    size_t min_free = esp_get_minimum_free_heap_size();
    size_t largest_block = get_largest_free_block();
    
    ESP_LOGI(TAG, "Free: %d, Min: %d, Largest: %d bytes", 
             free_heap, min_free, largest_block);
}

bool is_memory_critical(void) {
    return esp_get_free_heap_size() < CRITICAL_HEAP_SIZE;
}