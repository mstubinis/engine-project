#include <core/engine/renderer/opengl/glsl/Common.h>
#include <core/engine/shaders/ShaderHelper.h>
#include <core/engine/materials/MaterialEnums.h>

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;


void opengl::glsl::Common::convert(string& code, const unsigned int& versionNumber) {

#pragma region constants
    if (ShaderHelper::sfind(code, "ConstantOneVec3")) {
        if (!ShaderHelper::sfind(code, "const vec3 ConstantOneVec3")) {
            ShaderHelper::insertStringAtLine(code, "const vec3 ConstantOneVec3 = vec3(1.0,1.0,1.0);\n", 1);
        }
    }
    if (ShaderHelper::sfind(code, "ConstantOneVec2")) {
        if (!ShaderHelper::sfind(code, "const vec2 ConstantOneVec2")) {
            ShaderHelper::insertStringAtLine(code, "const vec2 ConstantOneVec2 = vec2(1.0,1.0);\n", 1);
        }
    }
    if (ShaderHelper::sfind(code, "ConstantAlmostOneVec3")) {
        if (!ShaderHelper::sfind(code, "const vec3 ConstantAlmostOneVec3")) {
            ShaderHelper::insertStringAtLine(code, "const vec3 ConstantAlmostOneVec3 = vec3(0.9999,0.9999,0.9999);\n", 1);
        }
    }
    if (ShaderHelper::sfind(code, "ConstantAlmostOneVec2")) {
        if (!ShaderHelper::sfind(code, "const vec2 ConstantAlmostOneVec2")) {
            ShaderHelper::insertStringAtLine(code, "const vec2 ConstantAlmostOneVec2 = vec2(0.9999,0.9999);\n", 1);
        }
    }
    if (ShaderHelper::sfind(code, "ConstantZeroVec3")) {
        if (!ShaderHelper::sfind(code, "const vec3 ConstantZeroVec3")) {
            ShaderHelper::insertStringAtLine(code, "const vec3 ConstantZeroVec3 = vec3(0.0,0.0,0.0);\n", 1);
        }
    }
    if (ShaderHelper::sfind(code, "ConstantZeroVec2")) {
        if (!ShaderHelper::sfind(code, "const vec2 ConstantZeroVec2")) {
            ShaderHelper::insertStringAtLine(code, "const vec2 ConstantZeroVec2 = vec2(0.0,0.0);\n", 1);
        }
    }
    if (ShaderHelper::sfind(code, "KPI")) {
        if (!ShaderHelper::sfind(code, "const float KPI")) {
            ShaderHelper::insertStringAtLine(code, "const float KPI = 3.1415926535898;\n", 1);
        }
    }
#pragma endregion

#pragma region normal map
    if (ShaderHelper::sfind(code, "CalcBumpedNormal(") || ShaderHelper::sfind(code, "CalcBumpedNormalCompressed(")) {
        if (!ShaderHelper::sfind(code, "vec3 CalcBumpedNormal(")) {
            if (ShaderHelper::sfind(code, "varying mat3 TBN;")) {
                boost::replace_all(code, "varying mat3 TBN;", "");
            }
            const string normal_map =
                "varying mat3 TBN;\n"
                "vec3 CalcBumpedNormal(vec2 _uv,sampler2D _inTexture){//generated\n"
                "    vec3 _t = (texture2D(_inTexture, _uv).xyz) * 2.0 - 1.0;\n"
                "    return normalize(TBN * _t);\n"
                "}\n"
                "vec3 CalcBumpedNormalCompressed(vec2 _uv,sampler2D _inTexture){//generated\n"
                "    vec2 _t = (texture2D(_inTexture, _uv).yx) * 2.0 - 1.0;\n" //notice the yx flip, its needed
                "    float _z = sqrt(1.0 - _t.x * _t.x - _t.y * _t.y);\n"
                "    vec3 normal = vec3(_t.xy, _z);\n"//recalc z in the shader
                "    return normalize(TBN * normal);\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, normal_map, 1);
        }
    }
#pragma endregion

#pragma region pack nibble
    if (ShaderHelper::sfind(code, "Pack2NibblesInto8BitChannel(")) {
        if (!ShaderHelper::sfind(code, "float Pack2NibblesInto8BitChannel(")) {
            const string pack_nibble =
                "float Pack2NibblesInto8BitChannel(float x,float y){\n"
                "    float xF = round(x / 0.0666);\n"
                "    float yF = round(y / 0.0666) * 16.0;\n"
                "    return (xF + yF) / 255.0;\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, pack_nibble, 1);
        }
    }
#pragma endregion

#pragma region unpack nibble
    if (ShaderHelper::sfind(code, "Unpack2NibblesFrom8BitChannel(")) {
        if (!ShaderHelper::sfind(code, "vec2 Unpack2NibblesFrom8BitChannel(")) {
            const string unpack_nibble =
                "vec2 Unpack2NibblesFrom8BitChannel(float data){\n"
                "    float d = data * 255.0;\n"
                "    float y = fract(d / 16.0);\n"
                "    float x = (d - (y * 16.0));\n"
                "    return vec2(y, x / 255.0);\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, unpack_nibble, 1);
        }
    }
#pragma endregion

#pragma region painters algorithm
    if (ShaderHelper::sfind(code, "PaintersAlgorithm(")) {
        if (!ShaderHelper::sfind(code, "vec4 PaintersAlgorithm(")) {
            const string painters = "\n"
                "vec4 PaintersAlgorithm(vec4 paint, vec4 canvas){//generated\n"
                "    float alpha = paint.a + canvas.a * (1.0 - paint.a);\n"
                "    vec4 ret = vec4(0.0);\n"
                "    ret = ((paint * paint.a + canvas * canvas.a * (1.0 - paint.a)) / alpha);\n"
                "    ret.a = alpha;\n"
                "    return ret;\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, painters, 1);
        }
    }
#pragma endregion

#pragma region invert color
    if (ShaderHelper::sfind(code, "InvertColor(")) {
        if (!ShaderHelper::sfind(code, "vec4 InvertColor(")) {
            const string invCol = "\n"
                "vec4 InvertColor(vec4 color){//generated\n"
                "    return vec4(vec4(1.0) - color);\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, invCol, 1);
        }
    }
#pragma endregion

#pragma region invert color 255
    if (ShaderHelper::sfind(code, "InvertColor255(")) {
        if (!ShaderHelper::sfind(code, "vec4 InvertColor255(")) {
            const string invCol255 = "\n"
                "vec4 InvertColor255(vec4 color){//generated\n"
                "    return vec4(vec4(255.0) - color);\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, invCol255, 1);
        }
    }
#pragma endregion

#pragma region range to 255
    if (ShaderHelper::sfind(code, "RangeTo255(")) {
        if (!ShaderHelper::sfind(code, "vec4 RangeTo255(")) {
            const string range255 = "\n"
                "vec4 RangeTo255(vec4 color){//generated\n"
                "    return color * 255.0;\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, range255, 1);
        }
    }
#pragma endregion

#pragma region range to 1
    if (ShaderHelper::sfind(code, "RangeTo1(")) {
        if (!ShaderHelper::sfind(code, "vec4 RangeTo1(")) {
            const string range1 = "\n"
                "vec4 RangeTo1(vec4 color){//generated\n"
                "    return color / 255.0;\n"
                "}\n";
            ShaderHelper::insertStringAtLine(code, range1, 1);
        }
    }
#pragma endregion

#pragma region UBO
    if (ShaderHelper::sfind(code, "CameraView") || ShaderHelper::sfind(code, "CameraProj") || ShaderHelper::sfind(code, "CameraViewProj") ||
    ShaderHelper::sfind(code, "CameraPosition") || ShaderHelper::sfind(code, "CameraInvView") || ShaderHelper::sfind(code, "CameraInvProj") ||
    ShaderHelper::sfind(code, "CameraInvViewProj") || ShaderHelper::sfind(code, "CameraNear") || ShaderHelper::sfind(code, "CameraFar") ||
    ShaderHelper::sfind(code, "CameraInfo1") || ShaderHelper::sfind(code, "CameraInfo2") || ShaderHelper::sfind(code, "CameraViewVector") ||
    ShaderHelper::sfind(code, "CameraRealPosition") || ShaderHelper::sfind(code, "CameraInfo3")) {
        string uboCameraString;
        if (versionNumber >= 140) { //UBO only supported at 140 or above
            if (!ShaderHelper::sfind(code, "layout (std140) uniform Camera //generated")) {
                uboCameraString = "\n"
                    "layout (std140) uniform Camera //generated\n"
                    "{\n"
                    "    mat4 CameraView;\n"
                    "    mat4 CameraProj;\n"
                    "    mat4 CameraViewProj;\n"
                    "    mat4 CameraInvView;\n"
                    "    mat4 CameraInvProj;\n"
                    "    mat4 CameraInvViewProj;\n"
                    "    vec4 CameraInfo1;\n"
                    "    vec4 CameraInfo2;\n"
                    "    vec4 CameraInfo3;\n"
                    "};\n"
                    "vec3 CameraPosition = CameraInfo1.xyz;\n"
                    "vec3 CameraViewVector = CameraInfo2.xyz;\n"
                    "vec3 CameraRealPosition = CameraInfo3.xyz;\n"
                    "float CameraNear = CameraInfo1.w;\n"
                    "float CameraFar = CameraInfo2.w;\n"
                    "\n";
                ShaderHelper::insertStringAtLine(code, uboCameraString, 1);
            }
        }else{ //no UBO's, just add a bunch of uniforms
            if (!ShaderHelper::sfind(code, "uniform mat4 CameraView;//generated")) {
                uboCameraString = "\n"
                    "uniform mat4 CameraView;//generated;\n"
                    "uniform mat4 CameraProj;\n"
                    "uniform mat4 CameraViewProj;\n"
                    "uniform mat4 CameraInvView;\n"
                    "uniform mat4 CameraInvProj;\n"
                    "uniform mat4 CameraInvViewProj;\n"
                    "uniform vec4 CameraInfo1;\n"
                    "uniform vec4 CameraInfo2;\n"
                    "uniform vec4 CameraInfo3;\n"
                    "vec3 CameraPosition = CameraInfo1.xyz;\n"
                    "vec3 CameraViewVector = CameraInfo2.xyz;\n"
                    "vec3 CameraRealPosition = CameraInfo3.xyz;\n"
                    "float CameraNear = CameraInfo1.w;\n"
                    "float CameraFar = CameraInfo2.w;\n"
                    "\n";
                ShaderHelper::insertStringAtLine(code, uboCameraString, 1);
            }
        }
    }
#pragma endregion

#pragma region material constants
    if (ShaderHelper::sfind(code, "USE_MAX_MATERIAL_LAYERS_PER_COMPONENT") && !ShaderHelper::sfind(code, "//USE_MAX_MATERIAL_LAYERS_PER_COMPONENT")) {
        boost::replace_all(code, "USE_MAX_MATERIAL_LAYERS_PER_COMPONENT", "#define MAX_MATERIAL_LAYERS_PER_COMPONENT " + to_string(MAX_MATERIAL_LAYERS_PER_COMPONENT) + "\n");
    }
    if (ShaderHelper::sfind(code, "USE_MAX_MATERIAL_COMPONENTS") && !ShaderHelper::sfind(code, "//USE_MAX_MATERIAL_COMPONENTS")) {
        boost::replace_all(code, "USE_MAX_MATERIAL_COMPONENTS", "#define MAX_MATERIAL_COMPONENTS " + to_string(MAX_MATERIAL_COMPONENTS) + "\n");
    }
#pragma endregion
}