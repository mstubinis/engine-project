
attribute vec3 position;

uniform mat4 Model;
uniform float fcoeff;
uniform int nSamples;

uniform vec4 VertDataMisc1;         //xyz = camPos,                              w = lightDir.x
uniform vec4 VertDataMisc2;         //x = camHeight, y = camHeight2, z = UNUSED, w = lightDir.y
uniform vec4 VertDataMisc3;         //xyz = v3InvWaveLength,                     w = lightDir.z
uniform vec4 VertDataScale;         //fScale,fScaledepth,fScaleOverScaleDepth,fSamples
uniform vec4 VertDataRadius;        //out, out2, inn, inn2
uniform vec4 VertDatafK;            //fKrESun,fKmESun,fKr4PI,fKm4PI

varying vec3 c0;
varying vec3 c1;
varying vec3 v3Direction;
varying vec3 v3LightPosition;
varying vec3 WorldPosition;

varying float Depth;
varying float cameraHeight;
varying float outerRadius;
varying float planetRadius;
varying float logz_f;
flat varying float FC;

float scale(float fCos){
    float x = 1.0 - fCos;
    return VertDataScale.y * exp(-0.00287 + x * (0.459 + x * (3.83 + x * (-6.80 + x * 5.25))));
}
void main(){
    vec3 v3Pos = position * vec3(VertDataRadius.x);
    vec3 v3Ray = v3Pos - VertDataMisc1.xyz;
	vec3 v3LightDir = vec3(VertDataMisc1.w,VertDataMisc2.w,VertDataMisc3.w);
    float fFar = length(v3Ray);
    v3Ray /= fFar;

    vec3 v3Start = VertDataMisc1.xyz;
    float fHeight = length(v3Start);
    float fDepth = exp(VertDataScale.z * (VertDataRadius.z - VertDataMisc2.x));
    float fStartAngle = dot(v3Ray, v3Start) / fHeight;
    float fStartOffset = fDepth*scale(fStartAngle);

    float fStartDepth = exp(VertDataScale.z * (VertDataRadius.z - VertDataMisc2.x));
    Depth = clamp(fStartDepth*scale(fStartAngle),0.0,1.0);

    float fSampleLength = fFar / VertDataScale.w;
    float fScaledLength = fSampleLength * VertDataScale.x;
    vec3 v3SampleRay = v3Ray * fSampleLength;
    vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;

    vec3 v3FrontColor = vec3(0.0);
    vec3 v3Attenuate = vec3(0.0);
    for(int i = 0; i < nSamples; ++i) {
        float fHeight = length(v3SamplePoint);
        float fDepth = exp(VertDataScale.z * (VertDataRadius.z - fHeight));
        float fLightAngle = dot(v3LightDir, v3SamplePoint) / fHeight;
        float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;
        float fScatter = (fStartOffset + fDepth * (scale(fLightAngle) - scale(fCameraAngle)));

        vec3 v3Attenuate = exp(-fScatter * (VertDataMisc3.xyz * VertDatafK.z + VertDatafK.w));
        v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
        v3SamplePoint += v3SampleRay;
    }
    gl_Position = CameraViewProj * Model * vec4(position, 1.0);

    v3LightPosition = v3LightDir;
    v3Direction = VertDataMisc1.xyz - v3Pos;

    c0 = v3FrontColor * (VertDataMisc3.xyz * VertDatafK.x);
    c1 = v3FrontColor * VertDatafK.y;

    cameraHeight = VertDataMisc2.x;
    outerRadius = VertDataRadius.x;
    planetRadius = VertDataRadius.z;

    WorldPosition = (Model * vec4(position, 1.0)).xyz;

    logz_f = 1.0 + gl_Position.w;
    gl_Position.z = (log2(max(1e-6, logz_f)) * fcoeff - 1.0) * gl_Position.w;
    FC = fcoeff;
}