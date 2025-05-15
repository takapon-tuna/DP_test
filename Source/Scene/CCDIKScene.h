#pragma once

#include <memory>
#include "Scene.h"
#include "Camera.h"
#include "FreeCameraController.h"

// CCD-IK�����V�[��
class CCDIKScene : public Scene
{
public:
	CCDIKScene();
	~CCDIKScene() override = default;

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
	};

	Camera								camera;
	FreeCameraController				cameraController;
	Bone								bones[5];
	DirectX::XMFLOAT4X4					targetTransform;
	int									quality = 5;
};
