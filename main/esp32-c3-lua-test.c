#include <stdio.h>
#include <string.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"

#define WIFI_SCAN_LIST_SIZE 10

static const char *TAG = "WIFI_SCAN";

// Function to log memory usage with the message at the end
void log_memory_usage(const char *message)
{
    printf("Free heap: %d, Min free heap: %d, Largest free block: %d, %s\n",
           heap_caps_get_free_size(MALLOC_CAP_DEFAULT),
           heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT),
           heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT),
           message);
}

// Function to run a Lua test
void run_lua_test(const char *lua_script, const char *test_name)
{
    printf("Starting Lua test: %s\n", test_name);

    log_memory_usage("Start of test");

    lua_State *L = luaL_newstate();
    log_memory_usage("After luaL_newstate");

    luaL_openlibs(L);
    log_memory_usage("After luaL_openlibs");

    if (luaL_dostring(L, lua_script) == LUA_OK) {
        lua_pop(L, lua_gettop(L));
    }
    log_memory_usage("After executing Lua script");

    lua_close(L);
    log_memory_usage("After lua_close");

    printf("End of Lua test: %s\n", test_name);
}

// Function to run a Lua test and print returned QR code
void run_lua_qr_code_test(const char *lua_script, const char *test_name)
{
    printf("Starting Lua QR Code test: %s\n", test_name);

    log_memory_usage("Start of test");

    lua_State *L = luaL_newstate();
    log_memory_usage("After luaL_newstate");

    luaL_openlibs(L);
    log_memory_usage("After luaL_openlibs");

    if (luaL_dostring(L, lua_script) == LUA_OK) {
        // Retrieve the QR code string from the Lua stack
        const char *qr_code_string = lua_tostring(L, -1);
        if (qr_code_string) {
            printf("QR Code:\n%s\n", qr_code_string);
        }
        lua_pop(L, lua_gettop(L));
    }
    log_memory_usage("After executing Lua script");

    lua_close(L);
    log_memory_usage("After lua_close");

    printf("End of Lua QR Code test: %s\n", test_name);
}

// Function to initialize and perform Wi-Fi scan
void scan_wifi_networks(void)
{
    printf("Starting Wi-Fi scan...\n");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize Wi-Fi
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Start Wi-Fi in STA mode
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Set the Wi-Fi scan parameters
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = true
    };

    // Perform Wi-Fi scan
    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));

    // Get the scan results
    uint16_t ap_count = WIFI_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[WIFI_SCAN_LIST_SIZE];
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, ap_info));

    // Print the scan results
    printf("Found %d access points:\n", ap_count);
    for (int i = 0; i < ap_count; i++) {
        printf("SSID: %s, RSSI: %d\n", ap_info[i].ssid, ap_info[i].rssi);
    }

    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_deinit());

    printf("Wi-Fi scan completed.\n");
}

void app_main(void)
{
    // Test 1: Simple Lua script
    const char *simple_script = "answer = 42; print('The answer is: '..answer)";
    run_lua_test(simple_script, "Simple Script");

    // Test 2: Simple Lua script to create a mock QR code representation
    const char *mock_qr_code_script = 
        "local function generate_mock_qr(text)\n"
        "    local result = ''\n"
        "    for i = 1, #text do\n"
        "        local char = text:byte(i)\n"
        "        local line = ''\n"
        "        for j = 1, 10 do\n"  // 10x10 mock representation
        "            if ((char + j) % 2 == 0) then\n"
        "                line = line .. '##'\n"
        "            else\n"
        "                line = line .. '  '\n"
        "            end\n"
        "        end\n"
        "        result = result .. line .. '\\n'\n"
        "    end\n"
        "    return result\n"
        "end\n"
        "local qr_string = generate_mock_qr('Hello, ESP32-C3!')\n"
        "return qr_string";
    run_lua_qr_code_test(mock_qr_code_script, "Mock QR Code Script");

    // Perform Wi-Fi scan
    scan_wifi_networks();

    // Test 3: Lua script to compute Fibonacci sequence
    const char *fibonacci_script =
        "local function fibonacci(n)\n"
        "    if n <= 1 then return n end\n"
        "    return fibonacci(n - 1) + fibonacci(n - 2)\n"
        "end\n"
        "local fib_10 = fibonacci(10)\n"
        "print('Fibonacci of 10 is: ' .. fib_10)\n"
        "return fib_10";
    run_lua_test(fibonacci_script, "Fibonacci Script");

    printf("End of testing application.\n");

    // Prevent the task from ending
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
