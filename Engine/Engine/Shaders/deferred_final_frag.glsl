#version 110

uniform sampler2D gColorMap;
uniform sampler2D gLightMap;
uniform sampler2D gNormalMap;
uniform sampler2D gGlowMap;
uniform sampler2D gBloomMap;

uniform vec2 gScreenSize;
uniform vec4 gAmbientColor;

vec2 CalcTexCoord(){ return gl_FragCoord.xy / gScreenSize; }

void main(){
	vec2 texCoords = CalcTexCoord();
	vec4 image = texture2D(gColorMap, texCoords);
	vec4 lighting = texture2D(gLightMap, texCoords);
	vec4 normals = texture2D(gNormalMap,texCoords);
	float ssao = texture2D(gGlowMap, texCoords).g;
	float glow = texture2D(gGlowMap, texCoords).r;
	vec4 bloom = texture2D(gBloomMap,texCoords);

	if(normals.r > 0.9999 && normals.g > 0.9999 && normals.b > 0.9999)
		gl_FragColor = bloom+image;
	else{
		vec4 light = max(gAmbientColor,max(vec4(glow),(lighting*ssao)));
		vec4 imageLight = image * light;
		gl_FragColor = imageLight + max((bloom*(1.0-light)), glow*image);
	}
}