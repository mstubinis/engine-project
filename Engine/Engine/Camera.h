#ifndef _CAMERA_H
#define _CAMERA_H

#include "Object.h"

class ObjectDisplay;

enum CAMERA_TYPE { CAMERA_TYPE_PERSPECTIVE, CAMERA_TYPE_ORTHOGRAPHIC };

class Scene;
class Camera: public Object{
	private:
		void _constructFrustrum();
		glm::vec4 m_Planes[6];
	protected:
		CAMERA_TYPE m_Type;

		float m_Angle;
		float m_AspectRatio;

		float m_Near, m_Far;
		glm::mat4 m_View, m_Projection;
	public:
		Camera(std::string name, float angle, float ratio, float near, float far,Scene* = nullptr);				           // Perspective camera Constructor
		Camera(std::string name, float left, float right, float bottom, float top, float near, float far,Scene* = nullptr); // Orthographic camera Constructor
		~Camera();

		void resize(unsigned int w, unsigned int h);
		void setPerspectiveProjection();
		void setOrthoProjection(float,float,float,float);

		virtual void update(float);

		void lookAt(glm::v3);  
		void lookAt(glm::v3,glm::v3); 
		void lookAt(glm::v3,glm::v3,glm::v3); 
		void lookAt(Object*,bool targetUp = false);

		void setAspectRatio(float);

		const float getAngle() const { return m_Angle; }
		const float getAspectRatio() const { return m_AspectRatio; }
		const float getNear() const { return m_Near; }
		const float getFar() const { return m_Far; }

		glm::mat4 calculateViewProjInverted();
		glm::mat4 calculateProjection(glm::mat4);
		glm::mat4 calculateModelView(glm::mat4);
		glm::mat4 getProjection(){ return m_Projection; }
		glm::mat4 getView(){ return m_View; }
		glm::mat4 getViewProjection(){ return m_Projection * m_View; }
		glm::vec3 getViewVector(){ return glm::vec3(m_View[0][2],m_View[1][2],m_View[2][2]); }
		const CAMERA_TYPE getType() const{ return m_Type; }

		bool sphereIntersectTest(ObjectDisplay*);
		bool sphereIntersectTest(glm::v3 pos, float radius);

		//ray tests
		bool rayIntersectSphere(ObjectDisplay*);
};
#endif