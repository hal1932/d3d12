#pragma once

#include <lib.h>
#include <memory>

class Graphics;
class ModelPass;

class GameScene
{
public:
	~GameScene();

	void Update();
	void Render(Graphics* pGraphics);

private:
	float angle_ = 0.0f;
	//std::unique_ptr<ModelPass> modelPassPtr_;
};

