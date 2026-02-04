#include "cc1101_driver.h"
#include "board_config.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "CC1101";
static spi_device_handle_t spi_handle = NULL;
static bool initialized = false;

// CC1101 Register addresses
#define CC1101_IOCFG2       0x00
#define CC1101_FREQ2        0x0D
#define CC1101_FREQ1        0x0E
#define CC1101_FREQ0        0x0F
#define CC1101_MDMCFG4      0x10
#define CC1101_MDMCFG3      0x11
#define CC1101_MDMCFG2      0x12
#define CC1101_DEVIATN      0x15
#define CC1101_MCSM0        0x18
#define CC1101_FOCCFG       0x19
#define CC1101_AGCCTRL2     0x17
#define CC1101_WORCTRL      0x20
#define CC1101_FREND0       0x22
#define CC1101_FSCAL3       0x23
#define CC1101_FSCAL2       0x24
#define CC1101_FSCAL1       0x25
#define CC1101_FSCAL0       0x26
#define CC1101_TEST2        0x2C
#define CC1101_TEST1        0x2D
#define CC1101_TEST0        0x2E
#define CC1101_PARTNUM      0x30
#define CC1101_VERSION      0x31
#define CC1101_RSSI         0x34
#define CC1101_TXFIFO       0x3F
#define CC1101_RXFIFO       0x3F

// Command strobes
#define CC1101_SRES         0x30
#define CC1101_SRX          0x34
#define CC1101_STX          0x35
#define CC1101_SIDLE        0x36

static void cc1101_write_reg(uint8_t reg, uint8_t value) {
    uint8_t tx[2] = {reg, value};
    spi_transaction_t t = {
        .length = 16,
        .tx_buffer = tx
    };
    spi_device_transmit(spi_handle, &t);
}

static uint8_t cc1101_read_reg(uint8_t reg) {
    uint8_t tx[2] = {reg | 0x80, 0x00};
    uint8_t rx[2] = {0};
    spi_transaction_t t = {
        .length = 16,
        .tx_buffer = tx,
        .rx_buffer = rx
    };
    spi_device_transmit(spi_handle, &t);
    return rx[1];
}

static void cc1101_strobe(uint8_t strobe) {
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &strobe
    };
    spi_device_transmit(spi_handle, &t);
}

esp_err_t cc1101_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << CC1101_GDO0_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = 1000000,
        .mode = 0,
        .spics_io_num = CC1101_CS_PIN,
        .queue_size = 7
    };

    esp_err_t ret = spi_bus_add_device(SPI3_HOST, &dev_cfg, &spi_handle);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "CC1101 not connected (SPI add failed)");
        initialized = false;
        return ESP_ERR_NOT_FOUND;
    }

    cc1101_strobe(CC1101_SRES);
    vTaskDelay(pdMS_TO_TICKS(10));

    uint8_t partnum = cc1101_read_reg(CC1101_PARTNUM);
    uint8_t version = cc1101_read_reg(CC1101_VERSION);
    
    if (partnum == 0x00 || partnum == 0xFF) {
        ESP_LOGW(TAG, "CC1101 not detected");
        initialized = false;
        return ESP_ERR_NOT_FOUND;
    }

    ESP_LOGI(TAG, "CC1101 detected: Part=0x%02X Ver=0x%02X", partnum, version);
    
    // Configure for 433.92 MHz (module's antenna frequency)
    cc1101_set_frequency(CC1101_FREQ_433);
    
    // Basic configuration for ASK/OOK modulation
    cc1101_write_reg(CC1101_MDMCFG2, 0x30);  // ASK/OOK, no sync
    cc1101_write_reg(CC1101_MDMCFG4, 0xC8);  // Channel BW
    cc1101_write_reg(CC1101_DEVIATN, 0x47);  // Deviation
    cc1101_write_reg(CC1101_FREND0, 0x11);   // Front end TX config
    cc1101_write_reg(CC1101_MCSM0, 0x18);    // Auto-calibrate
    
    initialized = true;
    return ESP_OK;
}

bool cc1101_is_connected(void) {
    return initialized;
}

esp_err_t cc1101_set_frequency(cc1101_freq_t freq) {
    if (!initialized) return ESP_ERR_INVALID_STATE;
    
    uint32_t freq_val;
    switch (freq) {
        case CC1101_FREQ_315: freq_val = 0x0C1D89; break; // 315 MHz
        case CC1101_FREQ_433: freq_val = 0x10A762; break; // 433.92 MHz
        case CC1101_FREQ_868: freq_val = 0x216276; break; // 868 MHz
        case CC1101_FREQ_915: freq_val = 0x2338C4; break; // 915 MHz
        default: return ESP_ERR_INVALID_ARG;
    }
    
    cc1101_write_reg(CC1101_FREQ2, (freq_val >> 16) & 0xFF);
    cc1101_write_reg(CC1101_FREQ1, (freq_val >> 8) & 0xFF);
    cc1101_write_reg(CC1101_FREQ0, freq_val & 0xFF);
    
    return ESP_OK;
}

esp_err_t cc1101_set_rx_mode(void) {
    if (!initialized) return ESP_ERR_INVALID_STATE;
    cc1101_strobe(CC1101_SRX);
    return ESP_OK;
}

esp_err_t cc1101_set_tx_mode(void) {
    if (!initialized) return ESP_ERR_INVALID_STATE;
    cc1101_strobe(CC1101_STX);
    return ESP_OK;
}

int8_t cc1101_get_rssi(void) {
    if (!initialized) return -128;
    uint8_t rssi_raw = cc1101_read_reg(CC1101_RSSI);
    return (rssi_raw >= 128) ? (rssi_raw - 256) / 2 - 74 : rssi_raw / 2 - 74;
}

esp_err_t cc1101_transmit(const uint8_t *data, size_t len) {
    if (!initialized) return ESP_ERR_INVALID_STATE;
    if (len > 64) return ESP_ERR_INVALID_SIZE;
    
    cc1101_strobe(CC1101_SIDLE);
    cc1101_write_reg(CC1101_TXFIFO, len);
    for (size_t i = 0; i < len; i++) {
        cc1101_write_reg(CC1101_TXFIFO, data[i]);
    }
    cc1101_strobe(CC1101_STX);
    
    return ESP_OK;
}

esp_err_t cc1101_receive(uint8_t *data, size_t *len, uint32_t timeout_ms) {
    if (!initialized) return ESP_ERR_INVALID_STATE;
    
    cc1101_strobe(CC1101_SRX);
    
    uint32_t start = xTaskGetTickCount();
    while ((xTaskGetTickCount() - start) < pdMS_TO_TICKS(timeout_ms)) {
        if (gpio_get_level(CC1101_GDO0_PIN)) {
            uint8_t rx_len = cc1101_read_reg(CC1101_RXFIFO);
            if (rx_len > 0 && rx_len <= 64) {
                *len = rx_len;
                for (size_t i = 0; i < rx_len; i++) {
                    data[i] = cc1101_read_reg(CC1101_RXFIFO);
                }
                return ESP_OK;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    return ESP_ERR_TIMEOUT;
}
