#pragma once

#include <glad/glad.h>
#include <glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include <vector>
#include "Util.h"
#define BASE_JUMP_VEL 4.5f
//#define TESTING
#ifdef TESTING
	#define FLY
	#define HIGHSPEED
	#define NOCLIP
#endif
//#define FLY
namespace Game {
	enum Camera_Movement {
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT,
		STOP,
		FAST,
		FLEFT,
		FRIGHT
	};

	const float YAW = -90.0f;
	const float PITCH = 0.0f;
#ifdef HIGHSPEED
	const float SPEED = 7.5f;
#else
	const float SPEED = 2.5f;
#endif
	const float SENSITIVTY = 0.1f;
	const float ZOOM = 45.0f;

	class Camera
	{
	private:
		float yPos = 0;		
		Camera_Movement dir;
	public:
		glm::vec3 Position;
		glm::vec3 Front;
		glm::vec3 lastFront;
		glm::vec3 walk;
		glm::vec3 Up;
		glm::vec3 Right;
		glm::vec3 WorldUp;
		float Yaw, b4LookYaw;
		glm::vec3 oldPos;
		float Pitch;
		float MovementSpeed;
		float MouseSensitivity;
		float Zoom;
		bool look;
		bool jump = false;
		bool firstJump = true;
		float Jump_Height = 2.2f;
		double airTime;
		float yJumpVel = BASE_JUMP_VEL;
		bool move, fastMove, shouldPlay = false;
		double moveTime = 0;

		Camera(glm::vec3 position = glm::vec3(-27.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM)
		{
			Position = position;
			WorldUp = up;
			Yaw = yaw;	
			Pitch = pitch;
			look = false;
			updateCameraVectors();
		}
		Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM)
		{
			Position = glm::vec3(posX, posY, posZ);
			WorldUp = glm::vec3(upX, upY, upZ);
			Yaw = yaw;
			Pitch = pitch;
			updateCameraVectors();
		}

		glm::mat4 GetViewMatrix()
		{
			return glm::lookAt(Position, Position + Front, Up);
		}

		void ProcessKeyboard(Camera_Movement direction, float deltaTime)
		{
			if(M_DISTANCE(Position, oldPos) > 1) oldPos = Position;
			dir = direction;
			float velocity = MovementSpeed * deltaTime;
			if (direction == FORWARD) {
				Position += walk * velocity;
				move = true; fastMove = false;
			}
			else if (direction == BACKWARD) {
				Position -= walk * glm::vec3(velocity * 0.6);
				move = true; fastMove = false;
			}
			else if (direction == LEFT) {
				Position -= Right * glm::vec3(velocity * 0.8);
				move = true; fastMove = false;
			}
			else if (direction == RIGHT) {
				Position += Right * glm::vec3(velocity * 0.8);
				move = true; fastMove = false;
			}
			else if (direction == FAST) {
				Position += walk * (velocity * 2);
				move = false; fastMove = true;
			}
			else if (direction == FRIGHT) {
				Position += Right * glm::vec3(velocity * 1.6);
				move = false; fastMove = true;
			}
			else if (direction == FLEFT) {
				Position -= Right * glm::vec3(velocity * 1.6);
				move = false; fastMove = true;
			}
			else {
				move = false; fastMove = false;
			}

		}
		void moveBack() {
#ifndef NOCLIP
			Position = oldPos;
#endif
		}
		void applyForces(float dt, const std::vector<BoundingBox> & hitBoxes) {
			if (Position.x <= -50) Position.x = -50;
			else if (Position.x >= 50) Position.x = 50;
			if (Position.z <= -50) Position.z = -50;
			else if (Position.z >= 50) Position.z = 50;
			if (jump) {
				move = false; fastMove = false;
				if (firstJump) {
					shouldPlay = true;
					firstJump = false;
					airTime = glfwGetTime();
				}
				else {
					shouldPlay = false;
				}
				yJumpVel -= 0.3 * dt * 20;
				Position.y += yJumpVel * dt;
			}
#ifndef FLY
			else if (Position.y > -2.5) {
				Position.y -= 0.3 * dt * 20;
			}
#endif
#ifndef NOCLIP
			if (Position.y < -2.5) {
				if (jump) shouldPlay = true;
				else shouldPlay = false;
				Position.y += 0.3 * dt * 20;
				yJumpVel = BASE_JUMP_VEL;
				airTime = glfwGetTime();
				firstJump = true;
				jump = false;
			}
			for (int i = 0; i < hitBoxes.size(); i++) {
				glm::vec3 min = hitBoxes[i].min;
				glm::vec3 max = hitBoxes[i].max;
				if (hitBoxes[i][Position]) {
					firstJump = true;
					jump = false;
					yJumpVel = BASE_JUMP_VEL;
//					Position = oldPos;
/*					if (Position.x > min.x && Position.x <= max.x - ((max.x - min.x) / 2.0)) Position.x = min.x;
					else if (Position.x > min.x + ((max.x - min.x) / 2.0) && Position.x < max.x) Position.x = max.x;
					else if (Position.z > min.z && Position.z <= max.z - ((max.z - min.z) / 2.0)) Position.z = min.z;
					else if (Position.z > min.z + ((max.z - min.z) / 2.0) && Position.z < max.z) Position.z = max.z;
					else if (Position.y > min.y && Position.y <= max.y - ((max.y - min.y) / 2.0)) Position.y = min.y;
					else if (Position.y > min.y + ((max.y - min.y) / 2.0) && Position.y < max.y) Position.y = max.y;
					*/
					glm::vec3 checkPos = Position;
					if (oldPos.y - 0.5 >= max.y) Position.y = max.y;
					else if (oldPos.y <= min.y) Position.y = min.y;
					if (oldPos.x <= min.x) Position.x = min.x;
					else if (oldPos.x >= max.x) Position.x = max.x;
					if (oldPos.z <= min.z) Position.z = min.z;
					else if (oldPos.z >= max.z) Position.z = max.z;
					if (Position == checkPos) Position = oldPos;
				}
			}
			
#endif
		}

		void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
		{
			xoffset *= MouseSensitivity;
			yoffset *= MouseSensitivity;

			Yaw += xoffset;
			Pitch -= yoffset;

			if (constrainPitch)
			{
				if (Pitch > 89.0f)
					Pitch = 89.0f;
				if (Pitch < -89.0f)
					Pitch = -89.0f;
			}
			if (look) {
				if (abs(b4LookYaw - Yaw) > 100.0) {
					if (Yaw < b4LookYaw) {
						Yaw = b4LookYaw - 100;
					}
					else {
						Yaw = b4LookYaw + 100;
					}
				}
			}

			updateCameraVectors();
		}

		void ProcessMouseScroll(float yoffset)
		{
			if (Zoom >= 1.0f && Zoom <= 45.0f)
				Zoom -= yoffset;
			if (Zoom <= 1.0f)
				Zoom = 1.0f;
			if (Zoom >= 45.0f)
				Zoom = 45.0f;
		}
	private:
		void updateCameraVectors()
		{
			glm::vec3 front;
			front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
			front.y = sin(glm::radians(Pitch));
			front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
			Front = glm::normalize(front);
			if (!look) {
				lastFront = Front;
				walk = Front;
#ifndef FLY
				walk.y = 0;
#endif
			}

			Right = glm::normalize(glm::cross(Front, WorldUp));  
			Up = glm::normalize(glm::cross(Right, Front));
		}
	};
}

