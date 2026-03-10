# Build Environment Variables (Compile-Time Defines)

This document lists the **compile-time preprocessor defines** (build flags) that alter ESP32APRS_Audio behavior. These are set in `platformio.ini` per build environment and are passed to the compiler via `-D` flags. They are the embedded equivalent of environment variables—changing them requires a rebuild.

## Global Build Flags (All Environments)

| Define | Value | Effect |
|--------|-------|--------|
| `WEBSERVER_MAX_POST_ARGS` | 80 | Maximum number of POST arguments the web server accepts |
| `CONFIG_I2C_ENABLE_DEBUG_LOG` | 0 | Disables I2C debug logging |
| `CORE_DEBUG_LEVEL` | 0 | Sets ESP-IDF core debug verbosity (0=none) |
| `ENABLE_FX25` | (defined) | Enables FX.25 forward error correction in AX.25 layer |
| `RFMODULE` | (defined) | Enables RF module support (SA868/SR series transceivers) |

## Display & Hardware

| Define | Environments | Effect |
|--------|--------------|--------|
| `OLED` | esp32-sh1106, esp32-ssd1306, esp32-8MB, esp32c3-sh1106, esp32c3-ssd1306, esp32c3-8MB, esp32s3-sh1106, esp32s3-N16R8 | Enables OLED display support; default `oled_enable` is true |
| `SH1106` | esp32-sh1106, esp32-8MB, esp32c3-sh1106, esp32c3-8MB, esp32s3-sh1106, esp32s3-N16R8 | Uses SH1106 driver instead of SSD1306 |
| `SSD1306_72x40` | (optional, commented in esp32c3-sh1106) | 72×40 pixel SSD1306 variant; changes screen dimensions |
| `STRIP_PIN` | esp32s3, esp32c3 | Replaces discrete LED_TX/LED_RX with NeoPixel RGB strip; GPIO defined by `STRIP_PIN` value |
| `ST7735_160x80` | (board-specific) | ST7735 color LCD 160×80; used with `HELTEC_HTIT_TRACKER` or `APRS_LORA_DONGLE` |
| `ST7735_LED_K_Pin` | (in main.h) | Backlight PWM pin for ST7735; 21 for HELTEC_HTIT_TRACKER, 16 for APRS_LORA_DONGLE |

## Connectivity & Peripherals

| Define | Environments | Effect |
|--------|--------------|--------|
| `BLUETOOTH` | esp32-sh1106, esp32-8MB, esp32c3-8MB, esp32s3-sh1106, esp32s3-N16R8 | Enables NimBLE Bluetooth (TNC2/KISS over BLE) |
| `PPPOS` | esp32-8MB, esp32c3-8MB, esp32s3-sh1106, esp32s3-N16R8 | Enables PPP-over-serial cellular modem (4G) support |
| `MQTT` | (not in current envs) | Enables MQTT client; adds MQTT config fields and handlers |

## Board & MCU

| Define | Environments | Effect |
|--------|--------------|--------|
| `ESP32C3_MINI` | esp32c3-* | ESP32-C3 Mini board; different GPIO defaults (UART, RF, I2C) |
| `ESP32C6` | esp32c6 | ESP32-C6 target |
| `BOARD_HAS_PSRAM` | esp32s3-N16R8 | Enables PSRAM; increases `TLMLISTSIZE`, `PKGLISTSIZE`, `PKGTXSIZE` |
| `TTGO_T_Beam_S3_SUPREME_V3` | (board-specific) | TTGO T-Beam S3 Supreme V3; sensor/display tweaks |
| `TTGO_T_Beam_V1_2` | (board-specific) | TTGO T-Beam V1.2 |
| `HELTEC_HTIT_TRACKER` | (board-specific) | Sets `ST7735_LED_K_Pin` = 21 |
| `APRS_LORA_DONGLE` | (board-specific) | Sets `ST7735_LED_K_Pin` = 16 |

## ESP-IDF / Arduino (Auto-Defined)

| Define | Source | Effect |
|--------|--------|--------|
| `CONFIG_IDF_TARGET_ESP32` | ESP-IDF | Original ESP32 (Xtensa) |
| `CONFIG_IDF_TARGET_ESP32C3` | ESP-IDF | ESP32-C3 (RISC-V) |
| `CONFIG_IDF_TARGET_ESP32C6` | ESP-IDF | ESP32-C6 |
| `CONFIG_IDF_TARGET_ESP32S3` | ESP-IDF | ESP32-S3 |
| `ARDUINO_USB_MODE` | esp32s3, esp32c3 | USB CDC mode |
| `ARDUINO_USB_CDC_ON_BOOT` | esp32s3, esp32c3 | USB serial on boot |

## STRIP_PIN Values by Environment

| Environment | STRIP_PIN | GPIO |
|------------|-----------|------|
| esp32s3-sh1106, esp32s3-N16R8 | 48 | NeoPixel data pin |
| esp32c3-sh1106, esp32c3-ssd1306, esp32c3-nodisp, esp32c3-8MB | 2 | NeoPixel data pin |

## Debug

| Define | Effect |
|--------|--------|
| `DEBUG` | Enables extra `log_d()` debug output in webservice, main, etc. (not set in default envs) |

## How to Change

1. Edit `platformio.ini` and add/remove `-D` flags under the desired `[env:...]` section.
2. Rebuild: `pio run -e <env_name>`.
3. Flash the new firmware.

## Notes

- No traditional `getenv()` or runtime environment variables are used.
- `CONFIG_IDF_TARGET_*` and `CONFIG_*` are set by ESP-IDF based on the selected board.
- Some defines (e.g. `TTGO_T_Beam_S3_SUPREME_V3`) are referenced in code but not defined in the current `platformio.ini`; they are for custom board builds.
