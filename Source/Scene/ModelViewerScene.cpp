#include <functional>
#include <imgui.h>
#include "ModelViewerScene.h"
#include "Graphics.h"
#include "TransformUtils.h"
#include "Dialog.h"

// �R���X�g���N�^
ModelViewerScene::ModelViewerScene()
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
		{ 0, 3, 5 },		// ���_
		{ 0, 0, 0 },		// �����_
		{ 0, 1, 0 }			// ��x�N�g��
	);
	cameraController.SyncCameraToController(camera);

	shaderId = static_cast<int>(ShaderId::Basic);

	// ���C�g�ݒ�
	DirectionalLight directionalLight;
	directionalLight.direction = { 0, -1, -1 };
	directionalLight.color = { 1, 1, 1 };
	lightManager.SetDirectionalLight(directionalLight);
}

// �X�V����
void ModelViewerScene::Update(float elapsedTime)
{
	// �J�����X�V����
	cameraController.Update();
	cameraController.SyncControllerToCamera(camera);

	if (model != nullptr)
	{
		// �A�j���[�V�����X�V
		if (animationPlaying && currentAnimationIndex >= 0)
		{
			model->ComputeAnimation(currentAnimationIndex, currentAnimationSeconds, nodePoses);
			model->SetNodePoses(nodePoses);

			// ���ԍX�V
			const Model::Animation& animation = model->GetAnimations().at(currentAnimationIndex);
			currentAnimationSeconds += elapsedTime * animationSpeed;
			if (currentAnimationSeconds > animation.secondsLength)
			{
				if (animationLoop)
				{
					currentAnimationSeconds -= animation.secondsLength;
				}
				else
				{
					currentAnimationSeconds = animation.secondsLength;
				}
			}
		}

		// �g�����X�t�H�[���X�V
		DirectX::XMFLOAT4X4 worldTransform;
		DirectX::XMStoreFloat4x4(&worldTransform, DirectX::XMMatrixIdentity());
		model->UpdateTransform(worldTransform);
	}

}

// �`�揈��
void ModelViewerScene::Render(float elapsedTime)
{
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
	RenderState* renderState = Graphics::Instance().GetRenderState();
	PrimitiveRenderer* primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();
	ModelRenderer* modelRenderer = Graphics::Instance().GetModelRenderer();

	// �O���b�h�`��
	primitiveRenderer->DrawGrid(20, 1);
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// �`��R���e�L�X�g�ݒ�
	RenderContext rc;
	rc.deviceContext = dc;
	rc.renderState = renderState;
	rc.camera = &camera;
	rc.lightManager = &lightManager;

	// �`��
	if (model != nullptr)
	{
		// ���f���`��
		modelRenderer->Draw(static_cast<ShaderId>(shaderId), model);
		modelRenderer->Render(rc);

		// �����_�[�X�e�[�g�ݒ�
		dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), nullptr, 0xFFFFFFFF);
		dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::NoTestNoWrite), 0);
		dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullNone));

		// ���`��
		const std::vector<Model::Node>& nodes = model->GetNodes();
		if (selectionNode != nullptr)
		{
			DirectX::XMFLOAT3 p, x, y, z;
			const float length = 0.1f;
			DirectX::XMMATRIX W = DirectX::XMLoadFloat4x4(&selectionNode->worldTransform);
			DirectX::XMVECTOR X = DirectX::XMVector3Transform(DirectX::XMVectorSet(length, 0, 0, 0), W);
			DirectX::XMVECTOR Y = DirectX::XMVector3Transform(DirectX::XMVectorSet(0, length, 0, 0), W);
			DirectX::XMVECTOR Z = DirectX::XMVector3Transform(DirectX::XMVectorSet(0, 0, length, 0), W);
			DirectX::XMStoreFloat3(&p, W.r[3]);
			DirectX::XMStoreFloat3(&x, X);
			DirectX::XMStoreFloat3(&y, Y);
			DirectX::XMStoreFloat3(&z, Z);
			primitiveRenderer->AddVertex(p, { 1, 0, 0, 1 });
			primitiveRenderer->AddVertex(x, { 1, 0, 0, 1 });
			primitiveRenderer->AddVertex(p, { 0, 1, 0, 1 });
			primitiveRenderer->AddVertex(y, { 0, 1, 0, 1 });
			primitiveRenderer->AddVertex(p, { 0, 0, 1, 1 });
			primitiveRenderer->AddVertex(z, { 0, 0, 1, 1 });
		}
		primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	}
}

// GUI�`�揈��
void ModelViewerScene::DrawGUI()
{
	DrawMenuGUI();
	DrawHierarchyGUI();
	DrawPropertyGUI();
	DrawAnimationGUI();
	DrawMaterialGUI();
}

// ���j���[GUI�`��
void ModelViewerScene::DrawMenuGUI()
{
	if (ImGui::BeginMainMenuBar())
	{
		// �t�@�C�����j���[
		if (ImGui::BeginMenu("File"))
		{
			bool check = false;
			if (ImGui::MenuItem("Open Model", "", &check))
			{
				static const char* filter = "Model Files(*.gltf;*.glb)\0*.gltf;*.glb;\0All Files(*.*)\0*.*;\0\0";

				char filename[256] = { 0 };
				HWND hWnd = Graphics::Instance().GetWindowHandle();
				DialogResult result = Dialog::OpenFileName(filename, sizeof(filename), filter, nullptr, hWnd);
				if (result == DialogResult::OK)
				{
					ID3D11Device* device = Graphics::Instance().GetDevice();
					model = std::make_shared<Model>(device, filename, animationSamplingRate);
					animationSpeed = 1.0f;
					currentAnimationSeconds = 0.0f;
					currentAnimationIndex = -1;
				}
			}


			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

// �q�G�����L�[GUI�`��
void ModelViewerScene::DrawHierarchyGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 30), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_Once);

	if (ImGui::Begin("Hierarchy", nullptr, ImGuiWindowFlags_None))
	{
		// �m�[�h�c���[���ċA�I�ɕ`�悷��֐�
		std::function<void(Model::Node*)> drawNodeTree = [&](Model::Node* node)
		{
			// �����N���b�N�A�܂��̓m�[�h���_�u���N���b�N�ŊK�w���J��
			ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow
				| ImGuiTreeNodeFlags_OpenOnDoubleClick;

			// �q�����Ȃ��ꍇ�͖������Ȃ�
			size_t childCount = node->children.size();
			if (childCount == 0)
			{
				nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
			}

			// �I���t���O
			if (selectionNode == node)
			{
				nodeFlags |= ImGuiTreeNodeFlags_Selected;
			}

			// �c���[�m�[�h��\��
			bool opened = ImGui::TreeNodeEx(node, nodeFlags, node->name.c_str());

			// �t�H�[�J�X���ꂽ�m�[�h��I������
			if (ImGui::IsItemFocused())
			{
				selectionNode = node;
			}

			// �J����Ă���ꍇ�A�q�K�w�������������s��
			if (opened && childCount > 0)
			{
				for (Model::Node* child : node->children)
				{
					drawNodeTree(child);
				}
				ImGui::TreePop();
			}
		};
		// �ċA�I�Ƀm�[�h��`��
		if (model != nullptr)
		{
			drawNodeTree(model->GetRootNode());
		}
	}
	ImGui::End();
}

// �v���p�e�BGUI�`��
void ModelViewerScene::DrawPropertyGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
	ImGui::SetNextWindowPos(ImVec2(pos.x + 970, pos.y + 30), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_Once);

	if (ImGui::Begin("Property", nullptr, ImGuiWindowFlags_None))
	{
		if (selectionNode != nullptr)
		{
			if (ImGui::CollapsingHeader("Node", ImGuiTreeNodeFlags_DefaultOpen))
			{
				// �ʒu
				if (ImGui::DragFloat3("Local Position", &selectionNode->position.x, 0.1f))
				{
					animationPlaying = false;
				}
				// �ʒu
				DirectX::XMFLOAT3 globalPosition =
				{
					selectionNode->globalTransform._41,
					selectionNode->globalTransform._42,
					selectionNode->globalTransform._43,
				};
				if (ImGui::DragFloat3("Global Position", &globalPosition.x, 0.1f))
				{
					if (selectionNode->parent != nullptr)
					{
						DirectX::XMMATRIX ParentGlobalTransform = DirectX::XMLoadFloat4x4(&selectionNode->parent->globalTransform);
						DirectX::XMMATRIX InverseParentGlobalTransform = DirectX::XMMatrixInverse(nullptr, ParentGlobalTransform);
						DirectX::XMVECTOR GlobalPosition = DirectX::XMLoadFloat3(&globalPosition);
						DirectX::XMVECTOR LocalPosition = DirectX::XMVector3Transform(GlobalPosition, InverseParentGlobalTransform);
						DirectX::XMStoreFloat3(&selectionNode->position, LocalPosition);
					}
					else
					{
						selectionNode->position = globalPosition;
					}
					animationPlaying = false;
				}

				// ��]
				DirectX::XMFLOAT3 angle;
				TransformUtils::QuaternionToRollPitchYaw(selectionNode->rotation, angle.x, angle.y, angle.z);
				angle.x = DirectX::XMConvertToDegrees(angle.x);
				angle.y = DirectX::XMConvertToDegrees(angle.y);
				angle.z = DirectX::XMConvertToDegrees(angle.z);
				if (ImGui::DragFloat3("Local Rotation", &angle.x, 1.0f))
				{
					angle.x = DirectX::XMConvertToRadians(angle.x);
					angle.y = DirectX::XMConvertToRadians(angle.y);
					angle.z = DirectX::XMConvertToRadians(angle.z);
					DirectX::XMVECTOR Rotation = DirectX::XMQuaternionRotationRollPitchYaw(angle.x, angle.y, angle.z);
					DirectX::XMStoreFloat4(&selectionNode->rotation, Rotation);
				}

				TransformUtils::MatrixToRollPitchYaw(selectionNode->globalTransform, angle.x, angle.y, angle.z);
				angle.x = DirectX::XMConvertToDegrees(angle.x);
				angle.y = DirectX::XMConvertToDegrees(angle.y);
				angle.z = DirectX::XMConvertToDegrees(angle.z);
				if (ImGui::DragFloat3("Global Rotation", &angle.x, 0.1f))
				{
					angle.x = DirectX::XMConvertToRadians(angle.x);
					angle.y = DirectX::XMConvertToRadians(angle.y);
					angle.z = DirectX::XMConvertToRadians(angle.z);
					DirectX::XMVECTOR GlobalRotation = DirectX::XMQuaternionRotationRollPitchYaw(angle.x, angle.y, angle.z);

					if (selectionNode->parent != nullptr)
					{
						DirectX::XMMATRIX ParentGlobalTransform = DirectX::XMLoadFloat4x4(&selectionNode->parent->globalTransform);
						ParentGlobalTransform.r[0] = DirectX::XMVector3Normalize(ParentGlobalTransform.r[0]);
						ParentGlobalTransform.r[1] = DirectX::XMVector3Normalize(ParentGlobalTransform.r[1]);
						ParentGlobalTransform.r[2] = DirectX::XMVector3Normalize(ParentGlobalTransform.r[2]);
						DirectX::XMVECTOR ParentGlobalRotation = DirectX::XMQuaternionRotationMatrix(ParentGlobalTransform);
						DirectX::XMVECTOR InverseParentGlobalRotation = DirectX::XMQuaternionInverse(ParentGlobalRotation);
						DirectX::XMVECTOR LocalRotation = DirectX::XMQuaternionMultiply(GlobalRotation, InverseParentGlobalRotation);
						DirectX::XMStoreFloat4(&selectionNode->rotation, LocalRotation);
					}
					else
					{
						DirectX::XMStoreFloat4(&selectionNode->rotation, GlobalRotation);
					}
					animationPlaying = false;
				}

				// �X�P�[��
				if (ImGui::DragFloat3("Local Scale", &selectionNode->scale.x, 0.01f))
				{
					animationPlaying = false;
				}

				DirectX::XMMATRIX GlobalTransform = DirectX::XMLoadFloat4x4(&selectionNode->globalTransform);
				DirectX::XMFLOAT3 globalScale =
				{
					DirectX::XMVectorGetX(DirectX::XMVector3Length(GlobalTransform.r[0])),
					DirectX::XMVectorGetX(DirectX::XMVector3Length(GlobalTransform.r[1])),
					DirectX::XMVectorGetX(DirectX::XMVector3Length(GlobalTransform.r[2]))
				};
				if (ImGui::DragFloat3("Global Scale", &globalScale.x, 0.1f))
				{
				}

			}
		}
	}
	ImGui::End();
}

// �A�j���[�V����GUI�`��
void ModelViewerScene::DrawAnimationGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 350), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_Once);

	if (ImGui::Begin("Animation", nullptr, ImGuiWindowFlags_None))
	{
		ImGui::Checkbox("Loop", &animationLoop); ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::InputFloat("SamplingRate", &animationSamplingRate);
		ImGui::DragFloat("BlendSeconds", &animationBlendSeconds, 0.01f);

		if (model != nullptr)
		{
			float secondsLength = currentAnimationIndex >= 0 ? model->GetAnimations().at(currentAnimationIndex).secondsLength : 0;
			int currentFrame = static_cast<int>(currentAnimationSeconds * 60.0f);
			int frameLength = static_cast<int>(secondsLength * 60);

			ImGui::SetNextItemWidth(50);
			ImGui::PushID(u8"�t���[��");
			if (ImGui::DragInt("##v", &currentFrame, 1, 0, frameLength))
			{
				animationPlaying = true;
				currentAnimationSeconds = currentFrame / 60.0f;
				animationSpeed = 0.0f;
			}
			ImGui::PopID();

			ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::PushID(u8"�^�C�����C��");
			if (ImGui::SliderFloat("##v", &currentAnimationSeconds, 0, secondsLength, "%.3f"))
			{
				animationPlaying = true;
				animationSpeed = 0.0f;
			}
			ImGui::PopID();

			int index = 0;
			for (const Model::Animation& animation : model->GetAnimations())
			{
				ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf;

				ImGui::TreeNodeEx(&animation, nodeFlags, animation.name.c_str());

				// �_�u���N���b�N�ŃA�j���[�V�����Đ�
				if (ImGui::IsItemClicked())
				{
					if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						animationPlaying = true;
						currentAnimationIndex = index;
						currentAnimationSeconds = 0.0f;
						animationSpeed = 1.0f;
					}
				}

				ImGui::TreePop();

				index++;
			}
		}
	}
	ImGui::End();
}

// �}�e���A��GUI�`��
void ModelViewerScene::DrawMaterialGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
	ImGui::SetNextWindowPos(ImVec2(pos.x + 970, pos.y + 350), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_Once);

	if (ImGui::Begin("Material", nullptr, ImGuiWindowFlags_None))
	{
		if (model != nullptr)
		{
			const char* shaderNames[] =
			{
				"Basic",
				"Lambert",
			};
			ImGui::Combo("Shader", &shaderId, shaderNames, _countof(shaderNames));

			int index = 0;
			for (const Model::Material& material : model->GetMaterials())
			{
				ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow
					| ImGuiTreeNodeFlags_OpenOnDoubleClick;

				if (ImGui::TreeNodeEx(&material, nodeFlags, material.name.c_str()))
				{
					ImGui::Text("BaseMap");
					ImGui::Image(material.baseMap.Get(), ImVec2(50, 50));
					DirectX::XMFLOAT4 baseColor = material.baseColor;
					ImGui::ColorEdit4("BaseColor", &baseColor.x, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoInputs);

					ImGui::TreePop();
				}

				index++;
			}
		}
	}
	ImGui::End();
}
