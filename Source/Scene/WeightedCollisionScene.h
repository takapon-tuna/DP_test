#pragma once

#include <memory>
#include "Scene.h"
#include "Camera.h"
#include "FreeCameraController.h"

// �d�݂̂���Փˏ����V�[��
class WeightedCollisionScene : public Scene
{
public:
	WeightedCollisionScene();
	~WeightedCollisionScene() override = default;

	// �X�V����
	void Update(float elapsedTime) override;

	// �`�揈��
	void Render(float elapsedTime) override;

	// GUI�`�揈��
	void DrawGUI() override;

private:
	struct Object
	{
		DirectX::XMFLOAT3	position = { 0, 0, 0 };
		float				radius = 0.5f;
		float				weight = 5.0f;
		DirectX::XMFLOAT4	color = { 1, 1, 0, 1 };
	};

	Camera								camera;
	FreeCameraController				cameraController;
	Object								objs[2];
};
