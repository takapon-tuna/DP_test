#include <imgui.h>
#include <ImGuizmo.h>
#include "Graphics.h"
#include "Scene/SphereVsTriangleCollisionScene.h"

// �R���X�g���N�^
SphereVsTriangleCollisionScene::SphereVsTriangleCollisionScene()
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
		{ 0, 10, 10 },		// ���_
		{ 0, 0, 0 },		// �����_
		{ 0, 1, 0 }			// ��x�N�g��
	);
	cameraController.SyncCameraToController(camera);

	// �I�u�W�F�N�g������
	obj.position = { 0, 2, 1 };
}

// �X�V����
void SphereVsTriangleCollisionScene::Update(float elapsedTime)
{
	// �J�����X�V����
	cameraController.Update();
	cameraController.SyncControllerToCamera(camera);

	// �I�u�W�F�N�g�ړ�����
	const float speed = 2.0f * elapsedTime;
	DirectX::XMFLOAT3 vec = { 0, 0, 0 };
	if (GetAsyncKeyState(VK_UP) & 0x8000)
	{
		vec.z += speed;
	}
	if (GetAsyncKeyState(VK_DOWN) & 0x8000)
	{
		vec.z -= speed;
	}
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
	{
		vec.x += speed;
	}
	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
	{
		vec.x -= speed;
	}
	if (GetAsyncKeyState('Z') & 0x8000)
	{
		vec.y += speed;
	}
	if (GetAsyncKeyState('X') & 0x8000)
	{
		vec.y -= speed;
	}
	// �J�����̌������l�������ړ�����
	const DirectX::XMFLOAT3& front = camera.GetFront();
	const DirectX::XMFLOAT3& right = camera.GetRight();
	float frontLengthXZ = sqrtf(front.x * front.x + front.z * front.z);
	float rightLengthXZ = sqrtf(right.x * right.x + right.z * right.z);
	float frontX = front.x / frontLengthXZ;
	float frontZ = front.z / frontLengthXZ;
	float rightX = right.x / rightLengthXZ;
	float rightZ = right.z / rightLengthXZ;
	obj.position.x += frontX * vec.z + rightX * vec.x;
	obj.position.z += frontZ * vec.z + rightZ * vec.x;
	obj.position.y += vec.y;
}

// �`�揈��
void SphereVsTriangleCollisionScene::Render(float elapsedTime)
{
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
	RenderState* renderState = Graphics::Instance().GetRenderState();
	PrimitiveRenderer* primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();
	ShapeRenderer* shapeRenderer = Graphics::Instance().GetShapeRenderer();

	// �O�p�`���_
	DirectX::XMFLOAT3 v[3] = {
		{ -2, 2, 0 },
		{ 0, 1, 2 },
		{ 2, 2, 0 },
	};
	// ���ƎO�p�`�̏Փˏ���
	DirectX::XMFLOAT4 c = { 0, 1, 0, 1 };
	DirectX::XMFLOAT3 position, normal;
	if (SphereIntersectTriangle(obj.position, obj.radius, v[0], v[1], v[2], position, normal))
	{
		obj.position = position;
		c = { 1, 0, 0, 1 };
	}

	// �����_�[�X�e�[�g�ݒ�
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), nullptr, 0xFFFFFFFF);
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullBack));

	// �O�p�`�|���S���`��
	primitiveRenderer->AddVertex(v[0], c);
	primitiveRenderer->AddVertex(v[1], c);
	primitiveRenderer->AddVertex(v[2], c);
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullNone));

	// �O�p�`�G�b�W�`��
	primitiveRenderer->AddVertex(v[0], { 0, 0, 1, 1 });
	primitiveRenderer->AddVertex(v[1], { 0, 0, 1, 1 });
	primitiveRenderer->AddVertex(v[2], { 0, 0, 1, 1 });
	primitiveRenderer->AddVertex(v[0], { 0, 0, 1, 1 });
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	// �O���b�h�`��
	primitiveRenderer->DrawGrid(20, 1);
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// �V�F�C�v�`��
	shapeRenderer->DrawSphere(obj.position, obj.radius, obj.color);
	shapeRenderer->Render(dc, camera.GetView(), camera.GetProjection());
}

// GUI�`�揈��
void SphereVsTriangleCollisionScene::DrawGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Once);

	if (ImGui::Begin(u8"���ƎO�p�`�̏Փˏ���"))
	{
		ImGui::Text(u8"�O�㍶�E�ړ�����F�����L�[");
		ImGui::Text(u8"�㉺�ړ�����FZX");
	}
	ImGui::End();
}

// ���ƎO�p�`�Ƃ̌����𔻒肷��
bool SphereVsTriangleCollisionScene::SphereIntersectTriangle(
	const DirectX::XMFLOAT3& sphereCenter,
	const float sphereRadius,
	const DirectX::XMFLOAT3& triangleVertexA,
	const DirectX::XMFLOAT3& triangleVertexB,
	const DirectX::XMFLOAT3& triangleVertexC,
	DirectX::XMFLOAT3& hitPosition,
	DirectX::XMFLOAT3& hitNormal)
{
	// TODO�@:�����O�p�`�ɉ����o�����Փˏ�������������
	{
	}
	return false;
}
