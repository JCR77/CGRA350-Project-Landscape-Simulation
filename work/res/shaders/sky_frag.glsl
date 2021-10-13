#version 330 core

// uniform data
uniform samplerCube uSkyTexture;
uniform float uFog;

in VertexData {
	vec3 textureCoord;
} f_in;

out vec4 fb_color;

/**
* Calculcations done in view space
*/
void main() {
    vec4 colour = texture(uSkyTexture, f_in.textureCoord); 
    float fog = uFog / 10;
	fb_color = mix(colour, vec4(1, 1, 1, 1), fog);
}