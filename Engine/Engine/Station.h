#ifndef STATION_H
#define STATION_H

#include "ObjectDynamic.h"

class Station: public ObjectDynamic{
	private:

	protected:

	public:
		Station(std::string = "",
			    std::string = "",
			    glm::vec3 = glm::vec3(0,0,0),         //Position
				glm::vec3 = glm::vec3(1,1,1),         //Scale
				std::string = "Station",              //Object name
				Engine::Physics::Collision* = nullptr,//Bullet Collision Shape
				Scene* = nullptr
			   );
		~Station();

		void update(float);
		void render();
};
#endif