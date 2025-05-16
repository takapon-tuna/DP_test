#include "Scene/ConfirmCommandScene.h"

#include <imgui.h>

#include "Graphics.h"

// �R���X�g���N�^
ConfirmCommandScene::ConfirmCommandScene() {
  ID3D11Device* device = Graphics::Instance().GetDevice();
  float screenWidth = Graphics::Instance().GetScreenWidth();
  float screenHeight = Graphics::Instance().GetScreenHeight();

  // �J�����ݒ�
  camera.SetPerspectiveFov(DirectX::XMConvertToRadians(45),  // ��p
                           screenWidth / screenHeight,  // ��ʃA�X�y�N�g��
                           0.1f,                        // �j�A�N���b�v
                           1000.0f                      // �t�@�[�N���b�v
  );
  camera.SetLookAt({8, 3, 0},  // ���_
                   {0, 2, 0},  // �����_
                   {0, 1, 0}   // ��x�N�g��
  );

  // �X�v���C�g�ǂݍ���
  sprite = std::make_unique<Sprite>(device, "Data/Sprite/InputKeyIcon.png");

  // ���f���ǂݍ���
  character = std::make_shared<Model>(
      device, "Data/Model/RPG-Character/RPG-Character.glb");
  character->GetNodePoses(nodePoses);
}

// �X�V����
void ConfirmCommandScene::Update(float elapsedTime) {
  // �L�[���͏��擾
  bool up = GetAsyncKeyState('W') & 0x8000;
  bool left = GetAsyncKeyState('A') & 0x8000;
  bool down = GetAsyncKeyState('S') & 0x8000;
  bool right = GetAsyncKeyState('D') & 0x8000;
  bool punch = GetAsyncKeyState('P') & 0x8000;
  bool kick = GetAsyncKeyState('K') & 0x8000;

  InputKey key = 0;
  key |= (!up && down && left && !right) ? KEY_1 : NOT_1;
  key |= (!up && down && !left && !right) ? KEY_2 : NOT_2;
  key |= (!up && down && !left && right) ? KEY_3 : NOT_3;
  key |= (!up && !down && left && !right) ? KEY_4 : NOT_4;
  key |= (!up && !down && !left && !right) ? KEY_5 : NOT_5;
  key |= (!up && !down && !left && right) ? KEY_6 : NOT_6;
  key |= (up && !down && left && !right) ? KEY_7 : NOT_7;
  key |= (up && !down && !left && !right) ? KEY_8 : NOT_8;
  key |= (up && !down && !left && right) ? KEY_9 : NOT_9;
  key |= punch ? KEY_P : NOT_P;
  key |= kick ? KEY_K : NOT_K;

  // �L�[���͐ݒ�
  SetInputKey(key);

  // �R�}���h���m�F
  static const Command command = Command({KEY_2, KEY_3, KEY_6, KEY_P});
  if (ConfirmCommand(command, 60)) {
    animationIndex = character->GetAnimationIndex("Slash");
    animationSeconds = 0;
  }

  // �w�莞�Ԃ̃A�j���[�V����
  if (animationIndex >= 0) {
    character->ComputeAnimation(animationIndex, animationSeconds, nodePoses);

    // ���ԍX�V
    const Model::Animation& animation =
        character->GetAnimations().at(animationIndex);
    animationSeconds += elapsedTime;
    if (animationSeconds > animation.secondsLength) {
      animationSeconds = animation.secondsLength;
    }
  }

  // �m�[�h�|�[�Y�X�V
  character->SetNodePoses(nodePoses);

  // �g�����X�t�H�[���X�V
  DirectX::XMFLOAT4X4 worldTransform;
  DirectX::XMStoreFloat4x4(&worldTransform, DirectX::XMMatrixIdentity());
  character->UpdateTransform(worldTransform);
}

// �`�揈��
void ConfirmCommandScene::Render(float elapsedTime) {
  ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
  RenderState* renderState = Graphics::Instance().GetRenderState();
  PrimitiveRenderer* primitiveRenderer =
      Graphics::Instance().GetPrimitiveRenderer();
  ModelRenderer* modelRenderer = Graphics::Instance().GetModelRenderer();

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

  // �`��R���e�L�X�g�ݒ�
  RenderContext rc;
  rc.deviceContext = dc;
  rc.renderState = renderState;
  rc.camera = &camera;
  rc.lightManager = &lightManager;

  // ���f���`��
  modelRenderer->Draw(ShaderId::Basic, character);
  modelRenderer->Render(rc);

  // �����_�[�X�e�[�g�ݒ�
  dc->OMSetBlendState(renderState->GetBlendState(BlendState::Transparency),
                      nullptr, 0xFFFFFFFF);
  dc->OMSetDepthStencilState(
      renderState->GetDepthStencilState(DepthState::NoTestNoWrite), 0);
  dc->RSSetState(
      renderState->GetRasterizerState(RasterizerState::SolidCullNone));
  // �T���v���X�e�[�g�ݒ�
  ID3D11SamplerState* samplerStates[] = {
      rc.renderState->GetSamplerState(SamplerState::LinearWrap)};
  dc->PSSetSamplers(0, _countof(samplerStates), samplerStates);

  // �L�[���̓o�b�t�@�`��
  DrawInputKeys(dc, 0, 160, inputKeys);
}

// GUI�`�揈��
void ConfirmCommandScene::DrawGUI() {
  ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();

  ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
  ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_Once);

  if (ImGui::Begin(u8"�R�}���h���菈��")) {
    ImGui::Text(u8"�����L�[�FWASD");
    ImGui::Text(u8"�p���`�L�[�FP");
    ImGui::Text(u8"�L�b�N�L�[�FK");
    ImGui::Text(u8"�R�}���h�F236P");
  }
  ImGui::End();
}

// ���̓L�[�ݒ�
void ConfirmCommandScene::SetInputKey(InputKey key) {
  {
    for (int i = MAX_INPUT_KEY - 1; i > 0; --i) {
      inputKeys[i] = inputKeys[i - 1];
    }
    inputKeys[0] = key;
  }
}

// �R�}���h�m�F
bool ConfirmCommandScene::ConfirmCommand(const Command& command, int frame) {
  {
    int index = 0;
    for (auto it = command.rbegin(); it != command.rend(); ++it) {
      InputKey key = *it;
      auto found = std::find_if(
          inputKeys + index, inputKeys + MAX_INPUT_KEY,
          [key](InputKey inputKey) { return (inputKey & key) == key; });
      if (found == inputKeys + MAX_INPUT_KEY ||
          std::distance(inputKeys, found) >= frame) {
        return false;
      }
      index = std::distance(inputKeys, found) + 1;
    }
  }
  return true;
}

// ���̓L�[�`��
void ConfirmCommandScene::DrawInputKeys(ID3D11DeviceContext* dc, float x,
                                        float y, const InputKey inputKeys[]) {
  const float iconSize = 112;
  const float size = iconSize * 0.5f;
  float offsetY = 0;
  // �R�}���h���t���Ɋm�F����
  for (int index = 0; index < MAX_INPUT_KEY; ++index) {
    InputKey key = inputKeys[index];

    float offsetX = 0;
    float column = 0, row = 0;
    if (key & KEY_1) {
      column = 1;
      row = 3;
    } else if (key & KEY_2) {
      column = 2;
      row = 3;
    } else if (key & KEY_3) {
      column = 3;
      row = 3;
    } else if (key & KEY_4) {
      column = 3;
      row = 1;
    }
    // else if (key & KEY_5) { column = 4; row = 1; }
    else if (key & KEY_6) {
      column = 0;
      row = 2;
    } else if (key & KEY_7) {
      column = 0;
      row = 0;
    } else if (key & KEY_8) {
      column = 1;
      row = 0;
    } else if (key & KEY_9) {
      column = 2;
      row = 0;
    }
    if (key & (KEY_1 | KEY_2 | KEY_3 | KEY_4 | KEY_6 | KEY_7 | KEY_8 | KEY_9)) {
      sprite->Render(dc, x + offsetX, y + offsetY, 0, size, size,
                     iconSize * column, iconSize * row, iconSize, iconSize, 0,
                     1, 1, 1, 1);
      offsetX += size;
    }
    if (key & KEY_P) {
      sprite->Render(dc, x + offsetX, y + offsetY, 0, size, size, iconSize * 1,
                     iconSize * 1, iconSize, iconSize, 0, 1, 1, 1, 1);
      offsetX += size;
    }
    if (key & KEY_K) {
      sprite->Render(dc, x + offsetX, y + offsetY, 0, size, size, iconSize * 2,
                     iconSize * 1, iconSize, iconSize, 0, 1, 1, 1, 1);
      offsetX += size;
    }

    // �����L�[�������ꍇ�̓X�L�b�v
    while (index < MAX_INPUT_KEY && (inputKeys[index] & key) == key) {
      index++;
    }
    if (offsetX > 0) {
      offsetY += size;
    }
  }
}
