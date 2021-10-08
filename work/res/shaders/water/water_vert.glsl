#version 330 core

// uniform data
uniform mat4 uProjectionMatrix;
uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform vec4 uClipPlane;

// mesh data
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out VertexData {
	vec2 textureCoord;
    // In view space
	vec3 position;
	vec3 normal;
    vec4 clipSpacePosition;
    vec3 lightDirection;
} v_out;

const vec4 lightDirection = vec4(0, 40, -100, 0);

void main() {
    mat4 modelView = uViewMatrix * uModelMatrix;

	// transform vertex data to viewspace
	v_out.position = (modelView * vec4(aPosition, 1)).xyz;
	v_out.normal = normalize((modelView * vec4(aNormal, 0)).xyz);
	v_out.textureCoord = aTexCoord;
    v_out.lightDirection = normalize(vec3(uViewMatrix * lightDirection));

    v_out.clipSpacePosition = uProjectionMatrix * modelView * vec4(aPosition, 1);
	// set the screenspace position (needed for converting to fragment data)
	gl_Position = v_out.clipSpacePosition;
}