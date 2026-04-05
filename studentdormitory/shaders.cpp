#include "shaders.h"
#include <glad/glad.h>
#include <iostream>

const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec3 GouraudColor;

uniform mat4 model, view, projection;
uniform vec3 objectColor, viewPos;
uniform int  texMode;
uniform float uvTile;

uniform vec3 dirLight_direction;
uniform vec3 dirLight_ambient, dirLight_diffuse, dirLight_specular;
uniform bool ambientOn, diffuseOn, specularOn;
uniform bool directionalLightOn;

void main()
{
    FragPos  = vec3(model * vec4(aPos, 1.0));
    Normal   = mat3(transpose(inverse(model))) * aNormal;
    TexCoord = aTexCoord * uvTile;
    gl_Position = projection * view * vec4(FragPos, 1.0);

    if (texMode == 2 && directionalLightOn) {
        vec3 norm     = normalize(Normal);
        vec3 lightDir = normalize(-dirLight_direction);
        vec3 viewDir  = normalize(viewPos - FragPos);
        vec3 reflDir  = reflect(-lightDir, norm);
        float diff = max(dot(norm, lightDir), 0.0);
        float spec = pow(max(dot(viewDir, reflDir), 0.0), 32.0);
        vec3 a = ambientOn  ? dirLight_ambient  * objectColor          : vec3(0.0);
        vec3 d = diffuseOn  ? dirLight_diffuse  * diff * objectColor   : vec3(0.0);
        vec3 s = specularOn ? dirLight_specular * spec * vec3(0.5)     : vec3(0.0);
        GouraudColor = a + d + s;
    } else {
        GouraudColor = objectColor * (ambientOn ? 0.30 : 0.05);
    }
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 GouraudColor;

uniform vec3 objectColor, viewPos;
uniform sampler2D texSampler;
uniform bool hasTexture;
uniform int  texMode;

uniform bool directionalLightOn, pointLightsOn, spotLightOn;
uniform bool ambientOn, diffuseOn, specularOn;
uniform vec3 dirLight_direction, dirLight_ambient, dirLight_diffuse, dirLight_specular;

#define NR_POINT_LIGHTS 6
uniform vec3  pointLights_position [NR_POINT_LIGHTS];
uniform vec3  pointLights_ambient  [NR_POINT_LIGHTS];
uniform vec3  pointLights_diffuse  [NR_POINT_LIGHTS];
uniform vec3  pointLights_specular [NR_POINT_LIGHTS];
uniform float pointLights_constant [NR_POINT_LIGHTS];
uniform float pointLights_linear   [NR_POINT_LIGHTS];
uniform float pointLights_quadratic[NR_POINT_LIGHTS];

uniform vec3  spotLight_position,  spotLight_direction;
uniform vec3  spotLight_ambient,   spotLight_diffuse,  spotLight_specular;
uniform float spotLight_cutOff;
uniform vec3  emissive;

vec3 CalcDirLight(vec3 norm, vec3 viewDir) {
    vec3 ld    = normalize(-dirLight_direction);
    float diff = max(dot(norm, ld), 0.0);
    vec3 ref   = reflect(-ld, norm);
    float spec = pow(max(dot(viewDir, ref), 0.0), 32.0);
    vec3 a = ambientOn  ? dirLight_ambient            : vec3(0.0);
    vec3 d = diffuseOn  ? dirLight_diffuse  * diff    : vec3(0.0);
    vec3 s = specularOn ? dirLight_specular * spec     : vec3(0.0);
    return a + d + s;
}
vec3 CalcPointLight(int idx, vec3 norm, vec3 fp, vec3 viewDir) {
    vec3 ld    = normalize(pointLights_position[idx] - fp);
    float diff = max(dot(norm, ld), 0.0);
    vec3 ref   = reflect(-ld, norm);
    float spec = pow(max(dot(viewDir, ref), 0.0), 32.0);
    float dist = length(pointLights_position[idx] - fp);
    float att  = 1.0 / (pointLights_constant[idx]
                      + pointLights_linear[idx] * dist
                      + pointLights_quadratic[idx] * dist * dist);
    vec3 a = ambientOn  ? pointLights_ambient [idx]        : vec3(0.0);
    vec3 d = diffuseOn  ? pointLights_diffuse [idx] * diff : vec3(0.0);
    vec3 s = specularOn ? pointLights_specular[idx] * spec : vec3(0.0);
    return (a + d + s) * att;
}
vec3 CalcSpotLight(vec3 norm, vec3 fp, vec3 viewDir) {
    vec3 ld     = normalize(spotLight_position - fp);
    float theta = dot(ld, normalize(-spotLight_direction));
    if (theta > spotLight_cutOff) {
        float diff = max(dot(norm, ld), 0.0);
        vec3 ref   = reflect(-ld, norm);
        float spec = pow(max(dot(viewDir, ref), 0.0), 32.0);
        vec3 a = ambientOn  ? spotLight_ambient            : vec3(0.0);
        vec3 d = diffuseOn  ? spotLight_diffuse  * diff    : vec3(0.0);
        vec3 s = specularOn ? spotLight_specular * spec     : vec3(0.0);
        return a + d + s;
    }
    return ambientOn ? spotLight_ambient : vec3(0.0);
}
vec3 CalcFullPhong(vec3 norm, vec3 viewDir) {
    vec3 light = emissive;
    if (directionalLightOn) light += CalcDirLight(norm, viewDir);
    if (pointLightsOn)
        for (int i = 0; i < NR_POINT_LIGHTS; i++)
            light += CalcPointLight(i, norm, FragPos, viewDir);
    if (spotLightOn) light += CalcSpotLight(norm, FragPos, viewDir);
    return light;
}

void main()
{
    vec3 norm    = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 texColor = hasTexture ? texture(texSampler, TexCoord).rgb : objectColor;
    vec3 result;
    if      (texMode == 0) result = CalcFullPhong(norm, viewDir) * objectColor;
    else if (texMode == 1) result = CalcFullPhong(norm, viewDir) * texColor;
    else if (texMode == 2) result = mix(GouraudColor, texColor, 0.5);
    else                   result = mix(CalcFullPhong(norm, viewDir) * objectColor, texColor, 0.5);
    FragColor = vec4(result, 1.0);
}
)";

unsigned int compileShader(unsigned int type, const char* src) {
    unsigned int s = glCreateShader(type);
    glShaderSource(s, 1, &src, NULL); glCompileShader(s);
    int ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) { char log[512]; glGetShaderInfoLog(s, 512, NULL, log); std::cerr << "Shader error:\n" << log << std::endl; }
    return s;
}

unsigned int createShaderProgram() {
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    unsigned int prog = glCreateProgram();
    glAttachShader(prog, vs); glAttachShader(prog, fs); glLinkProgram(prog);
    glDeleteShader(vs); glDeleteShader(fs); return prog;
}
