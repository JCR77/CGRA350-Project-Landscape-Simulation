#version 330 core

// uniform data
uniform samplerCube uSkyTexture;

in VertexData {
	vec3 textureCoord;
} f_in;

out vec4 fb_color;

/**
* Calculcations done in view space
*/
void main() {
	fb_color = texture(uSkyTexture, f_in.textureCoord);
}