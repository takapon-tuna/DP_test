#pragma once

#include <memory>
#include <string>
#include "Scene.h"
#include "Camera.h"
#include "FreeCameraController.h"

// ���[�X���ʔ��菈���V�[��
class RaceRankingScene : public Scene
{
public:
	RaceRankingScene();
	~RaceRankingScene() override = default;

	// �X�V����
	void Update(float elapsedTime) override;

	// �`�揈��
	void Render(float elapsedTime) override;

	// GUI�`�揈��
	void DrawGUI() override;

private:
	struct CheckPoint
	{
		DirectX::XMFLOAT3	direction;
		DirectX::XMFLOAT3	position;
	};
	struct Car
	{
		std::string			name;
		DirectX::XMFLOAT3	position;
		DirectX::XMFLOAT3	oldPosition;
		DirectX::XMFLOAT3	direction;
		DirectX::XMFLOAT4	color;
		float				timer = 3.0f;
		float				radius = 0.5f;
		float				speed = 1.0f;
		int					nowCheckPointIndex;
		int					ranking = 0;
		int					checkCount = 0;
		float				progress;
	};

	Camera								camera;
	FreeCameraController				cameraController;
	CheckPoint							checkPoints[17];
	Car									cars[3];
	std::vector<Car*>					rankingSortedCars;
};
