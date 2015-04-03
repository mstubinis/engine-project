#version 330

uniform sampler2D gColorMap;
uniform sampler2D gLightMap;
uniform sampler2D gSSAOMap;
uniform sampler2D gGlowMap;

uniform vec2 gScreenSize;

vec2 CalcTexCoord(){ return gl_FragCoord.xy / gScreenSize; }
void main(){
	vec2 texCoords = CalcTexCoord();
	vec4 image = texture2D(gColorMap, texCoords);
	vec4 lighting = texture2D(gLightMap, texCoords);
	float ssao = texture2D(gSSAOMap, texCoords);
	vec4 glow = texture2D(gGlowMap, texCoords);

	if(lighting.r < 0.00001 && lighting.g < 0.00001 && lighting.b < 0.00001)
		gl_FragColor = image;
	else{
		vec4 light = max(vec4(0.05),lighting*ssao);
		vec4 bloom = vec4(glow.r);
		vec4 lightAndBloom = max(light,bloom);
		vec4 Final = image*lightAndBloom;
		gl_FragColor = Final;
	}
}