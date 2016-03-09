#include "ObjectDisplay.h"
#include "Engine_Resources.h"
#include "Camera.h"
#include "Material.h"
#include "Mesh.h"
#include "ShaderProgram.h"

using namespace Engine;

ObjectDisplay::ObjectDisplay(std::string mesh, std::string mat, glm::v3 pos, glm::vec3 scl, std::string name,Scene* scene):Object(pos,scl,name,scene){
	m_Radius = 0;
	m_Visible = true;
	m_BoundingBoxRadius = glm::vec3(0);
	m_DisplayItems.push_back(new DisplayItem(Resources::getMesh(mesh),Resources::getMaterial(mat)));
	m_Color = glm::vec4(1);
	calculateRadius();
}
ObjectDisplay::~ObjectDisplay(){
	for(auto item:m_DisplayItems) SAFE_DELETE(item);
}
void ObjectDisplay::render(GLuint shader,bool debug){
	//add to render queue
	if(shader == 0){
		shader = Resources::getShader("Deferred")->getShaderProgram();
	}
	Engine::Renderer::Detail::RenderManagement::getObjectRenderQueue().push_back(GeometryRenderInfo(this,shader));
}
void ObjectDisplay::draw(GLuint shader, bool debug){
	Camera* camera = Resources::getActiveCamera();
	if((m_DisplayItems.size() == 0 || m_Visible == false) || (!camera->sphereIntersectTest(this)) || (camera->getDistance(this) > 1100 * getRadius()))
		return;	
	glUseProgram(shader);

	glUniformMatrix4fv(glGetUniformLocation(shader, "VP" ), 1, GL_FALSE, glm::value_ptr(camera->getViewProjection()));
	glUniform1f(glGetUniformLocation(shader, "far"),camera->getFar());
	glUniform1f(glGetUniformLocation(shader, "C"),1.0f);
	glUniform4f(glGetUniformLocation(shader, "Object_Color"),m_Color.x,m_Color.y,m_Color.z,m_Color.w);

	for(auto item:m_DisplayItems){
		glm::mat4 m = glm::mat4(m_Model);
		m = glm::translate(m,item->position);
		m *= glm::mat4_cast(item->orientation);
		m = glm::scale(m,item->scale);

		glUniform1i(glGetUniformLocation(shader, "Shadeless"),static_cast<int>(item->material->getShadeless()));
		glUniform1f(glGetUniformLocation(shader, "BaseGlow"),item->material->getBaseGlow());
		glUniform1f(glGetUniformLocation(shader, "Specularity"),item->material->getSpecularity());

		glUniformMatrix4fv(glGetUniformLocation(shader, "World" ), 1, GL_FALSE, glm::value_ptr(m));

		for(auto component:item->material->getComponents())
			item->material->bindTexture(component.first,shader);
		item->mesh->render();
	}
	glUseProgram(0);
}
void ObjectDisplay::calculateRadius(){
	if(m_DisplayItems.size() == 0){
		m_BoundingBoxRadius = glm::vec3(0);
		return;
	}
	float maxLength = 0;
	for(auto item:m_DisplayItems){
		float length = 0;
		glm::mat4 m = glm::mat4(1);
		m = glm::translate(m,item->position);
		m *= glm::mat4_cast(item->orientation);
		m = glm::scale(m,item->scale);

		glm::vec3 localPosition = glm::vec3(m[3][0],m[3][1],m[3][2]);
		
		length = glm::length(localPosition) + item->mesh->getRadius() * glm::max(glm::abs(item->scale.z), glm::max(glm::abs(item->scale.x),glm::abs(item->scale.y)));

		if(length > maxLength){
			maxLength = length;
		}
	}
	m_BoundingBoxRadius = maxLength * m_Scale;
	m_Radius = glm::max(glm::abs(m_BoundingBoxRadius.x),glm::max(glm::abs(m_BoundingBoxRadius.y),glm::abs(m_BoundingBoxRadius.z)));
}
void ObjectDisplay::setColor(float x, float y, float z,float a){ 
	m_Color.x = x; m_Color.y = y; m_Color.z = z; m_Color.w = a; 
}
void ObjectDisplay::setColor(glm::vec4 color){ setColor(color.x,color.y,color.z,color.w); }
void ObjectDisplay::setVisible(bool vis){ m_Visible = vis; }

void ObjectDisplay::scale(float x, float y,float z){
	Object::scale(x,y,z);
	calculateRadius(); 
}
void ObjectDisplay::scale(glm::vec3 scl){ ObjectDisplay::scale(scl.x,scl.y,scl.z); }
bool ObjectDisplay::rayIntersectSphere(Camera* cam){
	return cam->rayIntersectSphere(this);
}
bool ObjectDisplay::rayIntersectSphere(glm::v3 A, glm::vec3 rayVector){
	glm::vec3 a1 = glm::vec3(A);
	glm::vec3 B = a1 + rayVector;

	glm::vec3 C = glm::vec3(getPosition());
	float r = getRadius();

	//check if point is behind
	float dot = glm::dot(rayVector,C-a1);
	if(dot >= 0)
		return false;

	glm::nType a = ((B.x-A.x)*(B.x-A.x))  +  ((B.y - A.y)*(B.y - A.y))  +  ((B.z - A.z)*(B.z - A.z));
	glm::nType b = 2* ((B.x - A.x)*(A.x - C.x)  +  (B.y - A.y)*(A.y - C.y)  +  (B.z - A.z)*(A.z-C.z));
	glm::nType c = (((A.x-C.x)*(A.x-C.x))  +  ((A.y - C.y)*(A.y - C.y))  +  ((A.z - C.z)*(A.z - C.z))) - (r*r);

	glm::nType Delta = (b*b) - (4*a*c);

	if(Delta < 0)
		return false;
	return true;
}