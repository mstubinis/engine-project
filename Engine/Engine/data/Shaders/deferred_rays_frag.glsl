#version 120

uniform float exposure;
uniform float decay;
uniform float density;
uniform float weight;
uniform vec2 lightPositionOnScreen;
uniform sampler2D firstPass;
uniform int samples;

uniform int behind;
uniform float alpha;

void main(){
	if(behind == 0){
		vec2 uv = gl_TexCoord[0].st*2.0;
		vec2 deltaUV = vec2(uv - lightPositionOnScreen);
		deltaUV *= 1.0 /  float(samples) * density;

		float illuminationDecay = 1.0;
		for(int i=0; i < samples; i++){
			uv -= deltaUV/2.0;

			vec4 sample = texture2D(firstPass,uv);	
			sample *= illuminationDecay * weight;
			gl_FragColor += (sample * alpha);
			illuminationDecay *= decay;
		}
		gl_FragColor *= exposure;
	}
}