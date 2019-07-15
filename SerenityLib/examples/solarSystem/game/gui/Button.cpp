#include "Button.h"

#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/events/Engine_Events.h>
#include <core/engine/Engine_Math.h>
#include <core/engine/fonts/Font.h>

using namespace Engine;

namespace Engine {
    namespace epriv {
        struct emptyFunctor { template<class T> void operator()(T& r) const {} };
    };
};

Button::Button(Font& font, const float& x, const float& y, const float& width, const float& height) :Widget(x, y, width, height) {
    setFont(font);
    m_Text = "Button";
    setOnClickFunctor(epriv::emptyFunctor());
    m_TextScale = glm::vec2(1.0f);

    m_Height = getTextHeight() + 20;
    m_Width = getTextWidth() + 20;
}
Button::Button(Font& font, const glm::vec2& position, const float& width, const float& height) : Widget(position, width, height) {
    setFont(font);
    m_Text = "Button";
    setOnClickFunctor(epriv::emptyFunctor());
    m_TextScale = glm::vec2(1.0f);

    m_Height = getTextHeight() + 20;
    m_Width = getTextWidth() + 20;
}
Button::~Button() {

}
void Button::setTextScale(const glm::vec2& scale) {
    m_TextScale = scale;
    m_Height = getTextHeight() + 20;
    m_Width = getTextWidth() + 20;
}
void Button::setTextScale(const float& x, const float& y) {
    m_TextScale.x = x;
    m_TextScale.y = y;
    m_Height = getTextHeight() + 20;
    m_Width = getTextWidth() + 20;
}
const glm::vec2& Button::getTextScale() const {
    return m_TextScale;
}
const std::string& Button::text() const {
    return m_Text;
}

const float Button::getTextHeight() const {
    return m_Font->getTextHeight(m_Text) * m_TextScale.y;
}
const float Button::getTextWidth() const {
    return m_Font->getTextWidth(m_Text) * m_TextScale.x;
}

void Button::setTextColor(const float& r, const float& g, const float& b, const float& a) {
    Math::setColor(m_TextColor, r, g, b, a);
}
void Button::setTextColor(const glm::vec4& color) {
    Math::setColor(m_TextColor, color.r, color.g, color.b, color.a);
}
void Button::setFont(Font& font) {
    m_Font = &font;
}
void Button::setText(const char* text) {
    m_Text = text;
    m_Height = getTextHeight() + 20;
    m_Width = getTextWidth() + 20;
}
void Button::setText(const std::string& text) {
    m_Text = text;
    m_Height = getTextHeight() + 20;
    m_Width = getTextWidth() + 20;
}
void Button::update(const double& dt) {
    Widget::update(dt);
    if (m_MouseIsOver == true && Engine::isMouseButtonDownOnce(MouseButton::Left) == true) {
        m_FunctorOnClick();
    }
}
void Button::render() {
    float xOffset, yOffset;
    xOffset = yOffset = 0;
    switch (m_Alignment) {
        case WidgetAlignment::TopLeft: {
            xOffset -= m_Width / 2;
            yOffset -= m_Height / 2;
            break;
        }case WidgetAlignment::TopCenter: {
            yOffset -= m_Height / 2;
            break;
        }case WidgetAlignment::TopRight: {
            xOffset += m_Width / 2;
            yOffset -= m_Height / 2;
            break;
        }case WidgetAlignment::Left: {
            xOffset -= m_Width / 2;
            break;
        }case WidgetAlignment::Center: {
            break;
        }case WidgetAlignment::Right: {
            xOffset += m_Width / 2;
            break;
        }case WidgetAlignment::BottomLeft: {
            xOffset -= m_Width / 2;
            yOffset += m_Height / 2;
            break;
        }case WidgetAlignment::BottomCenter: {
            yOffset += m_Height / 2;
            break;
        }case WidgetAlignment::BottomRight: {
            xOffset += m_Width / 2;
            yOffset += m_Height / 2;
            break;
        }default: {
            break;
        }
    }  
    const auto newPos = glm::vec2(m_Position.x + xOffset, m_Position.y + yOffset);
    Renderer::renderRectangle(newPos, m_Color, m_Width, m_Height, 0, 0.01f);

    const auto newPosTxt = glm::vec2(m_Position.x, m_Position.y + getTextHeight());
    m_Font->renderText(m_Text, newPosTxt, m_TextColor,0, m_TextScale, 0.008f,TextAlignment::Center);
}