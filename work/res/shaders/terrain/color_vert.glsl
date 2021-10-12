#version 330 core

// uniform data
uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform vec3 uColor;
uniform vec4 uClipPlane;

//uniform float[201*201] trasitionHeightOffsets;

// mesh data
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in float atransitionOffset;

// model data (this must match the input of the vertex shader)
out VertexData {
	vec3 position;
	vec3 world_pos;
	vec3 normal;
	vec3 world_normal;
	vec2 textureCoord;
	float transitionOffset;
} v_out;

void main() {
    // Calculates whether this vertex should be clipped or not
    gl_ClipDistance[0] = dot(vec4(aPosition, 1), uClipPlane);

	// transform vertex data to viewspace
	v_out.position = (uModelViewMatrix * vec4(aPosition, 1)).xyz;
	v_out.world_pos = aPosition;
	v_out.normal = normalize((uModelViewMatrix * vec4(aNormal, 0)).xyz);
	v_out.world_normal = normalize(aNormal);
	v_out.textureCoord = aTexCoord;
	int i = 201 * int(aTexCoord.y) + int(aTexCoord.x);
	v_out.transitionOffset = atransitionOffset * 0.3f;

	// set the screenspace position (needed for converting to fragment data)
	gl_Position = uProjectionMatrix * uModelViewMatrix * vec4(aPosition, 1);
}