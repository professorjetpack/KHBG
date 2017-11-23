#version 440 core
out vec4 FragColor;
in vec2 texCoords;

uniform sampler2D hdrBuffer;
uniform float exposure;
float near_plane = 1.0f;
float far_plane = 120.0f;
float linearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main(){
///*
	vec3 hdrColor = texture(hdrBuffer, texCoords).rgb;
	vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
	float gamma = 1.5;
	result = pow(result, vec3(1.0/gamma));
	FragColor = vec4(result, 1.0);
//*/	
	//debugging:
//	float depthValue = texture(hdrBuffer, texCoords).r;
//	depthValue = linearizeDepth(depthValue) / far_plane;
//	FragColor = vec4(vec3(depthValue), 1.0);
}
