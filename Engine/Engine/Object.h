#ifndef OBJECT_H
#define OBJECT_H

#include "Engine_Renderer.h"
#include "Mesh.h"
#include "Material.h"
#include <string>
#include <vector>
#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
                                                                                 
class Object{
	protected:
		std::string m_Name;
		glm::mat4 m_WorldMatrix, m_Model;
		glm::vec3 m_Position, m_Rotation, m_Scale, m_Forward, m_Right, m_Up, m_Color, m_Radius;

		Object* m_Parent;
		std::vector<Object*> m_Children;

		Mesh* m_Mesh;
		Material* m_Material;

		virtual void m_Calculate_Radius();

	public:
		Object(
				std::string = "",
				std::string = "",
			    glm::vec3 = glm::vec3(0,0,0),   //Position
			    glm::vec3 = glm::vec3(1,1,1),   //Scale
			    glm::vec3 = glm::vec3(0,0,0),   //Rotation
			    std::string = "Meshless Object",//Object
				bool addToResources = true      //Add this to the resource generic object pool (Don't do for lights)
			  );
		~Object();

		virtual void Translate(float,float,float); virtual void Translate(glm::vec3&);
		virtual void Rotate(float,float,float); virtual void Rotate(glm::vec3&);
		virtual void Scale(float,float,float); virtual void Scale(glm::vec3&);
		virtual void Pitch(float); void Yaw(float); virtual void Roll(float);

		virtual void Set_Position(float,float,float); virtual void Set_Position(glm::vec3&);
		virtual void Set_Scale(float,float,float); virtual void Set_Scale(glm::vec3&);

		void Add_Child(Object*);

		virtual void Update(float);
		virtual void Render(Mesh*, Material*,RENDER_TYPE);
		virtual void Render(RENDER_TYPE = RENDER_TYPE_FORWARD);

		#pragma region Getters
		virtual glm::vec3 Position(); 
		virtual glm::vec3 Scale();
		virtual glm::vec3 Forward(); 
		virtual glm::vec3 Right(); 
		virtual glm::vec3 Up();
		const glm::vec3& Radius() const;
		const glm::vec3& Color() const;
		const glm::mat4& Model() const;
		const glm::mat4& World() const;
		const std::string Name() const;
		const Object* Parent() const;
		std::vector<Object*> Children() const;
		#pragma endregion

		#pragma region Setters
		void Set_Mesh(Mesh*);
		void Set_Material(Material*);
		#pragma endregion
};
#endif