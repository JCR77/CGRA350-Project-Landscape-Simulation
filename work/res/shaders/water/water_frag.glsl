#version 330 core

// uniform data
uniform vec3 uColor;
uniform sampler2D uRefraction;
uniform sampler2D uReflection;

in VertexData {
	vec2 textureCoord;
    // In view space
	vec3 position;
	vec3 normal;
    vec4 clipSpacePosition;
    vec3 lightDirection;
} f_in;

const vec3 lightColour = vec3(1, 1, 1);
// framebuffer output
out vec4 fb_color;

/**
* Calculations done in view space
*/
void main() {
    // map clip space coordinates to uv
    vec2 projectedCoords = f_in.clipSpacePosition.xy / f_in.clipSpacePosition.w;
    // map to range [0, 1]
    vec2 uv = projectedCoords * 0.5 + 0.5;

    vec4 refractionColour = texture(uRefraction, uv);
    refractionColour = mix(refractionColour, vec4(0, 1, 1, 1), 0.5);
    vec4 reflectionColour = texture(uReflection, uv);

    vec3 toEye = normalize(-f_in.position);
    vec3 normal = normalize(f_in.normal);

    // fresnel effect
    float fBias = 0.20373;
    float facing = 1.0 - max(dot(toEye, normal), 0);
    float weight = max(fBias + (1.0 - fBias) * pow(facing, 1), 0);

    // mix refraction and reflection textures using fresnel effect
    vec3 colour = mix(refractionColour, reflectionColour, weight).xyz;
    // mix with a little bit of blue
    // colour = mix(colour, vec3(0.0, 1, 1), 0.1).xyz;

	/* calculate specular lighting */
    float specularStrength = 0.5;
    vec3 reflectDirection = reflect(-f_in.lightDirection, normal);
    float spec = pow(max(dot(toEye, reflectDirection), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColour;

    colour = specular + colour;

	// output to the frambuffer
	fb_color = vec4(colour, 1.0);
	// fb_color = reflectionColour;
}