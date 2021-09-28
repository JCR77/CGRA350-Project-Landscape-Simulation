#version 330 core

// uniform data
uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform vec3 uColor;
uniform sampler2D textureMap;

// viewspace data (this must match the output of the fragment shader)
in VertexData {
	vec3 position;
	vec3 normal;
	vec2 textureCoord;
} f_in;

// framebuffer output
out vec4 fb_color;

float near = 0.1f;
float far = 100.0f;

float linearizeDepth(float depth)
{
	return (2.0 * near * far) / (far + near - (depth * 2.0 - 1.0) * (far - near));
}

float steepness = 0.5f;
float offset = 5.0f;

float logisticDepth(float depth)
{
	float zVal = linearizeDepth(depth);
	return (1 / 1 + exp(-steepness * (zVal - offset)));
}


void main() {
	//Original Code
	// calculate lighting (hack)
	vec3 eye = normalize(-f_in.position);
	float light = abs(dot(normalize(f_in.normal), eye));
	vec3 color = mix(uColor / 4, uColor, light);

	// output to the frambuffer
	//fb_color = vec4(color, 1);
	fb_color = vec4(vec3(linearizeDepth(gl_FragCoord.z) / far), 1.0f);
	//float depth = logisticDepth(gl_FragCoord.z);
	//fb_color = vec4(color, 1) * (1.0f - depth) + vec4(depth * vec3(0.85f,0.85f,0.90f), 1.0f);
}