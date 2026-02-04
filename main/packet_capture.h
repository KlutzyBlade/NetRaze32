#ifndef PACKET_CAPTURE_H
#define PACKET_CAPTURE_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "esp_wifi_types.h"

// PCAP file format structures
typedef struct {
    uint32_t magic_number;
    uint16_t version_major;
    uint16_t version_minor;
    int32_t  thiszone;
    uint32_t sigfigs;
    uint32_t snaplen;
    uint32_t network;
} pcap_file_header_t;

typedef struct {
    uint32_t ts_sec;
    uint32_t ts_usec;
    uint32_t incl_len;
    uint32_t orig_len;
} pcap_packet_header_t;

typedef struct {
    bool active;
    uint32_t packet_count;
    uint32_t bytes_captured;
    char filename[64];
    void* file_handle;
} packet_capture_t;

// Initialize packet capture system
esp_err_t packet_capture_init(void);

// Start capturing packets to file
esp_err_t packet_capture_start(const char* filename);

// Stop capturing and close file
esp_err_t packet_capture_stop(void);

// Get capture status
bool packet_capture_is_active(void);
uint32_t packet_capture_get_count(void);

// Packet handler callback
void packet_capture_handler(void* buf, wifi_promiscuous_pkt_type_t type);

#endif // PACKET_CAPTURE_H
