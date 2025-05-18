#include "math_helper.h"

#define CMP_EPSILON 0.00001

// After a long time researching, I was able to use and adapt Godot's implementation of movable handles (godot/editor/plugins/gizmos/gizmo_3d_helper.cpp)
// All thanks goes to them and David Eberly for his algorithm.

void get_closest_points_between_segments(const glm::vec3 &p_p0, const glm::vec3 &p_p1, const glm::vec3 &p_q0, const glm::vec3 &p_q1, glm::vec3 &r_ps, glm::vec3 &r_qt) {
    // Based on David Eberly's Computation of Distance Between Line Segments algorithm.

	glm::vec3 p = p_p1 - p_p0;
	glm::vec3 q = p_q1 - p_q0;
	glm::vec3 r = p_p0 - p_q0;

	float a = glm::dot(p, p);
	float b = glm::dot(p, q);
	float c = glm::dot(q, q);
	float d = glm::dot(p, r);
	float e = glm::dot(q, r);

	float s = 0.0f;
	float t = 0.0f;

	float det = a * c - b * b;
	if (det > CMP_EPSILON) {
		// Non-parallel segments
		float bte = b * e;
		float ctd = c * d;

		if (bte <= ctd) {
			// s <= 0.0f
			if (e <= 0.0f) {
				// t <= 0.0f
				s = (-d >= a ? 1 : (-d > 0.0f ? -d / a : 0.0f));
				t = 0.0f;
			} else if (e < c) {
				// 0.0f < t < 1
				s = 0.0f;
				t = e / c;
			} else {
				// t >= 1
				s = (b - d >= a ? 1 : (b - d > 0.0f ? (b - d) / a : 0.0f));
				t = 1;
			}
		} else {
			// s > 0.0f
			s = bte - ctd;
			if (s >= det) {
				// s >= 1
				if (b + e <= 0.0f) {
					// t <= 0.0f
					s = (-d <= 0.0f ? 0.0f : (-d < a ? -d / a : 1));
					t = 0.0f;
				} else if (b + e < c) {
					// 0.0f < t < 1
					s = 1;
					t = (b + e) / c;
				} else {
					// t >= 1
					s = (b - d <= 0.0f ? 0.0f : (b - d < a ? (b - d) / a : 1));
					t = 1;
				}
			} else {
				// 0.0f < s < 1
				float ate = a * e;
				float btd = b * d;

				if (ate <= btd) {
					// t <= 0.0f
					s = (-d <= 0.0f ? 0.0f : (-d >= a ? 1 : -d / a));
					t = 0.0f;
				} else {
					// t > 0.0f
					t = ate - btd;
					if (t >= det) {
						// t >= 1
						s = (b - d <= 0.0f ? 0.0f : (b - d >= a ? 1 : (b - d) / a));
						t = 1;
					} else {
						// 0.0f < t < 1
						s /= det;
						t /= det;
					}
				}
			}
		}
	} else {
		// Parallel segments
		if (e <= 0.0f) {
			s = (-d <= 0.0f ? 0.0f : (-d >= a ? 1 : -d / a));
			t = 0.0f;
		} else if (e >= c) {
			s = (b - d <= 0.0f ? 0.0f : (b - d >= a ? 1 : (b - d) / a));
			t = 1;
		} else {
			s = 0.0f;
			t = e / c;
		}
	}

	r_ps = (1 - s) * p_p0 + s * p_p1;
	r_qt = (1 - t) * p_q0 + t * p_q1;
}

void expandAABB(glm::vec3& min, glm::vec3& max, glm::vec3 point) {
	min = glm::vec3(glm::min(min.x, point.x), glm::min(min.y, point.y), glm::min(min.z, point.z));
	max = glm::vec3(glm::max(max.x, point.x), glm::max(max.y, point.y), glm::max(max.z, point.z));
}

void computeAABBFromPoints(glm::vec3& min, glm::vec3& max, glm::vec3* points, int count) {
	if (count == 0) return;

	min = points[0];
	max = points[0];

	for (int i = 0; i < count; i++) {
		min = glm::vec3(glm::min(min.x, points[i].x), glm::min(min.y, points[i].y), glm::min(min.z, points[i].z));
		max = glm::vec3(glm::max(max.x, points[i].x), glm::max(max.y, points[i].y), glm::max(max.z, points[i].z));
	}
}

void getAABBCoords(glm::vec3 &pos, glm::vec3 &size, glm::vec3 min, glm::vec3 max) {
	pos = (max + min) / 2.f;
	size = (max - min);
}