#include <lua5.1/lua.h>
#include <lua5.1/lauxlib.h>
#include <lua5.1/lualib.h>
#include "../include/kinect.h"

static lua_State *L;

static int nui_setup_device() {
    lua_pushnumber(L, setup_device());
    return 1;
}

static int nui_start_device() {
    lua_pushnumber(L, start_device());
    return 1;
}

static void nui_stop_device() {
    stop_device();
}

static void nui_get_closest_point(int x, int y) {
    lua_getglobal(L, "closestPoint");
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pcall(L, 2, 0, 0);
}

static int nui_set_device_tilt_angle() {
    double angle = (double) luaL_checkint(L, 1);
    set_tilt_angle(angle);
    return 0;
}

int luaopen_libnui(lua_State *lua) {

    L = lua;

    lua_register(L, "setupDevice", nui_setup_device);
    lua_register(L, "startDevice", nui_start_device);
    lua_register(L, "stopDevice", nui_stop_device);
    lua_register(L, "setDeviceTiltAngle", nui_set_device_tilt_angle);

    set_closest_point_cb(nui_get_closest_point);

    return 0;
}
