#include <memory>
#include <sstream>
#include <imgui.h>

#include "Framework.h"
#include "Graphics.h"
#include "ImGuiRenderer.h"
#include "Scene/ModelViewerScene.h"
#include "Scene/WeightedCollisionScene.h"
#include "Scene/RaceRankingScene.h"
#include "Scene/SphereCastMoveScene.h"
#include "Scene/SwordTrailScene.h"
#include "Scene/RootMotionScene.h"
#include "Scene/ConfirmCommandScene.h"
#include "Scene/SpaceDivisionRaycastScene.h"
#include "Scene/SortingAndFilteringScene.h"
#include "Scene/LookAtScene.h"
#include "Scene/TwoBoneIKScene.h"
#include "Scene/SphereVsTriangleCollisionScene.h"
#include "Scene/PhysicsRopeScene.h"
#include "Scene/PhysicsBoneScene.h"
#include "Scene/CCDIKScene.h"

// ���������Ԋu�ݒ�
static const int syncInterval = 1;

// �R���X�g���N�^
Framework::Framework(HWND hWnd)
	: hWnd(hWnd)
{
	hDC = GetDC(hWnd);

	// �O���t�B�b�N�X������
	Graphics::Instance().Initialize(hWnd);

	// IMGUI������
	ImGuiRenderer::Initialize(hWnd, Graphics::Instance().GetDevice(), Graphics::Instance().GetDeviceContext());

	// �V�[��������
	//scene = std::make_unique<ModelViewerScene>();
	scene = std::make_unique<WeightedCollisionScene>();
	//scene = std::make_unique<RaceRankingScene>();
	//scene = std::make_unique<SphereCastMoveScene>();
	//scene = std::make_unique<SwordTrailScene>();
	//scene = std::make_unique<RootMotionScene>();
	//scene = std::make_unique<ConfirmCommandScene>();
	//scene = std::make_unique<SpaceDivisionRaycastScene>();
	//scene = std::make_unique<SortingAndFilteringScene>();
	//scene = std::make_unique<LookAtScene>();
	//scene = std::make_unique<TwoBoneIKScene>();
	//scene = std::make_unique<SphereVsTriangleCollisionScene>();
	//scene = std::make_unique<PhysicsRopeScene>();
	//scene = std::make_unique<PhysicsBoneScene>();
	//scene = std::make_unique<CCDIKScene>();
}

// �f�X�g���N�^
Framework::~Framework()
{
	// IMGUI�I����
	ImGuiRenderer::Finalize();

	ReleaseDC(hWnd, hDC);
}

// �X�V����
void Framework::Update(float elapsedTime)
{
	// IMGUI�t���[���J�n����	
	ImGuiRenderer::NewFrame();

	// �V�[���X�V����
	scene->Update(elapsedTime);
}

// �`�揈��
void Framework::Render(float elapsedTime)
{
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();

	// ��ʃN���A
	Graphics::Instance().Clear(0.5f, 0.5f, 0.5f, 1);

	// �����_�[�^�[�Q�b�g�ݒ�
	Graphics::Instance().SetRenderTargets();

	// �V�[���`�揈��
	scene->Render(elapsedTime);

	// �V�[��GUI�`�揈��
	scene->DrawGUI();

	// �V�[���؂�ւ�GUI
	SceneSelectGUI();
#if 0
	// IMGUI�f���E�C���h�E�`��iIMGUI�@�\�e�X�g�p�j
	ImGui::ShowDemoWindow();
#endif
	// IMGUI�`��
	ImGuiRenderer::Render(dc);

	// ��ʕ\��
	Graphics::Instance().Present(syncInterval);
}

template<class T>
void Framework::ChangeSceneButtonGUI(const char* name)
{
	if (ImGui::Button(name))
	{
		scene = std::make_unique<T>();
	}
}

// �V�[���؂�ւ�GUI
void Framework::SceneSelectGUI()
{
	ImVec2 displaySize = ImGui::GetIO().DisplaySize;
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
	float width = 210;
	float height = 460;
	ImGui::SetNextWindowPos(ImVec2(pos.x + displaySize.x - width - 10, pos.y + 10), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Once);

	if (ImGui::Begin("Scene"))
	{
		ChangeSceneButtonGUI<ModelViewerScene>(u8"00.���f���r���[�A");
		ChangeSceneButtonGUI<WeightedCollisionScene>(u8"01.�d�݂̂���Փˏ���");
		ChangeSceneButtonGUI<RaceRankingScene>(u8"02.���[�X���ʔ��菈��");
		ChangeSceneButtonGUI<SphereCastMoveScene>(u8"03.�X�t�B�A�L���X�g�ړ�����");
		ChangeSceneButtonGUI<SwordTrailScene>(u8"04.���̋O�Տ���");
		ChangeSceneButtonGUI<RootMotionScene>(u8"05.���[�g���[�V��������");
		ChangeSceneButtonGUI<ConfirmCommandScene>(u8"06.�R�}���h���菈��");
		ChangeSceneButtonGUI<SpaceDivisionRaycastScene>(u8"07.��ԕ������C�L���X�g");
		ChangeSceneButtonGUI<SortingAndFilteringScene>(u8"08.�\�[�g���t�B���^�����O����");
		ChangeSceneButtonGUI<LookAtScene>(u8"09.���b�N�A�b�g����");
		ChangeSceneButtonGUI<TwoBoneIKScene>(u8"10.2�{�̃{�[��IK����");
		ChangeSceneButtonGUI<SphereVsTriangleCollisionScene>(u8"11.���ƎO�p�`�̏Փˏ���");
		ChangeSceneButtonGUI<PhysicsRopeScene>(u8"12.�h����̏���(���[�v)");
		ChangeSceneButtonGUI<PhysicsBoneScene>(u8"13.�h����̏���(�{�[��)");
		ChangeSceneButtonGUI<CCDIKScene>(u8"14.3�{�ȏ�̃{�[��IK����");
	}
	ImGui::End();
}

// �t���[�����[�g�v�Z
void Framework::CalculateFrameStats()
{
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.
	static int frames = 0;
	static float time_tlapsed = 0.0f;

	frames++;

	// Compute averages over one second period.
	if ((timer.TimeStamp() - time_tlapsed) >= 1.0f)
	{
		float fps = static_cast<float>(frames); // fps = frameCnt / 1
		float mspf = 1000.0f / fps;
		std::ostringstream outs;
		outs.precision(6);
		outs << "FPS : " << fps << " / " << "Frame Time : " << mspf << " (ms)";
		SetWindowTextA(hWnd, outs.str().c_str());

		// Reset for next average.
		frames = 0;
		time_tlapsed += 1.0f;
	}
}

// �A�v���P�[�V�������[�v
int Framework::Run()
{
	MSG msg = {};

	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			timer.Tick();
			CalculateFrameStats();

			float elapsedTime = syncInterval == 0
				? timer.TimeInterval()
				: syncInterval / static_cast<float>(GetDeviceCaps(hDC, VREFRESH))
				;
			Update(elapsedTime);
			Render(elapsedTime);
		}
	}
	return static_cast<int>(msg.wParam);
}

// ���b�Z�[�W�n���h��
LRESULT CALLBACK Framework::HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGuiRenderer::HandleMessage(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc;
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CREATE:
		break;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) PostMessage(hWnd, WM_CLOSE, 0, 0);
		break;
	case WM_ENTERSIZEMOVE:
		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
		timer.Stop();
		break;
	case WM_EXITSIZEMOVE:
		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
		timer.Start();
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}
