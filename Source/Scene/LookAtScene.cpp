#include <functional>
#include <imgui.h>
#include <ImGuizmo.h>
#include "Graphics.h"
#include "Scene/LookAtScene.h"

// �R���X�g���N�^
LookAtScene::LookAtScene()
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
		{ 3, 2, 3 },		// ���_
		{ 0, 1, 0 },		// �����_
		{ 0, 1, 0 }			// ��x�N�g��
	);
	cameraController.SyncCameraToController(camera);

	// ���f���ǂݍ���
	character = std::make_shared<Model>(device, "Data/Model/unitychan/unitychan.glb");
	character->GetNodePoses(nodePoses);

	// ���m�[�h�擾
	int headNodeIndex = character->GetNodeIndex("Character1_Head");
	Model::Node& headNode = character->GetNodes().at(headNodeIndex);

	// TODO�@:�����p�����̓��m�[�h�̃��[�J����ԑO���������߂�
	{
	}

	// �^�[�Q�b�g�ʒu
	targetPosition = { 0, 2, 1 };

}

// �X�V����
void LookAtScene::Update(float elapsedTime)
{
	// �J�����X�V����
	cameraController.Update();
	cameraController.SyncControllerToCamera(camera);

	const int animationIndex = character->GetAnimationIndex("Idle");

	// �w�莞�Ԃ̃A�j���[�V�����̎p�����擾
	character->ComputeAnimation(animationIndex, animationSeconds, nodePoses);

	// �A�j���[�V�������ԍX�V
	const Model::Animation& animation = character->GetAnimations().at(animationIndex);
	animationSeconds += elapsedTime;
	if (animationSeconds > animation.secondsLength)
	{
		animationSeconds -= animation.secondsLength;
	}

	// �p���X�V
	character->SetNodePoses(nodePoses);

	// �L�����N�^�[�g�����X�t�H�[���X�V
	DirectX::XMFLOAT4X4 worldTransform;
	DirectX::XMStoreFloat4x4(&worldTransform, DirectX::XMMatrixIdentity());
	character->UpdateTransform(worldTransform);

	// ���[���h�s��X�V�����֐�
	std::function<void(Model::Node&)> updateWorldTransforms = [&](Model::Node& node)
	{
		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(node.scale.x, node.scale.y, node.scale.z);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&node.rotation));
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(node.position.x, node.position.y, node.position.z);
		DirectX::XMMATRIX LocalTransform = S * R * T;
		DirectX::XMMATRIX ParentGlobalTransform = node.parent != nullptr
			? DirectX::XMLoadFloat4x4(&node.parent->globalTransform)
			: DirectX::XMMatrixIdentity();
		DirectX::XMMATRIX GlobalTransform = LocalTransform * ParentGlobalTransform;
		DirectX::XMMATRIX WorldTransform = GlobalTransform * DirectX::XMLoadFloat4x4(&worldTransform);
		DirectX::XMStoreFloat4x4(&node.localTransform, LocalTransform);
		DirectX::XMStoreFloat4x4(&node.globalTransform, GlobalTransform);
		DirectX::XMStoreFloat4x4(&node.worldTransform, WorldTransform);

		for (Model::Node* child : node.children)
		{
			updateWorldTransforms(*child);
		}
	};

	// ���m�[�h�擾
	int headNodeIndex = character->GetNodeIndex("Character1_Head");
	Model::Node& headNode = character->GetNodes().at(headNodeIndex);

	// TOOD�A:�����^�[�Q�b�g�ʒu�𐳖ʂɑ�����悤�ɉ�]������
	{
	}
}

// �`�揈��
void LookAtScene::Render(float elapsedTime)
{
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
	RenderState* renderState = Graphics::Instance().GetRenderState();
	PrimitiveRenderer* primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();
	ShapeRenderer* shapeRenderer = Graphics::Instance().GetShapeRenderer();
	ModelRenderer* modelRenderer = Graphics::Instance().GetModelRenderer();

	// �^�[�Q�b�g�ʒu���M�Y���œ�����
	const DirectX::XMFLOAT4X4& view = camera.GetView();
	const DirectX::XMFLOAT4X4& projection = camera.GetProjection();
	DirectX::XMFLOAT4X4 world;
	DirectX::XMStoreFloat4x4(&world, DirectX::XMMatrixTranslation(targetPosition.x, targetPosition.y, targetPosition.z));
	ImGuizmo::Manipulate(
		&view._11, &projection._11,
		ImGuizmo::TRANSLATE,
		ImGuizmo::WORLD,
		&world._11,
		nullptr);
	targetPosition.x = world._41;
	targetPosition.y = world._42;
	targetPosition.z = world._43;

	// �����_�[�X�e�[�g�ݒ�
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), nullptr, 0xFFFFFFFF);
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullNone));

	// �O���b�h�`��
	primitiveRenderer->DrawGrid(20, 1);
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// �^�[�Q�b�g���`��
	shapeRenderer->DrawSphere(targetPosition, 0.1f, { 1, 0, 0, 1 });
	shapeRenderer->Render(dc, camera.GetView(), camera.GetProjection());

	// �`��R���e�L�X�g�ݒ�
	RenderContext rc;
	rc.deviceContext = dc;
	rc.renderState = renderState;
	rc.camera = &camera;

	// ���f���`��
	modelRenderer->Draw(ShaderId::Basic, character);
	modelRenderer->Render(rc);
}

// GUI�`�揈��
void LookAtScene::DrawGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Once);

	if (ImGui::Begin(u8"���b�N�A�b�g"))
	{
	}
	ImGui::End();
}
