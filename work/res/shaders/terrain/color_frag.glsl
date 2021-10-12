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
	float transitionOffset;
} f_in;

// framebuffer output
out vec4 fb_color;

void main() {

	//calculate textures for diff heights
	vec3 xzTex0 = texture(textureSampler0, vec2(f_in.world_pos.x/25, f_in.world_pos.z/25)).rgb * abs(f_in.world_normal.y);
	vec3 xyTex0 = texture(textureSampler0, vec2(f_in.world_pos.x/25, f_in.world_pos.y)).rgb * abs(f_in.world_normal.z);
	vec3 zyTex0 = texture(textureSampler0, vec2(f_in.world_pos.z/25, f_in.world_pos.y)).rgb * abs(f_in.world_normal.x);
	vec3 textureColor0 = (xzTex0 + xyTex0 + zyTex0);// / 3.0f;

	vec3 xzTex1 = texture(textureSampler1, vec2(f_in.world_pos.x/25, f_in.world_pos.z/25)).rgb * abs(f_in.world_normal.y);
	vec3 xyTex1 = texture(textureSampler1, vec2(f_in.world_pos.x/25, f_in.world_pos.y)).rgb * abs(f_in.world_normal.z);
	vec3 zyTex1 = texture(textureSampler1, vec2(f_in.world_pos.z/25, f_in.world_pos.y)).rgb * abs(f_in.world_normal.x);
	vec3 textureColor1 = (xzTex1 + xyTex1 + zyTex1);// / 3.0f;

	vec3 xzTex2 = texture(textureSampler2, vec2(f_in.world_pos.x/25, f_in.world_pos.z/25)).rgb * abs(f_in.world_normal.y);
	vec3 xyTex2 = texture(textureSampler2, vec2(f_in.world_pos.x/25, f_in.world_pos.y)).rgb * abs(f_in.world_normal.z);
	vec3 zyTex2 = texture(textureSampler2, vec2(f_in.world_pos.z/25, f_in.world_pos.y)).rgb * abs(f_in.world_normal.x);
	vec3 textureColor2 = (xzTex2 + xyTex2 + zyTex2);// / 3.0f;
	
	//blend textures
	float blendDist = 2.0f;
	vec3 blendedTex;
	if(f_in.world_pos.y/scale < 0.1 + f_in.transitionOffset){
		float transitionHeight = (-0.05 + f_in.transitionOffset)*scale;
		float blendAmount = (f_in.world_pos.y - (transitionHeight - blendDist)) / (2*blendDist);
		blendAmount = clamp(blendAmount, 0, 1);
		blendedTex = mix(textureColor0, textureColor1, blendAmount);
	}else{
		float transitionHeight = (0.3 + f_in.transitionOffset)*scale;
		float blendAmount = (f_in.world_pos.y - (transitionHeight - blendDist)) / (2*blendDist);
		blendAmount = clamp(blendAmount, 0, 1);
		blendedTex = mix(textureColor1, textureColor2, blendAmount);
	}



	// calculate lighting (hack)
	vec3 eye = normalize(-f_in.position);
	float light = abs(dot(normalize(f_in.normal), eye));
	vec3 color = mix(blendedTex / 4, blendedTex, light);



	// output to the frambuffer
	fb_color = vec4(color, 1);
}
