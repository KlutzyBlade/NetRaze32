#include "display.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <string.h>

static const char* TAG = "DISPLAY";
spi_device_handle_t spi_device;
static SemaphoreHandle_t display_mutex = NULL;

// ILI9341 Commands
#define ILI9341_SWRESET     0x01
#define ILI9341_SLPOUT      0x11
#define ILI9341_DISPON      0x29
#define ILI9341_CASET       0x2A
#define ILI9341_PASET       0x2B
#define ILI9341_RAMWR       0x2C
#define ILI9341_MADCTL      0x36
#define ILI9341_COLMOD      0x3A

esp_err_t lcd_cmd(uint8_t cmd) {
    if (spi_device == NULL) {
        ESP_LOGE(TAG, "SPI device not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (display_mutex && xSemaphoreTake(display_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }
    
    gpio_set_level(LCD_DC_PIN, 0);
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &cmd,
    };
    esp_err_t ret = spi_device_polling_transmit(spi_device, &t);
    
    if (display_mutex) {
        xSemaphoreGive(display_mutex);
    }
    return ret;
}

esp_err_t lcd_data(uint8_t data) {
    if (spi_device == NULL) {
        ESP_LOGE(TAG, "SPI device not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (display_mutex && xSemaphoreTake(display_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }
    
    gpio_set_level(LCD_DC_PIN, 1);
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &data,
    };
    esp_err_t ret = spi_device_polling_transmit(spi_device, &t);
    
    if (display_mutex) {
        xSemaphoreGive(display_mutex);
    }
    return ret;
}

esp_err_t display_init(void) {
    // Create mutex first
    display_mutex = xSemaphoreCreateMutex();
    if (display_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create display mutex");
        return ESP_ERR_NO_MEM;
    }
    
    // Configure GPIO pins (DC, BL)
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << LCD_DC_PIN) | (1ULL << LCD_BL_PIN),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf);

    // Configure SPI bus
    spi_bus_config_t buscfg = {
        .miso_io_num = LCD_MISO_PIN,
        .mosi_io_num = LCD_MOSI_PIN,
        .sclk_io_num = LCD_SCK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * 2,
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = SPI_FREQUENCY,
        .mode = 0,
        .spics_io_num = LCD_CS_PIN,
        .queue_size = 7,
    };

    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "SPI bus init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi_device));

    // Hardware reset (skip if RST connected to ESP32-EN)
    vTaskDelay(pdMS_TO_TICKS(120));
    
    // Turn on backlight
    gpio_set_level(LCD_BL_PIN, 1);
    
    // Initialize ILI9341 with complete sequence
    lcd_cmd(ILI9341_SWRESET);
    vTaskDelay(pdMS_TO_TICKS(150));
    
    lcd_cmd(ILI9341_SLPOUT);
    vTaskDelay(pdMS_TO_TICKS(150));
    
    // Power control
    lcd_cmd(0xCB);
    lcd_data(0x39); lcd_data(0x2C); lcd_data(0x00); lcd_data(0x34); lcd_data(0x02);
    
    lcd_cmd(0xCF);
    lcd_data(0x00); lcd_data(0xC1); lcd_data(0x30);
    
    lcd_cmd(0xE8);
    lcd_data(0x85); lcd_data(0x00); lcd_data(0x78);
    
    lcd_cmd(0xEA);
    lcd_data(0x00); lcd_data(0x00);
    
    lcd_cmd(0xED);
    lcd_data(0x64); lcd_data(0x03); lcd_data(0x12); lcd_data(0x81);
    
    lcd_cmd(0xF7);
    lcd_data(0x20);
    
    lcd_cmd(0xC0);
    lcd_data(0x23);
    
    lcd_cmd(0xC1);
    lcd_data(0x10);
    
    lcd_cmd(0xC5);
    lcd_data(0x3E); lcd_data(0x28);
    
    lcd_cmd(0xC7);
    lcd_data(0x86);
    
    lcd_cmd(ILI9341_MADCTL);
    lcd_data(0x48); // BGR + MX for correct orientation
    
    lcd_cmd(ILI9341_COLMOD);
    lcd_data(0x55); // 16-bit color
    
    lcd_cmd(0xB1);
    lcd_data(0x00); lcd_data(0x18);
    
    lcd_cmd(0xB6);
    lcd_data(0x08); lcd_data(0x82); lcd_data(0x27);
    
    lcd_cmd(0xF2);
    lcd_data(0x00);
    
    lcd_cmd(0x26);
    lcd_data(0x01);
    
    lcd_cmd(0xE0);
    lcd_data(0x0F); lcd_data(0x31); lcd_data(0x2B); lcd_data(0x0C);
    lcd_data(0x0E); lcd_data(0x08); lcd_data(0x4E); lcd_data(0xF1);
    lcd_data(0x37); lcd_data(0x07); lcd_data(0x10); lcd_data(0x03);
    lcd_data(0x0E); lcd_data(0x09); lcd_data(0x00);
    
    lcd_cmd(0xE1);
    lcd_data(0x00); lcd_data(0x0E); lcd_data(0x14); lcd_data(0x03);
    lcd_data(0x11); lcd_data(0x07); lcd_data(0x31); lcd_data(0xC1);
    lcd_data(0x48); lcd_data(0x08); lcd_data(0x0F); lcd_data(0x0C);
    lcd_data(0x31); lcd_data(0x36); lcd_data(0x0F);
    
    lcd_cmd(ILI9341_SLPOUT);
    vTaskDelay(pdMS_TO_TICKS(120));
    
    lcd_cmd(ILI9341_DISPON);
    vTaskDelay(pdMS_TO_TICKS(120));
    
    display_fill_screen(COLOR_BLACK);

    ESP_LOGI(TAG, "Display initialized");
    return ESP_OK;
}

void display_fill_screen(uint16_t color) {
    display_fill_rect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, color);
}

void set_addr_window(int16_t x, int16_t y, int16_t w, int16_t h) {
    lcd_cmd(ILI9341_CASET);
    lcd_data(x >> 8);
    lcd_data(x & 0xFF);
    lcd_data((x + w - 1) >> 8);
    lcd_data((x + w - 1) & 0xFF);

    lcd_cmd(ILI9341_PASET);
    lcd_data(y >> 8);
    lcd_data(y & 0xFF);
    lcd_data((y + h - 1) >> 8);
    lcd_data((y + h - 1) & 0xFF);

    lcd_cmd(ILI9341_RAMWR);
}

void display_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT || x < 0 || y < 0) return;
    if (w <= 0 || h <= 0) return;
    if (x + w > DISPLAY_WIDTH) w = DISPLAY_WIDTH - x;
    if (y + h > DISPLAY_HEIGHT) h = DISPLAY_HEIGHT - y;
    if (w <= 0 || h <= 0) return;

    set_addr_window(x, y, w, h);
    
    uint16_t color_be = (color >> 8) | (color << 8);
    uint32_t pixels = w * h;
    
    gpio_set_level(LCD_DC_PIN, 1);
    
    if (pixels <= 64) {
        for (uint32_t i = 0; i < pixels; i++) {
            spi_transaction_t t = {
                .length = 16,
                .tx_buffer = &color_be,
            };
            spi_device_polling_transmit(spi_device, &t);
        }
    } else {
        static uint16_t line_buffer[240];
        for (int i = 0; i < (w > 240 ? 240 : w); i++) {
            line_buffer[i] = color_be;
        }
        
        for (int row = 0; row < h; row++) {
            spi_transaction_t t = {
                .length = w * 16,
                .tx_buffer = line_buffer,
            };
            spi_device_polling_transmit(spi_device, &t);
        }
    }
}

void display_draw_pixel(int16_t x, int16_t y, uint16_t color) {
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT || x < 0 || y < 0) return;
    display_fill_rect(x, y, 1, 1, color);
}

void display_draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    display_fill_rect(x, y, w, 1, color);
    display_fill_rect(x, y + h - 1, w, 1, color);
    display_fill_rect(x, y, 1, h, color);
    display_fill_rect(x + w - 1, y, 1, h, color);
}

// Complete font system
static const uint8_t font_5x7_letters[][5] = {
    {0x7C, 0x12, 0x11, 0x12, 0x7C}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
};

static const uint8_t font_5x7_numbers[][5] = {
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
};

static const uint8_t font_5x7_symbols[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // Space
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
};

void display_draw_text(int16_t x, int16_t y, const char* text, uint16_t color, uint16_t bg_color) {
    if (text == NULL) return;
    
    int len = strlen(text);
    if (len == 0) return;
    
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT || x < -30 || y < -7) return;
    
    for (int i = 0; i < len; i++) {
        char c = text[i];
        const uint8_t* bitmap = NULL;
        
        if (c >= 'A' && c <= 'Z') {
            bitmap = font_5x7_letters[c - 'A'];
        } else if (c >= 'a' && c <= 'z') {
            bitmap = font_5x7_letters[c - 'a'];
        } else if (c >= '0' && c <= '9') {
            bitmap = font_5x7_numbers[c - '0'];
        } else {
            switch (c) {
                case ' ': bitmap = font_5x7_symbols[0]; break;
                case '!': bitmap = font_5x7_symbols[1]; break;
                case '"': bitmap = font_5x7_symbols[2]; break;
                case '#': bitmap = font_5x7_symbols[3]; break;
                case '$': bitmap = font_5x7_symbols[4]; break;
                case '%': bitmap = font_5x7_symbols[5]; break;
                case '&': bitmap = font_5x7_symbols[6]; break;
                case '\'': bitmap = font_5x7_symbols[7]; break;
                case '(': bitmap = font_5x7_symbols[8]; break;
                case ')': bitmap = font_5x7_symbols[9]; break;
                case '*': bitmap = font_5x7_symbols[10]; break;
                case '+': bitmap = font_5x7_symbols[11]; break;
                case ',': bitmap = font_5x7_symbols[12]; break;
                case '-': bitmap = font_5x7_symbols[13]; break;
                case '.': bitmap = font_5x7_symbols[14]; break;
                case '/': bitmap = font_5x7_symbols[15]; break;
                case ':': bitmap = font_5x7_symbols[16]; break;
                case ';': bitmap = font_5x7_symbols[17]; break;
                default: bitmap = font_5x7_symbols[0]; break;
            }
        }
        
        if (bitmap) {
            for (int col = 0; col < 5; col++) {
                for (int row = 0; row < 7; row++) {
                    if (bitmap[col] & (1 << row)) {
                        display_draw_pixel(x + i * 6 + col, y + row, color);
                    } else {
                        display_draw_pixel(x + i * 6 + col, y + row, bg_color);
                    }
                }
            }
        }
    }
}


void display_draw_text_2x(int16_t x, int16_t y, const char* text, uint16_t color, uint16_t bg_color) {
    if (text == NULL) return;
    
    int len = strlen(text);
    if (len == 0) return;
    
    for (int i = 0; i < len; i++) {
        char c = text[i];
        const uint8_t* bitmap = NULL;
        
        if (c >= 'A' && c <= 'Z') {
            bitmap = font_5x7_letters[c - 'A'];
        } else if (c >= 'a' && c <= 'z') {
            bitmap = font_5x7_letters[c - 'a'];
        } else if (c >= '0' && c <= '9') {
            bitmap = font_5x7_numbers[c - '0'];
        } else if (c == ' ') {
            bitmap = font_5x7_symbols[0];
        }
        
        if (bitmap) {
            for (int col = 0; col < 5; col++) {
                for (int row = 0; row < 7; row++) {
                    uint16_t pixel_color = (bitmap[col] & (1 << row)) ? color : bg_color;
                    display_fill_rect(x + i * 12 + col * 2, y + row * 2, 2, 2, pixel_color);
                }
            }
        }
    }
}
