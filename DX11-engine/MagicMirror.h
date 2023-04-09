#pragma once

#include "GameEntity.h"

class MagicMirror : public GameEntity
{
public:
	MagicMirror();
	MagicMirror(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);
};