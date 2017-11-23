#version 440 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in vec3 tangent;

out vec2 TexCoords;

out VS_OUT {
	vec3 fragPos;
	vec3 normal;
	vec2 texCoord;
	vec4 fragPosLightSpace;
	mat3 TBN;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;

void main(){
	vec3 T = normalize(vec3(model * vec4(tangent, 0.0)));
	vec3 N = normalize(vec3(model * vec4(normal, 0.0)));
	vec3 B = cross(T, N);
	vs_out.TBN = mat3(T, B, N);
	vs_out.fragPos = vec3(model * vec4(pos, 1.0));
	vs_out.normal = transpose(inverse(mat3(model))) * normal;
	vs_out.texCoord = texCoord;
	vs_out.fragPosLightSpace = lightSpaceMatrix * vec4(vs_out.fragPos, 1.0);
	gl_Position = projection * view * model * vec4(pos, 1.0);
}