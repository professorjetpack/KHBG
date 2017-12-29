#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
//layout (location = 3) in vec3 tangent;


out VS_OUT {
	vec3 fragPos;
	vec3 normal;
	vec2 texCoord;
	vec4 fragPosLightSpace;	
	vec3 lightPos;
	vec3 viewPos;
//	mat3 TBN;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;

uniform vec3 viewPos;
uniform vec3 lightPos;


void main(){
//	vec3 T = normalize(vec3(model * vec4(tangent, 0.0)));
//	vec3 N = normalize(vec3(model * vec4(normal, 0.0)));
//	T = normalize(T - dot(T, N) * N);
//	vec3 B = cross(N, T);
//	vs_out.TBN = mat3(T, B, N);
	vs_out.fragPos = vec3(model * vec4(pos, 1.0));
	vs_out.normal = transpose(inverse(mat3(model))) * normal;
	vs_out.normal = normalize(vs_out.normal);		
	vs_out.fragPosLightSpace = lightSpaceMatrix * vec4(vs_out.fragPos, 1.0);
	vs_out.texCoord = texCoord;
	vs_out.lightPos = lightPos;
	vs_out.viewPos = viewPos;
	gl_Position = projection * view * model * vec4(pos, 1.0);
}