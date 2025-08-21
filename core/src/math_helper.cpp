#include "math_helper.h"

#define CMP_EPSILON 0.00001


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


// ==================== THIRD-PARTY SOURCE CODE ==================== //

// After a long time researching, I was able to use and adapt Godot's implementation of movable handles (godot/editor/plugins/gizmos/gizmo_3d_helper.cpp)
// All thanks goes to them and David Eberly for his algorithm.

/**************************************************************************/
/*  geometry_3d.cpp                                                       */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

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

// https://github.com/erich666/GraphicsGems/tree/master?tab=License-1-ov-file#readme
// Note: The code below predates open source

/*
 * EULA: The Graphics Gems code is copyright-protected. In other words, you cannot claim the text of the code as your own
 * and resell it. Using the code is permitted in any program, product, or library, non-commercial or commercial. Giving
 * credit is not required, though is a nice gesture. The code comes as-is, and if there are any flaws or problems with
 * any Gems code, nobody involved with Gems - authors, editors, publishers, or webmasters - are to be held responsible.
 * Basically, don't be a jerk, and remember that anything free comes with no guarantee.
 */

/* 
Fast Ray-Box Intersection
by Andrew Woo
from "Graphics Gems", Academic Press, 1990
*/

#define RIGHT	0
#define LEFT	1
#define MIDDLE	2

bool HitBoundingBox(
	glm::vec3 minB, glm::vec3 maxB,             /*box */
	glm::vec3 origin, glm::vec3 dir,            /*ray */
	glm::vec3 &coord                            /* hit point */
) {
	bool inside = true;
	glm::vec3 quadrant;
	int i;
	int whichPlane;
	glm::vec3 maxT;
	glm::vec3 candidatePlane;

	/* Find candidate planes; this loop can be avoided if
   	rays cast all from the eye(assume perpsective view) */
	for (i = 0; i < 3; i++)
		if(origin[i] < minB[i]) {
			quadrant[i] = LEFT;
			candidatePlane[i] = minB[i];
			inside = false;
		}else if (origin[i] > maxB[i]) {
			quadrant[i] = RIGHT;
			candidatePlane[i] = maxB[i];
			inside = false;
		}else	{
			quadrant[i] = MIDDLE;
		}

	/* Ray origin inside bounding box */
	if (inside)	{
		coord = origin;
		return (true);
	}


	/* Calculate T distances to candidate planes */
	for (i = 0; i < 3; i++)
		if (quadrant[i] != MIDDLE && dir[i] !=0.)
			maxT[i] = (candidatePlane[i]-origin[i]) / dir[i];
		else
			maxT[i] = -1.;

	/* Get largest of the maxT's for final choice of intersection */
	whichPlane = 0;
	for (i = 1; i < 3; i++)
		if (maxT[whichPlane] < maxT[i])
			whichPlane = i;

	/* Check final candidate actually inside box */
	if (maxT[whichPlane] < 0.) return (false);
	for (i = 0; i < 3; i++)
		if (whichPlane != i) {
			coord[i] = origin[i] + maxT[whichPlane] * dir[i];
			if (coord[i] < minB[i] || coord[i] > maxB[i])
				return (false);
		} else {
			coord[i] = candidatePlane[i];
		}
	return true;				/* ray hits box */
}