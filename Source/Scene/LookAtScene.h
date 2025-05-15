#pragma once

#include <memory>
#include "Scene.h"
#include "Camera.h"
#include "Light.h"
#include "Model.h"
#include "FreeCameraController.h"

// ���b�N�A�b�g�����V�[��
class LookAtScene : public Scene
{
public:
	LookAtScene();
	~LookAtScene() override = default;

	// �X�V����
	void Update(float elapsedTime) override;

	// �`�揈��
	void Render(float elapsedTime) override;

	// GUI�`�揈��
	void DrawGUI() override;

private:
	Camera								camera;
	FreeCameraController				cameraController;
	std::shared_ptr<Model>				character;
	std::vector<Model::NodePose>		nodePoses;
	DirectX::XMFLOAT3					headLocalForward = { 0, 0, 1 };
	DirectX::XMFLOAT3					targetPosition = { 0, 0, 0 };
	float								animationSeconds = 0;
};
