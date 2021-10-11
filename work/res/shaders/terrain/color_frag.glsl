#version 330 core

// uniform data
uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform vec3 uColor;

uniform sampler2D textureSampler0;
uniform sampler2D textureSampler1;
uniform sampler2D textureSampler2;

uniform float scale;

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

	vec3 xzTex;
	vec3 xyTex;
	vec3 zyTex;
	if(f_in.world_pos.y/scale < 0){
		xzTex = texture(textureSampler0, vec2(f_in.world_pos.x/25, f_in.world_pos.z/25)).rgb * abs(f_in.world_normal.y);
		xyTex = texture(textureSampler0, vec2(f_in.world_pos.x/25, f_in.world_pos.y)).rgb * abs(f_in.world_normal.z);
		zyTex = texture(textureSampler0, vec2(f_in.world_pos.z/25, f_in.world_pos.y)).rgb * abs(f_in.world_normal.x);
	}else if(f_in.world_pos.y/scale < 1/3.0f){
		xzTex = texture(textureSampler1, vec2(f_in.world_pos.x/25, f_in.world_pos.z/25)).rgb * abs(f_in.world_normal.y);
		xyTex = texture(textureSampler1, vec2(f_in.world_pos.x/25, f_in.world_pos.y)).rgb * abs(f_in.world_normal.z);
		zyTex = texture(textureSampler1, vec2(f_in.world_pos.z/25, f_in.world_pos.y)).rgb * abs(f_in.world_normal.x);
	}else{
		xzTex = texture(textureSampler2, vec2(f_in.world_pos.x/25, f_in.world_pos.z/25)).rgb * abs(f_in.world_normal.y);
		xyTex = texture(textureSampler2, vec2(f_in.world_pos.x/25, f_in.world_pos.y)).rgb * abs(f_in.world_normal.z);
		zyTex = texture(textureSampler2, vec2(f_in.world_pos.z/25, f_in.world_pos.y)).rgb * abs(f_in.world_normal.x);
	}
	
	vec3 textureColor = (xzTex + xyTex + zyTex);// / 3.0f;



	// calculate lighting (hack)
	vec3 eye = normalize(-f_in.position);
	float light = abs(dot(normalize(f_in.normal), eye));
	vec3 color = mix(textureColor / 4, textureColor, light);



	// output to the frambuffer
	fb_color = vec4(color, 1);
}
