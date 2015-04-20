#include "HUD.h"
#include "Engine_Resources.h"
#include "Font.h"
#include "Texture.h"
#include "Scene.h"
#include "Ship.h"
#include "Camera.h"

#include <boost/lexical_cast.hpp>

using namespace Engine;

HUD::HUD(PlayerShip* player){
	m_Font = new Font("Fonts/consolas.fnt");
	m_Player = player;
	m_Color = glm::vec3(1,1,1);
}
HUD::~HUD(){
	delete m_Font;
}
void HUD::update(float dt){
}
void HUD::render(){
	//render hud stuff
	m_Font->renderText("Delta Time: " + boost::lexical_cast<std::string>(Resources::dt()) +"\nFPS: " + boost::lexical_cast<std::string>(static_cast<unsigned int>(1.0f/Resources::dt())),glm::vec2(10,10),glm::vec3(1,1,1),0,glm::vec2(0.8f,0.8f));

	if(m_Player->getTarget() != nullptr){
		glm::vec3 pos = m_Player->getTarget()->getScreenCoordinates();
		float scl = glm::max(0.5f,m_Player->getTarget()->getRadius()*35/m_Player->getTarget()->getDistance(Resources::getActiveCamera()));

		#pragma region renderCrossHairAndOtherInfo
		if(pos.z == 1){
			Resources::getTexture("Textures/HUD/Crosshair")->render(glm::vec2(pos.x,pos.y),m_Color,0,glm::vec2(scl,scl),0);
			unsigned long long distanceInKm = (m_Player->getTarget()->getDistanceLL(m_Player) / 10);
			std::string stringRepresentation = "";
			if(distanceInKm > 0){
				stringRepresentation = Engine::convertNumberToStringCommas(static_cast<unsigned long long>(distanceInKm)) + " Km";
			}
			else{
				float distanceInm = (m_Player->getTarget()->getDistance(m_Player))*100.0f;
				stringRepresentation = boost::lexical_cast<std::string>(static_cast<unsigned int>(distanceInm)) + " m";
			}
			m_Font->renderText(m_Player->getTarget()->getName() + "\n"+stringRepresentation,glm::vec2(pos.x+40,pos.y-15),m_Color,0,glm::vec2(0.7f,0.7f),0);
		}
		else{
			glm::vec2 winSize = glm::vec2(Resources::getWindow()->getSize().x,Resources::getWindow()->getSize().y);
			scl = 1;

			float angle = 0;
			if(pos.y > 2 && pos.y < winSize.y - 2)
				if(pos.x < 2)
					angle = 45;
				else
					angle = 225;
			else if(pos.y <= 1){
				if(pos.x <= 1)
					angle = 0;
				else if(pos.x > winSize.x - 2)
					angle = -90;
				else 
					angle = -45;
			}
			else{
				if(pos.x < 2)
					angle = 90;
				else if(pos.x > winSize.x - 2)
					angle = 180;
				else
					angle = 135;
			}
			Resources::getTexture("Textures/HUD/CrosshairArrow")->render(glm::vec2(pos.x,pos.y),m_Color,angle,glm::vec2(scl,scl),0);
		}
		#pragma endregion

	}
}