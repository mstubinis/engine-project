#pragma once
#ifndef ENGINE_ENGINE_MATH_H
#define ENGINE_ENGINE_MATH_H

#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>

#include <vector>
#include <assimp/Importer.hpp>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

class btVector3;
class btRigidBody;
class btQuaternion;
class Camera;
typedef std::uint32_t   uint;
typedef unsigned short  ushort;
typedef unsigned char   uchar;
namespace glm{
    //floats
    typedef glm::tquat<float> q_f;	
    typedef glm::tvec2<float> v2_f;
    typedef glm::tvec3<float> v3_f;
    typedef glm::tvec4<float> v4_f;
    typedef glm::tmat2x2<float> m2_f;
    typedef glm::tmat2x2<float> m2x2_f;
    typedef glm::tmat2x3<float> m2x3_f;
    typedef glm::tmat2x4<float> m2x4_f;
    typedef glm::tmat3x3<float> m3_f;
    typedef glm::tmat3x3<float> m3x3_f;
    typedef glm::tmat3x2<float> m3x2_f;
    typedef glm::tmat3x4<float> m3x4_f;
    typedef glm::tmat4x2<float> m4x2_f;
    typedef glm::tmat4x3<float> m4x3_f;
    typedef glm::tmat4x4<float> m4_f;
    typedef glm::tmat4x4<float> m4x4_f;
    //doubles
    typedef glm::tquat<double> q_d;
    typedef glm::tvec2<double> v2_d;
    typedef glm::tvec3<double> v3_d;
    typedef glm::tvec4<double> v4_d;
    typedef glm::tmat2x2<double> m2_d;
    typedef glm::tmat2x2<double> m2x2_d;
    typedef glm::tmat2x3<double> m2x3_d;
    typedef glm::tmat2x4<double> m2x4_d;
    typedef glm::tmat3x3<double> m3_d;
    typedef glm::tmat3x3<double> m3x3_d;
    typedef glm::tmat3x2<double> m3x2_d;
    typedef glm::tmat3x4<double> m3x4_d;
    typedef glm::tmat4x2<double> m4x2_d;
    typedef glm::tmat4x3<double> m4x3_d;
    typedef glm::tmat4x4<double> m4_d;
    typedef glm::tmat4x4<double> m4x4_d;
    //long doubles
    typedef glm::tquat<long double> q_ld;
    typedef glm::tvec2<long double> v2_ld;
    typedef glm::tvec3<long double> v3_ld;
    typedef glm::tvec4<long double> v4_ld;
    typedef glm::tmat2x2<long double> m2_ld;
    typedef glm::tmat2x2<long double> m2x2_ld;
    typedef glm::tmat2x3<long double> m2x3_ld;
    typedef glm::tmat2x4<long double> m2x4_ld;
    typedef glm::tmat3x3<long double> m3_ld;
    typedef glm::tmat3x3<long double> m3x3_ld;
    typedef glm::tmat3x2<long double> m3x2_ld;
    typedef glm::tmat3x4<long double> m3x4_ld;
    typedef glm::tmat4x2<long double> m4x2_ld;
    typedef glm::tmat4x3<long double> m4x3_ld;
    typedef glm::tmat4x4<long double> m4_ld;
    typedef glm::tmat4x4<long double> m4x4_ld;
    
    #ifdef ENGINE_PRECISION_NORMAL
        typedef float number;
        typedef number num;
        typedef q_f q;	
        typedef v2_f v2;
        typedef v3_f v3;
        typedef v4_f v4;
        typedef m2_f m2;
        typedef m2x2_f m2x2;
        typedef m2x3_f m2x3;
        typedef m2x4_f m2x4;
        typedef m3_f m3;
        typedef m3x3_f m3x3;
        typedef m3x2_f m3x2;
        typedef m3x4_f m3x4;
        typedef m4x2_f m4x2;
        typedef m4x3_f m4x3;
        typedef m4_f m4;
        typedef m4x4_f m4x4;
    #endif
    #ifdef ENGINE_PRECISION_HIGH
        typedef double number;
        typedef number num;
        typedef q_d q;
        typedef v2_d v2;
        typedef v3_d v3;
        typedef v4_d v4;
        typedef m2_d m2;
        typedef m2x2_d m2x2;
        typedef m2x3_d m2x3;
        typedef m2x4_d m2x4;
        typedef m3_d m3;
        typedef m3x3_d m3x3;
        typedef m3x2_d m3x2;
        typedef m3x4_d m3x4;
        typedef m4x2_d m4x2;
        typedef m4x3_d m4x3;
        typedef m4_d m4;
        typedef m4x4_d m4x4;
    #endif
    #ifdef ENGINE_PRECISION_VERY_HIGH
        typedef long double number;
        typedef number num;
        typedef q_ld q;
        typedef v2_ld v2;
        typedef v3_ld v3;
        typedef v4_ld v4;
        typedef m2_ld m2;
        typedef m2x2_ld m2x2;
        typedef m2x3_ld m2x3;
        typedef m2x4_ld m2x4;
        typedef m3_ld m3;
        typedef m3x3_ld m3x3;
        typedef m3x2_ld m3x2;
        typedef m3x4_ld m3x4;
        typedef m4x2_ld m4x2;
        typedef m4x3_ld m4x3;
        typedef m4_ld m4;
        typedef m4x4_ld m4x4;
    #endif
};

namespace Engine{
    namespace Math{
        void extractViewFrustumPlanesHartmannGribbs(glm::mat4 inViewProjection,glm::vec4* outPlanes);

        std::vector<glm::vec4> tiledFrustrum(Camera* camera,uint x,uint y);

        void Float32From16(float*    __restrict out, const uint16_t in);
        void Float16From32(uint16_t* __restrict out, const float    in);

        void Float32From16(float*    out, const uint16_t* in, const uint arraySize);
        void Float16From32(uint16_t* out, const float*    in, const uint arraySize);

        glm::vec2 rotate2DPoint(glm::vec2 point, float angle, glm::vec2 origin = glm::vec2(0.0f, 0.0f));

        glm::quat btToGLMQuat(btQuaternion q);
        btQuaternion glmToBTQuat(glm::quat q);

        glm::vec3 btVectorToGLM(btVector3);
        btVector3 btVectorFromGLM(glm::vec3);

        glm::vec3 assimpToGLMVec3(aiVector3D);
        glm::mat4 assimpToGLMMat4(aiMatrix4x4);
        glm::mat3 assimpToGLMMat3(aiMatrix3x3);

        glm::vec3 getScreenCoordinates(glm::vec3 _3Dposition, bool clampToEdge = true);
        glm::vec3 getScreenCoordinates(glm::vec3 _3Dposition,Camera&, bool clampToEdge = true);

        glm::vec3 midpoint(glm::vec3&,glm::vec3&);

        glm::vec3 direction(glm::vec3& eye,glm::vec3& target);


        void lookAtToQuat(glm::quat& o,glm::vec3& eye, glm::vec3& target, glm::vec3& up);

        void translate(btRigidBody&,btVector3&,bool local);

        uchar pack2NibblesIntoChar(float x, float y);
        glm::vec2 unpack2NibblesFromChar(uchar);
        float pack2NibblesIntoCharBasic(float x, float y);
        glm::vec2 unpack2NibblesFromCharBasic(float);

        GLuint pack3NormalsInto32Int(float& x, float& y, float& z);
        GLuint pack3NormalsInto32Int(glm::vec3&);

        float pack3FloatsInto1Float(float,float,float);
        float pack3FloatsInto1Float(glm::vec3&);
        glm::vec3 unpack3FloatsInto1Float(float i);

        float pack3FloatsInto1FloatUnsigned(float,float,float);
        float pack3FloatsInto1FloatUnsigned(glm::vec3&);
        glm::vec3 unpack3FloatsInto1FloatUnsigned(float i);
        
        float pack2FloatsInto1Float(float,float);
        float pack2FloatsInto1Float(glm::vec2&);
        glm::vec2 unpack2FloatsInto1Float(float i);

        float remainder(float,float);

        void removeMatrixPosition(glm::mat4&);

        void recalculateForwardRightUp(glm::quat&,glm::vec3&,glm::vec3&,glm::vec3&);
        void recalculateForwardRightUp(btRigidBody&,glm::vec3&,glm::vec3&,glm::vec3&);

        glm::vec3 getForward(glm::quat& q);
        glm::vec3 getRight(glm::quat& q);
        glm::vec3 getUp(glm::quat& q);
        glm::vec3 getColumnVector(btRigidBody& b, uint column);
        glm::vec3 getForward(btRigidBody& b);
        glm::vec3 getRight(btRigidBody& b);
        glm::vec3 getUp(btRigidBody& b);

        float getAngleBetweenTwoVectors(glm::vec3 a, glm::vec3 b, bool degrees = true);
        void alignTo(glm::quat& o, glm::vec3& direction,float speed=0);

        void setColor(glm::vec3& color,float r,float g,float b);
        void setColor(glm::vec4& color,float r,float g,float b,float a);

        bool isPointWithinCone(const glm::vec3& conePos,const glm::vec3& coneVector,glm::vec3& point,const float fovRadians);
        bool isPointWithinCone(const glm::vec3& conePos,const glm::vec3& coneVector,glm::vec3& point,const float fovRadians,const float fovDistance);

        float toRadians(float degrees);
        float toDegrees(float radians);
        float toRadians(double degrees);
        float toDegrees(double radians);

        float Max(glm::vec2);
        float Max(glm::vec3);
        float Max(glm::vec4);

        float Max(float,float);
        float Max(float,float,float);
        float Max(float,float,float,float);

        uint Max(uint,uint);
        uint Max(uint,uint,uint);
        uint Max(uint,uint,uint,uint);

        float fade(float t);
        double fade(double t);

        float lerp(float t, float a, float b);
        double lerp(double t, double a, double b);

        float grad(int hash, float x, float y, float z);
        double grad(int hash, double x, double y, double z);

        glm::vec4 PaintersAlgorithm(glm::vec4& paintColor, glm::vec4& canvasColor);

        bool rayIntersectSphere(glm::vec3 position, float radius, glm::vec3 A, glm::vec3 rayVector);
    };
};
#endif