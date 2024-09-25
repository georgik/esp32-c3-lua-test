#include <stdio.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"

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
            printf("Mock QR Code:\n%s\n", qr_code_string);
        }
        lua_pop(L, lua_gettop(L));
    }
    log_memory_usage("After executing Lua script");

    lua_close(L);
    log_memory_usage("After lua_close");

    printf("End of Lua QR Code test: %s\n", test_name);
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

    printf("End of testing application.\n");

    // Prevent the task from ending
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
