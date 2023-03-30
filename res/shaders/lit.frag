#version 330 core

struct Material 
{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
}; 

struct DirLight 
{
    bool enabled;

    vec3 direction;	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight 
{
    bool enabled;
    
    float constant;
    float linear;
    float quadratic;
	
	vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight 
{
    bool enabled;

    vec3 position;
    vec3 direction;
	
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};

#define NR_SPOT_LIGHTS 2

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLight;
uniform SpotLight spotLights[2];
uniform Material material;
uniform samplerCube skybox;

out vec4 FragColor;

// == Function prototypes ==
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);


void main()
{    
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    //Phase 1: directional lighting
    vec3 result = CalcDirLight(dirLight, norm, viewDir);
	
    //Phase 2: point lights
    result += CalcPointLight(pointLight, norm, FragPos, viewDir);
	
    //Phase 3: spot light
    for(int i = 0; i < NR_SPOT_LIGHTS; i++)
	{
        result += CalcSpotLight(spotLights[i], norm, FragPos, viewDir);
	}
	
    // Phase 4: reflections
    vec3 I = normalize(FragPos - viewPos);
    vec3 R = reflect(I, normalize(Normal));

    vec3 reflection = texture(skybox, R).rgb;

    result = reflection * (0.3) + (0.7) * result;
    FragColor = vec4(result, 1.0);
}


vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    if(!light.enabled) return vec3(0.0f);
    vec3 lightDir = normalize(-light.direction);
    //Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    //Specular shading
    vec3 reflectDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, reflectDir), 0.0), material.shininess);
    //Combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    return (ambient + diffuse + specular);
}


vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    if(!light.enabled) return vec3(0.0f);
    vec3 lightDir = normalize(light.position - fragPos);
    //Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    //Specular shading
    vec3 reflectDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, reflectDir), 0.0), material.shininess);
    //Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    //Combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}


vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    if(!light.enabled) return vec3(0.0f);
    vec3 lightDir = normalize(light.position - fragPos);
    //Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    //Specular shading
    vec3 reflectDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, reflectDir), 0.0), material.shininess);
    //Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    //Spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    //Combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}
