#include <imgui.h>
#include <ImGuizmo.h>
#include "Graphics.h"
#include "Scene/PhysicsRopeScene.h"

// �R���X�g���N�^
PhysicsRopeScene::PhysicsRopeScene()
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
		{ 0, 3, -10 },		// ���_
		{ 0, 3, 0 },		// �����_
		{ 0, 1, 0 }			// ��x�N�g��
	);
	cameraController.SyncCameraToController(camera);

	// �W���C���g������
	for (int i = 0; i < _countof(joints); ++i)
	{
		Joint& joint = joints[i];
		joint.position.x = i * jointInterval;
		joint.position.y = 3.0f;
		joint.position.z = 0.0f;
		joint.oldPosition = joint.position;
	}

}

// �X�V����
void PhysicsRopeScene::Update(float elapsedTime)
{
	// �J�����X�V����
	cameraController.Update();
	cameraController.SyncControllerToCamera(camera);

	// ���[�g�W���C���g���M�Y���œ�����
	DirectX::XMFLOAT3& rootJointPosition = joints[0].position;
	const DirectX::XMFLOAT4X4& view = camera.GetView();
	const DirectX::XMFLOAT4X4& projection = camera.GetProjection();
	DirectX::XMFLOAT4X4 world;
	DirectX::XMStoreFloat4x4(&world, DirectX::XMMatrixTranslation(rootJointPosition.x, rootJointPosition.y, rootJointPosition.z));
	ImGuizmo::Manipulate(
		&view._11, &projection._11,
		ImGuizmo::TRANSLATE,
		ImGuizmo::WORLD,
		&world._11,
		nullptr);
	rootJointPosition.x = world._41;
	rootJointPosition.y = world._42;
	rootJointPosition.z = world._43;

	// ���[�v�V�~�����[�V����
	const float gravity = this->gravity * elapsedTime;
	for (int i = 1; i < _countof(joints); ++i)
	{
		Joint& parentJoint = joints[i - 1];
		Joint& joint = joints[i];

		// TODO�@:�W���C���g�̈ʒu�𐧌䂵�A���[�v�\���̕�����������������
		{

		}
	}

}

// �`�揈��
void PhysicsRopeScene::Render(float elapsedTime)
{
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
	RenderState* renderState = Graphics::Instance().GetRenderState();
	PrimitiveRenderer* primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();

	// �����_�[�X�e�[�g�ݒ�
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), nullptr, 0xFFFFFFFF);
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullNone));

	// ���[�v�`��
	for (const Joint& joint : joints)
	{
		primitiveRenderer->AddVertex(joint.position, { 1, 1, 0, 1 });
	}
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	// �O���b�h�`��
	primitiveRenderer->DrawGrid(20, 1);
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}

// GUI�`�揈��
void PhysicsRopeScene::DrawGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Once);

	if (ImGui::Begin(u8"�h����̏���(���[�v)"))
	{
		ImGui::SliderFloat("gravity", &gravity, 0, 1, "%.3f");
	}
	ImGui::End();
}
