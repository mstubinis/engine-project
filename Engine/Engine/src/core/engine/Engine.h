#pragma once
#ifndef ENGINE_ENGINE_H
#define ENGINE_ENGINE_H

#include "core/engine/Engine_Debugging.h"
#include "core/engine/Engine_ThreadManager.h"
#include "core/engine/Engine_Resources.h"
#include "core/engine/renderer/Engine_Renderer.h"
#include "core/engine/Engine_Sounds.h"
#include "core/engine/events/Engine_Events.h"
#include "core/engine/Engine_Noise.h"
#include "core/engine/Engine_Window.h"

typedef std::uint32_t uint;
namespace Engine{
    namespace epriv{
        struct Core final{
            static Core*          m_Engine;

            EventManager          m_EventManager;
            PhysicsManager        m_PhysicsManager;
            ResourceManager       m_ResourceManager;
            DebugManager          m_DebugManager;
            SoundManager          m_SoundManager;
            RenderManager         m_RenderManager;
            ThreadManager         m_ThreadManager;
            NoiseManager          m_NoiseManager;
            bool                  m_Paused, m_Destroyed;

            Core(const char* name,uint width,uint height);
            ~Core();
        };
    };
    void init(const char* name,uint width=0,uint height=0);
    void pause(bool=true);
    bool paused();
    void unpause();
    void run();

    const float getFPS();
    Engine_Window& getWindow();
    const glm::uvec2 getWindowSize();
    void setWindowIcon(const Texture& texture);
    void showMouseCursor();
    void hideMouseCursor();
    void stop();
    void setFullScreen(bool b);
};

namespace Game{
    void initResources();
    void initLogic();
    void update(const float& dt);
    void render();
    void cleanup();

    #pragma region EventHandlers
    void onResize(uint width,uint height);
    void onClose();
    void onLostFocus();
    void onGainedFocus();
    void onTextEntered(uint);
    void onKeyPressed(uint);
    void onKeyReleased(uint);
    void onMouseWheelMoved(int delta);
    void onMouseButtonPressed(uint);
    void onMouseButtonReleased(uint);
    void onMouseMoved(float mouseX,float mouseY);
    void onMouseEntered();
    void onMouseLeft();
    void onPreUpdate(float dt);
    void onPostUpdate(float dt);
    void onJoystickButtonPressed();
    void onJoystickButtonReleased();
    void onJoystickMoved();
    void onJoystickConnected();
    void onJoystickDisconnected();
    #pragma endregion
};
#endif
