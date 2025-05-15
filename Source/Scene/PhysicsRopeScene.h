#pragma once

#include <memory>
#include "Scene.h"
#include "Camera.h"
#include "FreeCameraController.h"

// �h����̏���(���[�v)�V�[��
class PhysicsRopeScene : public Scene
{
public:
	PhysicsRopeScene();
	~PhysicsRopeScene() override = default;

	// �X�V����
	void Update(float elapsedTime) override;

	// �`�揈��
	void Render(float elapsedTime) override;

	// GUI�`�揈��
	void DrawGUI() override;

private:
	struct Joint
	{
		DirectX::XMFLOAT3	position;
		DirectX::XMFLOAT3	oldPosition;
	};

	Camera								camera;
	FreeCameraController				cameraController;

	float		jointInterval = 1.0f;
	Joint		joints[5];
	float		gravity = 0.3f;
};
