#include <imgui.h>
#include "Graphics.h"
#include "Scene/RootMotionScene.h"

// コンストラクタ
RootMotionScene::RootMotionScene()
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
			{0, 10, -10}, // 視点
			{0, 0, 0},		// 注視点
			{0, 1, 0}			// 上ベクトル
	);

	// モデル読み込み
	character = std::make_shared<Model>(device, "Data/Model/RPG-Character/RPG-Character.glb");
	character->GetNodePoses(nodePoses);
}

// 更新処理
void RootMotionScene::Update(float elapsedTime)
{
	// アニメーション切り替え操作
	if (animationLoop)
	{
		// 歩行
		int newAnimationIndex = animationIndex;
		if (GetAsyncKeyState(VK_UP) & 0x8000)
		{
			newAnimationIndex = character->GetAnimationIndex("Walk_F");
		}
		else if (GetAsyncKeyState(VK_DOWN) & 0x8000)
		{
			newAnimationIndex = character->GetAnimationIndex("Walk_B");
		}
		else if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
		{
			newAnimationIndex = character->GetAnimationIndex("Walk_R");
		}
		else if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		{
			newAnimationIndex = character->GetAnimationIndex("Walk_L");
		}
		else
		{
			newAnimationIndex = character->GetAnimationIndex("Idle");
		}
		if (animationIndex != newAnimationIndex)
		{
			animationIndex = newAnimationIndex;
			animationSeconds = oldAnimationSeconds = 0;
		}
	}
	else
	{
		// ローリング
		if (GetAsyncKeyState(VK_UP) & 0x01)
		{
			animationIndex = character->GetAnimationIndex("Evade_F");
			animationSeconds = oldAnimationSeconds = 0;
		}
		if (GetAsyncKeyState(VK_DOWN) & 0x01)
		{
			animationIndex = character->GetAnimationIndex("Evade_B");
			animationSeconds = oldAnimationSeconds = 0;
		}
		if (GetAsyncKeyState(VK_RIGHT) & 0x01)
		{
			animationIndex = character->GetAnimationIndex("Evade_R");
			animationSeconds = oldAnimationSeconds = 0;
		}
		if (GetAsyncKeyState(VK_LEFT) & 0x01)
		{
			animationIndex = character->GetAnimationIndex("Evade_L");
			animationSeconds = oldAnimationSeconds = 0;
		}
	}

	// アニメーション更新処理
	if (animationIndex >= 0)
	{
		const Model::Animation &animation = character->GetAnimations().at(animationIndex);

		// 指定時間のアニメーションの姿勢を取得
		character->ComputeAnimation(animationIndex, animationSeconds, nodePoses);

		// TODO�@:ルートモーション処理を行い、キャラクターを移動せよ
		{
			// ルートモーションノード番号取得
			const int rootMotionNodeIndex = character->GetNodeIndex("B_Pelvis");

			// まずは初回、前回、今回のモーションの位置を取得
			Model::NodePose beginPose, oldPose, newPose;

			character->ComputeAnimation(animationIndex, rootMotionNodeIndex, 0, beginPose);
			character->ComputeAnimation(animationIndex, rootMotionNodeIndex, oldAnimationSeconds, oldPose);
			character->ComputeAnimation(animationIndex, rootMotionNodeIndex, animationSeconds, newPose);

			DirectX::XMFLOAT3 localTranslation; // ローカルの移動量を格納

			if (oldAnimationSeconds > animationSeconds)
			{
				// TODO�A:ループアニメーションに対応せよ
#if 1
				// 終端の姿勢を取り出す
				Model::NodePose endPose;
				character->ComputeAnimation(animationIndex, rootMotionNodeIndex, animation.secondsLength, endPose);
				// 前回から終端の移動差分値と初回から今回の移動差分値を算出
				DirectX::XMFLOAT3 endDiff, beginDiff;
				endDiff.x = endPose.position.x - oldPose.position.x;
				endDiff.y = endPose.position.y - oldPose.position.y;
				endDiff.z = endPose.position.z - oldPose.position.z;

				beginDiff.x = newPose.position.x - beginPose.position.x;
				beginDiff.y = newPose.position.y - beginPose.position.y;
				beginDiff.z = newPose.position.z - beginPose.position.z;
				// 算出した移動差分値の合計値をローカル移動量とする
				localTranslation.x = beginDiff.x + endDiff.x;
				localTranslation.y = beginDiff.y + endDiff.y;
				localTranslation.z = beginDiff.z + endDiff.z;
				// アニメーションの終端時間
#endif // 回答
#if 0
				Model::NodePose endPose;
				character->ComputeAnimation(animationIndex, rootMotionNodeIndex, animation.secondsLength, endPose);

				// 前回から終端までの移動差分
				DirectX::XMFLOAT3 endDiff;
				endDiff.x = endPose.position.x - oldPose.position.x;
				endDiff.y = endPose.position.y - oldPose.position.y;
				endDiff.z = endPose.position.z - oldPose.position.z;

				// 初回から今回までの移動差分
				DirectX::XMFLOAT3 startDiff;
				startDiff.x = newPose.position.x - beginPose.position.x;
				startDiff.y = newPose.position.y - beginPose.position.y;
				startDiff.z = newPose.position.z - beginPose.position.z;

				// 移動差分の合計値をローカル移動量とする
				localTranslation.x = endDiff.x + startDiff.x;
				localTranslation.y = endDiff.y + startDiff.y;
				localTranslation.z = endDiff.z + startDiff.z;
#endif // 自分の
			}
			else
			{
				// ローカルの移動量を求める
				// 今の位置 - 前の位置を求めて、localTranslationに格納

				localTranslation.x = newPose.position.x - oldPose.position.x;
				localTranslation.y = newPose.position.y - oldPose.position.y;
				localTranslation.z = newPose.position.z - oldPose.position.z;
			}
			// この移動量はB_Pelvisノードを基準とした空間（ローカル空間）での移動量なので
			// モデルが所属している空間（グローバル空間）に移動させる
			// 親ノードを取得
			Model::Node &rootMotionNode = character->GetNodes().at(rootMotionNodeIndex);

			// rootMotionNodeの親ノード(parentNode)からグローバル行列を取得できるのでローカルの移動量をグローバル空間に移動させれます
			DirectX::XMMATRIX ParentGlobalTransform;
			ParentGlobalTransform = DirectX::XMLoadFloat4x4(&rootMotionNode.parent->globalTransform);
			DirectX::XMVECTOR LocalTranslation = DirectX::XMLoadFloat3(&localTranslation);
			DirectX::XMVECTOR GlobalTranslation = DirectX::XMVector3TransformNormal(LocalTranslation, ParentGlobalTransform);

			if (bakeTranslationY)
			{
				// TODO�B:ルートモーションのY移動は適用しないようにせよ
				// グローバルの移動量からY成分の移動値を０にする
				GlobalTranslation = DirectX::XMVectorSetY(GlobalTranslation, 0);

				// 今回の姿勢のグローバル位置を算出
				DirectX::XMVECTOR LocalPosition, GlobalPosition;
				LocalPosition = DirectX::XMLoadFloat3(&newPose.position);
				GlobalPosition = DirectX::XMVector3Transform(LocalPosition, ParentGlobalTransform);

				// グローバル位置のXZ成分を０にする
				GlobalPosition = DirectX::XMVectorSetX(GlobalPosition, 0);
				GlobalPosition = DirectX::XMVectorSetZ(GlobalPosition, 0);

				// ルートモーションノードの位置を変更したいので、グローバル位置をルートモーションノードのローカル空間に変換してルートモーションノードの位置に設定
				DirectX::XMMATRIX InverseParentGlobalTransform;
				InverseParentGlobalTransform = DirectX::XMMatrixInverse(nullptr, ParentGlobalTransform);

				LocalPosition = DirectX::XMVector3Transform(GlobalPosition, InverseParentGlobalTransform);
				DirectX::XMStoreFloat3(&nodePoses[rootMotionNodeIndex].position, LocalPosition);
			}
			else
			{
				// 現在のルートノードモーションノードの座標を,
				// モーションの開始フレームの位置情報を適応して,
				// モーション側の移動はさせないようにする
				nodePoses[rootMotionNodeIndex].position = beginPose.position;
			}
			// グローバル空間の移動量をワールド空間に移動
			DirectX::XMMATRIX WorldTransform = DirectX::XMLoadFloat4x4(&worldTransform);
			DirectX::XMVECTOR WorldTranslation;
			WorldTranslation = DirectX::XMVector3TransformNormal(GlobalTranslation, WorldTransform);
			DirectX::XMFLOAT3 worldTranslation;
			DirectX::XMStoreFloat3(&worldTranslation, WorldTranslation);

			// 位置を更新
			position.x += worldTranslation.x;
			position.y += worldTranslation.y;
			position.z += worldTranslation.z;
		}

		// アニメーション時間更新
		oldAnimationSeconds = animationSeconds;
		animationSeconds += elapsedTime;
		if (animationSeconds > animation.secondsLength)
		{
			if (animationLoop)
			{
				animationSeconds -= animation.secondsLength;
			}
			else
			{
				animationSeconds = animation.secondsLength;
			}
		}

		// 姿勢更新
		character->SetNodePoses(nodePoses);
	}

	// ワールドトランスフォーム更新
	DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z);
	DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
	DirectX::XMMATRIX WorldTransform = S * R * T;
	DirectX::XMStoreFloat4x4(&worldTransform, WorldTransform);

	// モデルトランスフォーム更新処理
	character->UpdateTransform(worldTransform);
}

// 描画処理
void RootMotionScene::Render(float elapsedTime)
{
	ID3D11DeviceContext *dc = Graphics::Instance().GetDeviceContext();
	RenderState *renderState = Graphics::Instance().GetRenderState();
	PrimitiveRenderer *primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();
	ModelRenderer *modelRenderer = Graphics::Instance().GetModelRenderer();

	// レンダーステート設定
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), nullptr, 0xFFFFFFFF);
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullNone));

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
	modelRenderer->Render(rc);
}

// GUI描画処理
void RootMotionScene::DrawGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();

	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_Once);

	if (ImGui::Begin(u8"ルートモーション処理"))
	{
		ImGui::Text(u8"操作：方向キー");

		ImGui::Checkbox(u8"ループ", &animationLoop);
		ImGui::Checkbox(u8"Y軸移動無視", &bakeTranslationY);

		ImGui::DragFloat3("Position", &position.x, 0.1f);

		DirectX::XMFLOAT3 degree =
				{
						DirectX::XMConvertToDegrees(angle.x),
						DirectX::XMConvertToDegrees(angle.y),
						DirectX::XMConvertToDegrees(angle.z),
				};
		if (ImGui::DragFloat3("Angle", &degree.x, 1.0f))
		{
			angle.x = DirectX::XMConvertToRadians(degree.x);
			angle.y = DirectX::XMConvertToRadians(degree.y);
			angle.z = DirectX::XMConvertToRadians(degree.z);
		}
	}
	ImGui::End();
}
