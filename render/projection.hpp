#pragma once
#include <util/Matrix.hpp>
Matrix4 orthoM(float left, float right, float bottom, float top, float near, float far);
Matrix4 perspectiveM(float fovy, float aspect, float zNear, float zFar);
