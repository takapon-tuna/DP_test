#pragma once

#include <memory>
#include <DirectXCollision.h>
#include "Scene.h"
#include "Camera.h"
#include "FreeCameraController.h"
#include "HighResolutionTimer.h"
#include "Model.h"

// ��ԕ������C�L���X�g�V�[��
class SpaceDivisionRaycastScene : public Scene
{
public:
	SpaceDivisionRaycastScene();
	~SpaceDivisionRaycastScene() override = default;

	// �X�V����
	void Update(float elapsedTime) override;

	// �`�揈��
	void Render(float elapsedTime) override;

	// GUI�`�揈��
	void DrawGUI() override;

private:
	// ���C�L���X�g
	bool Raycast(
		const DirectX::XMFLOAT3& start,
		const DirectX::XMFLOAT3& end,
		DirectX::XMFLOAT3& hitPosition,
		DirectX::XMFLOAT3& hitNormal);

private:
	struct CollisionMesh
	{
		struct Triangle
		{
			DirectX::XMFLOAT3	positions[3];
			DirectX::XMFLOAT3	normal;
		};
		struct Area
		{
			DirectX::BoundingBox	boundingBox;
			std::vector<int>		triangleIndices;
		};

		std::vector<Triangle>	triangles;
		std::vector<Area>		areas;
	};

private:
	HighResolutionTimer					timer;
	Camera								camera;
	FreeCameraController				cameraController;
	std::shared_ptr<Model>				stage;
	std::shared_ptr<Model>				character;
	DirectX::XMFLOAT3					characterPosition;
	CollisionMesh						collisionMesh;
	bool								spaceDivision = false;

	float	totalTime = 0;
	float	averageTime = 0;
	int		frames = 0;
};
