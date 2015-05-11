#ifndef OBJECTDYNAMIC_H
#define OBJECTDYNAMIC_H

#include "ObjectDisplay.h"

class btCollisionShape;
class btRigidBody;
struct btDefaultMotionState;
class btVector3;

class ObjectDynamic: public ObjectDisplay{
	private:
		glm::vec3 _calculateForward(); 
		glm::vec3 _calculateRight(); 
		glm::vec3 _calculateUp();
		float m_Mass;
	protected:
		btCollisionShape* m_Collision_Shape;
		btRigidBody* m_RigidBody;
		btDefaultMotionState* m_MotionState;
		btVector3* m_Inertia;
	public:
		ObjectDynamic( std::string = "",
					   std::string = "",
					   glm::vec3 = glm::vec3(0,0,0),   //Position
					   glm::vec3 = glm::vec3(1,1,1),   //Scale
					   std::string = "Dynamic Object", //Object
					   btCollisionShape* = nullptr,    //Bullet Collision Shape
					   Scene* = nullptr
					 );
		~ObjectDynamic();

		virtual void setPosition(float,float,float); 
		virtual void setPosition(glm::vec3);

		void translate(float,float,float,bool local=true); 
		void translate(glm::vec3,bool local=true);
		void applyForce(float,float,float,bool local=true); 
		void applyForce(glm::vec3,glm::vec3 = glm::vec3(0,0,0),bool local=true);
		void applyImpulse(float,float,float); 
		void applyImpulse(glm::vec3,glm::vec3 = glm::vec3(0,0,0));
		void applyTorque(float,float,float); 
		void applyTorque(glm::vec3);
		void applyTorqueImpulse(float,float,float); 
		void applyTorqueImpulse(glm::vec3);

		void setLinearVelocity(float,float,float,bool local=true); 
		void setLinearVelocity(glm::vec3,bool local=true);
		void setAngularVelocity(float,float,float); 
		void setAngularVelocity(glm::vec3);

		void setMass(float);

		void update(float);
		void _updateMatrix(float);
		const float getMass() const { return m_Mass; }
};
#endif