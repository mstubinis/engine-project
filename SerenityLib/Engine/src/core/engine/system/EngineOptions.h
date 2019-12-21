#pragma once
#ifndef ENGINE_ENGINE_OPTIONS_H
#define ENGINE_ENGINE_OPTIONS_H

#include <core/engine/renderer/RendererIncludes.h>

struct EngineOptions final {
    AntiAliasingAlgorithm::Algorithm   aa_algorithm;
    bool                               ssao_enabled;
    bool                               hdr_enabled;
    bool                               fullscreen;
    bool                               god_rays_enabled;
    bool                               fog_enabled;
    unsigned int                       width;
    unsigned int                       height;
    const char* window_title;
    EngineOptions() {
        window_title      = "Engine";
        width             = 1024;
        height            = 768;
        ssao_enabled      = true;
        hdr_enabled       = true;
        god_rays_enabled  = true;
        fullscreen        = false;
        fog_enabled       = false;
        aa_algorithm      = AntiAliasingAlgorithm::FXAA;
    }
};

#endif