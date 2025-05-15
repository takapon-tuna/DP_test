#pragma once

#include <memory>
#include "Scene.h"
#include "Camera.h"
#include "FreeCameraController.h"
#include "Model.h"
#include "Light.h"

// ���f���r���[�A�V�[��
class ModelViewerScene : public Scene
{
public:
	ModelViewerScene();
	~ModelViewerScene() override = default;

	// �X�V����
	void Update(float elapsedTime) override;

	// �`�揈��
	void Render(float elapsedTime) override;

	// GUI�`�揈��
	void DrawGUI() override;

private:
	// ���j���[GUI�`��
	void DrawMenuGUI();

	// �q�G�����L�[GUI�`��
	void DrawHierarchyGUI();

	// �v���p�e�BGUI�`��
	void DrawPropertyGUI();

	// �A�j���[�V����GUI�`��
	void DrawAnimationGUI();

	// �}�e���A��GUI�`��
	void DrawMaterialGUI();

private:
	Camera								camera;
	FreeCameraController				cameraController;
	LightManager						lightManager;
	std::shared_ptr<Model>				model;
	Model::Node*						selectionNode = nullptr;
	std::vector<Model::NodePose>		nodePoses;
	bool								animationPlaying = false;
	bool								animationLoop = false;
	float								animationSamplingRate = 60;
	float								animationBlendSeconds = 0;
	float								animationSpeed = 1.0f;
	float								currentAnimationSeconds = 0;
	int									currentAnimationIndex = -1;
	int									shaderId;
};
