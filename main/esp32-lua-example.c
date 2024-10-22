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
#include "esp_vfs.h"
#include "esp_littlefs.h"
#include <dirent.h>

#define WIFI_SCAN_LIST_SIZE 10
#define LUA_FILE_PATH "/assets"

static const char *TAG = "lua_example";

// Function to log memory usage with the message at the end
void log_memory_usage(const char *message) {
    ESP_LOGI(TAG, "Free heap: %d, Min free heap: %d, Largest free block: %d, %s",
             heap_caps_get_free_size(MALLOC_CAP_DEFAULT),
             heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT),
             heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT),
             message);
}

// Initialize and mount the filesystem
void init_filesystem() {
    ESP_LOGI(TAG, "Initializing File System");

    esp_vfs_littlefs_conf_t conf = {
        .base_path = LUA_FILE_PATH,
        .partition_label = "assets",
        .format_if_mount_failed = false,
        .dont_mount = false,
    };

    esp_err_t err = esp_vfs_littlefs_register(&conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount or format filesystem");
    } else {
        ESP_LOGI(TAG, "Filesystem mounted at %s", LUA_FILE_PATH);
    }
}

// Function to run a Lua script from file
void run_lua_file(const char *file_name, const char *test_name) {
    ESP_LOGI(TAG, "Starting Lua test from file: %s", test_name);

    log_memory_usage("Start of test");

    lua_State *L = luaL_newstate();
    if (L == NULL) {
        ESP_LOGE(TAG, "Failed to create new Lua state");
        return;
    }
    log_memory_usage("After luaL_newstate");

    luaL_openlibs(L);

    // Set the Lua module search path to include the assets directory
    if (luaL_dostring(L, "package.path = package.path .. ';./?.lua;/assets/?.lua'")) {
        ESP_LOGE(TAG, "Failed to set package.path: %s", lua_tostring(L, -1));
        lua_pop(L, 1);  // Remove error message from the stack
    }

    log_memory_usage("After luaL_openlibs");

    // Construct the full file path
    char full_path[128];
    snprintf(full_path, sizeof(full_path), LUA_FILE_PATH"/%s", file_name);

    if (luaL_dofile(L, full_path) == LUA_OK) {
        lua_pop(L, lua_gettop(L));
    } else {
        ESP_LOGE(TAG, "Error running Lua script from file '%s': %s", full_path, lua_tostring(L, -1));
        lua_pop(L, 1);  // Remove error message from the stack
    }
    log_memory_usage("After executing Lua script from file");

    lua_close(L);
    log_memory_usage("After lua_close");

    ESP_LOGI(TAG, "End of Lua test from file: %s", test_name);
}

// Function to run an embedded Lua script
void run_embedded_lua_test(const char *lua_script, const char *test_name) {
    ESP_LOGI(TAG, "Starting Lua test: %s", test_name);

    log_memory_usage("Start of test");

    lua_State *L = luaL_newstate();
    if (L == NULL) {
        ESP_LOGE(TAG, "Failed to create new Lua state");
        return;
    }
    log_memory_usage("After luaL_newstate");

    luaL_openlibs(L);
    log_memory_usage("After luaL_openlibs");

    if (luaL_dostring(L, lua_script) == LUA_OK) {
        lua_pop(L, lua_gettop(L));
    } else {
        ESP_LOGE(TAG, "Error running embedded Lua script: %s", lua_tostring(L, -1));
        lua_pop(L, 1);  // Remove error message from the stack
    }
    log_memory_usage("After executing Lua script");

    lua_close(L);
    log_memory_usage("After lua_close");

    ESP_LOGI(TAG, "End of Lua test: %s", test_name);
}

// Function to scan Wi-Fi networks
void scan_wifi_networks(void) {
    ESP_LOGI(TAG, "Starting Wi-Fi scan...");

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS Flash init error (%s), erasing...", esp_err_to_name(ret));
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = true
    };

    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));

    uint16_t ap_count = WIFI_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[WIFI_SCAN_LIST_SIZE];
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, ap_info));

    ESP_LOGI(TAG, "Found %d access points:", ap_count);
    for (int i = 0; i < ap_count; i++) {
        ESP_LOGI(TAG, "SSID: %s, RSSI: %d", ap_info[i].ssid, ap_info[i].rssi);
    }

    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_deinit());

    ESP_LOGI(TAG, "Wi-Fi scan completed.");
}

void app_main(void) {
    // Initialize and mount the filesystem
    init_filesystem();

    // Test 1: Simple embedded Lua script
    const char *simple_script = "answer = 42; print('The answer is: '..answer)";
    run_embedded_lua_test(simple_script, "Simple Embedded Script");

    // Test 2: Run Lua script from a file (e.g., fibonacci.lua)
    run_lua_file("fibonacci.lua", "Fibonacci Script from File");

    // Test 3: Run Lua script to generate QR code (e.g., qr_code.lua)
    run_lua_file("qr_code.lua", "QR Code Script from File");

    // Perform Wi-Fi scan
    scan_wifi_networks();

    ESP_LOGI(TAG, "End of testing application.");

    // Prevent the task from ending
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
