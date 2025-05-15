#include <imgui.h>
#include "Graphics.h"
#include "Scene/RootMotionScene.h"

// �R���X�g���N�^
RootMotionScene::RootMotionScene()
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
		{ 0, 10, -10 },		// ���_
		{ 0, 0, 0 },		// �����_
		{ 0, 1, 0 }			// ��x�N�g��
	);

	// ���f���ǂݍ���
	character = std::make_shared<Model>(device, "Data/Model/RPG-Character/RPG-Character.glb");
	character->GetNodePoses(nodePoses);
}

// �X�V����
void RootMotionScene::Update(float elapsedTime)
{
	// �A�j���[�V�����؂�ւ�����
	if (animationLoop)
	{
		// ���s
		int newAnimationIndex = animationIndex;
		if (GetAsyncKeyState(VK_UP) & 0x8000)
		{
			newAnimationIndex = character->GetAnimationIndex("Walk_F");
		}
		else if (GetAsyncKeyState(VK_DOWN) & 0x8000)
		{
			newAnimationIndex = character->GetAnimationIndex("Walk_B");
		}
		else if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
		{
			newAnimationIndex = character->GetAnimationIndex("Walk_R");
		}
		else if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		{
			newAnimationIndex = character->GetAnimationIndex("Walk_L");
		}
		else
		{
			newAnimationIndex = character->GetAnimationIndex("Idle");
		}
		if (animationIndex != newAnimationIndex)
		{
			animationIndex = newAnimationIndex;
			animationSeconds = oldAnimationSeconds = 0;
		}
	}
	else
	{
		// ���[�����O
		if (GetAsyncKeyState(VK_UP) & 0x01)
		{
			animationIndex = character->GetAnimationIndex("Evade_F");
			animationSeconds = oldAnimationSeconds = 0;
		}
		if (GetAsyncKeyState(VK_DOWN) & 0x01)
		{
			animationIndex = character->GetAnimationIndex("Evade_B");
			animationSeconds = oldAnimationSeconds = 0;
		}
		if (GetAsyncKeyState(VK_RIGHT) & 0x01)
		{
			animationIndex = character->GetAnimationIndex("Evade_R");
			animationSeconds = oldAnimationSeconds = 0;
		}
		if (GetAsyncKeyState(VK_LEFT) & 0x01)
		{
			animationIndex = character->GetAnimationIndex("Evade_L");
			animationSeconds = oldAnimationSeconds = 0;
		}
	}


	// �A�j���[�V�����X�V����
	if (animationIndex >= 0)
	{
		const Model::Animation& animation = character->GetAnimations().at(animationIndex);

		// �w�莞�Ԃ̃A�j���[�V�����̎p�����擾
		character->ComputeAnimation(animationIndex, animationSeconds, nodePoses);

		// TODO�@:���[�g���[�V�����������s���A�L�����N�^�[���ړ�����
		{
			// ���[�g���[�V�����m�[�h�ԍ��擾
			const int rootMotionNodeIndex = character->GetNodeIndex("B_Pelvis");

			if (oldAnimationSeconds > animationSeconds)
			{
				// TODO�A:���[�v�A�j���[�V�����ɑΉ�����
			}
			else
			{

			}



			if (bakeTranslationY)
			{
				// TODO�B:���[�g���[�V������Y�ړ��͓K�p���Ȃ��悤�ɂ���
			}
			else
			{

			}
		}

		// �A�j���[�V�������ԍX�V
		oldAnimationSeconds = animationSeconds;
		animationSeconds += elapsedTime;
		if (animationSeconds > animation.secondsLength)
		{
			if (animationLoop)
			{
				animationSeconds -= animation.secondsLength;
			}
			else
			{
				animationSeconds = animation.secondsLength;
			}
		}

		// �p���X�V
		character->SetNodePoses(nodePoses);
	}

	// ���[���h�g�����X�t�H�[���X�V
	DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z);
	DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
	DirectX::XMMATRIX WorldTransform = S * R * T;
	DirectX::XMStoreFloat4x4(&worldTransform, WorldTransform);

	// ���f���g�����X�t�H�[���X�V����
	character->UpdateTransform(worldTransform);
}

// �`�揈��
void RootMotionScene::Render(float elapsedTime)
{
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
	RenderState* renderState = Graphics::Instance().GetRenderState();
	PrimitiveRenderer* primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();
	ModelRenderer* modelRenderer = Graphics::Instance().GetModelRenderer();

	// �����_�[�X�e�[�g�ݒ�
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), nullptr, 0xFFFFFFFF);
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullNone));

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
	modelRenderer->Render(rc);
}

// GUI�`�揈��
void RootMotionScene::DrawGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();

	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_Once);

	if (ImGui::Begin(u8"���[�g���[�V��������"))
	{
		ImGui::Text(u8"����F�����L�[");

		ImGui::Checkbox(u8"���[�v", &animationLoop);
		ImGui::Checkbox(u8"Y���ړ�����", &bakeTranslationY);

		ImGui::DragFloat3("Position", &position.x, 0.1f);

		DirectX::XMFLOAT3 degree =
		{
			DirectX::XMConvertToDegrees(angle.x),
			DirectX::XMConvertToDegrees(angle.y),
			DirectX::XMConvertToDegrees(angle.z),
		};
		if (ImGui::DragFloat3("Angle", &degree.x, 1.0f))
		{
			angle.x = DirectX::XMConvertToRadians(degree.x);
			angle.y = DirectX::XMConvertToRadians(degree.y);
			angle.z = DirectX::XMConvertToRadians(degree.z);
		}
	}
	ImGui::End();
}
