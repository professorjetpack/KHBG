#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#define M_C_SMALL_NUM 0.00000001
#define M_C_DEGENERATE -0xc786
#define M_C_SAME_PLANE 0x8f42
#define M_C_INTERSECT 0x78ecd
#define M_C_NO_INTERSECT 0x63cd
#define M_C_LINE_INTERSECT 0x63dd
#define M_C_RAY_POS 0
#define M_C_RAY_OFFSET 1

#define M_DISTANCE(p1, p2) (sqrt((((p1).x - (p2).x) * ((p1).x - (p2).x)) + (((p1).y - (p2).y) * ((p1).y - (p2).y)) + (((p1).z - (p2).z) * ((p1).z - (p2).z))))
#define M_DISTANCE_SQUARED(p1, p2) ((((p1).x - (p2).x) * ((p1).x - (p2).x)) + (((p1).y - (p2).y) * ((p1).y - (p2).y)) + (((p1).z - (p2).z) * ((p1).z - (p2).z)))
#define M_CENTROID(TRIANGLE) (glm::vec3{((TRIANGLE)[0].x + (TRIANGLE)[1].x + (TRIANGLE)[2].x) / 3.0, \
		((TRIANGLE)[0].y + (TRIANGLE)[1].y + (TRIANGLE)[2].y) / 3.0, \
		((TRIANGLE)[0].z + (TRIANGLE)[1].z + (TRIANGLE)[2].z) / 3.0})
#define M_TRGDISTANCE(TRIANGLE, VECTOR) (M_DISTANCE(M_CENTROID(TRIANGLE), (VECTOR)))
struct BoundingBox {
	glm::vec3 max;
	glm::vec3 min;
	bool operator[](const glm::vec3 & other) const {
		glm::vec3 tmax = this->max - glm::vec3(-50, -3, -50);
		glm::vec3 tmin = this->min - glm::vec3(-50, -3, -50);
		glm::vec3 tother = other - glm::vec3(-50, -3, -50);
		if ((tother.x > tmin.x && tother.x < tmax.x) &&
			(tother.y > tmin.y && tother.y < tmax.y) &&
			(tother.z > tmin.z && tother.z < tmax.z)) {
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
	BoundingBox(glm::vec3 min, glm::vec3 max) : max(max), min(min){}
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
			return M_C_DEGENERATE;
		}
		dir = ray[1] - ray[0];
		w0 = ray[0] - triangle[0];
		a = -glm::dot(n, w0);
		b = glm::dot(n, dir);
		if (fabs(b) < M_C_SMALL_NUM) {
			if (a == 0) return M_C_SAME_PLANE;
			else return M_C_NO_INTERSECT;
		}
		r = a / b;
		if (r < 0.0) return M_C_NO_INTERSECT;
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
		if (s < 0.0 || s > 1.0) return M_C_NO_INTERSECT;
		t = (uv * wu - uu * wv) / D;
		if (t < 0.0 || (s + t) > 1.0) return M_C_NO_INTERSECT;

		return M_C_INTERSECT;
	}
	int rayTriangleCollision2(glm::vec3 * ray, glm::vec3 * triangle) {
		glm::vec3 e1, e2, h, s, q;
		float a, f, u, v, t;
		e1 = triangle[1] - triangle[0];
		e2 = triangle[2] - triangle[0];
		h = glm::cross(ray[1], e2);
		a = glm::dot(e1, h);
		if (a > -0.0000001 && a < 0.0000001) return M_C_NO_INTERSECT;
		f = 1 / a;
		s = ray[0] - triangle[0];
		u = f * glm::dot(s, h);
		if (u < 0.0 || u > 1.0) return M_C_NO_INTERSECT;
		q = glm::cross(s, e1);
		v = f * glm::dot(ray[1], q);
		if (v < 0.0 || v > 1.0) return M_C_NO_INTERSECT;
		t = f * glm::dot(e2, q);
		if (t > 0.0000001) return M_C_INTERSECT; //ray intersection
		else return M_C_LINE_INTERSECT; //line intersection and no ray intersection

	}
	glm::vec3 vectorMatrixMultiply(glm::vec3 vector, glm::mat4 matrix) {
		glm::vec4 vec = glm::vec4(vector, 1.0);
		glm::vec4 buffer;
		for (int i = 0; i < 4; i+= 2) {
			for (int j = 0; j < 4; j++) {
				buffer[i] += matrix[i][j] * vec[j];
				buffer[i + 1] += matrix[i + 1][j] * vec[j];
			}
		}
		return glm::vec3(buffer);
	}
#define VECTOR_MATRIX_MULTIPLY(flArray3, flArray16) (vectorMatrixMultiply(glm::vec3((flArray3)[0], (flArray3)[1], (flArray3)[2]), glm::mat4((flArray16)[0], (flArray16)[1],(flArray16)[2],(flArray16)[3],(flArray16)[4],(flArray16)[5],(flArray16)[6],(flArray16)[7],(flArray16)[8],(flArray16)[9],(flArray16)[10],(flArray16)[11],(flArray16)[12],(flArray16)[13],(flArray16)[14],(flArray16)[15])))
}