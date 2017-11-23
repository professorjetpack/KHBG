#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#define M_SMALL_NUM 0.00000001
#define M_DEGENERATE -0xc786
#define M_SAME_PLANE 0x8f42
#define M_INTERSECT 0x78ecd
#define M_NO_INTERSECT 0x63cd
#define M_RAY_POS 0
#define M_RAY_OFFSET 1
struct BoundingBox {
	glm::vec3 max;
	glm::vec3 min;
	bool operator[](const glm::vec3 & other) const {
		if (other.x > min.x && other.x < max.x &&
			other.y > min.y && other.y < max.y &&
			other.z > min.z && other.z < min.z) {
			return true;
		}
		return false;
	}
	bool operator[](const BoundingBox & other) const {
		if (max.x > other.min.x &&
			min.x < other.max.x &&
			max.y > other.min.y &&
			min.y < other.max.y &&
			max.z > other.min.z &&
			min.z < other.max.z) {
			return true;
		}
		return false;
	}
};
namespace GameMath {
	glm::vec3 slerp(glm::vec3 start, glm::vec3 end, float percent) {
		float dot = glm::dot(start, end);
		glm::clamp(dot, -1.0f, 1.0f);
		float theta = glm::acos(dot) * percent;
		glm::vec3 relativeVec = end - start * dot;
		relativeVec = glm::normalize(relativeVec);
		return ((start * glm::cos(theta))) + (relativeVec * glm::sin(theta));
	}
	int rayTriangleCollision(glm::vec3 * ray, glm::vec3 * triangle) {
		glm::vec3 u, v, n;
		glm::vec3 dir, w0, w;
		float r, a, b;
		u = triangle[1] - triangle[0];
		v = triangle[2] - triangle[0];
		n = glm::cross(u, v);
		if (n == glm::vec3(0)) {
			return M_DEGENERATE;
		}
		dir = ray[1] - ray[0];
		w0 = ray[0] - triangle[0];
		a = -glm::dot(n, w0);
		b = glm::dot(n, dir);
		if (fabs(b) < M_SMALL_NUM) {
			if (a == 0) return M_SAME_PLANE;
			else return M_NO_INTERSECT;
		}
		r = a / b;
		if (r < 0.0) return M_NO_INTERSECT;
		glm::vec3 intersect = ray[0] + r * dir;
		float uu, uv, vv, wu, wv, D;
		uu = glm::dot(u, u);
		uv = glm::dot(u, v);
		vv = glm::dot(v, v);
		w = intersect - triangle[0];
		wu = glm::dot(w, u);
		wv = glm::dot(w, v);
		D = uv * uv - uu * vv;

		float s, t;
		s = (uv * wv - vv * wu) / D;
		if (s < 0.0 || s > 1.0) return M_NO_INTERSECT;
		t = (uv * wu - uu * wv) / D;
		if (t < 0.0 || (s + t) > 1.0) return M_NO_INTERSECT;

		return M_INTERSECT;
	}
	bool rayTriangleCol2(glm::vec3 * ray, glm::vec3 * triangle) {
		glm::vec3 e1, e2, h, s, q;
		float a, f, u, v, t;
		e1 = triangle[1] - triangle[0];
		e2 = triangle[2] - triangle[0];
		h = glm::cross(ray[1], e2);
		a = glm::dot(e1, h);
		if (a > -0.0000001 && a < 0.0000001) return false;
		f = 1 / a;
		s = ray[0] - triangle[0];
		u = f * glm::dot(s, h);
		if (u < 0.0 || u > 1.0) return false;
		q = glm::cross(s, e1);
		v = f * glm::dot(ray[1], q);
		if (v < 0.0 || v > 1.0) return false;
		t = f * glm::dot(e2, q);
		if (t > 0.0000001) return false; //ray intersection
		else return false; //line intersection and no ray intersection

	}
}