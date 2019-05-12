#include <core/engine/Engine_Math.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/Camera.h>
#include <core/Scene.h>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>

#include <bullet/btBulletDynamicsCommon.h>

using namespace Engine;
using namespace std;

typedef unsigned char uchar;


void Math::Float32From16(float*    __restrict out, const uint16_t in) {
    uint32_t t1, t2, t3;
    t1 = in & 0x7fff;                       // Non-sign bits
    t2 = in & 0x8000;                       // Sign bit
    t3 = in & 0x7c00;                       // Exponent
    t1 <<= 13;                              // Align mantissa on MSB
    t2 <<= 16;                              // Shift sign bit into position
    t1 += 0x38000000;                       // Adjust bias
    t1 = (t3 == 0 ? 0 : t1);                // Denormals-as-zero
    t1 |= t2;                               // Re-insert sign bit
    *((uint32_t*)out) = t1;
}
void Math::Float16From32(uint16_t* __restrict out, const float    in) {
    uint32_t inu = *((uint32_t*)&in);
    uint32_t t1, t2, t3;
    t1 = inu & 0x7fffffff;                 // Non-sign bits
    t2 = inu & 0x80000000;                 // Sign bit
    t3 = inu & 0x7f800000;                 // Exponent
    t1 >>= 13;                             // Align mantissa on MSB
    t2 >>= 16;                             // Shift sign bit into position
    t1 -= 0x1c000;                         // Adjust bias
    t1 = (t3 < 0x38800000) ? 0 : t1;
    t1 = (t3 > 0x47000000) ? 0x7bff : t1;
    t1 = (t3 == 0 ? 0 : t1);               // Denormals-as-zero
    t1 |= t2;                              // Re-insert sign bit
    *((uint16_t*)out) = t1;
}

void Math::Float32From16(float*    out, const uint16_t* in, const uint arraySize) {
    for (unsigned i = 0; i < arraySize; ++i) {
        Math::Float32From16(&(out[i]), in[i]);
    }
}
void Math::Float16From32(uint16_t* out, const float*    in, const uint arraySize) {
    for (uint i = 0; i < arraySize; ++i) {
        Math::Float16From32(&(out[i]), in[i]);
    }
}



glm::vec2 Math::rotate2DPoint(glm::vec2 point, float angle, glm::vec2 origin) {
    float s = glm::sin(angle);
    float c = glm::cos(angle);
    glm::v2 ret;
    ret.x = c * (point.x - origin.x) - s * (point.y - origin.y) + origin.x;
    ret.y = s * (point.x - origin.x) + c * (point.y - origin.y) + origin.x;
    return ret;
}



void Math::extractViewFrustumPlanesHartmannGribbs(glm::mat4 inViewProjection,glm::vec4* outPlanes){
    glm::vec4 rows[4];
    for(ushort i = 0; i < 4; ++i)
        rows[i] = glm::row(inViewProjection,i);
    //0 = left,1 = right,2 = top,3 = bottom,4 = near,5 = far
    for(ushort i = 0; i < 3; ++i){
        ushort index = i * 2;
        outPlanes[index  ] = -(rows[3] + rows[i]);  //0,2,4
        outPlanes[index+1] = -(rows[3] - rows[i]);  //1,3,5
    }
    //https://www.gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf
    //avoiding normalization of the planes for now so we can uitilize halfspace and thus negatives (outside frust / inside frust / intersecting frust)
    /*
    for(ushort i = 0; i < 6; ++i){
        float mag = glm::sqrt(outPlanes[i].x * outPlanes[i].x + outPlanes[i].y * outPlanes[i].y + outPlanes[i].z * outPlanes[i].z);
        outPlanes[i].x = outPlanes[i].x / mag;
        outPlanes[i].y = outPlanes[i].y / mag;
        outPlanes[i].z = outPlanes[i].z / mag;
        outPlanes[i].w = outPlanes[i].w / mag;
    }
    */
}
glm::quat Math::btToGLMQuat(btQuaternion q){ glm::quat glmQuat = glm::quat(q.getW(),q.getX(),q.getY(),q.getZ()); return glmQuat; }
btQuaternion Math::glmToBTQuat(glm::quat q){ btQuaternion btQuat = btQuaternion(q.x,q.y,q.z,q.w); return btQuat; }
glm::vec3 Math::assimpToGLMVec3(aiVector3D n){ glm::vec3 ret = glm::vec3(n.x,n.y,n.z); return ret; }
glm::mat4 Math::assimpToGLMMat4(aiMatrix4x4 n){ glm::mat4 ret = glm::mat4(n.a1,n.b1,n.c1,n.d1,n.a2,n.b2,n.c2,n.d2,n.a3,n.b3,n.c3,n.d3,n.a4,n.b4,n.c4,n.d4); return ret; }
glm::mat3 Math::assimpToGLMMat3(aiMatrix3x3 n){ glm::mat3 ret = glm::mat3(n.a1,n.b1,n.c1,n.a2,n.b2,n.c2,n.a3,n.b3,n.c3); return ret; }

float Math::toRadians(float degrees){ return degrees * 0.0174533f; }
float Math::toDegrees(float radians){ return radians * 57.2958f; }
float Math::toRadians(double degrees){ return Math::toRadians(float(degrees)); }
float Math::toDegrees(double radians){ return Math::toDegrees(float(radians)); }
float Math::remainder(float x,float y){ return x - (glm::round(x/y)*y); }

glm::vec3 Math::btVectorToGLM(btVector3 bt){ return glm::vec3(bt.x(),bt.y(),bt.z()); }
btVector3 Math::btVectorFromGLM(glm::vec3 v){ return btVector3(v.x,v.y,v.z); }

void Math::removeMatrixPosition(glm::mat4& m){
    m[3][0] = 0; m[3][1] = 0; m[3][2] = 0;
}

bool Math::isPointWithinCone(const glm::vec3& conePos,const glm::vec3& coneVector,glm::vec3& point,const float fovRadians){
    point.x += 0.0000001f;// forced protection against NaN if vectors happen to be equal
    glm::vec3 diff = glm::normalize(point - conePos);
    float t = glm::dot(coneVector,diff);
    return ( t >= glm::cos( fovRadians ) );
}
bool Math::isPointWithinCone(const glm::vec3& conePos,const glm::vec3& coneVector,glm::vec3& point,const float fovRadians,const float maxDistance){
    point.x += 0.0000001f;// forced protection against NaN if vectors happen to be equal
    glm::vec3 diff = glm::normalize(point - conePos);
    float t = glm::dot(coneVector,diff);
    float length = glm::length(point-conePos);
    if ( length > maxDistance ){ return false; }
    return ( t >= glm::cos( fovRadians ) );
}

vector<glm::vec4> Math::tiledFrustrum(Camera* camera,uint x,uint y){
    /*
    Tiled deferred rendering does not strictly require a compute shader.
    What it requires is that, for each tile, you have a series of lights which it will process.
    A compute shader is merely one way to accomplish that.

    An alternative is to build the light lists for each frustum on the CPU, then upload that
    data to the GPU for its eventual use. Obviously it requires much more memory work than the CS version.
    But it's probably not that expensive, and it allows you to easily play with tile sizes to find the most optimal.
    More tiles means more CPU work and more data to be uploaded, but fewer lights-per-tile (generally speaking) and more efficient processing.

    One way to do that for GL 3.3-class hardware is to make each tile a separate quad.
    The quad will be given, as part of its per-vertex parameters, the starting index for its part
    of the total light list and the total number of lights for that tile to process.
    The idea being that there is a globally-accessible array, and each tile has a contiguous region of this array that it will process.

    This array could be the actual lights themselves, or it could be indices into a
    second (much smaller) array of lights. You'll have to measure the difference to tell if
    it's worthwhile to have the additional indirection in the access.

    The primary array should probably be a buffer texture, since it can get quite large,
    depending on the number of lights and tiles. If you go with the indirect route, then
    the array of actual light data will likely fit into a uniform block. But in either case,
    you're going to need to employ buffer streaming techniques when uploading to it.
    */

    uint MAX_WORK_GROUP_SIZE = 16;
    glm::mat4 proj = camera->getProjection();
    //glm::mat4 proj = camera->getProjection() * camera->getView();
    glm::uvec2 winSize = Resources::getWindowSize();
    glm::vec4 frustumPlanes[6];
    glm::vec2 tileScale = glm::vec2(winSize.x,winSize.y) * (1.0f / float(2 * MAX_WORK_GROUP_SIZE));
    glm::vec2 tileBias = tileScale - glm::vec2(x,y);
    glm::vec4 col1 = glm::vec4(-proj[0][0]  * tileScale.x, proj[0][1], tileBias.x, proj[0][3]); 
    glm::vec4 col2 = glm::vec4(proj[1][0], -proj[1][1] * tileScale.y, tileBias.y, proj[1][3]);
    glm::vec4 col4 = glm::vec4(proj[3][0], proj[3][1],  -1.0f, proj[3][3]); 
    frustumPlanes[0] = col4 + col1; //Left plane
    frustumPlanes[1] = col4 - col1; //right plane
    frustumPlanes[2] = col4 - col2; //top plane
    frustumPlanes[3] = col4 + col2; //bottom plane
    frustumPlanes[4] = glm::vec4(0.0f, 0.0f, -1.0f,-camera->getNear()); //near
    frustumPlanes[5] = glm::vec4(0.0f, 0.0f, -1.0f,camera->getFar());  //far

    vector<glm::vec4> v;
    for(ushort i = 0; i < 4; ++i){
        frustumPlanes[i] *= 1.0f / glm::length(frustumPlanes[i]);
        v.push_back(frustumPlanes[i]);
    }
    return v;

    /* culling code

    uint threadCount = MAX_WORK_GROUP_SIZE * MAX_WORK_GROUP_SIZE;
    uint passCount = (NUM_OF_LIGHTS + threadCount - 1) / threadCount;
    for (uint passIt = 0; passIt < passCount; ++passIt){
        uint lightIndex =  passIt * threadCount + gl_LocalInvocationIndex;
        lightIndex = min(lightIndex, NUM_OF_LIGHTS);
        PointLight p = pointLights[lightIndex];
        vec4 pos = viewProjectionMatrix * vec4(p.posX,p.posY,p.posZ, 1.0f);
        float rad = p.radius/pos.w;
        if (pointLightCount < MAX_LIGHTS_PER_TILE){
            bool inFrustum = true;
            for (uint i = 3; i >= 0 && inFrustum; i--){
                float dist = dot(frustumPlanes[i], pos);
                inFrustum = (-rad <= dist);
            }
            if (inFrustum){
                uint id = atomicAdd(pointLightCount, 1);
                pointLightIndex[id] = lightIndex;
            }
        }
    }

    */
}

glm::vec3 Math::getScreenCoordinates(glm::vec3 objPos,Camera& camera, bool clampToEdge) {
    auto winSizeInt = Resources::getWindowSize();
    glm::vec2 winSize = glm::vec2(winSizeInt.x, winSizeInt.y);
    glm::vec4 viewport = glm::vec4(0, 0, winSize.x, winSize.y);
    glm::vec3 screen = glm::project(objPos, camera.getView(), camera.getProjection(), viewport);
    //check if point is behind
    float dot = glm::dot(camera.getViewVector(), objPos - camera.getPosition());
    float resX = screen.x;
    float resY = screen.y;
    uint inBounds = 1;
    if (clampToEdge) {
        if (screen.x < 0.0f) { 
            resX = 0.0f; 
            inBounds = 0; 
        }else if (screen.x > winSize.x) { 
            resX = winSize.x; 
            inBounds = 0; 
        }
        if (resY < 0.0f) { 
            resY = 0.0f; 
            inBounds = 0; 
        }else if (resY > winSize.y) { 
            resY = winSize.y; 
            inBounds = 0; 
        }
    }
    if (dot < 0.0f) {
        return glm::vec3(resX, resY, inBounds);
    }
    inBounds = 0;
    resX = winSize.x - screen.x;
    resY = winSize.y - screen.y;
    if (clampToEdge) {
        if (resX < winSize.x / 2) {
            resX = 0.0f;
        }else if (resX > winSize.x / 2) {
            resX = winSize.x;
        }
        if (resY < winSize.y / 2) {
            resY = 0.0f;
        }else if (resY > winSize.y / 2) {
            resY = winSize.y;
        }
    }
    return glm::vec3(resX, resY, inBounds);
}
glm::vec3 Math::getScreenCoordinates(glm::vec3 objPos,bool clampToEdge){
    return Math::getScreenCoordinates(objPos,*Resources::getCurrentScene()->getActiveCamera(),clampToEdge);
}
float Math::Max(glm::vec2 v){ return glm::max(v.x,v.y); }
float Math::Max(glm::vec3 v){ return glm::max(v.x,glm::max(v.y,v.z)); }
float Math::Max(glm::vec4 v){ return glm::max(v.x,glm::max(v.y,glm::max(v.z,v.w))); }
float Math::Max(float x, float y){ return glm::max(x,y); }
float Math::Max(float x, float y, float z){ return glm::max(x,glm::max(y,z)); }
float Math::Max(float x, float y, float z, float w){ return glm::max(x,glm::max(y,glm::max(z,w))); }
uint Math::Max(uint x,uint y){ return glm::max(x,y); }
uint Math::Max(uint x,uint y,uint z){ return glm::max(x,glm::max(y,z)); }
uint Math::Max(uint x,uint y,uint z,uint w){ return glm::max(x,glm::max(y,glm::max(z,w))); }

GLuint Math::pack3NormalsInto32Int(float& x, float& y, float& z){
   int _X = (int)(x * 511.0f);
   int _Y = (int)(y * 511.0f);
   int _Z = (int)(z * 511.0f);
   return (_X & 0x3FF) | ((_Y & 0x3FF) << 10) | ((_Z & 0x3FF) << 20);
}
GLuint Math::pack3NormalsInto32Int(glm::vec3& v){ return Math::pack3NormalsInto32Int(v.x,v.y,v.z); }


float Math::pack3FloatsInto1Float(float r,float g,float b){
    //Scale and bias
    r = (r + 1.0f) * 0.5f; uchar _r = (uchar)(r * 255.0f);
    g = (g + 1.0f) * 0.5f; uchar _g = (uchar)(g * 255.0f);
    b = (b + 1.0f) * 0.5f; uchar _b = (uchar)(b * 255.0f);
    uint packedColor = (_r << 16) | (_g << 8) | _b;
    float packedFloat = (float) ( ((double)packedColor) / ((double) (1 << 24)) );
    return packedFloat;
}
float Math::pack3FloatsInto1Float(glm::vec3& c){ return Math::pack3FloatsInto1Float(c.r,c.g,c.b); }
glm::vec3 Math::unpack3FloatsInto1Float(float v){
    glm::vec3 ret = glm::vec3((float)fmod(v, 1.0f), (float)fmod(v * 256.0f, 1.0f), (float)fmod(v * 65536.0f, 1.0f));
    //Unpack to the -1..1 range
    ret.r = (ret.r * 2.0f) - 1.0f;
    ret.g = (ret.g * 2.0f) - 1.0f;
    ret.b = (ret.b * 2.0f) - 1.0f;
    return ret;
}
float Math::pack3FloatsInto1FloatUnsigned(float r,float g,float b){
    uchar _r = (uchar)(r * 255.0f);
    uchar _g = (uchar)(g * 255.0f);
    uchar _b = (uchar)(b * 255.0f);
    uint packedColor = (_r << 16) | (_g << 8) | _b;
    float packedFloat = (float) ( ((double)packedColor) / ((double) (1 << 24)) );
    return packedFloat;
}
float Math::pack3FloatsInto1FloatUnsigned(glm::vec3& c){ return Math::pack3FloatsInto1Float(c.r,c.g,c.b); }
glm::vec3 Math::unpack3FloatsInto1FloatUnsigned(float v){
    glm::vec3 ret = glm::vec3((float)fmod(v, 1.0f), (float)fmod(v * 256.0f, 1.0f), (float)fmod(v * 65536.0f, 1.0f));
    return ret;
}
uchar Math::pack2NibblesIntoChar(float x, float y) {
    uchar _packedData = 0x00;
    int bits = (int)round(x / 0.0666f);
    int bits1 = (int)round(y / 0.0666f);
    _packedData |= (bits & 0x0F);
    _packedData |= ((bits1 << 4) & 0xF0);
    return _packedData;
}
glm::vec2 Math::unpack2NibblesFromChar(uchar _packedData) {
    int low = _packedData & 0x0F; /* binary 00000001 */
    int high = (_packedData >> 4); /* binary 10000000 */
    return glm::vec2((float)low * 0.0666f, (float)high * 0.0666f);
}
//attempt to do the above using non bitwise operations for glsl versions that do not support bitwise operations
float Math::pack2NibblesIntoCharBasic(float x, float y) {
    float lowEnd = (float)round(x / 0.0666f);
    float highEnd = (float)(round(y / 0.0666f) * 16.0f);
    return float(lowEnd + highEnd);
}
glm::vec2 Math::unpack2NibblesFromCharBasic(float _packedData) {
    float highEnd = (_packedData / 16.0f);
    highEnd = highEnd - glm::floor(highEnd);
    float lowEnd = (float)(_packedData - (highEnd * 16.0f));
    return glm::vec2(highEnd, (float)(lowEnd / 255.0));
}
float Math::pack2FloatsInto1Float(float x,float y){
    x = (x + 1.0f) * 0.5f;
    y = (y + 1.0f) * 0.5f;
    return glm::floor(x * 1000.0f) + y; 
}
float Math::pack2FloatsInto1Float(glm::vec2& v){ return Math::pack2FloatsInto1Float(v.x,v.y); }
glm::vec2 Math::unpack2FloatsInto1Float(float i){
    glm::vec2 res;
    res.y = i - glm::floor(i); //glm::fract(i)
    res.x = (i - res.y) / 1000.0f;
    res.x = (res.x - 0.5f) * 2.0f;
    res.y = (res.y - 0.5f) * 2.0f;
    return res;
}
void Math::translate(btRigidBody& body,btVector3& vec,bool local){
    if(local){
        btQuaternion q = body.getWorldTransform().getRotation().normalize();
        vec = vec.rotate(q.getAxis(),q.getAngle());
    }
}
void Math::lookAtToQuat(glm::quat& o,glm::vec3& eye, glm::vec3& target, glm::vec3& up){
    glm::vec3 forward = eye - target;
    glm::vec3 vector = glm::normalize(forward);
    glm::vec3 vector2 = glm::normalize(glm::cross(vector,up));
    glm::vec3 vector3 = glm::cross(vector,vector2);
    float m00 = vector2.x;
    float m01 = vector2.y;
    float m02 = vector2.z;
    float m10 = vector3.x;
    float m11 = vector3.y;
    float m12 = vector3.z;
    float m20 = vector.x;
    float m21 = vector.y;
    float m22 = vector.z;
    float num8 = (m00 + m11) + m22;
    if (num8 > 0.0f){
        float num = glm::sqrt(num8 + 1.0f);
        o.w = num * 0.5f;
        num = 0.5f / num;
        o.x = (m12 - m21) * num;
        o.y = (m20 - m02) * num;
        o.z = (m01 - m10) * num;
        return;
    }
    if ((m00 >= m11) && (m00 >= m22)){
        float num7 = glm::sqrt(((1.0f + m00) - m11) - m22);
        float num4 = 0.5f / num7;
        o.x = 0.5f * num7;
        o.y = (m01 + m10) * num4;
        o.z = (m02 + m20) * num4;
        o.w = (m12 - m21) * num4;
        return;
    }
    if (m11 > m22){
        float num6 = glm::sqrt(((1.0f + m11) - m00) - m22);
        float num3 = 0.5f / num6;
        o.x = (m10 + m01) * num3;
        o.y = 0.5f * num6;
        o.z = (m21 + m12) * num3;
        o.w = (m20 - m02) * num3;
        return;
    }
    float num5 = glm::sqrt(((1.0f + m22) - m00) - m11);
    float num2 = 0.5f / num5;
    o.x = (m20 + m02) * num2;
    o.y = (m21 + m12) * num2;
    o.z = 0.5f * num5;
    o.w = (m01 - m10) * num2;
}
glm::vec3 Math::midpoint(glm::vec3& a, glm::vec3& b){ return glm::vec3((a.x+b.x)/2.0f,(a.y+b.y)/2.0f,(a.z+b.z)/2.0f); }
glm::vec3 Math::direction(glm::vec3& eye,glm::vec3& target){ return glm::normalize(eye-target); }
glm::vec3 Math::getForward(glm::quat& q){return glm::normalize(q * glm::vec3(0.0f,0.0f,-1.0f));}
glm::vec3 Math::getRight(glm::quat& q){return glm::normalize(q * glm::vec3(1.0f,0.0f,0.0f));}
glm::vec3 Math::getUp(glm::quat& q){return glm::normalize(q * glm::vec3(0.0f,1.0f,0.0f));}
glm::vec3 Math::getColumnVector(btRigidBody& b, uint column){
    btTransform t;
    b.getMotionState()->getWorldTransform(t);
    btVector3 v = t.getBasis().getColumn(column);
    return glm::normalize(glm::vec3(v.x(),v.y(),v.z()));
}
glm::vec3 Math::getForward(btRigidBody& b){ return Math::getColumnVector(b,2); }
glm::vec3 Math::getRight(btRigidBody& b){ return Math::getColumnVector(b,0); }
glm::vec3 Math::getUp(btRigidBody& b){ return Math::getColumnVector(b,1); }
void Math::recalculateForwardRightUp(glm::quat& o,glm::vec3& f,glm::vec3& r,glm::vec3& u){
    f = Math::getForward(o); r = Math::getRight(o); u = Math::getUp(o);
}
void Math::recalculateForwardRightUp(btRigidBody& b,glm::vec3& f,glm::vec3& r,glm::vec3& u){
    f = Math::getForward(b); r = Math::getRight(b); u = Math::getUp(b);
}
float Math::getAngleBetweenTwoVectors(glm::vec3 a, glm::vec3 b, bool degrees){
    // forced protection against NaN if a and b happen to be equal
    a.x += 0.0000001f;
    float angle = glm::acos( glm::dot(glm::normalize(a),glm::normalize(b)) );
    if(degrees) angle *= 57.2958f;
    return angle;
}
void Math::alignTo(glm::quat& o,glm::vec3& direction,float speed){
    glm::quat original(o);
    direction = glm::normalize(direction);
    glm::vec3 xaxis = glm::normalize(glm::cross(glm::vec3(0.0f,1.0f,0.0f), direction));
    glm::vec3 yaxis = glm::normalize(glm::cross(direction, xaxis));
    glm::mat3 rot;
    rot[0][0] = float(xaxis.x);     rot[0][1] = float(xaxis.y);     rot[0][2] = float(xaxis.z);
    rot[1][0] = float(yaxis.x);     rot[1][1] = float(yaxis.y);     rot[1][2] = float(yaxis.z);
    rot[2][0] = float(direction.x); rot[2][1] = float(direction.y); rot[2][2] = float(direction.z);
    o = glm::quat_cast(rot);
    if(speed != 0.0f){
        float angle = Math::getAngleBetweenTwoVectors(direction,glm::vec3(getForward(original)),true); // degrees
        speed *= 1.0f/angle;
        speed *= (float)Resources::dt();
        o = glm::mix(original,o,speed*5.0f);
    }
    o = glm::normalize(o);
}
void Math::setColor(glm::vec3& c,float r, float g, float b){
    if(r > 1.0f) r = r / 255.0f;
    if(g > 1.0f) g = g / 255.0f;
    if(b > 1.0f) b = b / 255.0f;
    c.x = r; c.y = g; c.z = b;
}
void Math::setColor(glm::vec4& c,float r, float g, float b,float a){
    if(r > 1.0f) r = r / 255.0f;
    if(g > 1.0f) g = g / 255.0f;
    if(b > 1.0f) b = b / 255.0f;
    if(a > 1.0f) a = a / 255.0f;
    c.x = r; c.y = g; c.z = b; c.w = a; 
}
float Math::fade(float t){ return t*t*t*(t*(t*6.0f-15.0f)+10.0f); }
double Math::fade(double t){ return t*t*t*(t*(t*6.0-15.0)+10.0); }
float Math::lerp(float t, float a, float b){return a + t * (b - a);}
double Math::lerp(double t, double a, double b){return a + t * (b - a);}
float Math::grad(int hash, float x, float y, float z){
    int h = hash & 15;
    double u = h<8 ? x : y,v = h<4 ? y : h==12||h==14 ? x : z;
    return float(((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v));
}
double Math::grad(int hash, double x, double y, double z){
    int h = hash & 15;
    double u = h<8 ? x : y,v = h<4 ? y : h==12||h==14 ? x : z;
    return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}
glm::vec4 Math::PaintersAlgorithm(glm::vec4& p, glm::vec4& c){
    glm::vec4 ret(0.0f);
    float a = p.a + c.a * (1.0f-p.a);
    ret = ((p*p.a + c*c.a * (1.0f-p.a)) / a);
    ret.a = a;
    return ret;
}
bool Math::rayIntersectSphere(glm::vec3 C, float r,glm::vec3 A, glm::vec3 rayVector){
    glm::vec3 B = A + rayVector;
    float dot = glm::dot(rayVector,C - A); //check if point is behind
    if(dot >= 0.0f)
        return false;
    float a = ((B.x-A.x)*(B.x-A.x))  +  ((B.y - A.y)*(B.y - A.y))  +  ((B.z - A.z)*(B.z - A.z));
    float b = 2.0f * ((B.x - A.x)*(A.x - C.x)  +  (B.y - A.y)*(A.y - C.y)  +  (B.z - A.z)*(A.z-C.z));
    float c = (((A.x-C.x)*(A.x-C.x))  +  ((A.y - C.y)*(A.y - C.y))  +  ((A.z - C.z)*(A.z - C.z))) - (r*r);
    float d = (b*b) - (4.0f*a*c);
    if(d < 0.0f)
        return false;
    return true;
}