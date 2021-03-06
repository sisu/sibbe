#pragma once
#include "Model.hpp"
#include "Program.hpp"
#include "util/Matrix.hpp"
#include <map>

struct RenderObject {
	RenderObject(ModelPtr model, ProgramPtr program): model(model), program(program) {}

	Matrix4 transform;
	std::map<std::string, Vec3> paramsv3;
	std::map<std::string, int> uniform1i;
	std::map<std::string, float> uniform1f;
	std::map<std::string, Vec3> uniformv3;

	ModelPtr model;
	ProgramPtr program;
};
