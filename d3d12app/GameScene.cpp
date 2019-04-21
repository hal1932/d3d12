#include "GameScene.h"
#include "Graphics.h"

GameScene::~GameScene()
{
}

void GameScene::Update()
{
	angle_ += 0.01f;
}

void GameScene::Render(Graphics* pGraphics)
{
}
