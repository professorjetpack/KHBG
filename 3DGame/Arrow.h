#ifndef ARROW_H
#define ARROW_H
#include "Client.h"
#include "Camera.h"
#include "Assimp.h"
namespace Game {
#define ARROW_DEPTH_PASS 1
#define ARROW_HAS_SHADOW 2
	class Arrow {
	private:
		glm::vec3 pos;
		Model arrow;
		float angle;
		float dt;
		glm::vec3 velocity;
		double clock;
		unsigned long long id;
		bool isLive;
		glm::vec3 axis;
		glm::mat4 model;
		enum direction {
			d_up,
			d_down
		}arrow_direction;
		int shooter;
	public:
		bool newShot;
	public:
		Arrow() {};
		Arrow(const arrow_packet & arrow) : pos(glm::vec3(arrow.x, arrow.y, arrow.z)), velocity(glm::vec3(arrow.velX, arrow.velY, arrow.velZ)),
			clock(arrow.clock), shooter(arrow.shooter), isLive(arrow.isLive), id(arrow.arrowId){}
		Arrow(const Arrow & other): arrow(other.arrow), pos(other.pos), velocity(other.velocity), clock(other.clock), angle(other.angle), isLive(other.isLive), shooter(other.shooter), dt(other.dt), id(other.id){}
		Arrow(const Camera & cam, const Model arrow, uint16_t shooter, double clock) : arrow(arrow), shooter(shooter), clock(clock) {
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
/*			if (client::getServerTime(clock) != 0) {
				clock = glfwGetTime();
			}*/
		}
		void draw(Shader shader, float dt, char shadowParams, double timeNow) {
			if (!(shadowParams & ARROW_DEPTH_PASS) || (shadowParams & ARROW_HAS_SHADOW)) {
				glm::mat4 model = glm::mat4();
				double gravity = 8;
/*				double timeNow;
				if (client::getServerTime(timeNow) != 0) {
					timeNow = glfwGetTime();
				}*/
				if (isLive) {
					velocity.y = velocity.y - (gravity * ((timeNow - clock) / 1000.0));
					pos += velocity * glm::vec3(20) * glm::vec3(dt);
				}
				double nextVelY = velocity.y - (gravity * (((timeNow + dt) - clock) / 1000.0));
				glm::vec3 nextVelocity = velocity;
				nextVelocity.y = isLive ? nextVelY : velocity.y;

				model = glm::translate(model, pos);
				model = glm::scale(model, glm::vec3(0.05));

				glm::vec3 lookat = glm::vec3(nextVelocity * glm::vec3(20) * glm::vec3(dt));
				axis = glm::vec3();
				axis = glm::normalize(glm::cross(velocity, glm::vec3(0, 1, 0)));
				glm::vec3 test = glm::normalize(velocity);
				glm::vec3 rotation = glm::normalize(nextVelocity);
				if (1.0 - abs(test.z) < 0.4) {
					angle = atan(abs(rotation.y / rotation.z));
				}
				else {
					angle = atan(abs(rotation.y / rotation.x));
				}
				if ((pos + lookat).y > pos.y) {
					model = glm::rotate(model, angle + glm::radians(270.f), axis);
					arrow_direction = d_up;
				}
				else {
					model = glm::rotate(model, -angle - glm::radians(90.f), axis);
					arrow_direction = d_down;
				}
				SHADER_SET_MAT4(shader, "model", model);
				arrow.Draw(shader);
			}
			else if(shadowParams & ARROW_HAS_SHADOW && !(shadowParams & ARROW_DEPTH_PASS)){
				model = glm::mat4();
				double gravity = 8;
				double timeNow;
				if (client::getServerTime(timeNow) != 0) {
					timeNow = glfwGetTime();
				}
				if (isLive) {
					velocity.y = velocity.y - (gravity * ((timeNow - clock) / 1000.0));
					pos += velocity * glm::vec3(20) * glm::vec3(dt);
				}
				double nextVelY = velocity.y - (gravity * (((timeNow + dt) - clock) / 1000.0));
				glm::vec3 nextVelocity = velocity;
				nextVelocity.y = isLive ? nextVelY : velocity.y;

				model = glm::translate(model, pos);
				model = glm::scale(model, glm::vec3(0.05));

				glm::vec3 lookat = glm::vec3(nextVelocity * glm::vec3(20) * glm::vec3(dt));
				axis = glm::vec3();
				axis = glm::normalize(glm::cross(velocity, glm::vec3(0, 1, 0)));
				glm::vec3 test = glm::normalize(velocity);
				glm::vec3 rotation = glm::normalize(nextVelocity);
				if (1.0 - abs(test.z) < 0.4) {
					angle = atan(abs(rotation.y / rotation.z));
				}
				else {
					angle = atan(abs(rotation.y / rotation.x));
				}
				if ((pos + lookat).y > pos.y) {
					model = glm::rotate(model, angle + glm::radians(270.f), axis);
					arrow_direction = d_up;
				}
				else {
					model = glm::rotate(model, -angle - glm::radians(90.f), axis);
					arrow_direction = d_down;
				}
				SHADER_SET_MAT4(shader, "model", model);
				arrow.Draw(shader);
				
			}
		}
		inline unsigned long long & getId() { return id; }
		inline void setModel(const Model arrow) {
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
			pack.arrowId = id;
			pack.isLive = isLive;
			return pack;
		}
		inline int getShooter() {
			return shooter;
		}
		inline void collide() {
			isLive = false;
		}
		inline bool isAlive() { return isLive; }
		glm::vec3 getVelocity() {
			return velocity;
		}
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

