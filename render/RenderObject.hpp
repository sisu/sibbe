#pragma once
#include "Model.hpp"
#include "Program.hpp"
#include "util/Matrix.hpp"
#include <map>

struct RenderObject {
	RenderObject(ModelPtr model, ProgramPtr program): model(model), program(program) {}

	Matrix4 transform;
	std::map<std::string, Vec3> paramsv3;

	ModelPtr model;
	ProgramPtr program;
};
