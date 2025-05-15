#pragma once

#include <memory>
#include "Scene.h"
#include "Camera.h"
#include "FreeCameraController.h"

// �Q�{�̃{�[��IK����V�[��
class TwoBoneIKScene : public Scene
{
public:
	TwoBoneIKScene();
	~TwoBoneIKScene() override = default;

	// �X�V����
	void Update(float elapsedTime) override;

	// �`�揈��
	void Render(float elapsedTime) override;

	// GUI�`�揈��
	void DrawGUI() override;

private:
	struct Bone
	{
		Bone*				parent;
		Bone*				child;
		DirectX::XMFLOAT3	localPosition;
		DirectX::XMFLOAT4	localRotation;
		DirectX::XMFLOAT4X4	worldTransform;

		// ���[���h�s��X�V����
		void UpdateWorldTransform()
		{
			DirectX::XMMATRIX LocalRotationTransform = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&localRotation));
			DirectX::XMMATRIX LocalPositionTransform = DirectX::XMMatrixTranslation(localPosition.x, localPosition.y, localPosition.z);
			DirectX::XMMATRIX LocalTransform = DirectX::XMMatrixMultiply(LocalRotationTransform, LocalPositionTransform);
			if (parent != nullptr)
			{
				DirectX::XMMATRIX ParentWorldTransform = DirectX::XMLoadFloat4x4(&parent->worldTransform);
				DirectX::XMMATRIX WorldTransform = DirectX::XMMatrixMultiply(LocalTransform, ParentWorldTransform);
				DirectX::XMStoreFloat4x4(&worldTransform, WorldTransform);
			}
			else
			{
				DirectX::XMStoreFloat4x4(&worldTransform, LocalTransform);
			}
		}
		// ���g�ȉ��̃��[���h�s��X�V����
		void UpdateWorldTransforms()
		{
			Bone* bone = this;
			while (bone != nullptr)
			{
				bone->UpdateWorldTransform();
				bone = bone->child;
			}
		}
	};

	Camera								camera;
	FreeCameraController				cameraController;
	Bone								bones[4];
	DirectX::XMFLOAT4X4					targetWorldTransform;
	DirectX::XMFLOAT4X4					poleLocalTransform;
	DirectX::XMFLOAT4X4					poleWorldTransform;
};
