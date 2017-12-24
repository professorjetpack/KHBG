#version 330 core
out vec4 FragColor;

in VS_OUT{
	vec3 fragPos;
	vec3 normal;
	vec2 texCoord;
	vec4 fragPosLightSpace;
	mat3 TBN;
} fs_in;
//uniform sampler2D shadowMap;
uniform sampler2D diffuseTex;
uniform sampler2D normalMap;
uniform sampler2D specularMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform float far_plane;
uniform float near_plane;

uniform bool useNormal;
uniform bool useSpecular;

void main()
{
	vec4 color = texture(diffuseTex, fs_in.texCoord);
	if(color.a < 0.1)
		discard;
	if(distance(viewPos, fs_in.fragPos) < 20){
	vec3 normal = normalize(fs_in.normal);
    vec3 lightColor = vec3(0.9);
    vec3 ambient = 0.2 * color.rgb;
    vec3 lightDir = normalize(lightPos - fs_in.fragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    vec3 viewDir = normalize(viewPos - fs_in.fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
	vec3 specular = spec * lightColor;                    
    vec3 lighting = (ambient * (diffuse + specular)) * color.rgb;
    FragColor = vec4(lighting, 1.0);
	}else{
		FragColor = vec4(color.rgb * 0.2, 1.0);
	}
}