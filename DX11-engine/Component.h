#pragma once
#include <memory>

class GameEntity; // forward declare

class Component
{
public:
	std::shared_ptr<GameEntity> gameObject;
};