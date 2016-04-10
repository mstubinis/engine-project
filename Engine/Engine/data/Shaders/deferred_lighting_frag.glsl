#version 120

uniform int LightType;

uniform int HasSSAO;

uniform vec3 LightColor;
uniform float LightAmbientIntensity;
uniform float LightDiffuseIntensity;
uniform float LightSpecularPower;

uniform float LightConstant;
uniform float LightLinear;
uniform float LightExp;

uniform vec3 LightDirection;
uniform vec3 LightPosition;

uniform sampler2D gNormalMap;
uniform sampler2D gPositionMap;
uniform sampler2D gMiscMap;
uniform sampler2D gDiffuseMap;

uniform vec3 gCameraPosition;
uniform vec2 gScreenSize;

vec4 CalcLightInternal(vec3 LightDir,vec3 PxlWorldPos,vec3 PxlNormal,vec2 uv){
    vec4 AmbientColor = vec4(LightColor, 1.0) * LightAmbientIntensity;
    float Lambertian = max(dot(LightDir,PxlNormal), 0.0);
	float Glow = texture2D(gMiscMap,uv).r;

    vec4 diffuseMapColor = vec4(texture2D(gDiffuseMap,uv).rgb, 1.0);

    vec4 DiffuseColor  = vec4(0.0);
    vec4 SpecularColor = vec4(0.0);

    if (Lambertian > 0.0) {
        DiffuseColor = vec4(LightColor, 1.0) * LightDiffuseIntensity * Lambertian;
        vec3 ViewVector = normalize(-PxlWorldPos + gCameraPosition);

        // this is blinn phong
        vec3 halfDir = normalize(LightDir + ViewVector);
        float SpecularAngle = max(dot(halfDir, PxlNormal), 0.0);
        SpecularAngle = pow(SpecularAngle, LightSpecularPower);

        if (SpecularAngle > 0.0 && LightSpecularPower > 0.001f) {
            float materialSpecularity = texture2D(gMiscMap,uv).b;
            SpecularColor = vec4(LightColor, 1.0) * materialSpecularity * SpecularAngle;
        }
    }
    if(PxlNormal.r > 0.9999 && PxlNormal.g > 0.9999 && PxlNormal.b > 0.9999){
        return vec4(0);
    }
    float ssao = 1.0;
    if(HasSSAO == 1){
        ssao = texture2D(gMiscMap,uv).g;
    }
	vec4 lightWithoutSpecular = (((AmbientColor + DiffuseColor) * diffuseMapColor ) * vec4(ssao));
	if(Glow > 0.99){
		return diffuseMapColor;
	}
    return clamp(max(Glow*diffuseMapColor,lightWithoutSpecular + SpecularColor),0.01,0.95);
}
vec4 CalcPointLight(vec3 PxlWorldPos, vec3 PxlNormal, vec2 uv){
    vec3 LightDir = LightPosition - PxlWorldPos;
    float Distance = length(LightDir);
    LightDir = normalize(LightDir);

    vec4 c = CalcLightInternal(LightDir, PxlWorldPos, PxlNormal, uv);

    float a =  max(1.0 , LightConstant + (LightLinear * Distance) + (LightExp * Distance * Distance));
    return c / a;
}
vec4 CalcSpotLight(vec3 PxlWorldPos, vec3 PxlNormal, vec2 uv){
    return vec4(0);
}
void main(){
    vec2 uv = gl_FragCoord.xy / gScreenSize;
    vec3 PxlPosition = texture2D(gPositionMap,uv).xyz;
    vec3 PxlNormal = (texture2D(gNormalMap, uv).rgb);

    vec4 lightCalculation = vec4(0);

    if(LightType == 0)
        lightCalculation = CalcLightInternal(normalize(LightPosition - PxlPosition),PxlPosition,PxlNormal,uv);
    else if(LightType == 1)
        lightCalculation = CalcPointLight(PxlPosition,PxlNormal,uv);
    else if(LightType == 2)
        lightCalculation = CalcLightInternal(LightDirection,PxlPosition,PxlNormal,uv);
    else if(LightType == 3)
        lightCalculation = CalcSpotLight(PxlPosition,PxlNormal,uv);

    gl_FragColor = lightCalculation;
}