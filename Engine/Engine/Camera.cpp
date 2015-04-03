#include "Camera.h"
#include "Engine_Resources.h"
#include <glm\gtx\transform.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

Camera::Camera(float angleVal, float aspectRatioVal, float clipStartVal, float clipEndVal): Object("","",glm::vec3(0,0,0),glm::vec3(1,1,1),glm::vec3(0,0,0),"Camera"){//create a perspective camera
	m_Angle = angleVal;
	m_AspectRatio = aspectRatioVal;
	m_ClipStart = clipStartVal;
	m_ClipEnd = clipEndVal;

	m_Type = CAMERA_TYPE_PERSPECTIVE;

	Set_Perspective_Projection();
	m_State = CAMERA_STATE_FREEFORM;
	m_Friction = 0.001f;
	Set_View();
}
Camera::Camera(float leftVal, float rightVal, float bottomVal, float topVal, float clipStartVal, float clipEndVal): Object("","",glm::vec3(0,0,0),glm::vec3(1,1,1),glm::vec3(0,0,0),"Camera"){//create an orthographic camera
	m_Angle = 45.0f;
	m_AspectRatio = 1.0f;
	m_ClipStart = clipStartVal;
	m_ClipEnd = clipEndVal;

	m_Type = CAMERA_TYPE_ORTHOGRAPHIC;

	Set_Ortho_Projection(leftVal,rightVal,bottomVal,topVal);
	m_State = CAMERA_STATE_FREEFORM;
	m_Friction = 0.001f;
	Set_View();
}
Camera::~Camera(){ 
}
void Camera::Set_Perspective_Projection(){ m_Projection = glm::perspective(m_Angle,m_AspectRatio,m_ClipStart,m_ClipEnd); }
void Camera::Set_Ortho_Projection(float left, float right, float bottom, float top){
	m_Projection = glm::ortho(left,right,bottom,top,m_ClipStart,m_ClipEnd);
}
void Camera::Set_Aspect_Ratio(float ratio){ 
	m_AspectRatio = ratio;
	if(m_Type == CAMERA_TYPE_PERSPECTIVE)
		Set_Perspective_Projection();
}
void Camera::LookAt(glm::vec3& target){ m_View = glm::lookAt(m_Position,target,m_Up); }
void Camera::LookAt(glm::vec3& target,glm::vec3& up){ m_View = glm::lookAt(m_Position,target,up); }
void Camera::LookAt(glm::vec3& eye,glm::vec3& target,glm::vec3& up){ m_View = glm::lookAt(eye,target,up); }
void Camera::LookAt(Object* target, bool targetUp){
	if(!targetUp) m_View = glm::lookAt(m_Position,target->Position(),m_Up);
	else m_View = glm::lookAt(m_Position,target->Position(),target->Up());
}
void Camera::Set_View(){
	glm::vec3 p = Position();
	m_View = glm::lookAt(p,p + m_Forward,m_Up); 
}
glm::mat4 Camera::Calculate_Projection(glm::mat4& modelMatrix){ return m_Projection * m_View * modelMatrix; }
glm::mat4 Camera::Calculate_ModelView(glm::mat4& modelMatrix){ return m_View * modelMatrix; }
glm::mat4 Camera::Calculate_ViewProjInverted(){ return glm::inverse(m_Projection * m_View); }
void Camera::Update(float dt){
	if(m_Parent != nullptr){
		glm::mat4 newModel = m_Parent->Model();
		newModel = glm::translate(newModel, m_Position);
		m_Model = newModel;
		LookAt(Position(),m_Parent->Position(), m_Parent->Up());
	}
	else{
		glm::mat4 newModel = glm::mat4(1);
		newModel = glm::mat4(1);
		newModel = glm::translate(newModel, m_Position);
		m_Model = newModel;
		Set_View();
	}
}
void Camera::Render(Mesh* mesh, Material* material){}
void Camera::Render(){ Render(nullptr,nullptr); }
glm::mat4 Camera::Projection() const { return m_Projection; }
glm::mat4 Camera::View() const { return m_View; }
CAMERA_TYPE Camera::Get_Type() const { return m_Type; }
CAMERA_STATE Camera::Get_State() const { return m_State; }
float Camera::Get_Angle_Between(Object* other) const{
	glm::vec3 a = glm::normalize(m_Forward);
	glm::vec3 b = glm::normalize(other->Position() - m_Position); //object to camera
	float dot_product = glm::dot(a,b);
	float length_of_a = glm::length(a);
	float length_of_b = glm::length(b);

	float cos_angle = dot_product/(length_of_a*length_of_b);
	float angle = acos(cos_angle) * (180/3.14159265359f);
	if(a == b)
		return 0;
	return angle;
}
glm::vec3 Camera::Position() const { return glm::vec3(m_Model[3][0],m_Model[3][1],m_Model[3][2]); }