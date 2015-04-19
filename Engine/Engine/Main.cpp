#include "Engine.h"

class Renderer;
class PhysicsEngine;

Renderer* renderer;
PhysicsEngine* physicsEngine;

int main(){
	Engine::EngineClass* program = new Engine::EngineClass("Engine",1024,600);

	program->run();

	delete program;
    return 0;
}