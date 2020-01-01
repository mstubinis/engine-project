#include "MapDescriptionWindow.h"
#include "MapSelectionWindow.h"
#include "HostScreen.h"

#include <core/engine/fonts/Font.h>

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/textures/Texture.h>

#include <iostream>

#include "../Text.h"
#include "../../Menu.h"
#include "../../networking/client/Client.h"
#include "../../Core.h"
#include "../Button.h"
#include "../ScrollFrame.h"
#include "../../ResourceManifest.h"
#include "../Widget.h"
#include "../../Helper.h"

using namespace std;
using namespace Engine;

const auto y_padding = 110.0f;

MapDescriptionWindow::MapDescriptionWindow(HostScreen& hostScreen, Font& font, const float& x, const float& y, const float& width, const float& height, const float& depth, const unsigned int& borderSize, const string& labelText) : RoundedWindow(font,x,y,width,height,depth, borderSize, labelText), m_Font(font), m_HostScreen(hostScreen){
    m_UserPointer = nullptr;
    m_MapDescriptionTextScrollFrame = NEW ScrollFrame(font, x, y, width, height, depth - 0.001f);

    m_MapDescriptionTextScrollFrame->setColor(Menu::DEFAULT_COLORS[MenuDefaultColors::FederationBlue]);
    m_MapDescriptionTextScrollFrame->setAlignment(Alignment::BottomCenter);
}
MapDescriptionWindow::~MapDescriptionWindow() {
    SAFE_DELETE(m_MapDescriptionTextScrollFrame);
}
void MapDescriptionWindow::setPosition(const float x, const float y) {
    Button::setPosition(x,y);
    m_MapDescriptionTextScrollFrame->setPosition(x, y);
}
void MapDescriptionWindow::onResize(const unsigned int newWidth, const unsigned int newHeight) {
    RoundedWindow::onResize(newWidth, newHeight);

    const auto pos = positionWorld();
    const auto frame_size = glm::vec2(width(), height());

    const auto texture_size = glm::min(frame_size.x * 0.5f, frame_size.y * 0.5f);

    m_MapDescriptionTextScrollFrame->setPosition(pos.x, pos.y - (frame_size.y / 2.0f) + 30.0f);

    m_MapDescriptionTextScrollFrame->setSize(frame_size.x - 60.0f, texture_size - y_padding);
}
void MapDescriptionWindow::setSize(const float& w, const float& h) {
    m_MapDescriptionTextScrollFrame->setSize(w, h);
}
void MapDescriptionWindow::setUserPointer(void* ptr) {
    m_UserPointer = ptr;
}
void* MapDescriptionWindow::getUserPointer() {
    return m_UserPointer;
}
void MapDescriptionWindow::clear() {
    setLabelText("");
    m_MapDescriptionTextScrollFrame->clear();
}
void MapDescriptionWindow::addContent(Widget* widget) {
    m_MapDescriptionTextScrollFrame->addContent(widget);
}
Font& MapDescriptionWindow::getFont() {
    return m_Font;
}
ScrollFrame& MapDescriptionWindow::getWindowFrame() {
    return *m_MapDescriptionTextScrollFrame;
}
void MapDescriptionWindow::update(const double& dt) {
    RoundedWindow::update(dt);
    m_MapDescriptionTextScrollFrame->update(dt);
}
void MapDescriptionWindow::render() {
    RoundedWindow::render();
    m_MapDescriptionTextScrollFrame->render();

    //render map screenshot
    const auto frame_size = glm::vec2(width(), height());
    const auto texture_size = glm::vec2(glm::min(frame_size.x * 0.5f, frame_size.y * 0.5f));
    const auto& current_map_data = m_HostScreen.getCurrentChoice();

    const auto ss_pos = positionWorld() + glm::vec2(0, (frame_size.y / 2.0f) - 65.0f);
    if (!current_map_data.map_name.empty()) {
        Texture& texture = *(Texture*)current_map_data.map_screenshot_handle.get();
        auto scl = glm::vec2(texture_size) / glm::vec2(texture.size());
        Renderer::renderTexture(texture, ss_pos , glm::vec4(1.0f), 0.0f, scl, getDepth() - 0.001f, Alignment::TopCenter);
        Renderer::renderBorder(1, ss_pos, Menu::DEFAULT_COLORS[MenuDefaultColors::FederationBlueSlightlyDarker], texture_size.x, texture_size.y, 0.0f, getDepth() - 0.002f, Alignment::TopCenter);
    }
}