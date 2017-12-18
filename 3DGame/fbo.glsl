#version 330 core
layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 texCoord;

out vec2 texCoords;
uniform bool shake;
uniform float shakeTime;
uniform mat4 projection;
uniform mat4 model;

void main(){
	texCoords = texCoord;
	gl_Position = projection * vec4(pos, 1.0, 1.0);
	if(shake){
		gl_Position.x += cos(shakeTime * 10) * 0.01;
		gl_Position.y += cos(shakeTime * 15) * 0.01;
	}
}