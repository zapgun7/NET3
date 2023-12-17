#pragma once
#include <common.h>

#include <glm/glm.hpp>

#include <system/component.h>

gdpNamespaceBegin
struct PlayerControllerComponent : public Component
{
public:
	PlayerControllerComponent() { }

	bool moveForward = false;
	bool moveBackward = false;
	bool moveLeft = false;
	bool moveRight = false;

	bool shootDown = false;
	bool shootLeft = false;
	bool shootUp = false;
	bool shootRight = false;

	bool respawn = false;
};
gdpNamespaceEnd
