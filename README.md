# ESP32 Lua Evaluation Example

This example demonstrates how to use Lua 5.4 on ESP32, leveraging the LittleFS filesystem to run Lua scripts from external files. It also showcases Wi-Fi scanning and embedded Lua scripts.

## Prerequisites

- ESP-IDF version 5.1.0 or higher
- Components:
  - [Lua 5.4](https://components.espressif.com/components/georgik/lua/)
  - [LittleFS 1.14](https://components.espressif.com/components/joltwallet/littlefs/)

## File Structure

- `main.c`: Main program containing embedded Lua script and file-based Lua execution.
- `assets/`: Directory containing Lua scripts for QR code generation and Fibonacci computation.
  - `qr_code.lua`
  - `fibonacci.lua`

## Build and Flash

```bash
idf.py set-target esp32  # Replace esp32 with your specific target, e.g., esp32c3
idf.py build flash monitor
```
