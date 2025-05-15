#pragma once

#include <memory>
#include "Scene.h"
#include "Camera.h"
#include "Light.h"
#include "Model.h"

// ���̋O�Տ����V�[��
class SwordTrailScene : public Scene
{
public:
	SwordTrailScene();
	~SwordTrailScene() override = default;

	// �X�V����
	void Update(float elapsedTime) override;

	// �`�揈��
	void Render(float elapsedTime) override;

	// GUI�`�揈��
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
