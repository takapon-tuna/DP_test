#include <algorithm>
#include <imgui.h>
#include <ImGuizmo.h>
#include "Graphics.h"
#include "Scene/RaceRankingScene.h"

// �R���X�g���N�^
RaceRankingScene::RaceRankingScene()
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
		{ 5, 50, 0 },		// ���_
		{ 5, 0, 5 },		// �����_
		{ 0, 1, 0 }			// ��x�N�g��
	);
	cameraController.SyncCameraToController(camera);

	// �`�F�b�N�|�C���g������
	checkPoints[0].position  = {  29.0f, 0.0f,  00.0f };
	checkPoints[1].position  = {  28.9f, 0.0f,  14.3f };
	checkPoints[2].position  = {  21.6f, 0.0f,  21.5f };
	checkPoints[3].position  = { -17.8f, 0.0f,  21.7f };
	checkPoints[4].position  = { -24.8f, 0.0f,  15.1f };
	checkPoints[5].position  = { -25.7f, 0.0f, - 4.3f };
	checkPoints[6].position  = { -19.8f, 0.0f, -11.1f };
	checkPoints[7].position  = { -10.0f, 0.0f, -11.8f };
	checkPoints[8].position  = { -02.9f, 0.0f, -05.0f };
	checkPoints[9].position  = { -02.9f, 0.0f,  04.1f };
	checkPoints[10].position = {  04.1f, 0.0f,  10.7f };
	checkPoints[11].position = {  13.6f, 0.0f,  10.5f };
	checkPoints[12].position = {  20.5f, 0.0f,  03.7f };
	checkPoints[13].position = {  20.5f, 0.0f, -05.8f };
	checkPoints[14].position = {  23.6f, 0.0f, -09.0f };
	checkPoints[15].position = {  25.6f, 0.0f, -08.6f };
	checkPoints[16].position = {  28.9f, 0.0f, -04.9f };

	for (int i = 0; i < _countof(checkPoints); ++i)
	{
		CheckPoint& checkPoint = checkPoints[i];
		const CheckPoint& nextCheckPoint = checkPoints[(i + 1) % _countof(checkPoints)];

		DirectX::XMVECTOR Position = DirectX::XMLoadFloat3(&checkPoint.position);
		DirectX::XMVECTOR NextPosition = DirectX::XMLoadFloat3(&nextCheckPoint.position);
		DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(NextPosition, Position);
		DirectX::XMVECTOR Direction = DirectX::XMVector3Normalize(Vec);
		DirectX::XMStoreFloat3(&checkPoint.direction, Direction);
	}

	// �ԏ�����
	cars[0].name = u8"Red";
	cars[0].nowCheckPointIndex = 0;
	cars[0].position = checkPoints[0].position;
	cars[0].color = { 1, 0, 0, 1 };
	cars[0].speed = 8.0f;
	cars[1].name = u8"Green";
	cars[1].nowCheckPointIndex = 0;
	cars[1].position = checkPoints[0].position;
	cars[1].color = { 0, 1, 0, 1 };
	cars[1].speed = 9.0f;
	cars[2].name = u8"Blue";
	cars[2].nowCheckPointIndex = 0;
	cars[2].position = checkPoints[0].position;
	cars[2].color = { 0, 0, 1, 1 };
	cars[2].speed = 10.0f;

	for (Car& car : cars)
	{
		rankingSortedCars.emplace_back(&car);
	}
}

// �X�V����
void RaceRankingScene::Update(float elapsedTime)
{
	// �J�����X�V����
	cameraController.Update();
	cameraController.SyncControllerToCamera(camera);

	// �Ԃ̈ʒu�X�V
	int checkPoiontCount = _countof(checkPoints);
	for (int i = 0; i < _countof(cars); ++i)
	{
		Car& car = cars[i];
		const CheckPoint& nextCheckPoint = checkPoints[(car.nowCheckPointIndex + 1) % checkPoiontCount];

		// �^�[�Q�b�g�ʒu�Z�o
		DirectX::XMVECTOR NextCheckPointPosition = DirectX::XMLoadFloat3(&nextCheckPoint.position);
		DirectX::XMVECTOR NextCheckPointDirection = DirectX::XMLoadFloat3(&nextCheckPoint.direction);
		DirectX::XMVECTOR Cross = DirectX::XMVector3Cross(DirectX::XMVectorSet(0, 1, 0, 0), NextCheckPointDirection);
		Cross = DirectX::XMVector3Normalize(Cross);
		DirectX::XMVECTOR TargetPosition = DirectX::XMVectorAdd(NextCheckPointPosition, DirectX::XMVectorScale(Cross, i * car.radius * 2));
		// �ړ�����
		DirectX::XMVECTOR Position = DirectX::XMLoadFloat3(&car.position);
		DirectX::XMVECTOR Direction = DirectX::XMVectorSubtract(TargetPosition, Position);
		Direction = DirectX::XMVector3Normalize(Direction);
		Position = DirectX::XMVectorAdd(Position, DirectX::XMVectorScale(Direction, car.speed * elapsedTime));
		// �ʒu�X�V
		car.oldPosition = car.position;
		DirectX::XMStoreFloat3(&car.position, Position);
		DirectX::XMStoreFloat3(&car.direction, Direction);
	}

	// �`�F�b�N�|�C���g�̒ʉߐ��Ɛi�s�������烉���L���O�𔻒肵�A�\�[�g
	std::sort(rankingSortedCars.begin(), rankingSortedCars.end(),
		[this](const Car* lhs, const Car* rhs)
		{
			if (lhs->checkCount == rhs->checkCount)
			{
				return lhs->progress > rhs->progress;
			}
			return lhs->checkCount > rhs->checkCount;
		}
	);
	// �����L���O����ۑ�
	for (size_t i = 0; i < rankingSortedCars.size(); ++i)
	{
		Car* car = rankingSortedCars.at(i);
		car->ranking = static_cast<int>(i + 1);
	}
	// �`�F�b�N�|�C���g�̒ʉ߂�i�s������ۑ�
	for (Car& car : cars)
	{
		// ��莞�Ԃő��x��ς���
		car.timer -= elapsedTime;
		if (car.timer < 0.0f)
		{
			// ���x�͏��ʂ��Ⴂ�قǑ����Ȃ�悤�ɂ���
			car.speed = car.ranking * 2 + 5.0f;
			car.timer = car.ranking * 1.0f;
		}

		// �`�F�b�N�|�C���g���擾
		const CheckPoint& nowCheckPoint = checkPoints[car.nowCheckPointIndex];
		const CheckPoint& nextCheckPoint = checkPoints[(car.nowCheckPointIndex + 1) % checkPoiontCount];

		DirectX::XMVECTOR OldCarPosition = DirectX::XMLoadFloat3(&car.oldPosition);
		DirectX::XMVECTOR NowCarPosition = DirectX::XMLoadFloat3(&car.position);
		DirectX::XMVECTOR NextCheckPointPosition = DirectX::XMLoadFloat3(&nextCheckPoint.position);
		DirectX::XMVECTOR NextCheckPointDirection = DirectX::XMLoadFloat3(&nextCheckPoint.direction);

		// TODO�@:�`�F�b�N�|�C���g��ʉ߂��������肹��
		{
			// �ʉ߂����ꍇ�͈ȉ��̃p�����[�^���X�V����
			// car.nowCheckPointIndex �� �ʉ߂����`�F�b�N�|�C���g�̃C���f�b�N�X
			// car.checkPoint �� �ʉ߂����`�F�b�N�|�C���g�̐�
		}
		// TODO�A:�`�F�b�N�|�C���g�Ԃ̐i�s�������Z�o����
		{
			// ���t���[���ȉ��̃p�����[�^���X�V����
			// car.progress �� �`�F�b�N�|�C���g�Ԃ̐i�s����
		}
	}

}

// �`�揈��
void RaceRankingScene::Render(float elapsedTime)
{
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
	RenderState* renderState = Graphics::Instance().GetRenderState();
	PrimitiveRenderer* primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();
	ShapeRenderer* shapeRenderer = Graphics::Instance().GetShapeRenderer();

	// �ԕ`��
	for (Car& car : cars)
	{
		shapeRenderer->DrawSphere(car.position, car.radius, car.color);
	}

	// �R�[�X�`��
	for (size_t i = 0; i < _countof(checkPoints); ++i)
	{
		const CheckPoint& checkPoint = checkPoints[i];
		const CheckPoint& nextCheckPoint = checkPoints[(i + 1) % _countof(checkPoints)];

		primitiveRenderer->AddVertex(checkPoint.position, { 1, 1, 1, 1 });
		primitiveRenderer->AddVertex(nextCheckPoint.position, { 1, 1, 1, 1 });
	}

	// �����_�[�X�e�[�g�ݒ�
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), nullptr, 0xFFFFFFFF);
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullNone));

	// �V�F�C�v�`��
	shapeRenderer->Render(dc, camera.GetView(), camera.GetProjection());
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}

// GUI�`�揈��
void RaceRankingScene::DrawGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
	ImGui::SetNextWindowPos(ImVec2(pos.x + 300, pos.y + 100), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(430, 150), ImGuiCond_Once);

	if (ImGui::Begin(u8"���[�X���ʔ��菈��"))
	{
		ImGui::Columns(5, "columns");
		ImGui::SetColumnWidth(0, 100);
		ImGui::SetColumnWidth(1, 70);
		ImGui::SetColumnWidth(2, 70);
		ImGui::SetColumnWidth(3, 100);
		ImGui::SetColumnWidth(4, 70);
		ImGui::Separator();

		ImGui::Text("Name"); ImGui::NextColumn();
		ImGui::Text("Speed"); ImGui::NextColumn();
		ImGui::Text("Ranking"); ImGui::NextColumn();
		ImGui::Text("CheckCount"); ImGui::NextColumn();
		ImGui::Text("Progress"); ImGui::NextColumn();
		ImGui::Separator();

		for (int i = 0; i < _countof(cars); ++i)
		{
			Car& car = cars[i];

			ImGui::Text("%s", car.name.c_str()); ImGui::NextColumn();
			ImGui::Text("%.1f", car.speed); ImGui::NextColumn();
			ImGui::Text("%d", car.ranking); ImGui::NextColumn();
			ImGui::Text("%d", car.checkCount); ImGui::NextColumn();
			ImGui::Text("%.1f", car.progress); ImGui::NextColumn();
			ImGui::Separator();
		}
		ImGui::Columns(1);
	}
	ImGui::End();

	// �`�F�b�N�|�C���g�ԍ��\��
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
	D3D11_VIEWPORT viewport;
	UINT numViewports = 1;
	dc->RSGetViewports(&numViewports, &viewport);

	DirectX::XMMATRIX View = DirectX::XMLoadFloat4x4(&camera.GetView());
	DirectX::XMMATRIX Projection = DirectX::XMLoadFloat4x4(&camera.GetProjection());
	DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();

	for (size_t i = 0; i < _countof(checkPoints); ++i)
	{
		const CheckPoint& checkPoint = checkPoints[i];

		DirectX::XMVECTOR WorldPosition = DirectX::XMLoadFloat3(&checkPoint.position);

		// ���[���h���W����X�N���[�����W�֕ϊ�
		DirectX::XMVECTOR ScreenPosition = DirectX::XMVector3Project(
			WorldPosition,
			viewport.TopLeftX,
			viewport.TopLeftY,
			viewport.Width,
			viewport.Height,
			viewport.MinDepth,
			viewport.MaxDepth,
			Projection,
			View,
			World
		);

		// �X�N���[�����W
		DirectX::XMFLOAT2 screenPosition;
		DirectX::XMStoreFloat2(&screenPosition, ScreenPosition);

		ImGui::SetNextWindowPos(ImVec2(pos.x + screenPosition.x, pos.y + screenPosition.y));
		ImGui::SetNextWindowBgAlpha(0.0f);

		char name[32];
		::sprintf_s(name, sizeof(name), "%zd", i);

		ImGui::Begin(name, nullptr,
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBackground |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoScrollWithMouse |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoInputs);

		ImGui::Text(name);

		ImGui::End();
	}

	// �Ԕԍ��Ə��ʕ\��
	for (size_t i = 0; i < _countof(cars); ++i)
	{
		const Car& car = cars[i];

		DirectX::XMVECTOR WorldPosition = DirectX::XMLoadFloat3(&car.position);

		// ���[���h���W����X�N���[�����W�֕ϊ�
		DirectX::XMVECTOR ScreenPosition = DirectX::XMVector3Project(
			WorldPosition,
			viewport.TopLeftX,
			viewport.TopLeftY,
			viewport.Width,
			viewport.Height,
			viewport.MinDepth,
			viewport.MaxDepth,
			Projection,
			View,
			World
		);

		// �X�N���[�����W
		DirectX::XMFLOAT2 screenPosition;
		DirectX::XMStoreFloat2(&screenPosition, ScreenPosition);

		ImGui::SetNextWindowPos(ImVec2(pos.x + screenPosition.x, pos.y + screenPosition.y));
		ImGui::SetNextWindowBgAlpha(0.0f);

		char name[32];
		::sprintf_s(name, sizeof(name), "[%d]", car.ranking);

		ImGui::Begin(name, nullptr,
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBackground |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoScrollWithMouse |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoInputs);

		ImGui::Text(name);

		ImGui::End();
	}

	ImGui::SetNextWindowPos(ImVec2(pos.x + 410, pos.y + 10), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 30), ImGuiCond_Once);
}
