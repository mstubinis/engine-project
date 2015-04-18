#include "Object.h"
#include "Camera.h"
#include "Engine_Resources.h"
#include "ShaderProgram.h"
#include "Mesh.h"
#include "Material.h"
#include "Scene.h"

#include <boost/lexical_cast.hpp>

using namespace Engine;

Object::Object(std::string mesh, std::string mat, glm::vec3 pos, glm::vec3 scl,std::string name,bool isNotLight,Scene* scene){
	m_Changed = true;
	m_Radius = 0;
	m_BoundingBoxRadius = glm::vec3(0,0,0);
	m_Forward = glm::vec3(0,0,-1);
	m_Right = glm::vec3(1,0,0);
	m_Up = glm::vec3(0,1,0);
	Set_Mesh(Resources::getMesh(mesh));
	Set_Material(Resources::getMaterial(mat));
	m_Parent = nullptr;

	m_Name = name;
	m_Model = m_WorldMatrix = glm::mat4(1);
	m_Orientation = glm::quat();
	m_Color = glm::vec3(1,1,1);

	Object::Set_Position(pos);
	Object::Set_Scale(scl);

	unsigned int count = 0;
	if(scene == nullptr){
		scene = Resources::getCurrentScene();
	}
	if (scene->Objects().size() > 0){
		while(scene->Objects().count(m_Name)){
			m_Name = name + " " + boost::lexical_cast<std::string>(count);
			count++;
		}
	}
	if(isNotLight)
		scene->Objects()[m_Name] = this;
}
Object::~Object()
{
}
void Object::m_Calculate_Radius(){
	if(m_Mesh == nullptr){
		m_BoundingBoxRadius = glm::vec3(0,0,0);
		return;
	}
	m_BoundingBoxRadius = m_Mesh->Radius() * m_Scale;
	m_Radius = glm::max(glm::abs(m_BoundingBoxRadius.x),glm::max(glm::abs(m_BoundingBoxRadius.y),glm::abs(m_BoundingBoxRadius.z)));
}
glm::vec3 Object::_Forward(){ return glm::normalize(glm::cross(Right(),Up())); }
glm::vec3 Object::_Right(){
	float x = m_Orientation.x;
	float y = m_Orientation.y;
	float z = m_Orientation.z;
	float w = m_Orientation.w;
	return glm::normalize(glm::vec3( 1 - 2 * (y * y + z * z),
                                     2 * (x * y + w * z),
                                     2 * (x * z - w * y)));
}
glm::vec3 Object::_Up(){
	float x = m_Orientation.x;
	float y = m_Orientation.y;
	float z = m_Orientation.z;
	float w = m_Orientation.w;
	return glm::normalize(glm::vec3( 2 * (x * y - w * z), 
                                     1 - 2 * (x * x + z * z),
                                     2 * (y * z + w * x)));
}
void Object::Translate(float x, float y, float z,bool local){
	float dt = Resources::Detail::ResourceManagement::m_DeltaTime;
	x *= dt; y*= dt; z *= dt;
	if(local){
		m_Position += Forward() * z;
		m_Position += Right() * x;
		m_Position += Up() * y;
	}
	else{
		m_Position += glm::vec3(x,y,z);
	}
	Flag_As_Changed();
}
void Object::Translate(glm::vec3& translation,bool local){ Translate(translation.x,translation.y,translation.z); }
void Object::Rotate(float x, float y, float z){ 
	float threshold = 0.025f;
	if(abs(x) < threshold && abs(y) < threshold && abs(z) < threshold)
		return;
	Pitch(x); 
	Yaw(y); 
	Roll(z); 
	m_Changed = true;
}
void Object::Rotate(glm::vec3& rotation){ Rotate(rotation.x,rotation.y,rotation.z); }
void Object::Scale(float x, float y, float z){
	float dt = Resources::Detail::ResourceManagement::m_DeltaTime;
	m_Scale.x += x * dt; 
	m_Scale.y += y * dt; 
	m_Scale.z += z * dt; 
	m_Calculate_Radius(); 
	Flag_As_Changed();
}
void Object::Scale(glm::vec3& scale){ Scale(scale.x,scale.y,scale.z); }
void Object::Set_Position(float x, float y, float z){ 
	m_Position.x = x; 
	m_Position.y = y; 
	m_Position.z = z; 
	Flag_As_Changed();
}
void Object::Set_Position(glm::vec3 position){ Set_Position(position.x,position.y,position.z); }
void Object::Set_Scale(float x, float y, float z){ 
	m_Scale.x = x; 
	m_Scale.y = y; 
	m_Scale.z = z; 
	m_Calculate_Radius(); 
	Flag_As_Changed();
}
void Object::Set_Scale(glm::vec3& scale){ Set_Scale(scale.x,scale.y,scale.z); }
void Object::Pitch(float amount){ 
	m_Orientation = m_Orientation * glm::normalize(glm::angleAxis(-amount, glm::vec3(1,0,0)));
	Flag_As_Changed();
}
void Object::Yaw(float amount){ 
	m_Orientation = m_Orientation * glm::normalize(glm::angleAxis(-amount, glm::vec3(0,1,0)));
	Flag_As_Changed();
}
void Object::Roll(float amount){ 
	m_Orientation = m_Orientation * glm::normalize(glm::angleAxis(amount,glm::vec3(0,0,1)));
	Flag_As_Changed();
}
void Object::Update(float dt){
	glm::mat4 newModel = glm::mat4(1);
	if(m_Parent != nullptr){
		newModel = m_Parent->m_Model;
	}
	if(m_Changed){
		m_Forward = _Forward();
		m_Right = _Right();
		m_Up = _Up();

		newModel = glm::translate(newModel, m_Position);
		newModel *= glm::mat4_cast(m_Orientation);
		newModel = glm::scale(newModel,m_Scale);
		m_Model = newModel;
		m_Changed = false;
	}
	m_WorldMatrix = Resources::getActiveCamera()->Calculate_Projection(m_Model);
}
float Object::Distance(Object* other){
	glm::vec3 vecTo = other->Position() - Position();
	return abs(glm::length(vecTo));
}
void Object::Render(Mesh* mesh, Material* material,bool debug){
	if(mesh == nullptr)
		return;
	if(!Resources::getActiveCamera()->SphereIntersectTest(this))
		return;
	if(Resources::getActiveCamera()->Distance(this) > 450 * Radius())
		return;

	GLuint shader = Resources::getShader("Deferred")->Get_Shader_Program();

	glUseProgram(shader);
	glUniformMatrix4fv(glGetUniformLocation(shader, "MVP" ), 1, GL_FALSE, glm::value_ptr(m_WorldMatrix));
	glUniformMatrix4fv(glGetUniformLocation(shader, "World" ), 1, GL_FALSE, glm::value_ptr(m_Model));
	glUniform3f(glGetUniformLocation(shader, "Object_Color"),m_Color.x,m_Color.y,m_Color.z);
	glUniform1i(glGetUniformLocation(shader, "Shadeless"),static_cast<int>(material->Shadeless()));

	glUniform1f(glGetUniformLocation(shader, "far"),Resources::getActiveCamera()->Far());
	glUniform1f(glGetUniformLocation(shader, "C"),1.0f);
	for(auto component:material->Components())
		material->Bind_Texture(component.second,shader);

	mesh->Render();
}
void Object::Render(bool debug){ Object::Render(m_Mesh,m_Material,debug); }
void Object::Add_Child(Object* child){
	child->m_Parent = this;
	this->m_Children.push_back(child);
	child->Flag_As_Changed();
	Flag_As_Changed();
}
void Object::Set_Color(float x, float y, float z){ m_Color.x = x; m_Color.y = y; m_Color.z = z; }
void Object::Set_Color(glm::vec3& color){ Object::Set_Color(color.x,color.y,color.z); }

void Object::Set_Mesh(Mesh* mesh){ 
	m_Mesh = mesh; 
	if(m_Mesh == nullptr){
		m_BoundingBoxRadius = glm::vec3(0,0,0); 
		return; 
	} 
	m_BoundingBoxRadius = mesh->Radius();  
	m_Calculate_Radius();
}
void Object::Set_Material(Material* material){ m_Material = material; }
void Object::Flag_As_Changed(){
	m_Changed = true;
	for(auto child:m_Children)
		child->m_Changed = true;
}