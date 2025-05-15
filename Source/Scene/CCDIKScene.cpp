#include <imgui.h>
#include <ImGuizmo.h>
#include "Graphics.h"
#include "Scene/CCDIKScene.h"

// �R���X�g���N�^
CCDIKScene::CCDIKScene()
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
		{ 10, 3, 5 },		// ���_
		{ 0, 3, 5 },		// �����_
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
	}
	targetTransform = bones[_countof(bones) - 1].worldTransform;
}

// �X�V����
void CCDIKScene::Update(float elapsedTime)
{
	// �J�����X�V����
	cameraController.Update();
	cameraController.SyncControllerToCamera(camera);

	// �^�[�Q�b�g���M�Y���œ�����
	const DirectX::XMFLOAT4X4& view = camera.GetView();
	const DirectX::XMFLOAT4X4& projection = camera.GetProjection();
	ImGuizmo::Manipulate(
		&view._11, &projection._11,
		ImGuizmo::TRANSLATE,
		ImGuizmo::WORLD,
		&targetTransform._11,
		nullptr);

	DirectX::XMMATRIX TargetWorldTransform = DirectX::XMLoadFloat4x4(&targetTransform);
	DirectX::XMVECTOR TargetWorldPosition = TargetWorldTransform.r[3];

	bool firsts[_countof(bones)] = {};
	// IK�V�~�����[�V����
	Bone& effector = bones[_countof(bones) - 1];
	for (int q = 0; q < quality; ++q)
	{
		// �G�t�F�N�^�{�[�����^�[�Q�b�g�ʒu�������悤�ɐ�[���獪���Ɍ������Čv�Z����
		for (int i = _countof(bones) - 2; i > 0; --i)
		{
			Bone& bone = bones[i];

			// TODO�@:CCD-IK�A���S���Y���ŃG�t�F�N�^�{�[���ʒu���^�[�Q�b�g�ʒu�ɋ߂Â��悤�Ƀ{�[���̉�]���������
			{

			}
		}
	}
}

// �`�揈��
void CCDIKScene::Render(float elapsedTime)
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
void CCDIKScene::DrawGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();

	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_Once);

	if (ImGui::Begin(u8"3�{�ȏ�̃{�[��IK����"))
	{
		ImGui::DragInt("Quality", &quality, 1, 1, 100);
	}
	ImGui::End();
}
