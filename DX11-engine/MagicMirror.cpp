#include "MagicMirror.h"

using namespace std;

MagicMirror::MagicMirror() : GameEntity() {}

MagicMirror::MagicMirror(shared_ptr<Mesh> mesh, shared_ptr<Material> material) 
	: GameEntity(mesh, material) {}