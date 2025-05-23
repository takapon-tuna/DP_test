#pragma once

#include <memory>
#include "Scene.h"
#include "Camera.h"
#include "Light.h"
#include "Model.h"

// 剣の軌跡処理シーン
class SwordTrailScene : public Scene
{
public:
	SwordTrailScene();
	~SwordTrailScene() override = default;

	// 更新処理
	void Update(float elapsedTime) override;

	// 描画処理
	void Render(float elapsedTime) override;

	// GUI描画処理
	void DrawGUI() override;

private:
	Camera								camera;
	LightManager						lightManager;
	std::shared_ptr<Model>				character;
	std::shared_ptr<Model>				weapon;
	std::vector<Model::NodePose>		nodePoses;
	float								animationSeconds = 0;
	bool								spline = false;

	static const int MAX_POLYGON = 32;
	DirectX::XMFLOAT3					trailPositions[2][MAX_POLYGON];
};
