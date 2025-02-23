#pragma once
#include <glm/glm.hpp>

// From godot/editor/plugins/gizmos/gizmo_3d_helper.h
void get_closest_points_between_segments(const glm::vec3 &p_p0, const glm::vec3 &p_p1, const glm::vec3 &p_q0, const glm::vec3 &p_q1, glm::vec3 &r_ps, glm::vec3 &r_qt);