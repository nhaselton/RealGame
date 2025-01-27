#include "def.h"
#include "Entity.h"
#include "renderer/Camera.h"

class Player : public Entity {
public:
	Camera camera;
};

Player* CreatePlayer( Vec3 pos );
void UpdatePlayer( Entity* entity );