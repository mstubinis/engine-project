#version 110

varying vec2 offset[14];

void main(){
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord[0] = gl_MultiTexCoord0;

    offset[ 0] = vec2(-0.028, 0);
    offset[ 1] = vec2(-0.024, 0);
    offset[ 2] = vec2(-0.020, 0);
    offset[ 3] = vec2(-0.016, 0);
    offset[ 4] = vec2(-0.012, 0);
    offset[ 5] = vec2(-0.008, 0);
    offset[ 6] = vec2(-0.004, 0);
    offset[ 7] = vec2( 0.004, 0);
    offset[ 8] = vec2( 0.008, 0);
    offset[ 9] = vec2( 0.012, 0);
    offset[10] = vec2( 0.016, 0);
    offset[11] = vec2( 0.020, 0);
    offset[12] = vec2( 0.024, 0);
    offset[13] = vec2( 0.028, 0);
}