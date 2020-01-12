#pragma once
#ifndef ENGINE_WINDOW_H
#define ENGINE_WINDOW_H

namespace sf {
    class Window;
};
namespace Engine {
    namespace epriv {
        class  EngineCore;
    };
};
class  Texture;

#include <memory>
#include <string>
#include <SFML/Window.hpp>
#include <glm/vec2.hpp>

class Engine_Window_Flags final { public: enum Flag: unsigned int {
    WindowedFullscreen = 1 << 0,
    Windowed           = 1 << 1,
    Fullscreen         = 1 << 2,
    MouseVisible       = 1 << 3,
    Active             = 1 << 4,
    Vsync              = 1 << 5,
    MouseGrabbed       = 1 << 6,
    KeyRepeat          = 1 << 7,
};};


class Engine_Window final{
    friend class Engine::epriv::EngineCore;
    class Engine_WindowData final {
        friend class Engine::epriv::EngineCore;
        friend class Engine_Window;
        private:
            glm::uvec2          m_OldWindowSize;
            unsigned int        m_Style;
            sf::VideoMode       m_VideoMode;
            const char*         m_WindowName;
            sf::Window          m_SFMLWindow;
            unsigned int        m_FramerateLimit;

            unsigned int        m_Flags;


            int                 m_OpenGLMajorVersion;
            int                 m_OpenGLMinorVersion;
            int                 m_GLSLVersion;
            sf::ContextSettings m_SFContextSettings;

            void restore_state();
            const sf::ContextSettings create(Engine_Window&, const std::string& _name, const unsigned int& _width, const unsigned int& _height);
        public:
            Engine_WindowData();
            ~Engine_WindowData();
    };

    private:
        Engine_WindowData m_Data;
        void internal_on_fullscreen(const bool isToBeFullscreen, const bool isMaximized, const bool isMinimized);
        const bool remove_flag(const Engine_Window_Flags::Flag& flag);
        const bool add_flag(const Engine_Window_Flags::Flag& flag);
        const bool has_flag(const Engine_Window_Flags::Flag& flag);
        void restore_state();
        sf::VideoMode get_default_desktop_video_mode();
    public:
        Engine_Window(const char* name, const unsigned int& width, const unsigned int& height);
        ~Engine_Window();

        const char* name() const;
        const glm::uvec2 getSize();
        const glm::uvec2 getPosition();

        void setName(const char* name);
        void setSize(const unsigned int& width, const unsigned int& height);
        void setIcon(const Texture& texture);
        void setIcon(const char* file);
        void setIcon(const std::string& file);
        void setMouseCursorVisible(const bool);
        void setPosition(const unsigned int& x, const unsigned int& y);

        //currently specific to windows os only
        const bool maximize();
        //currently specific to windows os only
        const bool minimize();

        //If key repeat is enabled, you will receive repeated KeyPressed events while keeping a key pressed.
        //If it is disabled, you will only get a single event when the key is pressed.
        //Key repeat is enabled by default.
        void setKeyRepeatEnabled(const bool);

        //Activating vertical synchronization will limit the number of frames displayed to the refresh rate of the monitor.
        //This can avoid some visual artifacts, and limit the framerate to a good value (but not constant across different computers).
        //Vertical synchronization is disabled by default.
        void setVerticalSyncEnabled(const bool);

        //Close the window and destroy all the attached resources. After calling this function, the sf::Window instance
        //remains valid and you can call create() to recreate the window. All other functions such as pollEvent() or
        //display() will still work (i.e. you don't have to test isOpen() every time), and will have no effect on closed windows.
        void close();

        //Request the current window to be made the active foreground window. At any given time, only one window may have the input
        //focus to receive input events such as keystrokes or mouse events. If a window requests focus, it only hints to the
        //operating system, that it would like to be focused. The operating system is free to deny the request.
        //This is not to be confused with setActive().
        void requestFocus();

        //Activate or deactivate the window as the current target for OpenGL rendering. A window is active only on the current thread,
        //if you want to make it active on another thread you have to deactivate it on the previous thread first if it was active.
        //Only one window can be active on a thread at a time, thus the window previously active (if any) automatically gets deactivated.
        //This is not to be confused with requestFocus().
        void setActive(const bool active = true);

        const unsigned int getStyle();
        const bool hasFocus();
        const bool isOpen();

        //returns true if the window is in fullscreen OR windowed fullscreen mode
        const bool isFullscreen();

        //returns true if the window is in windowed fullscreen mode
        const bool isFullscreenWindowed();

        //returns true if the window is in regular fullscreen mode
        const bool isFullscreenNonWindowed();

        //currently specific to windows os only
        const bool isMaximized();
        //currently specific to windows os only
        const bool isMinimized();

        const bool isActive();

        const bool setFullscreen(const bool isFullscreen = true);
        const bool setFullscreenWindowed(const bool isFullscreen = true);

        void setStyle(const unsigned int& style);

        void display();

        //Grab or release the mouse cursor. If set, grabs the mouse cursor inside this window's client area so it may no
        //longer be moved outside its bounds. Note that grabbing is only active while the window has focus.
        void keepMouseInWindow(const bool=true);

        //Limit the framerate to a maximum fixed frequency. If a limit is set, the window will use a small delay after
        //each call to display() to ensure that the current frame lasted long enough to match the framerate limit.
        //SFML will try to match the given limit as much as it can, but since it internally uses sf::sleep,
        //whose precision depends on the underlying OS, the results may be a little unprecise as well
        //(for example, you can get 65 FPS when requesting 60).
        void setFramerateLimit(const unsigned int& limit);

        const unsigned int getFramerateLimit() const;

        sf::Window& getSFMLHandle() const;
};
#endif
