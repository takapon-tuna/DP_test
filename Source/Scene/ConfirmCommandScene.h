#pragma once

#include <memory>
#include "Scene.h"
#include "Camera.h"
#include "Light.h"
#include "Model.h"
#include "Sprite.h"

// �R�}���h���菈���V�[��
class ConfirmCommandScene : public Scene
{
public:
	ConfirmCommandScene();
	~ConfirmCommandScene() override = default;

	// �X�V����
	void Update(float elapsedTime) override;

	// �`�揈��
	void Render(float elapsedTime) override;

	// GUI�`�揈��
	void DrawGUI() override;
	
private:
	// �F�G�H
	// �C�D�E
	// �@�A�B
	using InputKey = unsigned int;
	using Command = std::vector<InputKey>;

	// ���̓L�[(�����Ă�����)
	static const InputKey KEY_1 = (1 << 0);		// ����
	static const InputKey KEY_2 = (1 << 1);		// ��
	static const InputKey KEY_3 = (1 << 2);		// �E��
	static const InputKey KEY_4 = (1 << 3);		// ��
	static const InputKey KEY_5 = (1 << 4);		// ��
	static const InputKey KEY_6 = (1 << 5);		// �E
	static const InputKey KEY_7 = (1 << 6);		// ����
	static const InputKey KEY_8 = (1 << 7);		// ��
	static const InputKey KEY_9 = (1 << 8);		// �E��
	static const InputKey KEY_P = (1 << 9);		// �p���`
	static const InputKey KEY_K = (1 << 10);	// �L�b�N
	// ���̓L�[(�����Ă��Ȃ����)
	static const InputKey NOT_1 = (1 << 11);	// ����
	static const InputKey NOT_2 = (1 << 12);	// ��
	static const InputKey NOT_3 = (1 << 13);	// �E��
	static const InputKey NOT_4 = (1 << 14);	// ��
	static const InputKey NOT_5 = (1 << 15);	// ��
	static const InputKey NOT_6 = (1 << 16);	// �E
	static const InputKey NOT_7 = (1 << 17);	// ����
	static const InputKey NOT_8 = (1 << 18);	// ��
	static const InputKey NOT_9 = (1 << 19);	// �E��
	static const InputKey NOT_P = (1 << 20);	// �p���`
	static const InputKey NOT_K = (1 << 21);	// �L�b�N

	// ���̓L�[�ݒ�
	void SetInputKey(InputKey key);

	// �R�}���h�m�F
	bool ConfirmCommand(const Command& command, int frame);

	// ���̓L�[�`��
	void DrawInputKeys(ID3D11DeviceContext* dc, float x, float y, const InputKey inputKeys[]);

private:
	Camera								camera;
	LightManager						lightManager;
	std::unique_ptr<Sprite>				sprite;
	std::shared_ptr<Model>				character;
	std::vector<Model::NodePose>		nodePoses;
	int									animationIndex = -1;
	float								animationSeconds = 0;

	static const int MAX_INPUT_KEY = 60;
	InputKey							inputKeys[MAX_INPUT_KEY];
};
