#version 330 core
out vec4 FragColor;
in vec2 texCoords;

uniform sampler2D hdrBuffer;
uniform float exposure;

uniform int health;
uniform bool damage;
uniform bool clearUp;
uniform float red;
uniform bool dead;

uniform bool mainQuad;

uniform sampler2D text;
uniform vec3 textColor;

float near_plane = 1.0f;
float far_plane = 120.0f;
const float blurX = 1.0f / 1153.0f;
const float blurY = 1.0f / 850.0f;

float linearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main(){
	vec3 color = vec3(0);
	if(dead){
		FragColor = vec4(color, 1.0);
	}else{
		if(health < 45){
			float blurValx = blurX * (1.0 + ((100 - health) / 100.0));
			float blurValy = blurY * (1.0 + ((100 - health) / 100.0));
			for(int x = -3; x <= 3; x++){
				for(int y = -3; y <= 3; y++){
					color += texture(hdrBuffer, vec2(texCoords.x + x * blurValx, texCoords.y + y * blurValy)).rgb;
				}
			}		
		}
		vec3 hdrColor = health >= 45 ? texture(hdrBuffer, texCoords).rgb : color / 49;
		vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
		float gamma = 1.5;
		result = pow(result, vec3(1.0/gamma));
		FragColor = vec4(result, 1.0);
		if(damage){
			FragColor *= vec4(red, 1.0, 1.0, 1.0);
		}else if(clearUp){
			float rr = red < 1.0 ? 1.0 : red;
			FragColor *= vec4(rr, 1.0, 1.0, 1.0);
		}
		if(health < 50){
			FragColor += vec4(1.0 + ((100.0 - health) / 100.0), 0.0 , 0.0, 0.0);
		}
		vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, texCoords).r);
		if(!mainQuad){
			if(sampled.a > 0.1) FragColor = vec4(textColor, 1.0) * sampled;
			else discard;
		}
	}
//*/	
	//debugging:
//	float depthValue = texture(hdrBuffer, texCoords).r;
//	depthValue = linearizeDepth(depthValue) / far_plane;
//	FragColor = vec4(vec3(depthValue), 1.0);
}
