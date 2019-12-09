#pragma once
#ifndef GAME_BUTTON_H
#define GAME_BUTTON_H

#include "GUIRenderElement.h"

#include "Widget.h"
#include <core/engine/fonts/FontIncludes.h>
#include <string>
#include <functional>

class Font;
class GUIRenderElement;
class Button : public Widget {
    protected:
        Font*                     m_Font;
        std::string               m_Text;
        glm::vec2                 m_TextScale;
        glm::vec4                 m_TextColor;
        std::function<void()>     m_FunctorOnClick;
        float                     m_Padding;
        TextAlignment::Type       m_TextAlignment;

        GUIRenderElement          m_RenderElement;

        void internalSetSize();
    public:
        Button(const Font& font, const float x, const float y, const float width, const float height);
        Button(const Font& font, const glm::vec2& position, const float width, const float height);
        virtual ~Button();

        const std::string& text() const;

        template<class T> void setOnClickFunctor(const T& functor) { 
            m_FunctorOnClick = std::bind<void>(functor, this); 
        }

        const float getCornerWidth();
        const float getCornerHeight();

        const float getTextHeight() const;
        const float getTextWidth() const;
        void setTextAlignment(const TextAlignment::Type&);
        const TextAlignment::Type& getTextAlignment() const;

        virtual void setTextScale(const glm::vec2& scale);
        virtual void setTextScale(const float x, const float y);
        virtual void setTextScale(const float scale);

        void setTexture(Texture*);
        void setTextureCorner(Texture*);
        void setTextureEdge(Texture*);
        void setTexture(Handle&);
        void setTextureCorner(Handle&);
        void setTextureEdge(Handle&);

        const glm::vec2& getTextScale() const;

        void setFont(const Font& font);
        void setTextColor(const float& r, const float& g, const float& b, const float& a);
        void setTextColor(const glm::vec4& color);

        void setColor(const float& r, const float& g, const float& b, const float& a);
        void setColor(const glm::vec4& color);

        void setWidth(const float);
        void setHeight(const float);
        void setSize(const float width, const float height);

        virtual void setText(const char* text);
        virtual void setText(const std::string& text);

        virtual void update(const double& dt);
        virtual void render(const glm::vec4& scissor);
        virtual void render();
};

#endif