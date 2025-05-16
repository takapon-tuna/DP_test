#include "Scene/TwoBoneIKScene.h"

#include <imgui.h>
#include <ImGuizmo.h>

#include <algorithm>

#include "Graphics.h"

// �R���X�g���N�^
TwoBoneIKScene::TwoBoneIKScene() {
  ID3D11Device *device = Graphics::Instance().GetDevice();
  float screenWidth = Graphics::Instance().GetScreenWidth();
  float screenHeight = Graphics::Instance().GetScreenHeight();

  // �J�����ݒ�
  camera.SetPerspectiveFov(DirectX::XMConvertToRadians(45),  // ��p
                           screenWidth / screenHeight,  // ��ʃA�X�y�N�g��
                           0.1f,                        // �j�A�N���b�v
                           1000.0f                      // �t�@�[�N���b�v
  );
  camera.SetLookAt({10, 5, 5},  // ���_
                   {0, 0, 5},   // �����_
                   {0, 1, 0}    // ��x�N�g��
  );
  cameraController.SyncCameraToController(camera);

  // �{�[���f�[�^������
  for (int i = 0; i < _countof(bones); ++i) {
    Bone &bone = bones[i];

    if (i == 0) {
      bone.localPosition = {0, 0, 0};
      bone.localRotation = {0, 0, 0, 1};
      bone.parent = nullptr;
      bone.child = &bones[i + 1];
    } else {
      bone.localPosition = {0, 0, 2};
      bone.localRotation = {0, 0, 0, 1};
      bone.parent = &bones[i - 1];
      bone.child = (i != _countof(bones) - 1) ? &bones[i + 1] : nullptr;
    }

    // �s��v�Z
    DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(
        bone.localPosition.x, bone.localPosition.y, bone.localPosition.z);
    DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(
        DirectX::XMLoadFloat4(&bone.localRotation));
    DirectX::XMMATRIX LocalTransform = DirectX::XMMatrixMultiply(R, T);
    DirectX::XMMATRIX ParentWorldTransform =
        bone.parent != nullptr
            ? DirectX::XMLoadFloat4x4(&bone.parent->worldTransform)
            : DirectX::XMMatrixIdentity();
    DirectX::XMMATRIX WorldTransform =
        DirectX::XMMatrixMultiply(LocalTransform, ParentWorldTransform);
    DirectX::XMStoreFloat4x4(&bone.worldTransform, WorldTransform);
  }
  // �^�[�Q�b�g�s���������
  targetWorldTransform = bones[_countof(bones) - 1].worldTransform;

  // �|�[���^�[�Q�b�g�s���������
  Bone &midBone = bones[2];
  DirectX::XMMATRIX MidWorldTransform =
      DirectX::XMLoadFloat4x4(&midBone.worldTransform);
  DirectX::XMMATRIX PoleLocalTransform = DirectX::XMMatrixTranslation(0, 1, 0);
  DirectX::XMMATRIX PoleWorldTransform =
      DirectX::XMMatrixMultiply(PoleLocalTransform, MidWorldTransform);
  DirectX::XMStoreFloat4x4(&poleLocalTransform, PoleLocalTransform);
  DirectX::XMStoreFloat4x4(&poleWorldTransform, PoleWorldTransform);
}

// �X�V����
void TwoBoneIKScene::Update(float elapsedTime) {
  // �J�����X�V����
  cameraController.Update();
  cameraController.SyncControllerToCamera(camera);

  // �|�[���^�[�Q�b�g�̃��[���h�s����X�V
  {
    // ���ԃ{�[���t�߂Ƀ|�[���^�[�Q�b�g��z�u����
    Bone &midBone = bones[2];
    DirectX::XMMATRIX MidWorldTransform =
        DirectX::XMLoadFloat4x4(&midBone.worldTransform);
    DirectX::XMMATRIX PoleLocalTransform =
        DirectX::XMLoadFloat4x4(&poleLocalTransform);
    DirectX::XMMATRIX PoleWorldTransform =
        DirectX::XMMatrixMultiply(PoleLocalTransform, MidWorldTransform);
    DirectX::XMStoreFloat4x4(&poleWorldTransform, PoleWorldTransform);
  }
  // �|�[���^�[�Q�b�g���M�Y���œ�����
  const DirectX::XMFLOAT4X4 &view = camera.GetView();
  const DirectX::XMFLOAT4X4 &projection = camera.GetProjection();
  if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
    DirectX::XMMATRIX PoleWorldTransform =
        DirectX::XMLoadFloat4x4(&poleWorldTransform);
    ImGuizmo::Manipulate(&view._11, &projection._11, ImGuizmo::TRANSLATE,
                         ImGuizmo::LOCAL, &poleWorldTransform._11, nullptr);
    PoleWorldTransform = DirectX::XMLoadFloat4x4(&poleWorldTransform);

    Bone &midBone = bones[2];
    DirectX::XMMATRIX MidWorldTransform =
        DirectX::XMLoadFloat4x4(&midBone.worldTransform);
    DirectX::XMMATRIX InverseMidWorldTransform =
        DirectX::XMMatrixInverse(nullptr, MidWorldTransform);
    DirectX::XMMATRIX PoleLocalTransform =
        DirectX::XMMatrixMultiply(PoleWorldTransform, InverseMidWorldTransform);
    DirectX::XMStoreFloat4x4(&poleLocalTransform, PoleLocalTransform);
  }
  // �^�[�Q�b�g���M�Y���œ�����
  else {
    ImGuizmo::Manipulate(&view._11, &projection._11, ImGuizmo::TRANSLATE,
                         ImGuizmo::WORLD, &targetWorldTransform._11, nullptr);
  }

  // �^�[�Q�b�g���W�ƃ|�[�����W���擾
  DirectX::XMMATRIX TargetWorldTransform =
      DirectX::XMLoadFloat4x4(&targetWorldTransform);
  DirectX::XMVECTOR TargetWorldPosition = TargetWorldTransform.r[3];
  DirectX::XMMATRIX PoleWorldTransform =
      DirectX::XMLoadFloat4x4(&poleWorldTransform);
  DirectX::XMVECTOR PoleWorldPosition = PoleWorldTransform.r[3];

  // �����p���ɖ߂�
  for (Bone &bone : bones) {
    bone.localRotation = {0, 0, 0, 1};
  }
  bones[0].UpdateWorldTransforms();

  // 3�̊֐߂��g����IK���������
  Bone &rootBone = bones[1];  // �����{�[��
  Bone &midBone = bones[2];   // ���ԃ{�[��
  Bone &tipBone = bones[3];   // ��[�{�[��

  // TODO�@:��[�{�[�����W���^�[�Q�b�g���W�ɋ߂Â��悤�ɍ��{�{�[���ƒ��ԃ{�[������]���䂹��
  {
    // �e�{�[���̃��[���h���W���擾
    DirectX::XMMATRIX RootBoneWorldTransform, MidBoneWorldTransform,
        TipBoneWorldTransform;
    RootBoneWorldTransform = DirectX::XMLoadFloat4x4(&rootBone.worldTransform);
    MidBoneWorldTransform = DirectX::XMLoadFloat4x4(&midBone.worldTransform);
    TipBoneWorldTransform = DirectX::XMLoadFloat4x4(&tipBone.worldTransform);

    DirectX::XMVECTOR RootWorldPosition, MidWorldPosition, TipWorldPosition;
    RootWorldPosition = RootBoneWorldTransform.r[3];
    MidWorldPosition = MidBoneWorldTransform.r[3];
    TipWorldPosition = TipBoneWorldTransform.r[3];

    // ���� > ���ԁA���� > ��[�A���� > �Ώۂւ̃x�N�g�����Z�o
    DirectX::XMVECTOR RootMidVector, MidTipVector, RootTargetVector;
    RootMidVector =
        DirectX::XMVectorSubtract(MidWorldPosition, RootWorldPosition);
    MidTipVector =
        DirectX::XMVectorSubtract(TipWorldPosition, MidWorldPosition);
    RootTargetVector =
        DirectX::XMVectorSubtract(TargetWorldPosition, RootWorldPosition);

    float rootMidLength =
        DirectX::XMVectorGetX(DirectX::XMVector3Length(RootMidVector));
    float MidTipLength =
        DirectX::XMVectorGetX(DirectX::XMVector3Length(MidTipVector));
    float rootTargetLength =
        DirectX::XMVectorGetX(DirectX::XMVector3Length(RootTargetVector));

    // �{�[����]�֐�(�Q�̃x�N�g�������]�p�Ɖ�]�����Z�o����A�{�[������]������)
    auto RotateBone = [](Bone &bone, const DirectX::XMVECTOR &Direction1,
                         const DirectX::XMVECTOR &Direction2) {
      DirectX::XMVECTOR WorldAxis =
          DirectX::XMVector3Cross(Direction1, Direction2);
      if (DirectX::XMVector3Equal(WorldAxis, DirectX::XMVectorZero())) return;
      WorldAxis = DirectX::XMVector3Normalize(WorldAxis);

      DirectX::XMMATRIX ParentWorldTransform, InverseParentWorldTransfrom;
      ParentWorldTransform =
          DirectX::XMLoadFloat4x4(&bone.parent->worldTransform);
      InverseParentWorldTransfrom =
          DirectX::XMMatrixInverse(nullptr, ParentWorldTransform);
      DirectX::XMVECTOR LocalAxis = DirectX::XMVector3TransformNormal(
          WorldAxis, InverseParentWorldTransfrom);

      DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(Direction1, Direction2);
      float angle = acosf(std::clamp(DirectX::XMVectorGetX(Dot), -1.0f, 1.0f));

      DirectX::XMVECTOR AddLocalRotation =
          DirectX::XMQuaternionRotationAxis(LocalAxis, angle);
      DirectX::XMVECTOR LocalRotation =
          DirectX::XMLoadFloat4(&bone.localRotation);
      LocalRotation =
          DirectX::XMQuaternionMultiply(LocalRotation, AddLocalRotation);
      DirectX::XMStoreFloat4(&bone.localRotation, LocalRotation);
    };
    // ���� > ���Ԃ̃x�N�g������]
    RotateBone(rootBone, DirectX::XMVector3Normalize(RootMidVector),
               DirectX::XMVector3Normalize(RootTargetVector));
    // �����{�[���ȍ~�X�V
    rootBone.UpdateWorldTransforms();

    // ���� > �Ώۂ̃x�N�g�����{�[���̒��������Ȃ�ʏ������s��
    if (rootMidLength + MidTipLength > rootTargetLength) {
      float s = (rootMidLength + MidTipLength + rootTargetLength) / 2.0f;
      float S = sqrtf(s * (s - rootMidLength) * (s - MidTipLength) *
                      (s - rootTargetLength));

      float rootMidHeight = S * 2 / rootMidLength;

      float angle = asinf(rootMidHeight / rootTargetLength);
      if (angle > FLT_EPSILON) {
        // ��]����e��̃��O����ԕϊ�
        DirectX::XMMATRIX ParentWorldTransform, InverseParentWorldTransfrom;
        ParentWorldTransform =
            DirectX::XMLoadFloat4x4(&rootBone.parent->worldTransform);
        InverseParentWorldTransfrom =
            DirectX::XMMatrixInverse(nullptr, ParentWorldTransform);
        // ���� > �{�[���ւ̃x�N�g�����Z�o
        DirectX::XMVECTOR RootPoleVector;
        RootPoleVector =
            DirectX::XMVectorSubtract(PoleWorldPosition, RootWorldPosition);
        // ���� > �{�[���ւ̃x�N�g�������[�J����
        DirectX::XMVECTOR LocalRootPoleVector;
        LocalRootPoleVector = DirectX::XMVector3TransformNormal(
            RootPoleVector, InverseParentWorldTransfrom);
        LocalRootPoleVector = DirectX::XMVector3Normalize(LocalRootPoleVector);
        // ���� > ���Ԃ̃x�N�g�������[�J����
        DirectX::XMVECTOR LocalRootMidVector;
        LocalRootMidVector = DirectX::XMVector3TransformNormal(
            RootMidVector, InverseParentWorldTransfrom);
        LocalRootMidVector = DirectX::XMVector3Normalize(LocalRootMidVector);
        // ��]���Z�o
        DirectX::XMVECTOR RotationAxis;
        RotationAxis =
            DirectX::XMVector3Cross(LocalRootMidVector, LocalRootPoleVector);
        RotationAxis = DirectX::XMVector3Normalize(RotationAxis);
        // ��]����
        DirectX::XMVECTOR AddLocalRotation =
            DirectX::XMQuaternionRotationAxis(RotationAxis, angle);
        DirectX::XMVECTOR LocalRotation =
            DirectX::XMLoadFloat4(&rootBone.localRotation);
        LocalRotation =
            DirectX::XMQuaternionMultiply(LocalRotation, AddLocalRotation);
        DirectX::XMStoreFloat4(&rootBone.localRotation, LocalRotation);
        // �{�[���̍X�V
        rootBone.UpdateWorldTransforms();
      }
    }

    // ���� > ��[�̃x�N�g������]
    MidBoneWorldTransform = DirectX::XMLoadFloat4x4(&midBone.worldTransform);
    TipBoneWorldTransform = DirectX::XMLoadFloat4x4(&tipBone.worldTransform);
    MidWorldPosition = MidBoneWorldTransform.r[3];
    TipWorldPosition = TipBoneWorldTransform.r[3];

    // ���� > ��[�A���� > �Ώۂ̃x�N�g�����Z�o
    DirectX::XMVECTOR MidTargetVector;
    MidTargetVector =
        DirectX::XMVectorSubtract(TargetWorldPosition, MidWorldPosition);
    MidTipVector =
        DirectX::XMVectorSubtract(TipWorldPosition, MidWorldPosition);
    // ���� > ��[�̃x�N�g������]
    RotateBone(midBone, DirectX::XMVector3Normalize(MidTipVector),
               DirectX::XMVector3Normalize(MidTargetVector));
    // ���ԃ{�[���ȍ~���X�V
    midBone.UpdateWorldTransforms();
  }
}

// �`�揈��
void TwoBoneIKScene::Render(float elapsedTime) {
  ID3D11DeviceContext *dc = Graphics::Instance().GetDeviceContext();
  RenderState *renderState = Graphics::Instance().GetRenderState();
  PrimitiveRenderer *primitiveRenderer =
      Graphics::Instance().GetPrimitiveRenderer();
  ShapeRenderer *shapeRenderer = Graphics::Instance().GetShapeRenderer();

  // �{�[���`��
  for (size_t i = 1; i < _countof(bones) - 1; ++i) {
    const Bone &bone = bones[i];
    const Bone &child = bones[i + 1];

    DirectX::XMFLOAT4X4 world;
    DirectX::XMMATRIX World = DirectX::XMLoadFloat4x4(&bone.worldTransform);
    float length = child.localPosition.z;
    World.r[0] =
        DirectX::XMVectorScale(DirectX::XMVector3Normalize(World.r[0]), length);
    World.r[1] =
        DirectX::XMVectorScale(DirectX::XMVector3Normalize(World.r[1]), length);
    World.r[2] =
        DirectX::XMVectorScale(DirectX::XMVector3Normalize(World.r[2]), length);
    DirectX::XMStoreFloat4x4(&world, World);
    primitiveRenderer->DrawAxis(world, {1, 1, 0, 1});
    shapeRenderer->DrawBone(bone.worldTransform, length, {1, 1, 0, 1});
  }
  // �|�[���^�[�Q�b�g�`��
  DirectX::XMFLOAT3 poleWorldPosition = {
      poleWorldTransform._41, poleWorldTransform._42, poleWorldTransform._43};
  shapeRenderer->DrawSphere(poleWorldPosition, 0.1f, {0, 1, 0, 1});

  // �����_�[�X�e�[�g�ݒ�
  dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), nullptr,
                      0xFFFFFFFF);
  dc->OMSetDepthStencilState(
      renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
  dc->RSSetState(
      renderState->GetRasterizerState(RasterizerState::SolidCullNone));

  // �O���b�h�`��
  primitiveRenderer->DrawGrid(20, 1);
  primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(),
                            D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

  // �V�F�C�v�`��
  shapeRenderer->Render(dc, camera.GetView(), camera.GetProjection());
}

// GUI�`�揈��
void TwoBoneIKScene::DrawGUI() {
  ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();

  ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
  ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_Once);

  if (ImGui::Begin(u8"�Q�{�̃{�[��IK����")) {
    ImGui::Text(u8"Space�L�[�������F�|�[���^�[�Q�b�g����");
  }
  ImGui::End();
}
