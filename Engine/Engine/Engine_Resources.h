#pragma once
#ifndef ENGINE_ENGINE_RESOURCES_H
#define ENGINE_ENGINE_RESOURCES_H

#include "Engine_Physics.h"
#include "Engine_Utils.h"
#include "ShaderProgram.h"
#include "GLImageConstants.h"

#include <unordered_map>
#include <map>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <iostream>

typedef unsigned int uint;
typedef std::uint32_t uint32;

class Engine_Window;
class Scene;
class Font;
class Texture;
class Camera;
class Mesh;
class Material;
class SunLight;
class MeshInstance;
class SoundData;

class ResourceType final {public: enum Type {
	Empty, //do NOT move this specific enum
	Texture,
	Mesh,
	Material,
	Sound,
	MeshInstance,
	Object,
	Font,
	Camera,
	Shader,
	ShaderProgram,
	SoundData,
	Scene,
_TOTAL};};

struct Handle final {
	uint32 index : 12;
	uint32 counter : 15;
	uint32 type : 5;
	Handle();
	Handle(uint32 _index, uint32 _counter, uint32 _type);
	inline operator uint32() const;
	const bool null() const;
	const EngineResource* get() const;
	inline const EngineResource* operator ->() const;
};

namespace Engine{
    namespace epriv{
        class ResourceManager final: Engine::epriv::noncopyable{
            public:
				class impl;
                std::unique_ptr<impl> m_i;

                ResourceManager(const char* name,uint width,uint height);
                ~ResourceManager();

                void _init(const char* name,uint width,uint height);
 
                Handle _addTexture(Texture*);

                bool _hasScene(std::string);         void _addScene(Scene*);                std::string _buildSceneName(std::string);
                void _addMeshInstance(MeshInstance*);
                Texture* _hasTexture(std::string);
                uint _numScenes();
        };
    };
    namespace Resources{
        namespace Settings{
            void enableDynamicMemory(bool enable = true);
            void disableDynamicMemory();
        }

        Scene* getCurrentScene();
        void setCurrentScene(Scene* scene);
        void setCurrentScene(std::string sceneName);

        float& dt();

        Engine_Window* getWindow();
        glm::uvec2 getWindowSize();

        Scene* getScene(std::string sceneName);

        Font* getFont(std::string);

        void getShader(Handle& inHandle,Shader*& outPtr);         Shader* getShader(Handle& inHandle);
        void getSoundData(Handle& inHandle,SoundData*& outPtr);   SoundData* getSoundData(Handle& inHandle);
        void getCamera(Handle& inHandle,Camera*& outPtr);         Camera* getCamera(Handle& inHandle);
        void getFont(Handle& inHandle,Font*& outPtr);             Font* getFont(Handle& inHandle);
        void getTexture(Handle& inHandle,Texture*& outPtr);       Texture* getTexture(Handle& inHandle);
        void getMesh(Handle& inHandle,Mesh*& outPtr);             Mesh* getMesh(Handle& inHandle);
        void getMaterial(Handle& inHandle,Material*& outPtr);     Material* getMaterial(Handle& inHandle);
        void getShaderProgram(Handle& inHandle,ShaderP*& outPtr); ShaderP* getShaderProgram(Handle& inHandle);

        Handle addFont(std::string filename);

        Handle addMesh(std::string file, CollisionType::Type = CollisionType::None,bool fromFile = true,float threshhold = 0.0005f);
        Handle addMesh(std::string name, float x, float y, float w, float h,float threshhold = 0.0005f);
        Handle addMesh(std::string name, float w, float h,float threshhold = 0.0005f);
        Handle addMesh(std::string name, std::unordered_map<std::string,float>& grid, uint width, uint length,float threshhold = 0.0005f);
        Handle addMeshAsync(std::string file, CollisionType::Type = CollisionType::None,bool fromFile = true,float threshhold = 0.0005f);

        Handle addTexture(std::string file,ImageInternalFormat::Format = ImageInternalFormat::Format::SRGB8_ALPHA8,bool mipmaps = false);

        Handle addMaterial(std::string name, std::string diffuse, std::string normal = "", std::string glow = "",std::string specular = "",Handle shaderHandle = Handle());
        Handle addMaterial(std::string name, Texture* diffuse, Texture* normal = nullptr, Texture* glow = nullptr,Texture* specular = nullptr,ShaderP* = nullptr);
		Handle addMaterial(std::string name, Texture* diffuse, Texture* normal = nullptr, Texture* glow = nullptr,Texture* specular = nullptr,Handle shaderHandle = Handle());

        Handle addShader(std::string shaderFileOrData, ShaderType::Type shaderType, bool fromFile = true);
        Handle addSoundData(std::string file,std::string name = "",bool music = false);
        Handle addShaderProgram(std::string name, Shader* vertexShader, Shader* fragmentShader, ShaderRenderPass::Pass = ShaderRenderPass::Geometry);
        Handle addShaderProgram(std::string name, Handle& vertexShader, Handle& fragmentShader, ShaderRenderPass::Pass = ShaderRenderPass::Geometry);
    };
    namespace Data{
        std::string reportTime();
    };
};
#endif