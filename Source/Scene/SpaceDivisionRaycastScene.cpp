#include <imgui.h>
#include <ImGuizmo.h>
#include "Graphics.h"
#include "Scene/SpaceDivisionRaycastScene.h"

// �R���X�g���N�^
SpaceDivisionRaycastScene::SpaceDivisionRaycastScene()
{
	ID3D11Device *device = Graphics::Instance().GetDevice();
	float screenWidth = Graphics::Instance().GetScreenWidth();
	float screenHeight = Graphics::Instance().GetScreenHeight();

	// �J�����ݒ�
	camera.SetPerspectiveFov(
			DirectX::XMConvertToRadians(45), // ��p
			screenWidth / screenHeight,			 // ��ʃA�X�y�N�g��
			0.1f,														 // �j�A�N���b�v
			1000.0f													 // �t�@�[�N���b�v
	);
	camera.SetLookAt(
			{0, 10, 10}, // ���_
			{0, 0, 0},	 // �����_
			{0, 1, 0}		 // ��x�N�g��
	);
	cameraController.SyncCameraToController(camera);

	// ���f���ǂݍ���
	character = std::make_shared<Model>(device, "Data/Model/Mr.Incredible/Mr.Incredible.glb");
	stage = std::make_shared<Model>(device, "Data/Model/Stage/ExampleStage.glb");

	DirectX::XMVECTOR VolumeMin = DirectX::XMVectorReplicate(FLT_MAX);
	DirectX::XMVECTOR VolumeMax = DirectX::XMVectorReplicate(-FLT_MAX);

	// ���_�f�[�^�����[���h��ԕϊ����A�O�p�`�f�[�^���쐬
	for (const Model::Mesh &mesh : stage->GetMeshes())
	{
		DirectX::XMMATRIX WorldTransform = DirectX::XMLoadFloat4x4(&mesh.node->worldTransform);
		for (size_t i = 0; i < mesh.indices.size(); i += 3)
		{
			// ���_�f�[�^�����[���h��ԕϊ�
			uint32_t a = mesh.indices.at(i + 0);
			uint32_t b = mesh.indices.at(i + 1);
			uint32_t c = mesh.indices.at(i + 2);
			DirectX::XMVECTOR A = DirectX::XMLoadFloat3(&mesh.vertices.at(a).position);
			DirectX::XMVECTOR B = DirectX::XMLoadFloat3(&mesh.vertices.at(b).position);
			DirectX::XMVECTOR C = DirectX::XMLoadFloat3(&mesh.vertices.at(c).position);
			A = DirectX::XMVector3Transform(A, WorldTransform);
			B = DirectX::XMVector3Transform(B, WorldTransform);
			C = DirectX::XMVector3Transform(C, WorldTransform);

			// �@���x�N�g�����Z�o
			DirectX::XMVECTOR N = DirectX::XMVector3Cross(DirectX::XMVectorSubtract(B, A), DirectX::XMVectorSubtract(C, A));
			if (DirectX::XMVector3Equal(N, DirectX::XMVectorZero()))
			{
				// �ʂ��\���ł��Ȃ��ꍇ�͏��O
				continue;
			}
			N = DirectX::XMVector3Normalize(N);

			// �O�p�`�f�[�^���i�[
			CollisionMesh::Triangle &triangle = collisionMesh.triangles.emplace_back();
			DirectX::XMStoreFloat3(&triangle.positions[0], A);
			DirectX::XMStoreFloat3(&triangle.positions[1], B);
			DirectX::XMStoreFloat3(&triangle.positions[2], C);
			DirectX::XMStoreFloat3(&triangle.normal, N);

			// ���f���S�̂�AABB���v��
			VolumeMin = DirectX::XMVectorMin(VolumeMin, A);
			VolumeMin = DirectX::XMVectorMin(VolumeMin, B);
			VolumeMin = DirectX::XMVectorMin(VolumeMin, C);
			VolumeMax = DirectX::XMVectorMax(VolumeMax, A);
			VolumeMax = DirectX::XMVectorMax(VolumeMax, B);
			VolumeMax = DirectX::XMVectorMax(VolumeMax, C);
		}
	}

	// ���f���S�̂�AABB
	DirectX::XMFLOAT3 volumeMin, volumeMax;
	DirectX::XMStoreFloat3(&volumeMin, VolumeMin);
	DirectX::XMStoreFloat3(&volumeMax, VolumeMax);
#if 1
	// TODO�@:���f���S�̂�AABB����XZ���ʂɎw��̃T�C�Y�ŕ������ꂽ�R���W�����G���A���쐬����
	{
		const int cellSize = 4;
		// �G���A���쐬���Ă���
		for (float x = volumeMin.x; x < volumeMax.x; x += cellSize)
		{
			for (float z = volumeMin.z; z < volumeMax.z; z += cellSize)
			{
				CollisionMesh::Area area;
				// AABB���쐬
				area.boundingBox.Center.x = x + cellSize / 2;
				area.boundingBox.Center.y = volumeMin.y + (volumeMax.y - volumeMin.y) / 2;
				area.boundingBox.Center.z = z + cellSize / 2;
				area.boundingBox.Extents.x = cellSize / 2;
				area.boundingBox.Extents.y = (volumeMax.y - volumeMin.y) / 2;
				area.boundingBox.Extents.z = cellSize / 2;

				// �����|���S�������W����
				int triangleIndex = 0;
				for (auto &triangle : collisionMesh.triangles)
				{
					DirectX::XMVECTOR V1, V2, V3;
					V1 = DirectX::XMLoadFloat3(&triangle.positions[0]);
					V2 = DirectX::XMLoadFloat3(&triangle.positions[1]);
					V3 = DirectX::XMLoadFloat3(&triangle.positions[2]);
					if (area.boundingBox.Intersects(V1, V2, V3))
					{
						area.triangleIndices.emplace_back(triangleIndex);
					}
					++triangleIndex;
				}
				collisionMesh.areas.emplace_back(area);
			}
		}
	}
#endif // �搶�̃R�[�h

#if 0 
	{
    const int cellSize = 4; // �Z���̃T�C�Y��4�Ƃ���
    int numCellsX = static_cast<int>(ceil((volumeMax.x - volumeMin.x) / cellSize));
    int numCellsZ = static_cast<int>(ceil((volumeMax.z - volumeMin.z) / cellSize));

    // �G���A���쐬���Ă���
    for (int x = 0; x < numCellsX; ++x) {
        for (int z = 0; z < numCellsZ; ++z) {
            CollisionMesh::Area area;
            area.boundingBox.Center = {
                volumeMin.x + x * cellSize + cellSize / 2.0f,
                0.0f,
                volumeMin.z + z * cellSize + cellSize / 2.0f
            };
            area.boundingBox.Extents = {
                cellSize / 2.0f,
                FLT_MAX,
                cellSize / 2.0f
            };

            // ���̃G���A�Ɋ܂܂��O�p�`�̃C���f�b�N�X�����W
            for (size_t i = 0; i < collisionMesh.triangles.size(); ++i) {
                const auto& triangle = collisionMesh.triangles[i];
                DirectX::XMVECTOR p0 = DirectX::XMLoadFloat3(&triangle.positions[0]);
                DirectX::XMVECTOR p1 = DirectX::XMLoadFloat3(&triangle.positions[1]);
                DirectX::XMVECTOR p2 = DirectX::XMLoadFloat3(&triangle.positions[2]);

                if (DirectX::BoundingBox::Intersects(area.boundingBox, p0, p1, p2)) {
                    area.triangleIndices.push_back(static_cast<int>(i));
                }
            }

            collisionMesh.areas.push_back(area);
        }
    }
	}
#endif // �����̃R�[�h
}

// �X�V����
void SpaceDivisionRaycastScene::Update(float elapsedTime)
{
	// �J�����X�V����
	cameraController.Update();
	cameraController.SyncControllerToCamera(camera);

	// �I�u�W�F�N�g�ړ�����
	const float speed = 5.0f * elapsedTime;
	DirectX::XMFLOAT3 vec = {0, 0, 0};
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
	// �J�����̌������l�������ړ�����
	const DirectX::XMFLOAT3 &front = camera.GetFront();
	const DirectX::XMFLOAT3 &right = camera.GetRight();
	float frontLengthXZ = sqrtf(front.x * front.x + front.z * front.z);
	float rightLengthXZ = sqrtf(right.x * right.x + right.z * right.z);
	float frontX = front.x / frontLengthXZ;
	float frontZ = front.z / frontLengthXZ;
	float rightX = right.x / rightLengthXZ;
	float rightZ = right.z / rightLengthXZ;
	characterPosition.x += frontX * vec.z + rightX * vec.x;
	characterPosition.z += frontZ * vec.z + rightZ * vec.x;

	float distance = 100.0f;
	DirectX::XMFLOAT3 start = {characterPosition.x, characterPosition.y + 1.0f, characterPosition.z};
	DirectX::XMFLOAT3 end = {characterPosition.x, characterPosition.y + -1.0f, characterPosition.z};
	DirectX::XMFLOAT3 hitPosition, hitNormal;
	timer.Tick();
	if (Raycast(start, end, hitPosition, hitNormal))
	{
		characterPosition.y = hitPosition.y;
	}
	timer.Tick();

	// ���Ԍv��
	totalTime += timer.TimeInterval();
	frames++;
	if (frames == 60)
	{
		averageTime = totalTime / frames;
		frames = 0;
		totalTime = 0;
	}
}

// �`�揈��
void SpaceDivisionRaycastScene::Render(float elapsedTime)
{
	ID3D11DeviceContext *dc = Graphics::Instance().GetDeviceContext();
	RenderState *renderState = Graphics::Instance().GetRenderState();
	PrimitiveRenderer *primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();
	ShapeRenderer *shapeRenderer = Graphics::Instance().GetShapeRenderer();
	ModelRenderer *modelRenderer = Graphics::Instance().GetModelRenderer();

	// �L�����N�^�[�g�����X�t�H�[���X�V
	DirectX::XMMATRIX WorldTransform = DirectX::XMMatrixTranslation(characterPosition.x, characterPosition.y, characterPosition.z);
	DirectX::XMFLOAT4X4 worldTransform;
	DirectX::XMStoreFloat4x4(&worldTransform, WorldTransform);
	character->UpdateTransform(worldTransform);

	// �����_�[�X�e�[�g�ݒ�
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), nullptr, 0xFFFFFFFF);
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullNone));

	// �O�p�`�G�b�W�`��
	const DirectX::XMFLOAT4 edgeColor = {0, 0, 1, 1};
	for (CollisionMesh::Triangle &triangle : collisionMesh.triangles)
	{
		primitiveRenderer->AddVertex(triangle.positions[0], edgeColor);
		primitiveRenderer->AddVertex(triangle.positions[1], edgeColor);
		primitiveRenderer->AddVertex(triangle.positions[1], edgeColor);
		primitiveRenderer->AddVertex(triangle.positions[2], edgeColor);
		primitiveRenderer->AddVertex(triangle.positions[2], edgeColor);
		primitiveRenderer->AddVertex(triangle.positions[0], edgeColor);
	}
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// �o�E���f�B���O�{�b�N�X�`��
	const DirectX::XMFLOAT4 boxColor = {0, 1, 0, 1};
	const DirectX::XMFLOAT3 boxAngle = {0, 0, 0};
	for (CollisionMesh::Area &area : collisionMesh.areas)
	{
		shapeRenderer->DrawBox(area.boundingBox.Center, boxAngle, area.boundingBox.Extents, boxColor);
	}
	shapeRenderer->Render(dc, camera.GetView(), camera.GetProjection());

	// �����_�[�X�e�[�g�ݒ�
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullBack));

	// �`��R���e�L�X�g�ݒ�
	RenderContext rc;
	rc.deviceContext = dc;
	rc.renderState = renderState;
	rc.camera = &camera;

	// ���f���`��
	modelRenderer->Draw(ShaderId::Basic, stage);
	modelRenderer->Draw(ShaderId::Basic, character);
	modelRenderer->Render(rc);

	// �����_�[�X�e�[�g�ݒ�
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Transparency), nullptr, 0xFFFFFFFF);

	// �O�p�`�|���S���`��
	const DirectX::XMFLOAT4 polygonColor = {1, 0, 0, 0.5f};
	for (CollisionMesh::Triangle &triangle : collisionMesh.triangles)
	{
		primitiveRenderer->AddVertex(triangle.positions[0], polygonColor);
		primitiveRenderer->AddVertex(triangle.positions[1], polygonColor);
		primitiveRenderer->AddVertex(triangle.positions[2], polygonColor);
	}
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

// GUI�`�揈��
void SpaceDivisionRaycastScene::DrawGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Once);

	if (ImGui::Begin(u8"��ԕ������C�L���X�g"))
	{
		ImGui::Checkbox(u8"��ԕ���", &spaceDivision);
		ImGui::InputFloat(u8"��������", &averageTime, 0, 0, "%.7f", ImGuiInputTextFlags_ReadOnly);
	}
	ImGui::End();
}

// ���C�L���X�g
bool SpaceDivisionRaycastScene::Raycast(
		const DirectX::XMFLOAT3 &start,
		const DirectX::XMFLOAT3 &end,
		DirectX::XMFLOAT3 &hitPosition,
		DirectX::XMFLOAT3 &hitNormal)
{
	bool hit = false;
	DirectX::XMVECTOR Start = DirectX::XMLoadFloat3(&start);
	DirectX::XMVECTOR End = DirectX::XMLoadFloat3(&end);
	DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(End, Start);
	DirectX::XMVECTOR Direction = DirectX::XMVector3Normalize(Vec);
	float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(Vec));

	// ��ԕ��������A���ʂɃ��C�L���X�g������
	if (!spaceDivision)
	{
		for (const CollisionMesh::Triangle &triangle : collisionMesh.triangles)
		{
			DirectX::XMVECTOR A = DirectX::XMLoadFloat3(&triangle.positions[0]);
			DirectX::XMVECTOR B = DirectX::XMLoadFloat3(&triangle.positions[1]);
			DirectX::XMVECTOR C = DirectX::XMLoadFloat3(&triangle.positions[2]);

			float dist = distance;
			if (DirectX::TriangleTests::Intersects(Start, Direction, A, B, C, dist))
			{
				if (distance < dist)
					continue;
				distance = dist;
				hitNormal = triangle.normal;
				hit = true;
			}
		}
	}
#if 1
	// TODO�A�F��ԕ��������f�[�^���g���A���C�L���X�g�������ɏ�������
	else
	{
		for (auto &area : collisionMesh.areas)
		{
			// �P�G���A�̃o�E���f�B���O�{�b�N�X�ƃ��C���q�b�g���Ă��邩����
			float dist = distance;
			if (!area.boundingBox.Intersects(Start, Direction, dist))
				continue;

			// �Q �q�b�g���Ă���΃G���A���̎O�p�`�Ɣ���
			for (auto &triangleIndex : area.triangleIndices)
			{
				auto &triangle = collisionMesh.triangles[triangleIndex];
				DirectX::XMVECTOR A = DirectX::XMLoadFloat3(&triangle.positions[0]);
				DirectX::XMVECTOR B = DirectX::XMLoadFloat3(&triangle.positions[1]);
				DirectX::XMVECTOR C = DirectX::XMLoadFloat3(&triangle.positions[2]);
				if (DirectX::TriangleTests::Intersects(Start, Direction, A, B, C, dist))
				{
					if (distance < dist)
						continue;
					distance = dist;
					hitNormal = triangle.normal;
					hit = true;
				}
			}
		}
		if (hit)
		{
			DirectX::XMVECTOR HitPosition = DirectX::XMVectorAdd(Start, DirectX::XMVectorScale(Direction, distance));
			DirectX::XMStoreFloat3(&hitPosition, HitPosition);
		}
#endif // �搶�̃R�[�h

#if 0
		else
		{
			DirectX::XMVECTOR HitPosition;
			float closestDist = FLT_MAX; // �ł��߂������_�܂ł̋�����������

			// �e�G���A�����[�v���ă��C�ƌ������邩�`�F�b�N
			for (const auto& area : collisionMesh.areas)
			{
				if (DirectX::BoundingBox::Intersects(area.boundingBox, Start, Direction, distance))
				{
					// �G���A���̊e�O�p�`�ɑ΂��Č����e�X�g
					for (int index : area.triangleIndices)
					{
						const auto& triangle = collisionMesh.triangles[index];
						DirectX::XMVECTOR A = DirectX::XMLoadFloat3(&triangle.positions[0]);
						DirectX::XMVECTOR B = DirectX::XMLoadFloat3(&triangle.positions[1]);
						DirectX::XMVECTOR C = DirectX::XMLoadFloat3(&triangle.positions[2]);

						float dist = distance;
						if (DirectX::TriangleTests::Intersects(Start, Direction, A, B, C, dist) && dist < closestDist)
						{
							closestDist = dist;					 // �ł��߂��������X�V
							hitNormal = triangle.normal; // ���������O�p�`�̖@����ۑ�
							hit = true;
							HitPosition = DirectX::XMVectorAdd(Start, DirectX::XMVectorScale(Direction, dist)); // �����ʒu���v�Z
						}
					}
				}
			}

			if (hit)
			{
				DirectX::XMStoreFloat3(&hitPosition, HitPosition); // �ł��߂������ʒu��ۑ�
			}
		}
#endif // �����̃R�[�h
#if 0
		else
		{
			for (auto &area : collisionMesh.areas)
			{
				// �P�G���A�̃o�E���f�B���O�{�b�N�X�ƃ��C���q�b�g���Ă��邩����
				if (area.boundingBox.Intersects(Start, Direction, distance))
				{
					// �Q �q�b�g���Ă���΃G���A���̎O�p�`�Ɣ���
					for (auto &triangleIndex : area.triangleIndices)
					{
						auto &triangle = collisionMesh.triangles[triangleIndex];
						DirectX::XMVECTOR A = DirectX::XMLoadFloat3(&triangle.positions[0]);
						DirectX::XMVECTOR B = DirectX::XMLoadFloat3(&triangle.positions[1]);
						DirectX::XMVECTOR C = DirectX::XMLoadFloat3(&triangle.positions[2]);

						float dist = distance;
						// �q�b�g���Ă��邩�͂�����g���Ƃ���
						if (DirectX::TriangleTests::Intersects(Start, Direction, A, B, C, dist))
						{
							if (distance < dist)
								continue;
							distance = dist;
							hitNormal = triangle.normal;
							hit = true;
						}
					}
				}
			}
			if (hit)
			{
				DirectX::XMVECTOR HitPosition = DirectX::XMVectorAdd(Start, DirectX::XMVectorScale(Direction, distance));
				DirectX::XMStoreFloat3(&hitPosition, HitPosition);
			}
		}
#endif // �����̃R�[�h�Q
		return hit;
	}
}