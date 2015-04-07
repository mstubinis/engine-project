#include "Planet.h"
#include "Engine_Resources.h"
#include "ShaderProgram.h"

Planet::Planet(std::string mat, PlanetType type, glm::vec3 pos, glm::vec3 scl, std::string name, bool add):Object("Planet",mat,pos,scl,name,add){
	m_AtmosphereHeight = 0.025f;
	m_Type = type;
}
Planet::~Planet()
{
}
void Planet::Update(float dt)
{
	Object::Update(dt);
}
void Planet::Render(Mesh* mesh, Material* mat,bool debug){
	if(mesh == nullptr)
		return;

	GLuint shaderProgram = Resources->Get_Shader_Program("AS_GroundFromSpace")->Get_Shader_Program();

	float innerRadius = 1.0f;
	float outerRadius = innerRadius + (innerRadius * m_AtmosphereHeight);

	glUseProgram(shaderProgram);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "MVP" ), 1, GL_FALSE, glm::value_ptr(m_WorldMatrix));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "World" ), 1, GL_FALSE, glm::value_ptr(m_Model));


	glUniform3f(glGetUniformLocation(shaderProgram, "Object_Color"),m_Color.x,m_Color.y,m_Color.z);

	for(auto component:mat->Components())
		mat->Bind_Texture(component.second,shaderProgram);

	if(m_AtmosphereHeight > 0){
		glUniform1i(glGetUniformLocation(shaderProgram,"hasAtmosphere"),1);
		glUniform1i(glGetUniformLocation(shaderProgram,"HasAtmosphere"),1);
	}
	else{
		glUniform1i(glGetUniformLocation(shaderProgram,"hasAtmosphere"),0);
		glUniform1i(glGetUniformLocation(shaderProgram,"HasAtmosphere"),0);
	}

	float Km = 0.0025f;
	float Kr = 0.0015f;
	float ESun = 20.0f;

	glm::vec3 camPos = Resources->Current_Camera()->Position() - Position();
	glUniform3f(glGetUniformLocation(shaderProgram,"v3CameraPos"), camPos.x,camPos.y,camPos.z);

	glm::vec3 lightDir = Resources->Lights.at(0)->Position() - Position();
	lightDir = glm::normalize(lightDir);
	glUniform3f(glGetUniformLocation(shaderProgram,"v3LightDir"), lightDir.x,lightDir.y,lightDir.z);

	glm::vec3 v3InvWaveLength = glm::vec3(1.0f / glm::pow(0.65f, 4.0f),1.0f / glm::pow(0.57f, 4.0f),1.0f / glm::pow(0.475f, 4.0f));

	glUniform3f(glGetUniformLocation(shaderProgram,"v3InvWavelength"), v3InvWaveLength.x,v3InvWaveLength.y,v3InvWaveLength.z);

	float camHeight = glm::length(camPos);
	float camHeight2 = camHeight*camHeight;

	glUniform1f(glGetUniformLocation(shaderProgram,"fCameraHeight2"), camHeight2);


	glUniform1f(glGetUniformLocation(shaderProgram,"fOuterRadius"), outerRadius);
	glUniform1f(glGetUniformLocation(shaderProgram,"fOuterRadius2"), outerRadius*outerRadius);
	glUniform1f(glGetUniformLocation(shaderProgram,"fInnerRadius"), innerRadius);
	glUniform1f(glGetUniformLocation(shaderProgram,"fInnerRadius2"), innerRadius*innerRadius);

	glUniform1f(glGetUniformLocation(shaderProgram,"fKrESun"), Kr * ESun);
	glUniform1f(glGetUniformLocation(shaderProgram,"fKmESun"), Km * ESun);

	glUniform1f(glGetUniformLocation(shaderProgram,"fKr4PI"), Kr * 4 * 3.14159f);
	glUniform1f(glGetUniformLocation(shaderProgram,"fKm4PI"), Km * 4 * 3.14159f);

	float fScaledepth = 0.25f;
	float fScale = 1.0f / (outerRadius - innerRadius);

	glUniform1f(glGetUniformLocation(shaderProgram,"fScaleDepth"),fScaledepth);
	glUniform1f(glGetUniformLocation(shaderProgram,"fScale"),fScale);
	glUniform1f(glGetUniformLocation(shaderProgram,"fScaleOverScaleDepth"), fScale / fScaledepth);

	glUniform1f(glGetUniformLocation(shaderProgram,"fExposure"), 2.0f);


	mesh->Render();

	//sky from space
	if(m_AtmosphereHeight <= 0.0f)
		return;
	GLuint shader = Resources->Get_Shader_Program("AS_SkyFromSpace")->Get_Shader_Program();
	glUseProgram(shader);

	glCullFace(GL_FRONT);

	glEnable(GL_BLEND);
   	glBlendEquation(GL_FUNC_ADD);
   	glBlendFunc(GL_ONE, GL_ONE);


	glm::mat4 obj = glm::mat4(1);
	obj = glm::translate(obj,Position());
	obj *= glm::mat4_cast(m_Orientation);
	float rad = outerRadius*m_Scale.x;
	obj = glm::scale(obj,glm::vec3(rad,rad,rad));

	glUniformMatrix4fv(glGetUniformLocation(shader, "MVP" ), 1, GL_FALSE, glm::value_ptr(Resources->Current_Camera()->Calculate_Projection(obj)));


	glUniform1i(glGetUniformLocation(shader,"nSamples"), 2);
	glUniform1f(glGetUniformLocation(shader,"fSamples"), 2.0f);

	glUniform3f(glGetUniformLocation(shader,"v3CameraPos"), camPos.x,camPos.y,camPos.z);

	glUniform3f(glGetUniformLocation(shader,"v3LightDir"), lightDir.x,lightDir.y,lightDir.z);

	glUniform3f(glGetUniformLocation(shader,"v3InvWavelength"), v3InvWaveLength.x,v3InvWaveLength.y,v3InvWaveLength.z);

	glUniform1f(glGetUniformLocation(shader,"fCameraHeight"),camHeight);
	glUniform1f(glGetUniformLocation(shader,"fCameraHeight2"), camHeight2);

	glUniform1f(glGetUniformLocation(shader,"fOuterRadius"), outerRadius);
	glUniform1f(glGetUniformLocation(shader,"fOuterRadius2"), outerRadius*outerRadius);
	glUniform1f(glGetUniformLocation(shader,"fInnerRadius"), innerRadius);
	glUniform1f(glGetUniformLocation(shader,"fInnerRadius2"), innerRadius*innerRadius);

	glUniform1f(glGetUniformLocation(shader,"fKrESun"), Kr * ESun);
	glUniform1f(glGetUniformLocation(shader,"fKmESun"), Km * ESun);

	glUniform1f(glGetUniformLocation(shader,"fKr4PI"), Kr * 4 * 3.14159f);
	glUniform1f(glGetUniformLocation(shader,"fKm4PI"), Km * 4 * 3.14159f);

	glUniform1f(glGetUniformLocation(shader,"fScaleDepth"),fScaledepth);
	glUniform1f(glGetUniformLocation(shader,"fScale"),fScale);
	glUniform1f(glGetUniformLocation(shader,"fScaleOverScaleDepth"), fScale / fScaledepth);

	// Gravity
	float g = -0.98f;
	glUniform1f(glGetUniformLocation(shader,"g"),g);
	glUniform1f(glGetUniformLocation(shader,"g2"), g*g);
	glUniform1f(glGetUniformLocation(shader,"fExposure"),2.0f);

	Resources->Get_Mesh("Planet")->Render();

	glUseProgram(0);

	glCullFace(GL_BACK);
	glDisable(GL_BLEND);
}
void Planet::Render(bool debug){
	this->Render(m_Mesh,m_Material,debug);
}