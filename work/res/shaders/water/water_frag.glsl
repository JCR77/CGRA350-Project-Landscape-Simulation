#version 330 core

// uniform data
uniform vec3 uColor;
uniform sampler2D uRefraction;
uniform sampler2D uReflection;
uniform sampler2D uNormalMap;
uniform sampler2D uDudvMap;

/**
* Primary water movement (e.g. if there were wind)
*/
uniform vec2 uPrimaryOffset;
/**
* Secondary movement, creating effect of more random distortion movement
*/
uniform vec2 uSecondaryOffset;
uniform float uDistortionStrength;

in VertexData {
    vec2 textureCoord;
    // In view space
    vec3 position;
    vec4 clipSpacePosition;
    vec3 lightDirection;
} f_in;

out vec4 fb_color;

vec3 sampleNormal(vec2 uvCoord) {
    // get normal from normal map
    vec3 normal = texture(uNormalMap, uvCoord).xyz;
    // map x and z current range of [0, 1] to [-1, 1],
    // and scale y, to get more normals facing upward
    normal = vec3(normal.x * 2 - 1, normal.y * 8, normal.z * 2 - 1);
    return normalize(normal);
}

const float specularStrength = 0.5;
const vec3 lightColour = vec3(1, 1, 1);

// calculate specular lighting
vec3 getSpecular(vec3 normal) {
    vec3 toEye = normalize(-f_in.position);

    vec3 reflectDirection = reflect(-f_in.lightDirection, normal);
    float spec = pow(max(dot(toEye, reflectDirection), 0.0), 50);
    return specularStrength * spec * lightColour;
}

const float fBias = 0.20373;

// fresnel effect
float getFresnelWeight(vec3 normal) {
    vec3 toEye = normalize(-f_in.position);

    float facing = 1.0 - max(dot(toEye, normal), 0);
    return max(fBias + (1.0 - fBias) * pow(facing, 2), 0);
}

/**
* Calculations done in view space
*/
void main() {
    // Create water distortion effect:
    // Offsetting the original uv coordinates
    vec2 primaryMotionCoords = f_in.textureCoord + uPrimaryOffset;
    vec2 secondaryMotionCoords = f_in.textureCoord + uSecondaryOffset;
    secondaryMotionCoords = texture(uDudvMap, secondaryMotionCoords).xy;
    secondaryMotionCoords = secondaryMotionCoords * 2 - 1;
    secondaryMotionCoords *= uDistortionStrength;

    // uv coordinates to use to sample from the dudv and normal map
    vec2 mapUv = primaryMotionCoords + secondaryMotionCoords;

    vec3 normal = sampleNormal(mapUv);

    // Sample from dudv map using offsetted uv coords, to get another
    // offset that will be used to distort the refraction and reflections
    vec2 distortion = texture(uDudvMap, mapUv).xy;
    // map components from range [0, 1] to [-1, 1] to get distortions in all directions
    distortion = distortion * 2 - 1;
    distortion *= uDistortionStrength;

    // Get uv coordinates for sampling from reflection and refraction textures
    // map clip space coordinates to uv space
    vec2 projectedCoords = f_in.clipSpacePosition.xy / f_in.clipSpacePosition.w;
    // map to range [0, 1]
    vec2 uv = projectedCoords * 0.5 + 0.5;
    // apply distortion offset and clamp in range [0, 1] to 
    // prevent sampling outside of the textures.
    uv = clamp(uv + distortion, 0.001, 0.999);

    vec4 refractionColour = texture(uRefraction, uv);
    vec4 reflectionColour = texture(uReflection, uv);

    float weight = getFresnelWeight(normal);
    // mix refraction and reflection textures using fresnel effect
    vec3 colour = mix(refractionColour, reflectionColour, weight).xyz;

    // output to the frambuffer
    fb_color = vec4(getSpecular(normal) + colour, 1.0);
}
