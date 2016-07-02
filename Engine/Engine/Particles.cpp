#include "Engine_Resources.h"
#include "Particles.h"
#include "Mesh.h"
#include "Material.h"
#include "Texture.h"
#include "Camera.h"
#include "Light.h"
#include "Scene.h"
#include "Engine.h"

#include <algorithm>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <boost/lexical_cast.hpp>


ParticleInfo::ParticleInfo(std::string _material,glm::vec4 _startColor, glm::vec4 _endColor){
    material = Engine::Resources::getMaterial(_material);
    startColor = _startColor;
    endColor = _endColor;
}
ParticleInfo::ParticleInfo(Material* _material,glm::vec4 _startColor, glm::vec4 _endColor){
    material = _material;
    startColor = _startColor;
    endColor = _endColor;
}
ParticleInfo::~ParticleInfo(){
}

ParticleEmitter::ParticleEmitter(ParticleInfo* info, glm::v3 pos, glm::vec3 scl,std::string name, Scene* scene):ObjectBasic(pos,scl,name,scene){
    m_info = info;
    unsigned int count = 0;
    if(scene == nullptr){
        scene = Engine::Resources::getCurrentScene();
    }
    if (scene->getParticleEmitters().size() > 0){
        while(scene->getParticleEmitters().count(m_Name)){
            m_Name = name + " " + boost::lexical_cast<std::string>(count);
            count++;
        }
    }
    scene->getParticleEmitters()[m_Name] = this;
}
ParticleEmitter::~ParticleEmitter(){
    for(auto particle:m_Particles) SAFE_DELETE(particle);
}
void ParticleEmitter::addParticle(){
    float rot = static_cast<float>(rand() % 360);

    glm::vec3 velocity = glm::vec3(0,0.05f,0);
    float rotationalVelocity = 0.05f;
    m_Particles.push_back(new Particle(this,this->getPosition(),m_info->startColor,glm::vec2(1,1),rot,velocity,rotationalVelocity,glm::vec2(0,0)));
}
void ParticleEmitter::deleteParticles(){
    std::vector<Particle*> copies = m_Particles;
    for(auto p:m_Particles){
        if(p->ToBeErased()){
            copies.erase(std::remove(copies.begin(), copies.end(), p), copies.end());
            SAFE_DELETE(p);
        }
    }
    m_Particles = copies;
}
void ParticleEmitter::update(float dt){
    Object::update(dt);
    for(auto particle:m_Particles)
        particle->update(dt);
    deleteParticles();
}
void ParticleEmitter::render(){
    return;
}

Particle::Particle(ParticleEmitter* _emitter,glm::v3 pos,glm::vec4 col,glm::vec2 scl ,float rot,glm::vec3 vel,float rVel, glm::vec2 sVel){
    emitter = _emitter;
    position = pos;
    color = col;
    scale = scl;
    zRot = rot;
    velocity = vel;
    zRotVelocity = rVel;
    scaleVelocity = sVel;
    toBeErased = false;

    model = glm::mat4(1);
    model = glm::translate(model,position);
    model = glm::rotate(model, zRot,glm::vec3(0,0,1));
    model = glm::scale(model, glm::vec3(scale.x,scale.y,1));
}
Particle::~Particle(){
    //delete light; 
}
void Particle::setPosition(glm::num x,glm::num y,glm::num z){
    position.x = float(x);
    position.y = float(y);
    position.z = float(z);

    model[3][0] = position.x;
    model[3][1] = position.y;
    model[3][2] = position.z;
}
void Particle::setPosition(glm::v3 pos){ setPosition(pos.x,pos.y,pos.z); }
void Particle::update(float dt){
    position += velocity*dt;
    zRot += zRotVelocity*dt;
    scale += scaleVelocity*dt;

    model = glm::mat4(1);
    model = glm::translate(model,position);
    model = glm::rotate(model, zRot,glm::vec3(0,0,1));
    model = glm::scale(model, glm::vec3(scale.x,scale.y,1));

    if(lifetime >= lifetimeMax)
        toBeErased = true;
}
void Particle::render(GLuint shader){

}