#version 330 core

// uniform data
uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform vec3 uColor;

uniform sampler2D textureSampler;

// viewspace data (this must match the output of the fragment shader)
in VertexData {
	vec3 position;
	vec3 world_pos;
	vec3 normal;
	vec3 world_normal;
	vec2 textureCoord;
} f_in;

// framebuffer output
out vec4 fb_color;

void main() {

	//calculate texture
	vec3 xzTex = texture(textureSampler, vec2(f_in.world_pos.x/100, f_in.world_pos.z/100)).rgb * abs(f_in.world_normal.y);
	vec3 xyTex = texture(textureSampler, vec2(f_in.world_pos.x/100, f_in.world_pos.y/2)).rgb * abs(f_in.world_normal.z);
	vec3 zyTex = texture(textureSampler, vec2(f_in.world_pos.z/100, f_in.world_pos.y/2)).rgb * abs(f_in.world_normal.x);
	vec3 textureColor = (xzTex + xyTex + zyTex);// / 3.0f;

	// calculate lighting (hack)
	vec3 eye = normalize(-f_in.position);
	float light = abs(dot(normalize(f_in.normal), eye));
	vec3 color = mix(textureColor / 4, textureColor, light);

	// output to the frambuffer
	fb_color = vec4(color, 1);
}