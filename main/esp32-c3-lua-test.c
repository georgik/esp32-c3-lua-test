#include <stdio.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    printf("Starting Lua\n");
    lua_State *L = luaL_newstate();
    printf("Opening Lua Libs\n");
    luaL_openlibs(L);

    printf("Calling Lua code: \n");
    lua_pushinteger(L, 42);
    lua_setglobal(L, "answer");

    char * code = "print(answer)";

    if (luaL_dostring(L, code) == LUA_OK) {
        lua_pop(L, lua_gettop(L));
    }

    printf("Closing Lua\n");
    lua_close(L);

    while(1) {
        vTaskDelay(pdMS_TO_TICKS(16));
    }
}
