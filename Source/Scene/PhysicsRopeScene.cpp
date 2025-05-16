#include <imgui.h>
#include <ImGuizmo.h>
#include "Graphics.h"
#include "Scene/PhysicsRopeScene.h"

// コンストラクタ
PhysicsRopeScene::PhysicsRopeScene() {
  ID3D11Device *device = Graphics::Instance().GetDevice();
  float screenWidth = Graphics::Instance().GetScreenWidth();
  float screenHeight = Graphics::Instance().GetScreenHeight();

  // カメラ設定
  camera.SetPerspectiveFov(DirectX::XMConvertToRadians(45),  // 画角
                           screenWidth / screenHeight,  // 画面アスペクト比
                           0.1f,                        // ニアクリップ
                           1000.0f                      // ファークリップ
  );
  camera.SetLookAt({0, 3, -10},  // 視点
                   {0, 3, 0},    // 注視点
                   {0, 1, 0}     // 上ベクトル
  );
  cameraController.SyncCameraToController(camera);

  // ジョイント初期化
  for (int i = 0; i < _countof(joints); ++i) {
    Joint &joint = joints[i];
    joint.position.x = i * jointInterval;
    joint.position.y = 3.0f;
    joint.position.z = 0.0f;
    joint.oldPosition = joint.position;
  }
}

// 更新処理
void PhysicsRopeScene::Update(float elapsedTime) {
  // カメラ更新処理
  cameraController.Update();
  cameraController.SyncControllerToCamera(camera);

  // ルートジョイントをギズモで動かす
  DirectX::XMFLOAT3 &rootJointPosition = joints[0].position;
  const DirectX::XMFLOAT4X4 &view = camera.GetView();
  const DirectX::XMFLOAT4X4 &projection = camera.GetProjection();
  DirectX::XMFLOAT4X4 world;
  DirectX::XMStoreFloat4x4(&world, DirectX::XMMatrixTranslation(
                                       rootJointPosition.x, rootJointPosition.y,
                                       rootJointPosition.z));
  ImGuizmo::Manipulate(&view._11, &projection._11, ImGuizmo::TRANSLATE,
                       ImGuizmo::WORLD, &world._11, nullptr);
  rootJointPosition.x = world._41;
  rootJointPosition.y = world._42;
  rootJointPosition.z = world._43;

  // ロープシミュレーション
  const float gravity = this->gravity * elapsedTime;
  for (int i = 1; i < _countof(joints); ++i) {
    Joint &parentJoint = joints[i - 1];
    Joint &joint = joints[i];

    // TODO①:ジョイントの位置を制御し、ロープ表現の物理処理を実装せよ
    {
      // 重力の影響を適用
      DirectX::XMFLOAT3 velocity = {
          joint.position.x - joint.oldPosition.x,
          joint.position.y - joint.oldPosition.y - gravity,
          joint.position.z - joint.oldPosition.z};

      // 位置を更新
      joint.oldPosition = joint.position;
      joint.position.x += velocity.x;
      joint.position.y += velocity.y;
      joint.position.z += velocity.z;

      // 親ジョイントとの距離を保つ
      DirectX::XMFLOAT3 direction = {joint.position.x - parentJoint.position.x,
                                     joint.position.y - parentJoint.position.y,
                                     joint.position.z - parentJoint.position.z};
      float length =
          sqrtf(direction.x * direction.x + direction.y * direction.y +
                direction.z * direction.z);
      float difference = jointInterval - length;
      float correctionFactor = difference / length / 2.0f;

      joint.position.x += direction.x * correctionFactor;
      joint.position.y += direction.y * correctionFactor;
      joint.position.z += direction.z * correctionFactor;

      parentJoint.position.x -= direction.x * correctionFactor;
      parentJoint.position.y -= direction.y * correctionFactor;
      parentJoint.position.z -= direction.z * correctionFactor;
    }
  }

  if (fixRootJoint) {
    joints[0].position = {0.0f, 3.0f, 0.0f};
    joints[0].oldPosition = joints[0].position;
  }
}

// 描画処理
void PhysicsRopeScene::Render(float elapsedTime) {
  ID3D11DeviceContext *dc = Graphics::Instance().GetDeviceContext();
  RenderState *renderState = Graphics::Instance().GetRenderState();
  PrimitiveRenderer *primitiveRenderer =
      Graphics::Instance().GetPrimitiveRenderer();

  // レンダーステート設定
  dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), nullptr,
                      0xFFFFFFFF);
  dc->OMSetDepthStencilState(
      renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
  dc->RSSetState(
      renderState->GetRasterizerState(RasterizerState::SolidCullNone));

  // ロープ描画
  for (const Joint &joint : joints) {
    primitiveRenderer->AddVertex(joint.position, {1, 1, 0, 1});
  }
  primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(),
                            D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

  // グリッド描画
  primitiveRenderer->DrawGrid(20, 1);
  primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(),
                            D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}

// GUI描画処理
void PhysicsRopeScene::DrawGUI() {
  ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
  ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
  ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Once);

  if (ImGui::Begin(u8"揺れもの処理(ロープ)")) {
    ImGui::SliderFloat("gravity", &gravity, 0, 1, "%.3f");
    // ImGui::Checkbox("Fix Root Joint", &fixRootJoint);
  }
  ImGui::End();
}
