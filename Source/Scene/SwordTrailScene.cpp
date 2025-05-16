#include <imgui.h>
#include "Graphics.h"
#include "Scene/SwordTrailScene.h"

// コンストラクタ
SwordTrailScene::SwordTrailScene()
{
	ID3D11Device *device = Graphics::Instance().GetDevice();
	float screenWidth = Graphics::Instance().GetScreenWidth();
	float screenHeight = Graphics::Instance().GetScreenHeight();

	// カメラ設定
	camera.SetPerspectiveFov(
			DirectX::XMConvertToRadians(45), // 画角
			screenWidth / screenHeight,			 // 画面アスペクト比
			0.1f,														 // ニアクリップ
			1000.0f													 // ファークリップ
	);
	camera.SetLookAt(
			{8, 3, 0}, // 視点
			{0, 2, 0}, // 注視点
			{0, 1, 0}	 // 上ベクトル
	);

	// モデル読み込み
	character = std::make_shared<Model>(device, "Data/Model/RPG-Character/RPG-Character.glb");
	character->GetNodePoses(nodePoses);
	weapon = std::make_shared<Model>(device, "Data/Model/RPG-Character/2Hand-Sword.glb");
}

// 更新処理
void SwordTrailScene::Update(float elapsedTime)
{
	const int animationIndex = character->GetAnimationIndex("Slash");

	// 指定時間のアニメーションの姿勢を取得
	character->ComputeAnimation(animationIndex, animationSeconds, nodePoses);

	// アニメーション時間更新
	const Model::Animation &animation = character->GetAnimations().at(animationIndex);
	animationSeconds += elapsedTime;
	if (animationSeconds > animation.secondsLength)
	{
		animationSeconds -= animation.secondsLength;
	}

	// 姿勢更新
	character->SetNodePoses(nodePoses);

	// キャラクタートランスフォーム更新
	DirectX::XMFLOAT4X4 worldTransform;
	DirectX::XMStoreFloat4x4(&worldTransform, DirectX::XMMatrixIdentity());
	character->UpdateTransform(worldTransform);

	// 武器トランスフォーム更新
	int handNodeIndex = character->GetNodeIndex("B_R_Hand");
	const Model::Node &handNode = character->GetNodes().at(handNodeIndex);
	weapon->UpdateTransform(handNode.worldTransform);

	// TODO②:保存していた頂点バッファを１フレーム分ずらせ
	for (int i = MAX_POLYGON - 1; i > 0; --i)
	{
		trailPositions[0][i] = trailPositions[0][i - 1];
		trailPositions[1][i] = trailPositions[1][i - 1];
	}

	// 剣の根本と先端のワールド座標を取得
	DirectX::XMMATRIX worldMatrix = DirectX::XMLoadFloat4x4(&handNode.worldTransform);
	DirectX::XMVECTOR rootPos = DirectX::XMVector3Transform(DirectX::XMVectorSet(0, 0, 0.3f, 0), worldMatrix);
	DirectX::XMVECTOR tipPos = DirectX::XMVector3Transform(DirectX::XMVectorSet(0, 0, 2.3f, 0), worldMatrix);
	DirectX::XMStoreFloat3(&trailPositions[0][0], rootPos);
	DirectX::XMStoreFloat3(&trailPositions[1][0], tipPos);

	DirectX::XMFLOAT4 color = {1, 0, 0, 1};

	// ポリゴン作成
	PrimitiveRenderer *primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();
	if (!spline)
	{
		// 保存していた頂点バッファでポリゴンを作る
		for (int i = 0; i < MAX_POLYGON - 1; ++i)
		{
			primitiveRenderer->AddVertex(trailPositions[0][i], color);
			primitiveRenderer->AddVertex(trailPositions[1][i], color);
		}
	}
	else
	{
		// スプライン補完処理を行い、滑らかなポリゴンを描画
		for (int i = 0; i < MAX_POLYGON - 3; i++)
		{
			for (float t = 0; t <= 1.0f; t += 0.1f)
			{
				DirectX::XMVECTOR p0 = DirectX::XMLoadFloat3(&trailPositions[1][i]);
				DirectX::XMVECTOR p1 = DirectX::XMLoadFloat3(&trailPositions[1][i + 1]);
				DirectX::XMVECTOR p2 = DirectX::XMLoadFloat3(&trailPositions[1][i + 2]);
				DirectX::XMVECTOR p3 = DirectX::XMLoadFloat3(&trailPositions[1][i + 3]);

				DirectX::XMVECTOR splinePos = DirectX::XMVectorCatmullRom(p0, p1, p2, p3, t);
				DirectX::XMFLOAT3 finalPos;
				DirectX::XMStoreFloat3(&finalPos, splinePos);
				primitiveRenderer->AddVertex(finalPos, color);
				primitiveRenderer->AddVertex(trailPositions[0][i], color);
			}
		}
	}
}

// 描画処理
void SwordTrailScene::Render(float elapsedTime)
{
	ID3D11DeviceContext *dc = Graphics::Instance().GetDeviceContext();
	RenderState *renderState = Graphics::Instance().GetRenderState();
	PrimitiveRenderer *primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();
	ModelRenderer *modelRenderer = Graphics::Instance().GetModelRenderer();

	// レンダーステート設定
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), nullptr, 0xFFFFFFFF);
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullNone));

	// ポリゴン描画
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// グリッド描画
	primitiveRenderer->DrawGrid(20, 1);
	primitiveRenderer->Render(dc, camera.GetView(), camera.GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// 描画コンテキスト設定
	RenderContext rc;
	rc.deviceContext = dc;
	rc.renderState = renderState;
	rc.camera = &camera;
	rc.lightManager = &lightManager;

	// モデル描画
	modelRenderer->Draw(ShaderId::Basic, character);
	modelRenderer->Draw(ShaderId::Basic, weapon);
	modelRenderer->Render(rc);
}

// GUI描画処理
void SwordTrailScene::DrawGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Once);

	if (ImGui::Begin(u8"剣の軌跡処理"))
	{
		ImGui::Checkbox(u8"スプライン補完", &spline);
	}
	ImGui::End();
}
