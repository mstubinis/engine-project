#pragma once
#ifndef ENGINE_OBJECTDISPLAY_H
#define ENGINE_OBJECTDISPLAY_H

#include "Object.h"
#include "MeshInstance.h"

class ObjectDisplay: public ObjectBasic{
    protected:
        bool m_Shadeless;
        bool m_Visible;
		bool m_PassedRenderCheck;
        std::vector<MeshInstance*> m_MeshInstances;
        glm::vec4 m_Color;
        glm::vec3 m_GodsRaysColor;
        glm::vec3 m_BoundingBoxRadius;
        virtual void calculateRadius();
    public:
        ObjectDisplay(
            std::string = "",
            std::string = "",
            glm::vec3 = glm::vec3(0.0f),
            glm::vec3 = glm::vec3(1.0f),
            std::string = "Visible Object",
            Scene* = nullptr
        );
        virtual ~ObjectDisplay();

        virtual void update(float);
        void checkRender(Camera*);

        virtual void suspend();
        virtual void resume();

        virtual void setScale(float,float,float); 
        virtual void setScale(glm::vec3);

        virtual void setColor(float,float,float,float); 
        virtual void setColor(glm::vec4);

        virtual void setGodsRaysColor(float,float,float); 
        virtual void setGodsRaysColor(glm::vec3);

        virtual void scale(float,float,float);
        virtual void scale(glm::vec3);

        void setMesh(Mesh*);
        void setMesh(const std::string& mesh);

        void setMaterial(Material*);
        void setMaterial(const std::string& material);

        virtual glm::vec3& getRadiusBox(){ return m_BoundingBoxRadius; }

        glm::vec4& getColor(){ return m_Color; }
        glm::vec3& getGodsRaysColor(){ return m_GodsRaysColor; }
        std::vector<MeshInstance*>&  getMeshInstances(){ return m_MeshInstances; }

        virtual void setVisible(bool b);
		bool passedRenderCheck(){return m_PassedRenderCheck;}
        virtual bool visible() { return m_Visible; }
        virtual bool rayIntersectSphere(Camera* = nullptr);
        virtual bool rayIntersectSphere(glm::vec3 origin, glm::vec3 vector);

        void playAnimation(const std::string& animName,float startTime = 0.0f);

        //plays the animation from a specified starting time to the specified ending time, looping through until the amount of requested
        //loops are made. if endTime is less than 0 (which it is by default), the ending time will be the animation's default duration.
        void playAnimation(const std::string& animName,float startTime,float endTime = -1.0f ,uint requestedLoops = 1);
};
#endif
