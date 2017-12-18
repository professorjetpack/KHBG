#version 330
layout (location = 0) in vec3 pos;
out vec3 vOut_color;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec3 color;

void main(){
	gl_Position = projection * view * model * vec4(pos, 1.0);
	vOut_color = color;
}