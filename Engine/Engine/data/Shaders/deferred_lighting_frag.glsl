#version 120
#define MATERIAL_COUNT_LIMIT 255


uniform vec4 LightDataA; //x = ambient, y = diffuse, z = specular, w = LightDirection.x
uniform vec4 LightDataB; //x = LightDirection.y, y = LightDirection.z, z = const, w = linear
uniform vec4 LightDataC; //x = exp, y = LightPosition.x, z = LightPosition.y, w = LightPosition.z
uniform vec4 LightDataD; //x = LightColor.r, y = LightColor.g, z = LightColor.b, w = LightType


uniform sampler2D gNormalMap;
uniform sampler2D gMiscMap;
uniform sampler2D gDepthMap;

uniform vec4 ScreenData; //x = near, y = far, z = winSize.x, w = winSize.y
uniform vec4 CamPosGamma; //x = camX, y = camY, z = camZ, w = monitorGamma
uniform vec4 materials[MATERIAL_COUNT_LIMIT]; //r = frensel, g = specPower, b = lightingMode, a = shadeless 

uniform mat4 VP;
uniform mat4 invVP;

float linearize_depth(float depth){
    float a = ScreenData.y / (ScreenData.y - ScreenData.x);
    float b = ScreenData.y * ScreenData.x / (ScreenData.x - ScreenData.y);
    return (a + b / depth);
}
float invertLogDepth(float log_depth){
    return linearize_depth(pow(ScreenData.y + 1.0, log_depth) - 1.0);
}
vec3 reconstruct_world_pos(vec2 _uv){
    float depth = texture2D(gDepthMap, _uv).r;
    vec4 wpos = invVP * (vec4(_uv, invertLogDepth(depth), 1.0) * 2.0 - 1.0);
    return wpos.xyz / wpos.w;
}
float BeckmannDist(float theta, float _alpha, float pi){
    float cos2a = theta * theta;
    float b = (1.0 - cos2a) / (cos2a * _alpha);
    return exp(-b) / (pi * _alpha * (cos2a * cos2a));
}
float SchlickFrensel(float f0, float theta){
    return f0 + (1.0-f0) * pow( 1.0 - theta, 5.0);
}
vec3 CookTorr(float frensel,float vdoth, float vdotn, float ldotn, float ndoth,float _alpha, float pi){
    float Beck = BeckmannDist(ndoth,_alpha,pi);

    float a = (2.0 * (ndoth) * (vdotn)) / (vdoth);
    float b = (2.0 * (ndoth) * (ldotn)) / (vdoth);
    float G = min(1.0,min(a,b));

    float fin = (Beck * frensel * G) / (max((4.0 * vdotn * ldotn),0.0) + 0.0001);

    return vec3(fin);
}
vec3 CalcLightInternal(vec3 LightDir,vec3 PxlWorldPos,vec3 PxlNormal,vec2 uv){
    float Glow = texture2D(gMiscMap,uv).r;
    if((PxlNormal.r > 0.9999 && PxlNormal.g > 0.9999 && PxlNormal.b > 0.9999)){
        return vec3(0.0);
    }
    vec3 AmbientColor  = LightDataD.xyz * LightDataA.x;
    vec3 DiffuseColor  = vec3(0.0);
    vec3 SpecularColor = vec3(0.0);
    vec3 TotalLight    = vec3(0.0);

    highp int index = int(texture2D(gMiscMap,uv).b * float(MATERIAL_COUNT_LIMIT));

    float SpecularAngle = 0.0;

    float kPi = 3.1415926535898;
    float smoothness = materials[index].g; //note: this value is automatically clamped 0 to 1 with the phyiscally based models below
	float F0 = materials[index].r;
    float roughness = 1.0 - smoothness; //only valid for physical lighting models
    float alpha = roughness * roughness;
    
    vec3 ViewDir = normalize(CamPosGamma.xyz - PxlWorldPos);
    vec3 Half = normalize(LightDir + ViewDir);
    float NdotL = max(dot(PxlNormal, LightDir), 0.0);
    float NdotH = max(dot(PxlNormal, Half), 0.0);
    
    DiffuseColor = (NdotL * LightDataD.xyz) * LightDataA.y;

    if(materials[index].b == 0.0){ // this is blinn phong (non-physical)

        float conserv = (8.0 + smoothness ) / (8.0 * kPi);
        SpecularAngle = conserv * pow(NdotH, smoothness);
    }		
    else if(materials[index].b == 1.0){ //this is phong (non-physical)

        float conserv = (2.0 + smoothness ) / (2.0 * kPi);
        vec3 Reflect = reflect(-LightDir, PxlNormal);
        SpecularAngle = conserv * pow(max(dot(ViewDir, Reflect), 0.0), smoothness);
    }
    else if(materials[index].b == 2.0){ //this is GGX (physical)
        float LdotH = max(0.0, dot(LightDir,Half));
        float alphaSqr = alpha * alpha;
        float denom = NdotH * NdotH * (alphaSqr - 1.0) + 1.0;
        float D = alphaSqr / (kPi * denom * denom);
        float Fresnel = SchlickFrensel(F0,LdotH);
        float k = 0.5 * alpha;
        float k2 = k * k;
        SpecularAngle = max(0.0, (NdotL * D * Fresnel / (LdotH*LdotH*(1.0-k2)+k2)) );
    }
    else if(materials[index].b == 3.0){ //this is Cook-Torrance (physical)
        float VdotH = max(0.0, dot(ViewDir,Half));
        float VdotN = max(0.0, dot(ViewDir,PxlNormal));
        float Frensel = SchlickFrensel(F0,VdotH);
		vec3 res = CookTorr(Frensel,VdotH,VdotN,NdotL,NdotH,alpha,kPi);
        SpecularAngle = res.x;
    }
    else if(materials[index].b == 4.0){ //this is gaussian (physical)
        float b = acos(NdotH); //this might also be cos. find out
        float fin = b / smoothness;
        SpecularAngle = exp(-fin*fin);
    }
    else if(materials[index].b == 5.0){ //this is beckmann (physical)
        SpecularAngle = BeckmannDist(NdotH,alpha,kPi);
    }
    else if(materials[index].b == 6.0){ //this is PBR (physical)
        /*
        vec3 f0 = mix(vec3(0.04), albedo, metallic);
        float VdotH = max(0.0, dot(ViewDir,Half));
        float VdotN = max(0.0, dot(ViewDir,PxlNormal));  
        vec3 Lo = vec3(0.0);
        float attenuation = 1.0;
        float Distance = length(LightDir);
        if(LightDataD.w != 0 && LightDataD.w != 2){
            attenuation = 1.0 / (max(1.0 , LightDataB.z + (LightDataB.w * Distance) + (LightDataC.x * Distance * Distance)));
        }
        
        vec3 radiance = LightDataD.xyz * attenuation; // calculate per-light radiance 

        vec3 Frensel = vec3(SchlickFrensel(f0,vdoth));
        vec3 kD = (vec3(1.0) - Frensel) * (1.0 - metallic);

        vec3 n  =  CookTorr(Frensel.r,VdotH,VdotN,LdotN,NdotH,alpha,kPi);
        float d = 4.0 * max(dot(PxlNormal, ViewDir), 0.0) * NdotL + 0.001; 
        
        Lo += (kD * albedo / vec3(kPi) + (n / vec3(d))) * radiance * NdotL; 

        // move this to the hdr pass. over there, Lo will be the lighting buffer. (will have to pass ao and albedo buffers in)
        //vec3 ambient = AmbientColor * albedo * ao; //should just ignore ambient for now
        vec3 color = (albedo * ao) + Lo;
        
        //here he tone mapped and gammad, but prob can skip this to HDR pass
        
        return max( vec3(Glow), Lo);
        */
    }

    SpecularColor = (LightDataD.xyz * LightDataA.z * SpecularAngle) * texture2D(gMiscMap,uv).g; //texture2D is specular map

    TotalLight = AmbientColor + DiffuseColor + SpecularColor;
    return max( vec3(Glow), TotalLight);
}
vec3 CalcPointLight(vec3 LightPos,vec3 PxlWorldPos, vec3 PxlNormal, vec2 uv){
    vec3 LightDir = normalize(LightPos - PxlWorldPos);
    float Distance = length(LightDir);

    vec3 c = CalcLightInternal(LightDir, PxlWorldPos, PxlNormal, uv);

    float attenuation =  1.0 / (max(1.0 , LightDataB.z + (LightDataB.w * Distance) + (LightDataC.x * Distance * Distance)));
    return c * attenuation;
}
vec3 CalcSpotLight(vec3 PxlWorldPos, vec3 PxlNormal, vec2 uv){
    return vec3(0.0);
}
void main(void){
    //vec2 uv = gl_TexCoord[0].st; //this cannot be used for point light mesh
    vec2 uv = gl_FragCoord.xy / vec2(ScreenData.z,ScreenData.w);

    vec3 PxlPosition = reconstruct_world_pos(uv);
    vec3 PxlNormal = texture2D(gNormalMap, uv).rgb;

    vec3 lightCalculation = vec3(0.0);
	vec3 LightPosition = vec3(LightDataC.yzw);
	vec3 LightDirection = vec3(LightDataA.w,LightDataB.x,LightDataB.y);

    if(LightDataD.w == 0){
        lightCalculation = CalcLightInternal(normalize(LightPosition - PxlPosition),PxlPosition,PxlNormal,uv);
    }
    else if(LightDataD.w == 1){
        lightCalculation = CalcPointLight(LightPosition,PxlPosition,PxlNormal,uv);
    }
    else if(LightDataD.w == 2){
        lightCalculation = CalcLightInternal(LightDirection,PxlPosition,PxlNormal,uv);
    }
    else if(LightDataD.w == 3){
        lightCalculation = CalcSpotLight(PxlPosition,PxlNormal,uv);
    }
    gl_FragData[0].rgb = lightCalculation;
}
