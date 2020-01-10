#include "FFAShipSelector.h"
#include "HostScreenFFA2.h"
#include "ShipToken.h"
#include "../../Menu.h"
#include "../../map/Map.h"
#include "../Text.h"
#include "../ScrollFrame.h"
#include "../../ships/Ships.h"

#include <core/engine/fonts/Font.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/textures/Texture.h>

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>

using namespace std;
using namespace Engine;

const auto dividor_width = 38.0f;
const auto scroll_frame_padding = 20.0f;


struct ShipTokenOnClickTotal final { void operator()(Button* button) const {
    auto& token = *static_cast<ShipToken*>(button);
    FFAShipSelector& ffaSelector = *static_cast<FFAShipSelector*>(token.getUserPointer());

    //ship pool
    if (token.isLit()) {
        for (auto& widget_row : ffaSelector.m_ShipsAllowedFrame->content()) {
            bool done = false;
            for (auto& widget : widget_row.widgets) {
                auto& token_1 = *static_cast<ShipToken*>(widget.widget);
                if (token_1.getShipClass() == token.getShipClass()) {
                    token_1.lightUp();
                    done = true;
                    break;
                }
            }
            if (done)
                break;
        }
        token.lightOff();
    }
}};
struct ShipTokenOnClickAllowed final { void operator()(Button* button) const {
    auto& token = *static_cast<ShipToken*>(button);
    FFAShipSelector& ffaSelector = *static_cast<FFAShipSelector*>(token.getUserPointer());

    //ships allowed
    if (token.isLit()) {
        for (auto& widget_row : ffaSelector.m_ShipsTotalFrame->content()) {
            bool done = false;
            for (auto& widget : widget_row.widgets) {
                auto& token_1 = *static_cast<ShipToken*>(widget.widget);
                if (token_1.getShipClass() == token.getShipClass()) {
                    token_1.lightUp();
                    done = true;
                    break;
                }
            }
            if (done)
                break;
        }
        token.lightOff();
    }
}};

FFAShipSelector::FFAShipSelector(HostScreenFFA2& hostScreen, Font& font, const float& x, const float& y, const float& width, const float& height, const float& depth, const unsigned int& borderSize, const string& labelText)
:RoundedWindow(font,x,y,width,height,depth,borderSize,labelText), m_HostScreen(hostScreen), m_Font(font){

    m_ShipsTotalFrame = new ScrollFrame(font, 
        x - (dividor_width / 2.0f) - scroll_frame_padding, 
        y - (height / 2.0f) + 30.0f, 
        ((width - 60.0f) / 2.0f) - (scroll_frame_padding * 2.0f),
        (height - 110.0f), 
    depth - 0.001f);
    m_ShipsTotalFrame->setAlignment(Alignment::BottomRight);

    m_ShipsAllowedFrame = new ScrollFrame(font, 
        x + (dividor_width / 2.0f) + scroll_frame_padding,
        y - (height / 2.0f) + 30.0f,
        ((width - 60.0f) / 2.0f) - (scroll_frame_padding * 2.0f),
        (height - 110.0f),
    depth - 0.001f);
    m_ShipsAllowedFrame->setAlignment(Alignment::BottomLeft);

    m_ShipsTotalLabel = new Text(x - (width / 4.0f), y + (height / 2.0f) - 9.0f, font, "Ship Database");
    m_ShipsTotalLabel->setAlignment(Alignment::Center);
    m_ShipsTotalLabel->setTextAlignment(TextAlignment::Center);
    m_ShipsTotalLabel->setColor(Menu::DEFAULT_COLORS[MenuDefaultColors::FederationBlue]);


    m_ShipsAllowedLabel = new Text(x + (width / 4.0f), y + (height / 2.0f) - 9.0f, font, "Ships Allowed");
    m_ShipsAllowedLabel->setAlignment(Alignment::Center);
    m_ShipsAllowedLabel->setTextAlignment(TextAlignment::Center);
    m_ShipsAllowedLabel->setColor(Menu::DEFAULT_COLORS[MenuDefaultColors::FederationBlue]);

    m_MiddleDivide = new Button(font, x, y - (height / 2.0f), dividor_width, height - 51.0f);
    m_MiddleDivide->setColor(Menu::DEFAULT_COLORS[MenuDefaultColors::FederationBlue]);
    m_MiddleDivide->setText("");
    m_MiddleDivide->disableMouseover();
    m_MiddleDivide->disable();
    m_MiddleDivide->setDepth(depth - 0.003f);
    m_MiddleDivide->setAlignment(Alignment::BottomCenter);

    init_window_contents();
}
FFAShipSelector::~FFAShipSelector() {
    SAFE_DELETE(m_ShipsTotalFrame);
    SAFE_DELETE(m_ShipsAllowedFrame);
    SAFE_DELETE(m_ShipsTotalLabel);
    SAFE_DELETE(m_ShipsAllowedLabel);
    SAFE_DELETE(m_MiddleDivide);
}
vector<string> FFAShipSelector::getAllowedShips() {
    vector<string> ret;
    for (auto& itr : m_ShipsAllowedFrame->content()) {
        for (auto& widget_entry : itr.widgets) {
            auto& token = *static_cast<ShipToken*>(widget_entry.widget);

            if (token.isLit()) {
                ret.push_back(token.getShipClass());
            }
        }
    }
    return ret;
}
void FFAShipSelector::init_window_contents() {
    for (size_t i = 0; i < ShipTier::_TOTAL; ++i) {
        ShipTier::Tier tier = static_cast<ShipTier::Tier>(i);
        auto classes = Ships::getShipClassesSortedByFaction(tier);
        for (auto& ship_class : classes) {
            auto& ship_info = Ships::Database.at(ship_class);
            if (ship_info.Faction == FactionEnum::Borg || ship_info.Tier == ShipTier::Station) { //no borg or stations
                continue;
            }

            ShipToken* ship_pool_button = new ShipToken(ship_info, m_Font, 0, 0, getDepth() - 0.004f, true);

            ship_pool_button->setUserPointer(this);
            ship_pool_button->setOnClickFunctor(ShipTokenOnClickTotal());
            

            ShipToken* ship_allowed_button = new ShipToken(ship_info, m_Font, 0, 0, getDepth() - 0.004f, false);

            ship_allowed_button->setUserPointer(this);
            ship_allowed_button->setOnClickFunctor(ShipTokenOnClickAllowed());

            m_ShipsTotalFrame->addContent(ship_pool_button, i);
            m_ShipsAllowedFrame->addContent(ship_allowed_button, i);
        }
    }
}
void FFAShipSelector::resetWindow() {
    m_ShipsTotalFrame->clear();
    m_ShipsAllowedFrame->clear();

    init_window_contents();
}

void FFAShipSelector::onResize(const unsigned int& newWidth, const unsigned int& newHeight) {
    RoundedWindow::onResize(newWidth, newHeight);

    const auto pos = positionWorld();
    const auto frame_size = glm::vec2(width(), height());

    m_ShipsTotalFrame->setPosition(pos.x - (dividor_width / 2.0f) - scroll_frame_padding, pos.y - (frame_size.y / 2.0f) + 30.0f);
    m_ShipsTotalFrame->setSize(((frame_size.x - 60.0f) / 2.0f) - (scroll_frame_padding * 2.0f), frame_size.y - 110.0f);

    m_ShipsAllowedFrame->setPosition(pos.x + (dividor_width / 2.0f) + scroll_frame_padding, pos.y - (frame_size.y / 2.0f) + 30.0f);
    m_ShipsAllowedFrame->setSize(((frame_size.x - 60.0f) / 2.0f) - (scroll_frame_padding * 2.0f), frame_size.y - 110.0f);

    m_ShipsTotalLabel->setPosition(pos.x - (frame_size.x / 4.0f), pos.y + (frame_size.y / 2.0f) - 9.0f);
    m_ShipsAllowedLabel->setPosition(pos.x + (frame_size.x / 4.0f), pos.y + (frame_size.y / 2.0f) - 9.0f);

    m_MiddleDivide->setPosition(pos.x, pos.y - (frame_size.y / 2.0f));
    m_MiddleDivide->setSize(dividor_width, frame_size.y - 51.0f);
}

void FFAShipSelector::update(const double& dt) {
    RoundedWindow::update(dt);

    m_ShipsTotalFrame->update(dt);
    m_ShipsAllowedFrame->update(dt);

    m_ShipsTotalLabel->update(dt);
    m_ShipsAllowedLabel->update(dt);

    m_MiddleDivide->update(dt);
}
void FFAShipSelector::render() {
    RoundedWindow::render();

    m_ShipsTotalFrame->render();
    m_ShipsAllowedFrame->render();

    m_ShipsTotalLabel->render();
    m_ShipsAllowedLabel->render();

    m_MiddleDivide->render();
}