
attribute vec3 position;
//attribute float uv;
attribute vec2 uv;
attribute vec4 normal;
attribute vec4 binormal;
attribute vec4 tangent;

uniform int HasAtmosphere;

uniform mat4 VP;
uniform mat4 Model;
uniform mat4 Rot;

uniform vec3 v3CameraPos;
uniform vec3 v3CameraPosReal;
uniform vec3 v3LightDir;            // The direction vector to the light source 
uniform vec3 v3InvWavelength;       // 1 / pow(wavelength, 4) for the red, green, and blue channels 
uniform float fCameraHeight2;       // fCameraHeight^2 
uniform float fOuterRadius;         // The outer (atmosphere) radius 
uniform float fOuterRadius2;        // fOuterRadius^2 
uniform float fInnerRadius;         // The inner (planetary) radius 
uniform float fKrESun;              // Kr * ESun 
uniform float fKmESun;              // Km * ESun 
uniform float fKr4PI;               // Kr * 4 * PI 
uniform float fKm4PI;               // Km * 4 * PI 
uniform float fScale;               // 1 / (fOuterRadius - fInnerRadius) 
uniform float fScaleDepth;          // The scale depth (i.e. the altitude at which the atmosphere's average density is found) 
uniform float fScaleOverScaleDepth; // fScale / fScaleDepth
uniform float fSamples;
uniform int nSamples;
uniform int fromAtmosphere;

varying vec3 c0;
varying vec3 c1;

varying vec2 UV;


varying vec3 WorldPosition;
varying vec3 CameraPosition;
varying vec3 CameraPositionReal;
varying vec3 Normals;
varying vec3 Binormals;
varying vec3 Tangents;

flat varying float HasAtmo;

varying float logz_f;
varying float FC;
uniform float fcoeff;

vec2 UnpackFloat32Into2Floats(float i){
    vec2 res;
    res.y = i - floor(i);
    res.x = (i - res.y) / 1000.0;
    res.x = (res.x - 0.5) * 2.0;
    res.y = (res.y - 0.5) * 2.0;
    return res;
}
float scale(float fCos) {   
    float x = 1.0 - fCos;   
    return fScaleDepth * exp(-0.00287 + x * (0.459 + x * (3.83 + x * (-6.80 + x * 5.25)))); 
}
float getNearIntersection(vec3 _p, vec3 _r, float _d2, float _r2){
    float B = 2.0 * dot(_p, _r);
    float C = _d2 - _r2;
    float fDet = max(0.0, B * B - 4.0 * C);
    return 0.5 * (-B - sqrt(fDet));
}
void main(){
    mat4 MVP = VP * Model;
	WorldPosition = (Model * vec4(position,1.0)).xyz;
    Normals = (Model * vec4(normal.zyx,0.0)).xyz; //Order is ZYXW so to bring it to XYZ we need to use ZYX
    Binormals = (Model * vec4(binormal.zyx,0.0)).xyz; //Order is ZYXW so to bring it to XYZ we need to use ZYX
    Tangents = (Model * vec4(tangent.zyx,0.0)).xyz; //Order is ZYXW so to bring it to XYZ we need to use ZYX
	UV = uv;

    gl_Position = MVP * vec4(position, 1.0);

    //UV = UnpackFloat32Into2Floats(uv);
    CameraPosition = v3CameraPos;
	CameraPositionReal = v3CameraPosReal;

	HasAtmo = HasAtmosphere;

    logz_f = 1.0 + gl_Position.w;
    gl_Position.z = (log2(max(1e-6, logz_f)) * fcoeff - 1.0) * gl_Position.w;
    FC = fcoeff;

    if(HasAtmosphere == 1){
		vec3 test = (Rot * vec4(position,1.0)).xyz;
		vec3 v3Pos = test * fInnerRadius;
        vec3 v3Ray = v3Pos - v3CameraPos;
        float fFar = length(v3Ray);
        v3Ray /= fFar;  
        
        vec3 v3Start;
        if(fromAtmosphere == 0){
            float fNear = getNearIntersection(v3CameraPos, v3Ray, fCameraHeight2, fOuterRadius2);
            v3Start = v3CameraPos + v3Ray * fNear; 
            fFar -= fNear;
        }
        else{
            v3Start = v3CameraPos;
        }
        
        vec3 normalSphere = normalize(v3Pos);
        float fDepth = exp((fInnerRadius - fOuterRadius) / fScaleDepth);    
        float fCameraAngle = dot(-v3Ray, normalSphere);
        float fLightAngle = dot(v3LightDir, normalSphere);
        float fCameraScale = scale(fCameraAngle);
        float fLightScale = scale(fLightAngle);
        float fCameraOffset = fDepth*fCameraScale;
        float fTemp = (fLightScale + fCameraScale);
    
        float fSampleLength = fFar / fSamples;
        float fScaledLength = fSampleLength * fScale;   
        vec3 v3SampleRay = v3Ray * fSampleLength;   
        vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;   
    
        vec3 v3FrontColor = vec3(0.0);    
        vec3 v3Attenuate = vec3(0.0);
        for(int i = 0; i < nSamples; ++i)   {   
            float fHeight = length(v3SamplePoint);  
            float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fHeight));    
            float fScatter = fDepth*fTemp - fCameraOffset;  
            v3Attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI)); 
            v3FrontColor += v3Attenuate * (fDepth * fScaledLength); 
            v3SamplePoint += v3SampleRay;
        }
        c0 = v3FrontColor * (v3InvWavelength * fKrESun + fKmESun);  
        c1 = v3Attenuate;
    }
}
