#version 330 core

// uniform data
uniform vec3 uColor;
uniform sampler2D uRefraction;
uniform sampler2D uReflection;
uniform sampler2D uNormalMap;
uniform sampler2D uDudvMap;

uniform float uDistortionStrength;

in VertexData {
	vec2 textureCoord;
    // In view space
	vec3 position;
	vec3 normal;
    vec4 clipSpacePosition;
    vec3 lightDirection;
} f_in;

const vec3 lightColour = vec3(1, 1, 1);

out vec4 fb_color;

/**
* Calculations done in view space
*/
void main() {
    // map clip space coordinates to uv
    vec2 projectedCoords = f_in.clipSpacePosition.xy / f_in.clipSpacePosition.w;
    // map to range [0, 1]
    vec2 uv = projectedCoords * 0.5 + 0.5;

    // apply distortion
    // sample from dudv map, and offset original uv by this amount
    vec2 distortion = texture(uDudvMap, f_in.textureCoord).xy;
    // map components from range [0, 1] to [-1, 1] to get values in all directions
    distortion = distortion * 2 - 1;
    distortion *= uDistortionStrength;

    uv = clamp(uv + distortion, 0.001, 0.999);
    vec2 normalMapUv = clamp(f_in.textureCoord + distortion, 0.001, 0.999);

    vec4 refractionColour = texture(uRefraction, uv);
    // refractionColour = mix(refractionColour, vec4(0, 1, 1, 1), 0.5);
    vec4 reflectionColour = texture(uReflection, uv);

    vec3 toEye = normalize(-f_in.position);
    // get normal from normal map
    vec3 normal = texture(uNormalMap, normalMapUv).xyz;
    // map x and z current range of [0, 1] to [-1, 1],
    // and scale y, to get more normals facing upward
    normal = vec3(normal.x * 2 - 1, normal.y * 8, normal.z * 2 - 1);
    normal = normalize(normal);
    // normal = normalize(f_in.normal);

    // fresnel effect
    float fBias = 0.20373;
    float facing = 1.0 - max(dot(toEye, normal), 0);
    float weight = max(fBias + (1.0 - fBias) * pow(facing, 2), 0);

    // mix refraction and reflection textures using fresnel effect
    vec3 colour = mix(refractionColour, reflectionColour, weight).xyz;

	// calculate specular lighting
    float specularStrength = 0.4;
    vec3 reflectDirection = reflect(-f_in.lightDirection, normal);
    float spec = pow(max(dot(toEye, reflectDirection), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColour;

    colour = specular + colour;

	// output to the frambuffer
	fb_color = vec4(colour, 1.0);
}