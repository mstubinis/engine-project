#version 130

in vec3 Position;
uniform mat4 MVP;

varying vec3 TexCoord0;
varying vec3 WP;

void main(){
    vec4 WVP_Pos = MVP * vec4(Position, 1);
    gl_Position = WVP_Pos.xyzw;
	gl_Position.z = gl_Position.w - 0.01f;
    TexCoord0 = Position;
	WP = gl_Position.xyz;
}