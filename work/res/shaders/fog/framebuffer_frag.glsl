#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D screenTexture2;


const float offset_x = 1.0f / 800.0f;
const float offset_y = 1.0f / 600.0f;


vec2 offsets[9] = vec2[]
(
    vec2(-offset_x,offset_y),vec2(0.0f,offset_y),vec2(offset_x,offset_y),
    vec2(-offset_x,0.0f),vec2(0.0f,0.0f),vec2(offset_x,0.0f),
    vec2(-offset_x,-offset_y),vec2(0.0f,-offset_y),vec2(offset_x,-offset_y)
);

float kernel[9] = float[]
(
    1,1,1,
    1,-8,1,
    1,1,1

);

float near = 0.1f;
float far = 100.0f;

float linearizeDepth(float depth)
{
	return (2.0 * near * far) / (far + near - (depth * 2.0 - 1.0) * (far - near));
}

void main()
{
    //vec3 col = texture(screenTexture, TexCoords).rgb;
    //FragColor = vec4(col, 1.0);
    //vec3 col = vec3(texture(screenTexture, TexCoords.st));
    //float depth          = texture(screenTexture, TexCoords.st).r;
    //vec3  depthGrayscale = vec3( depth );
    //for(int i = 0; i < 9; i++)
    //{
    //    col += vec3(texture(screenTexture, TexCoords.st + offsets[i])) * kernel[i];
    //}
    //FragColor = vec4(1 - depthGrayscale, 1.0f);
    //FragColor = vec4(col, 1.0f);
    FragColor = vec4(vec3(linearizeDepth(texture(screenTexture2, TexCoords.st).r) / far), 1.0f);
} 