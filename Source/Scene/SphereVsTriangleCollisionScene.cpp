#include <imgui.h>
#include <ImGuizmo.h>
#include "Graphics.h"
#include "Scene/SphereVsTriangleCollisionScene.h"

// コンストラクタ
SphereVsTriangleCollisionScene::SphereVsTriangleCollisionScene() {
  ID3D11Device *device = Graphics::Instance().GetDevice();
  float screenWidth = Graphics::Instance().GetScreenWidth();
  float screenHeight = Graphics::Instance().GetScreenHeight();

  // カメラ設定
  camera.SetPerspectiveFov(DirectX::XMConvertToRadians(45),  // 画角
                           screenWidth / screenHeight,  // 画面アスペクト比
                           0.1f,                        // ニアクリップ
                           1000.0f                      // ファークリップ
  );
  camera.SetLookAt({0, 10, 10},  // 視点
                   {0, 0, 0},    // 注視点
                   {0, 1, 0}     // 上ベクトル
  );
  cameraController.SyncCameraToController(camera);

  // オブジェクト初期化
  obj.position = {0, 2, 1};
}

// 更新処理
void SphereVsTriangleCollisionScene::Update(float elapsedTime) {
  // カメラ更新処理
  cameraController.Update();
  cameraController.SyncControllerToCamera(camera);

  // オブジェクト移動操作
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
  // カメラの向きを考慮した移動処理
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

// 描画処理
void SphereVsTriangleCollisionScene::Render(float elapsedTime) {
  ID3D11DeviceContext *dc = Graphics::Instance().GetDeviceContext();
  RenderState *renderState = Graphics::Instance().GetRenderState();
  PrimitiveRenderer *primitiveRenderer =
      Graphics::Instance().GetPrimitiveRenderer();
  ShapeRenderer *shapeRenderer = Graphics::Instance().GetShapeRenderer();

  // 三角形頂点
  DirectX::XMFLOAT3 v[3] = {
      {-2, 2, 0},
      {0, 1, 2},
      {2, 2, 0},
  };
  // 球と三角形の衝突処理
  DirectX::XMFLOAT4 c = {0, 1, 0, 1};
  DirectX::XMFLOAT3 position, normal;
  if (SphereIntersectTriangle(obj.position, obj.radius, v[0], v[1], v[2],
                              position, normal)) {
    obj.position = position;
    c = {1, 0, 0, 1};
  }

  // レンダーステート設定
  dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), nullptr,
                      0xFFFFFFFF);
  dc->OMSetDepthStencilState(
      renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
  dc->RSSetState(
      renderState->GetRasterizerState(RasterizerState::SolidCullBack));

  // 三角形ポリゴン描画
  primitiveRenderer->AddVertex(v[0], c);
  primitiveRenderer->AddVertex(v[1], c);
  primitiveRenderer->AddVertex(v[2], c);
  primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(),
                            D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  dc->RSSetState(
      renderState->GetRasterizerState(RasterizerState::SolidCullNone));

  // 三角形エッジ描画
  primitiveRenderer->AddVertex(v[0], {0, 0, 1, 1});
  primitiveRenderer->AddVertex(v[1], {0, 0, 1, 1});
  primitiveRenderer->AddVertex(v[2], {0, 0, 1, 1});
  primitiveRenderer->AddVertex(v[0], {0, 0, 1, 1});
  primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(),
                            D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

  // グリッド描画
  primitiveRenderer->DrawGrid(20, 1);
  primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(),
                            D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

  // シェイプ描画
  shapeRenderer->DrawSphere(obj.position, obj.radius, obj.color);
  shapeRenderer->Render(dc, camera.GetView(), camera.GetProjection());
}

// GUI描画処理
void SphereVsTriangleCollisionScene::DrawGUI() {
  ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
  ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
  ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Once);

  if (ImGui::Begin(u8"球と三角形の衝突処理")) {
    ImGui::Text(u8"前後左右移動操作：方向キー");
    ImGui::Text(u8"上下移動操作：ZX");
  }
  ImGui::End();
}

// 球と三角形との交差を判定する
bool SphereVsTriangleCollisionScene::SphereIntersectTriangle(
    const DirectX::XMFLOAT3 &sphereCenter, const float sphereRadius,
    const DirectX::XMFLOAT3 &triangleVertexA,
    const DirectX::XMFLOAT3 &triangleVertexB,
    const DirectX::XMFLOAT3 &triangleVertexC, DirectX::XMFLOAT3 &hitPosition,
    DirectX::XMFLOAT3 &hitNormal) {
  // TODO①:球が三角形に押し出される衝突処理を実装せよ
  {
    // 三角形の法線を計算
    DirectX::XMVECTOR v0 = DirectX::XMLoadFloat3(&triangleVertexA);
    DirectX::XMVECTOR v1 = DirectX::XMLoadFloat3(&triangleVertexB);
    DirectX::XMVECTOR v2 = DirectX::XMLoadFloat3(&triangleVertexC);
    DirectX::XMVECTOR edge1 = DirectX::XMVectorSubtract(v1, v0);
    DirectX::XMVECTOR edge2 = DirectX::XMVectorSubtract(v2, v0);
    DirectX::XMVECTOR normal = DirectX::XMVector3Cross(edge1, edge2);
    normal = DirectX::XMVector3Normalize(normal);

    // 球の中心から三角形の平面までの距離を計算
    DirectX::XMVECTOR spherePos = DirectX::XMLoadFloat3(&sphereCenter);
    DirectX::XMVECTOR toPlane = DirectX::XMVectorSubtract(spherePos, v0);
    float distance =
        DirectX::XMVectorGetX(DirectX::XMVector3Dot(toPlane, normal));

    // 衝突判定
    if (fabs(distance) <= sphereRadius) {
      // 衝突位置を計算
      DirectX::XMVECTOR correction =
          DirectX::XMVectorScale(normal, sphereRadius - distance);
      DirectX::XMVECTOR newPos = DirectX::XMVectorAdd(spherePos, correction);

      // 三角形の内部にあるかをチェック
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
        // 衝突位置と法線を設定
        DirectX::XMStoreFloat3(&hitPosition, newPos);
        DirectX::XMStoreFloat3(&hitNormal, normal);
        return true;
      } else {
        // 三角形の辺上の最近接点を計算
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
