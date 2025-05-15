#include <algorithm>
#include <imgui.h>
#include <ImGuizmo.h>
#include "Graphics.h"
#include "Scene/PhysicsBoneScene.h"

// �R���X�g���N�^
PhysicsBoneScene::PhysicsBoneScene()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	float screenWidth = Graphics::Instance().GetScreenWidth();
	float screenHeight = Graphics::Instance().GetScreenHeight();

	// �J�����ݒ�
	camera.SetPerspectiveFov(
		DirectX::XMConvertToRadians(45),	// ��p
		screenWidth / screenHeight,			// ��ʃA�X�y�N�g��
		0.1f,								// �j�A�N���b�v
		1000.0f								// �t�@�[�N���b�v
	);
	camera.SetLookAt(
		{ 10, 3, 0 },		// ���_
		{ 0, 3, 0 },		// �����_
		{ 0, 1, 0 }			// ��x�N�g��
	);
	cameraController.SyncCameraToController(camera);

	// �{�[���f�[�^������
	for (int i = 0; i < _countof(bones); ++i)
	{
		Bone& bone = bones[i];

		Bone* parent = nullptr;
		if (i == 0)
		{
			bone.localPosition = { 0, 3, 0 };
			bone.localRotation = { 0, 0, 0, 1 };
		}
		else
		{
			bone.localPosition = { 0, 0, 1 };
			bone.localRotation = { 0, 0, 0, 1 };

			parent = &bones[i - 1];
		}

		// �s��v�Z
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(bone.localPosition.x, bone.localPosition.y, bone.localPosition.z);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&bone.localRotation));
		DirectX::XMMATRIX LocalTransform = DirectX::XMMatrixMultiply(R, T);
		DirectX::XMMATRIX ParentWorldTransform = parent != nullptr ? DirectX::XMLoadFloat4x4(&parent->worldTransform) : DirectX::XMMatrixIdentity();
		DirectX::XMMATRIX WorldTransform = DirectX::XMMatrixMultiply(LocalTransform, ParentWorldTransform);
		DirectX::XMStoreFloat4x4(&bone.worldTransform, WorldTransform);

		bone.oldWorldPosition.x = bone.worldTransform._41;
		bone.oldWorldPosition.y = bone.worldTransform._42;
		bone.oldWorldPosition.z = bone.worldTransform._43;
	}

}

// �X�V����
void PhysicsBoneScene::Update(float elapsedTime)
{
	// �J�����X�V����
	cameraController.Update();
	cameraController.SyncControllerToCamera(camera);

	// ���[�g�{�[�����M�Y���œ�����
	Bone& root = bones[0];
	const DirectX::XMFLOAT4X4& view = camera.GetView();
	const DirectX::XMFLOAT4X4& projection = camera.GetProjection();
	ImGuizmo::Manipulate(
		&view._11, &projection._11,
		ImGuizmo::TRANSLATE,
		ImGuizmo::WORLD,
		&root.worldTransform._11,
		nullptr);

	// ���̃{�[���̃��[���h�s��v�Z
	Bone& next = bones[1];
	next.oldWorldPosition.x = bones[1].worldTransform._41;
	next.oldWorldPosition.y = bones[1].worldTransform._42;
	next.oldWorldPosition.z = bones[1].worldTransform._43;
	DirectX::XMMATRIX RootWorldTransform = DirectX::XMLoadFloat4x4(&root.worldTransform);
	DirectX::XMMATRIX NextLocalRotationTransform = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&next.localRotation));
	DirectX::XMMATRIX NextLocalPositionTransform = DirectX::XMMatrixTranslation(next.localPosition.x, next.localPosition.y, next.localPosition.z);
	DirectX::XMMATRIX NextLocalTransform = DirectX::XMMatrixMultiply(NextLocalRotationTransform, NextLocalPositionTransform);
	DirectX::XMMATRIX NextWorldTransform = DirectX::XMMatrixMultiply(NextLocalTransform, RootWorldTransform);
	DirectX::XMStoreFloat4x4(&next.worldTransform, NextWorldTransform);

	// �{�[���V�~�����[�V����
	const float gravity = this->gravity * elapsedTime;
	for (int i = 2; i < _countof(bones); ++i)
	{
		// �e�{�[�����擾
		Bone& parent = bones[i - 2];
		Bone& bone = bones[i - 1];
		Bone& child = bones[i];

		// TODO�@:�{�[���̉�]����Ń��[�v�\���̕�����������������
		{

		}
	}

}

// �`�揈��
void PhysicsBoneScene::Render(float elapsedTime)
{
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
	RenderState* renderState = Graphics::Instance().GetRenderState();
	PrimitiveRenderer* primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();
	ShapeRenderer* shapeRenderer = Graphics::Instance().GetShapeRenderer();

	// �{�[���`��
	for (size_t i = 0; i < _countof(bones) - 1; ++i)
	{
		const Bone& bone = bones[i];
		const Bone& child = bones[i + 1];

		DirectX::XMFLOAT4X4 world;
		DirectX::XMMATRIX World = DirectX::XMLoadFloat4x4(&bone.worldTransform);
		float length = child.localPosition.z;
		World.r[0] = DirectX::XMVectorScale(DirectX::XMVector3Normalize(World.r[0]), length);
		World.r[1] = DirectX::XMVectorScale(DirectX::XMVector3Normalize(World.r[1]), length);
		World.r[2] = DirectX::XMVectorScale(DirectX::XMVector3Normalize(World.r[2]), length);
		DirectX::XMStoreFloat4x4(&world, World);
		primitiveRenderer->DrawAxis(world, { 1, 1, 0, 1 });
		shapeRenderer->DrawBone(bone.worldTransform, length, { 1, 1, 0, 1 });
	}

	// �����_�[�X�e�[�g�ݒ�
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), nullptr, 0xFFFFFFFF);
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullNone));

	// �O���b�h�`��
	primitiveRenderer->DrawGrid(20, 1);
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// �V�F�C�v�`��
	shapeRenderer->Render(dc, camera.GetView(), camera.GetProjection());
}

// GUI�`�揈��
void PhysicsBoneScene::DrawGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();

	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_Once);

	if (ImGui::Begin(u8"�h����̏���(�{�[��)"))
	{
		ImGui::SliderFloat("gravity", &gravity, 0, 1, "%.3f");
	}
	ImGui::End();
}
