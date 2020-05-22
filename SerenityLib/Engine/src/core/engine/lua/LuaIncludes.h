#pragma once
#ifndef ENGINE_LUA_INCLUDES_H
#define ENGINE_LUA_INCLUDES_H

#ifdef __cplusplus
    #include <lua.h>
    #include <lualib.h>
    #include <lauxlib.h>
#else
    extern "C" {
        #include "lua.h"
        #include "lauxlib.h"
        #include "lualib.h"
    }
#endif
#include <LuaBridge/LuaBridge.h>

#endif