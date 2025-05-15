#pragma once

#include <memory>
#include "Scene.h"
#include "Camera.h"
#include "FreeCameraController.h"

// �n�`�R���W�����V�[��
class SphereVsTriangleCollisionScene : public Scene
{
public:
	SphereVsTriangleCollisionScene();
	~SphereVsTriangleCollisionScene() override = default;

	// �X�V����
	void Update(float elapsedTime) override;

	// �`�揈��
	void Render(float elapsedTime) override;

	// GUI�`�揈��
	void DrawGUI() override;

private:
	// ���ƎO�p�`�Ƃ̌����𔻒肷��
	static bool SphereIntersectTriangle(
		const DirectX::XMFLOAT3& sphereCenter,
		const float sphereRadius,
		const DirectX::XMFLOAT3& triangleVertexA,
		const DirectX::XMFLOAT3& triangleVertexB,
		const DirectX::XMFLOAT3& triangleVertexC,
		DirectX::XMFLOAT3& hitPosition,
		DirectX::XMFLOAT3& hitNormal);

private:
	struct Object
	{
		DirectX::XMFLOAT3	position = { 0, 0, 0 };
		float				radius = 0.5f;
		float				weight = 0.5f;
		DirectX::XMFLOAT4	color = { 1, 1, 0, 1 };
	};


	std::unique_ptr<Model>				terrain;

	Camera								camera;
	FreeCameraController				cameraController;
	Object								obj;
};
