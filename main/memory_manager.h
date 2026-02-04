#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Memory thresholds
#define MIN_FREE_HEAP_SIZE      8192   // 8KB minimum
#define CRITICAL_HEAP_SIZE      4096   // 4KB critical
#define HEAP_CHECK_INTERVAL_MS  5000   // Check every 5 seconds

// Memory management functions
void memory_manager_init(void);
bool check_memory_health(void);
void force_garbage_collection(void);
size_t get_largest_free_block(void);
void log_memory_stats(void);
bool is_memory_critical(void);

#endif // MEMORY_MANAGER_H