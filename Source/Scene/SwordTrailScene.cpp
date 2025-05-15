#include <imgui.h>
#include "Graphics.h"
#include "Scene/SwordTrailScene.h"

// �R���X�g���N�^
SwordTrailScene::SwordTrailScene()
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
		{ 8, 3, 0 },		// ���_
		{ 0, 2, 0 },		// �����_
		{ 0, 1, 0 }			// ��x�N�g��
	);

	// ���f���ǂݍ���
	character = std::make_shared<Model>(device, "Data/Model/RPG-Character/RPG-Character.glb");
	character->GetNodePoses(nodePoses);
	weapon = std::make_shared<Model>(device, "Data/Model/RPG-Character/2Hand-Sword.glb");
}

// �X�V����
void SwordTrailScene::Update(float elapsedTime)
{
	const int animationIndex = character->GetAnimationIndex("Slash");

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

	// ����g�����X�t�H�[���X�V
	int handNodeIndex = character->GetNodeIndex("B_R_Hand");
	const Model::Node& handNode = character->GetNodes().at(handNodeIndex);
	weapon->UpdateTransform(handNode.worldTransform);

	// TODO�A:�ۑ����Ă������_�o�b�t�@���P�t���[�������点
	{

	}

	// TODO�@:���̍��{�Ɛ�[�̍��W���擾���A���_�o�b�t�@�ɕۑ�����
	// trailPositions[2][MAX_POLYGON] �� ���_�o�b�t�@
	{
		// ���̌��_���獪�{�Ɛ�[�܂ł̃I�t�Z�b�g�l
		DirectX::XMVECTOR RootOffset = DirectX::XMVectorSet(0, 0, 0.3f, 0);
		DirectX::XMVECTOR TipOffset = DirectX::XMVectorSet(0, 0, 2.3f, 0);

	}

	DirectX::XMFLOAT4 color = { 1, 0, 0, 1 };

	// �|���S���쐬
	PrimitiveRenderer* primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();
	if (!spline)
	{
		// �ۑ����Ă������_�o�b�t�@�Ń|���S�������
		for (int i = 0; i < MAX_POLYGON; ++i)
		{
			primitiveRenderer->AddVertex(trailPositions[0][i], color);
			primitiveRenderer->AddVertex(trailPositions[1][i], color);
		}
	}
	else
	{
		// TODO�B:�ۑ����Ă������_�o�b�t�@��p���ăX�v���C���⊮�������s���A
		//        ���炩�ȃ|���S����`�悹��
		{

		}
	}
}

// �`�揈��
void SwordTrailScene::Render(float elapsedTime)
{
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
	RenderState* renderState = Graphics::Instance().GetRenderState();
	PrimitiveRenderer* primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();
	ModelRenderer* modelRenderer = Graphics::Instance().GetModelRenderer();

	// �����_�[�X�e�[�g�ݒ�
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), nullptr, 0xFFFFFFFF);
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullNone));

	// �|���S���`��
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// �O���b�h�`��
	primitiveRenderer->DrawGrid(20, 1);
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// �`��R���e�L�X�g�ݒ�
	RenderContext rc;
	rc.deviceContext = dc;
	rc.renderState = renderState;
	rc.camera = &camera;
	rc.lightManager = &lightManager;

	// ���f���`��
	modelRenderer->Draw(ShaderId::Basic, character);
	modelRenderer->Draw(ShaderId::Basic, weapon);
	modelRenderer->Render(rc);
}

// GUI�`�揈��
void SwordTrailScene::DrawGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Once);

	if (ImGui::Begin(u8"���̋O�Տ���"))
	{
		ImGui::Checkbox(u8"�X�v���C���⊮", &spline);
	}
	ImGui::End();
}
