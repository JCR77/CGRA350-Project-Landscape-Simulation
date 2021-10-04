#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D screenTexture2;
uniform float near;
uniform float far;
uniform float state;

uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform vec3 uColor;

vec3 cIn;

// viewspace data (this must match the output of the fragment shader)
in VertexData {
	vec3 position;
	vec3 normal;
	vec2 textureCoord;
} f_in;

float linearizeDepth(float depth)
{
	return (2.0 * near * far) / (far + near - (depth * 2.0 - 1.0) * (far - near));
}

float steepness = 0.5f;
float offset = 5.0f;

float logisticDepth(float depth)
{
    float zVal = linearizeDepth(depth);
    return (1 / (1 + exp(-steepness * (zVal - offset))));

}

void main()
{

    //Original Output
    //FragColor = vec4(texture(screenTexture, TexCoords).rgb,1);
    cIn = texture(screenTexture, TexCoords).rgb;


    //Fake Fog
    //float depth = logisticDepth(texture(screenTexture2, TexCoords.st).r);
    //FragColor = vec4(cIn, 1.0f) * (1.0f - depth) + vec4(depth * vec3(0.85f,0.85f,0.90f) , 1.0f);

    //Depthmap(Linear Version)
    //FragColor = vec4(vec3(linearizeDepth(texture(screenTexture2, TexCoords.st).r) / far), 1.0f);

    //Final Output
    //FragColor = f() * cIn + (1 - f()) * cFog;

    if(state >= 1.0f)
    {
        float depth = logisticDepth(texture(screenTexture2, TexCoords.st).r);
        FragColor = vec4(cIn, 1.0f) * (1.0f - depth) + vec4(depth * vec3(0.85f,0.85f,0.90f) , 1.0f);
    }
    else
    {
        FragColor = vec4(texture(screenTexture, TexCoords).rgb,1);
    }
    
}