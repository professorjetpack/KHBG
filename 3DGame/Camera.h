#pragma once

#include <glfw3.h>
#include <glad/glad.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include <vector>
#define BASE_JUMP_VEL 4.5f
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
//	const float SPEED = 1.5f; when in actual game
	const float SPEED = 3.5f;
	const float SENSITIVTY = 0.1f;
	const float ZOOM = 45.0f;

	class Camera
	{
	private:
		float yPos = 0;
	public:
		glm::vec3 Position;
		glm::vec3 Front;
		glm::vec3 lastFront;
		glm::vec3 walk;
		glm::vec3 Up;
		glm::vec3 Right;
		glm::vec3 WorldUp;
		float Yaw, b4LookYaw;
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

		Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM)
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

			float velocity = MovementSpeed * deltaTime;
			if (direction == FORWARD)
				Position += walk * velocity;
			if (direction == BACKWARD)
				Position -= walk * glm::vec3(velocity * 0.6);
			if (direction == LEFT)
				Position -= Right * glm::vec3(velocity * 0.8);
			if (direction == RIGHT)
				Position += Right * glm::vec3(velocity * 0.8);
			if (direction == FAST)
				Position += walk * (velocity * 2);
			if (direction == FRIGHT)
				Position += Right * glm::vec3(velocity * 1.6);
			if (direction == FLEFT)
				Position -= Right * glm::vec3(velocity * 1.6);

		}
		void applyForces(float dt) {
			if (jump) {
				if (firstJump) {
					firstJump = false;
					airTime = glfwGetTime();
				}
				yJumpVel -= 0.3 * dt * 20;
				Position.y += yJumpVel * dt;
			}
#ifndef FLY
			else if (Position.y > -2.5) {
				Position.y -= 0.3 * dt * 20;


			}
#endif
			if (Position.y < -2.5) {
				Position.y += 0.3 * dt * 20;
				yJumpVel = BASE_JUMP_VEL;
				airTime = glfwGetTime();
				firstJump = true;
				jump = false;
			}
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
			}
#endif
			Right = glm::normalize(glm::cross(Front, WorldUp));  
			Up = glm::normalize(glm::cross(Right, Front));
		}
	};
}

