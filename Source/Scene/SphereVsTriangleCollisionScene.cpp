#include <imgui.h>
#include <ImGuizmo.h>
#include "Graphics.h"
#include "Scene/SphereVsTriangleCollisionScene.h"

// �R���X�g���N�^
SphereVsTriangleCollisionScene::SphereVsTriangleCollisionScene() {
  ID3D11Device *device = Graphics::Instance().GetDevice();
  float screenWidth = Graphics::Instance().GetScreenWidth();
  float screenHeight = Graphics::Instance().GetScreenHeight();

  // �J�����ݒ�
  camera.SetPerspectiveFov(DirectX::XMConvertToRadians(45),  // ��p
                           screenWidth / screenHeight,  // ��ʃA�X�y�N�g��
                           0.1f,                        // �j�A�N���b�v
                           1000.0f                      // �t�@�[�N���b�v
  );
  camera.SetLookAt({0, 10, 10},  // ���_
                   {0, 0, 0},    // �����_
                   {0, 1, 0}     // ��x�N�g��
  );
  cameraController.SyncCameraToController(camera);

  // �I�u�W�F�N�g������
  obj.position = {0, 2, 1};
}

// �X�V����
void SphereVsTriangleCollisionScene::Update(float elapsedTime) {
  // �J�����X�V����
  cameraController.Update();
  cameraController.SyncControllerToCamera(camera);

  // �I�u�W�F�N�g�ړ�����
  const float speed = 2.0f * elapsedTime;
  DirectX::XMFLOAT3 vec = {0, 0, 0};
  if (GetAsyncKeyState(VK_UP) & 0x8000) {
    vec.z += speed;
  }
  if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
    vec.z -= speed;
  }
  if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
    vec.x += speed;
  }
  if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
    vec.x -= speed;
  }
  if (GetAsyncKeyState('Z') & 0x8000) {
    vec.y += speed;
  }
  if (GetAsyncKeyState('X') & 0x8000) {
    vec.y -= speed;
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
  obj.position.x += frontX * vec.z + rightX * vec.x;
  obj.position.z += frontZ * vec.z + rightZ * vec.x;
  obj.position.y += vec.y;
}

// �`�揈��
void SphereVsTriangleCollisionScene::Render(float elapsedTime) {
  ID3D11DeviceContext *dc = Graphics::Instance().GetDeviceContext();
  RenderState *renderState = Graphics::Instance().GetRenderState();
  PrimitiveRenderer *primitiveRenderer =
      Graphics::Instance().GetPrimitiveRenderer();
  ShapeRenderer *shapeRenderer = Graphics::Instance().GetShapeRenderer();

  // �O�p�`���_
  DirectX::XMFLOAT3 v[3] = {
      {-2, 2, 0},
      {0, 1, 2},
      {2, 2, 0},
  };
  // ���ƎO�p�`�̏Փˏ���
  DirectX::XMFLOAT4 c = {0, 1, 0, 1};
  DirectX::XMFLOAT3 position, normal;
  if (SphereIntersectTriangle(obj.position, obj.radius, v[0], v[1], v[2],
                              position, normal)) {
    obj.position = position;
    c = {1, 0, 0, 1};
  }

  // �����_�[�X�e�[�g�ݒ�
  dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), nullptr,
                      0xFFFFFFFF);
  dc->OMSetDepthStencilState(
      renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
  dc->RSSetState(
      renderState->GetRasterizerState(RasterizerState::SolidCullBack));

  // �O�p�`�|���S���`��
  primitiveRenderer->AddVertex(v[0], c);
  primitiveRenderer->AddVertex(v[1], c);
  primitiveRenderer->AddVertex(v[2], c);
  primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(),
                            D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  dc->RSSetState(
      renderState->GetRasterizerState(RasterizerState::SolidCullNone));

  // �O�p�`�G�b�W�`��
  primitiveRenderer->AddVertex(v[0], {0, 0, 1, 1});
  primitiveRenderer->AddVertex(v[1], {0, 0, 1, 1});
  primitiveRenderer->AddVertex(v[2], {0, 0, 1, 1});
  primitiveRenderer->AddVertex(v[0], {0, 0, 1, 1});
  primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(),
                            D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

  // �O���b�h�`��
  primitiveRenderer->DrawGrid(20, 1);
  primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(),
                            D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

  // �V�F�C�v�`��
  shapeRenderer->DrawSphere(obj.position, obj.radius, obj.color);
  shapeRenderer->Render(dc, camera.GetView(), camera.GetProjection());
}

// GUI�`�揈��
void SphereVsTriangleCollisionScene::DrawGUI() {
  ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
  ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
  ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Once);

  if (ImGui::Begin(u8"���ƎO�p�`�̏Փˏ���")) {
    ImGui::Text(u8"�O�㍶�E�ړ�����F�����L�[");
    ImGui::Text(u8"�㉺�ړ�����FZX");
  }
  ImGui::End();
}

// ���ƎO�p�`�Ƃ̌����𔻒肷��
bool SphereVsTriangleCollisionScene::SphereIntersectTriangle(
    const DirectX::XMFLOAT3 &sphereCenter, const float sphereRadius,
    const DirectX::XMFLOAT3 &triangleVertexA,
    const DirectX::XMFLOAT3 &triangleVertexB,
    const DirectX::XMFLOAT3 &triangleVertexC, DirectX::XMFLOAT3 &hitPosition,
    DirectX::XMFLOAT3 &hitNormal) {
  // TODO�@:�����O�p�`�ɉ����o�����Փˏ�������������
  {
    // �O�p�`�̖@�����v�Z
    DirectX::XMVECTOR v0 = DirectX::XMLoadFloat3(&triangleVertexA);
    DirectX::XMVECTOR v1 = DirectX::XMLoadFloat3(&triangleVertexB);
    DirectX::XMVECTOR v2 = DirectX::XMLoadFloat3(&triangleVertexC);
    DirectX::XMVECTOR edge1 = DirectX::XMVectorSubtract(v1, v0);
    DirectX::XMVECTOR edge2 = DirectX::XMVectorSubtract(v2, v0);
    DirectX::XMVECTOR normal = DirectX::XMVector3Cross(edge1, edge2);
    normal = DirectX::XMVector3Normalize(normal);

    // ���̒��S����O�p�`�̕��ʂ܂ł̋������v�Z
    DirectX::XMVECTOR spherePos = DirectX::XMLoadFloat3(&sphereCenter);
    DirectX::XMVECTOR toPlane = DirectX::XMVectorSubtract(spherePos, v0);
    float distance =
        DirectX::XMVectorGetX(DirectX::XMVector3Dot(toPlane, normal));

    // �Փ˔���
    if (fabs(distance) <= sphereRadius) {
      // �Փˈʒu���v�Z
      DirectX::XMVECTOR correction =
          DirectX::XMVectorScale(normal, sphereRadius - distance);
      DirectX::XMVECTOR newPos = DirectX::XMVectorAdd(spherePos, correction);

      // �O�p�`�̓����ɂ��邩���`�F�b�N
      DirectX::XMVECTOR c0 =
          DirectX::XMVector3Cross(DirectX::XMVectorSubtract(v1, v0),
                                  DirectX::XMVectorSubtract(newPos, v0));
      DirectX::XMVECTOR c1 =
          DirectX::XMVector3Cross(DirectX::XMVectorSubtract(v2, v1),
                                  DirectX::XMVectorSubtract(newPos, v1));
      DirectX::XMVECTOR c2 =
          DirectX::XMVector3Cross(DirectX::XMVectorSubtract(v0, v2),
                                  DirectX::XMVectorSubtract(newPos, v2));

      if (DirectX::XMVectorGetX(DirectX::XMVector3Dot(c0, normal)) >= 0 &&
          DirectX::XMVectorGetX(DirectX::XMVector3Dot(c1, normal)) >= 0 &&
          DirectX::XMVectorGetX(DirectX::XMVector3Dot(c2, normal)) >= 0) {
        // �Փˈʒu�Ɩ@����ݒ�
        DirectX::XMStoreFloat3(&hitPosition, newPos);
        DirectX::XMStoreFloat3(&hitNormal, normal);
        return true;
      } else {
        // �O�p�`�̕ӏ�̍ŋߐړ_���v�Z
        DirectX::XMVECTOR closestPoint = newPos;
        float minDistance = FLT_MAX;

        auto checkEdge = [&](DirectX::XMVECTOR p0, DirectX::XMVECTOR p1) {
          DirectX::XMVECTOR edge = DirectX::XMVectorSubtract(p1, p0);
          DirectX::XMVECTOR toSphere = DirectX::XMVectorSubtract(spherePos, p0);
          float t =
              DirectX::XMVectorGetX(DirectX::XMVector3Dot(toSphere, edge)) /
              DirectX::XMVectorGetX(DirectX::XMVector3Dot(edge, edge));
          if (t < 0.0f) t = 0.0f;
          if (t > 1.0f) t = 1.0f;
          DirectX::XMVECTOR projection =
              DirectX::XMVectorAdd(p0, DirectX::XMVectorScale(edge, t));
          DirectX::XMVECTOR diff =
              DirectX::XMVectorSubtract(spherePos, projection);
          float dist = DirectX::XMVectorGetX(DirectX::XMVector3Length(diff));
          if (dist < minDistance) {
            minDistance = dist;
            closestPoint = projection;
          }
        };

        checkEdge(v0, v1);
        checkEdge(v1, v2);
        checkEdge(v2, v0);

        if (minDistance <= sphereRadius) {
          DirectX::XMVECTOR correction = DirectX::XMVectorScale(
              DirectX::XMVector3Normalize(
                  DirectX::XMVectorSubtract(spherePos, closestPoint)),
              sphereRadius - minDistance);
          DirectX::XMVECTOR newPos =
              DirectX::XMVectorAdd(spherePos, correction);
          DirectX::XMStoreFloat3(&hitPosition, newPos);
          DirectX::XMStoreFloat3(
              &hitNormal, DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(
                              spherePos, closestPoint)));
          return true;
        }
      }
    }

    return false;
  }
}
