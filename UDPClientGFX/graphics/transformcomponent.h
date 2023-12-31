#pragma once

#include <common.h>

#include <system/component.h>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

gdpNamespaceBegin

struct TransformComponent : public Component
{
public:
	TransformComponent() { }
	TransformComponent(const glm::vec3& p, const glm::vec3& s, const glm::quat& r)
		: position(p), scale(s), orientation(r) { }
	glm::vec3 position;
	glm::vec3 scale;
	glm::quat orientation;
};

gdpNamespaceEnd