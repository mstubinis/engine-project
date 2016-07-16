#pragma once
#ifndef ENGINE_MESH_H
#define ENGINE_MESH_H

#define GL_TRIANGLES 0x0004

#include "Engine_Physics.h"
#include <unordered_map>

namespace sf{ class Image; };

class btHeightfieldTerrainShape;
struct MeshData;
typedef unsigned int GLuint;
typedef unsigned int uint;

const uint NUM_VERTEX_DATA = 5;
const uint VERTEX_AMOUNTS[NUM_VERTEX_DATA] = {3,2,3,3,3};

class Mesh final{
    private:
        GLuint m_buffers[NUM_VERTEX_DATA]; //0 - position, 1 - uv, 2 - normal, 3 - tangent, 4 - binormals
		GLuint m_elementbuffer;
        Collision* m_Collision;

        glm::vec3 m_radiusBox;
        float m_radius;
        std::vector<glm::vec3> m_Points;
        std::vector<glm::vec2> m_UVs;
        std::vector<glm::vec3> m_Normals;
        std::vector<glm::vec3> m_Tangents;
        std::vector<glm::vec3> m_Binormals;
		std::vector<unsigned short> m_Indices;

		void _loadData(MeshData&,float threshhold = 0.0001f);
        void _loadFromFile(std::string,COLLISION_TYPE);
        void _loadFromOBJ(std::string,COLLISION_TYPE);
        void _loadFromOBJMemory(std::string,COLLISION_TYPE);
        void _calculateMeshRadius();
    public:
        Mesh(btHeightfieldTerrainShape*);
		Mesh(std::unordered_map<std::string,float>& grid,uint width,uint length);
        Mesh(float width, float height);
        Mesh(float x, float y, float width, float height);
        Mesh(std::string = "",COLLISION_TYPE = COLLISION_TYPE_CONVEXHULL, bool notMemory = true);
        ~Mesh();

        void initRenderingContext(uint api);
        void cleanupRenderingContext(uint api);

        Collision* getCollision() const { return m_Collision; }

        const glm::vec3& getRadiusBox() const { return m_radiusBox; }
        const float getRadius() const { return m_radius; }

        void render(GLuint mode = GL_TRIANGLES);
};
#endif