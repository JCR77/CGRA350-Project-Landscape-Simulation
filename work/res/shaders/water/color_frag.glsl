#version 330 core

// uniform data
uniform vec3 uColor;
uniform sampler2D uRefraction;
uniform sampler2D uReflection;

// viewspace data (this must match the output of the fragment shader)
in VertexData {
	vec3 position;
	vec3 normal;
	vec2 textureCoord;
    vec4 clipSpacePosition;
} f_in;

// framebuffer output
out vec4 fb_color;

/**
* Calculcations done in view space
*/
void main() {
    // map clip space coordinates to uv
    vec2 projectedCoords = f_in.clipSpacePosition.xy / f_in.clipSpacePosition.w;
    // map to range [0, 1]
    vec2 uv = projectedCoords * 0.5 + 0.5;

    vec4 refractionColour = texture(uRefraction, uv);
    refractionColour = mix(refractionColour, vec4(0, 1, 1, 1), 0.5);
    vec4 reflectionColour = texture(uReflection, uv);

    // fresnel effect
    vec3 toEye = normalize(-f_in.position);
    vec3 normal = normalize(f_in.normal);
    float fresnel = dot(toEye, normal);

    // mix refraction and reflection textures using fresnel effect
    vec4 colour = mix(reflectionColour, refractionColour, fresnel);

	// calculate lighting (hack)
	vec3 eye = normalize(-f_in.position);
	float light = abs(dot(normalize(f_in.normal), eye));
	colour = mix(colour / 4, colour, light);
    colour = mix(colour, vec4(0.0, 0.1, 0.1, 1), 0.4);

	// output to the frambuffer
	fb_color = colour;
}