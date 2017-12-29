#version 330 core
out vec4 FragColor;

in VS_OUT{
	vec3 fragPos;
	vec3 normal;
	vec2 texCoord;
	vec4 fragPosLightSpace;
	vec3 lightPos;
	vec3 viewPos;	
//	mat3 TBN;
} fs_in;
uniform sampler2D shadowMap;
uniform sampler2D diffuseTex;
uniform sampler2D normalMap;
uniform sampler2D specularMap;

uniform float far_plane;
uniform float near_plane;

//uniform bool useNormal;
//uniform bool useSpecular;

uniform bool medium;
uniform bool low;

float linearizeDepth(float depth){
	float z = depth * 2.0 - 1.0; //convert to NDC
	return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));

}
float shadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
	if(projCoords.z > 1.0){
		return 0.0;
	}
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
	float bias = 0.002; //max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
     
    return shadow;
}

void main()
{
	vec4 color = texture(diffuseTex, fs_in.texCoord);
	if(color.a < 0.1)
		discard;
	if(((medium && distance(fs_in.fragPos, fs_in.viewPos) < 35) || !medium) && !low){
	vec3 normal = fs_in.normal;
//	if(useNormal){
//		normal = texture(normalMap, fs_in.texCoord).rgb;
//		normal = normalize(normal * 2.0 - 1.0);
//		normal = normalize(fs_in.TBN * normal);
//	}else{
//		normal = normalize(fs_in.normal);
//	}
    vec3 lightColor = vec3(0.9);
    vec3 ambient = 0.2 * color.rgb;
    vec3 lightDir = normalize(fs_in.lightPos - fs_in.fragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    vec3 viewDir = normalize(fs_in.viewPos - fs_in.fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
	vec3 specular = spec * lightColor;
//	if(useSpecular){
//		specular = lightColor * (spec * vec3(texture(specularMap, fs_in.texCoord)));
//	}else{
//		specular = spec * lightColor;  
//	}     
    float shadow = shadowCalculation(fs_in.fragPosLightSpace, normal, lightDir);                  
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color.rgb;


    FragColor = vec4(lighting, 1.0);
	}else{
		FragColor = vec4(color.rgb, 1.0);
	}
}