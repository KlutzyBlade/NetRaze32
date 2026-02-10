#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

// 2.8" ESP32-32E Display Pin Configuration
// Based on https://www.lcdwiki.com/2.8inch_ESP32-32E_Display

// LCD Display Pins (ILI9341) - ESP32-32E HSPI
#define LCD_CS_PIN      15
#define LCD_DC_PIN      2
#define LCD_RST_PIN     -1  // Connected to ESP32-EN
#define LCD_BL_PIN      21  // Re-enabled
#define LCD_MOSI_PIN    13
#define LCD_MISO_PIN    12
#define LCD_SCK_PIN     14

// Touch Screen Pins (XPT2046) - ESP32-32E separate pins
#define TOUCH_CS_PIN    33
#define TOUCH_IRQ_PIN   36
#define TOUCH_MOSI_PIN  32
#define TOUCH_MISO_PIN  39
#define TOUCH_SCK_PIN   25

// Display Configuration
#define DISPLAY_WIDTH   240
#define DISPLAY_HEIGHT  320

// Touch Calibration (ESP32-32E XPT2046)
#define TS_MINX         200
#define TS_MAXX         3900
#define TS_MINY         200
#define TS_MAXY         3900

// SPI Configuration (ESP32-32E optimized)
#define SPI_FREQUENCY   26000000  // ESP32-32E stable frequency
#define SPI_READ_FREQ   8000000
#define TOUCH_SPI_FREQ  2000000   // Higher for shared SPI

// Battery monitoring (ESP32-32E ADC)
#define BATTERY_ADC_CHANNEL  ADC_CHANNEL_0  // GPIO36 (VP)
#define BATTERY_VOLTAGE_DIVIDER  2.0f

// SD Card pins (SDIO 1-bit mode) - ESP32-32E built-in SD slot
#define SD_CMD_PIN      15  // SDIO CMD
#define SD_CLK_PIN      14  // SDIO CLK
#define SD_D0_PIN       2   // SDIO DAT0
#define SD_D3_PIN       13  // SDIO DAT3 (used as CS in 1-bit mode)

// CC1101 SubGHz Module (Optional - uses VSPI connector)
#define CC1101_MOSI_PIN 23  // VSPI MOSI (SPI connector)
#define CC1101_MISO_PIN 19  // VSPI MISO (SPI connector)
#define CC1101_SCK_PIN  18  // VSPI SCK (SPI connector)
#define CC1101_CS_PIN   27  // SPI connector CS pin
#define CC1101_GDO0_PIN 35  // Interrupt pin (input-only, external connector)
#define CC1101_GDO2_PIN -1  // Not used

#endif // BOARD_CONFIG_H
