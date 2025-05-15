#pragma once

#include <memory>
#include "Scene.h"
#include "Camera.h"
#include "FreeCameraController.h"

// �h����̏���(�{�[��)�V�[��
class PhysicsBoneScene : public Scene
{
public:
	PhysicsBoneScene();
	~PhysicsBoneScene() override = default;

	// �X�V����
	void Update(float elapsedTime) override;

	// �`�揈��
	void Render(float elapsedTime) override;

	// GUI�`�揈��
	void DrawGUI() override;

private:
	struct Bone
	{
		DirectX::XMFLOAT3	localPosition;
		DirectX::XMFLOAT4	localRotation;
		DirectX::XMFLOAT4X4	worldTransform;
		DirectX::XMFLOAT3	oldWorldPosition;
	};

	Camera								camera;
	FreeCameraController				cameraController;
	Bone								bones[8];
	float								gravity = 0.3f;
	float								maxVelocity = 0.3f;
};
