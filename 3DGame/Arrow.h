#ifndef ARROW_H
#define ARROW_H
#include "Client.h"
#include "Camera.h"
#include "Assimp.h"
namespace Game {
	class Arrow {
	private:
		glm::vec3 pos;
		Model arrow;
		float angle;
		glm::vec3 velocity;
		double clock;
		bool isLive;
		glm::vec3 axis;
		enum direction {
			d_up,
			d_down
		}arrow_direction;
		int shooter;
	public:
		Arrow() {};
		Arrow(const arrow_packet & arrow) : pos(glm::vec3(arrow.x, arrow.y, arrow.z)), velocity(glm::vec3(arrow.velX, arrow.velY, arrow.velZ)),
			clock(arrow.clock), shooter(arrow.shooter) {
		}
		Arrow(const Arrow & other): arrow(other.arrow), pos(other.pos), velocity(other.velocity), clock(other.clock), angle(other.angle), isLive(other.isLive), shooter(other.shooter){}
		Arrow(const Camera & cam, const Model arrow) : arrow(arrow) {
			shooter = -1;
			isLive = true;
			int xfactor = (cam.Front.z > 0) ? -1 : 1;
			int zfactor = (cam.Front.x < 0) ? -1 : 1;
			pos = (cam.Position - (glm::vec3(.5) * cam.Front)) + glm::vec3(xfactor * 0.05, 0, zfactor * 0.05);
			if (!cam.look) {
				velocity = cam.Front;
			}
			else {
				velocity = cam.lastFront;
			}
			clock = glfwGetTime();
		}
		void draw(Shader shader, float dt, bool depthPass) {
			glm::mat4 model;
			double gravity = 8;
			if (isLive && !depthPass) {
				velocity.y = velocity.y - (gravity * ((glfwGetTime() - clock) / 1000.0));
				pos += velocity * glm::vec3(20) * glm::vec3(dt);
			}
			double nextVelY = velocity.y - (gravity * (((glfwGetTime() + dt) - clock) / 1000.0));
			glm::vec3 nextVelocity = velocity;
			nextVelocity.y = isLive ? nextVelY : velocity.y;

			model = glm::translate(model, pos);
			model = glm::scale(model, glm::vec3(0.05));

			glm::vec3 lookat = glm::vec3(nextVelocity * glm::vec3(20) * glm::vec3(dt));
			axis = glm::vec3();
			axis = glm::normalize(glm::cross(velocity, glm::vec3(0, 1, 0)));
			glm::vec3 test = glm::normalize(velocity);
			if (1.0 - abs(test.z) < 0.4) {
				angle = atan(abs(nextVelocity.y / nextVelocity.z));
			}
			else {
				angle = atan(abs(nextVelocity.y / nextVelocity.x));
			}
			if ((pos + lookat).y > pos.y) {
				model = glm::rotate(model, angle + glm::radians(270.f), axis);
				arrow_direction = d_up;
			}
			else {
				model = glm::rotate(model, -angle - glm::radians(90.f), axis);
				arrow_direction = d_down; 
			}

			shader.setMat4("model", model);
			arrow.Draw(shader);
		}
		void setModel(const Model arrow) {
			this->arrow = arrow;
		}
		arrow_packet toPacket() {
			arrow_packet pack{};
			memset(&pack, 0, sizeof(pack));
			pack.x = pos.x;
			pack.y = pos.y;
			pack.z = pos.z;
			pack.velX = velocity.x;
			pack.velY = velocity.y;
			pack.velZ = velocity.z;
			pack.clock = clock;
			pack.shooter = shooter;
			return pack;
		}
		int getShooter() {
			return shooter;
		}
		void collide() {
			isLive = false;
		}
		bool isAlive() { return isLive; }

		glm::vec3 getPoint() {
			glm::vec3 point = glm::vec3(0, .47, 0);
			glm::mat4 rotation = glm::mat4();
			if (arrow_direction == d_up) {
				rotation = glm::rotate(rotation, angle + glm::radians(270.f), axis);
			}
			else {
				rotation = glm::rotate(rotation, -angle - glm::radians(90.f), axis);
			}
			point = GameMath::vectorMatrixMultiply(point, rotation);
			point += pos;
			return point;
		}
		glm::vec3 getPos() { return pos; }
	};
}
#endif // !ARROW_H

