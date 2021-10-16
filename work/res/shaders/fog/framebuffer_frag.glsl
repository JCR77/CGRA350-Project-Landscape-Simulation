#version 330 core
out vec4 FragColor;

#define PI 3.1415926538
#define PI2 6.2831853071

in vec2 TexCoords;

uniform sampler2D originalOutput;
uniform sampler2D depthBuffer;
uniform sampler2D fogTexture;
uniform float near;
uniform float far;
uniform float state;

uniform float amplitude;//How high and low the depth values get
uniform float period;//Period between wave points being the same value

uniform float waveOffset;
uniform float textureSpeed;

float linearize(float depth)
{
	return (2.0 * near * far) / (far + near - (depth * 2.0 - 1.0) * (far - near));
}

float logisticize(float depth)
{
    return (1 / (1 + exp(-0.5 * (linearize(depth) - 5.0))));
}

vec3 cosWave( vec3 p )
{
    float axis = p.x;//Axis determines what axis the waves are on. Operating on depth so can never be z.
    float z =  amplitude * cos( (PI2/period) * (p.x + waveOffset));//
    return vec3(p.x, p.y, p.z + z);//
}

vec2 cosWave( vec2 p ){
    float axis = p.x;
    float y =  0.015 * cos( (PI2/1) * (p.x + textureSpeed));//
    return vec2(p.x, p.y + y);//
}

void main()
{

    //Original Output
    //FragColor = vec4(texture(originalOutput, TexCoords).rgb,1);

    vec3 cIn = texture(originalOutput, TexCoords).rgb;
    vec3 cfog = texture(fogTexture, TexCoords).rgb;
    float depth = texture(depthBuffer, TexCoords.st).r;

    if(state >= 1.0f)
    {
        float depth = logisticize(texture(depthBuffer, TexCoords.st).r);
        depth = cosWave(vec3(TexCoords.x,TexCoords.y,depth)).z;
        depth = clamp(depth, 0.0f, 1.0f);        

        vec2 uv = cosWave(TexCoords.st); 
        cfog = texture(fogTexture, uv).rgb;        
        FragColor = vec4(cIn, 1.0f) * (1.0f - depth) + vec4(depth * cfog , 1.0f);
    }
    else
    {
        FragColor = vec4(texture(originalOutput, TexCoords).rgb,1);
    }  
}