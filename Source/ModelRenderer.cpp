#include <algorithm>
#include "Misc.h"
#include "GpuResourceUtils.h"
#include "ModelRenderer.h"
#include "BasicShader.h"
#include "LambertShader.h"

// �R���X�g���N�^
ModelRenderer::ModelRenderer(ID3D11Device* device)
{
	// �V�[���p�萔�o�b�t�@
	GpuResourceUtils::CreateConstantBuffer(
		device,
		sizeof(CbScene),
		sceneConstantBuffer.GetAddressOf());

	// �X�P���g���p�萔�o�b�t�@
	GpuResourceUtils::CreateConstantBuffer(
		device,
		sizeof(CbSkeleton),
		skeletonConstantBuffer.GetAddressOf());

	// �V�F�[�_�[����
	shaders[static_cast<int>(ShaderId::Basic)] = std::make_unique<BasicShader>(device);
	shaders[static_cast<int>(ShaderId::Lambert)] = std::make_unique<LambertShader>(device);
}

// ���`��
void ModelRenderer::Draw(ShaderId shaderId, std::shared_ptr<Model> model)
{
	DrawInfo& drawInfo = drawInfos.emplace_back();
	drawInfo.shaderId = shaderId;
	drawInfo.model = model;
}

// �`����s
void ModelRenderer::Render(const RenderContext& rc)
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	// �V�[���p�萔�o�b�t�@�X�V
	{
		static LightManager defaultLightManager;
		const LightManager* lightManager = rc.lightManager ? rc.lightManager : &defaultLightManager;

		CbScene cbScene{};
		DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&rc.camera->GetView());
		DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&rc.camera->GetProjection());
		DirectX::XMStoreFloat4x4(&cbScene.viewProjection, V * P);
		const DirectionalLight& directionalLight = lightManager->GetDirectionalLight();
		cbScene.lightDirection.x = directionalLight.direction.x;
		cbScene.lightDirection.y = directionalLight.direction.y;
		cbScene.lightDirection.z = directionalLight.direction.z;
		cbScene.lightColor.x = directionalLight.color.x;
		cbScene.lightColor.y = directionalLight.color.y;
		cbScene.lightColor.z = directionalLight.color.z;
		const DirectX::XMFLOAT3& eye = rc.camera->GetEye();
		cbScene.cameraPosition.x = eye.x;
		cbScene.cameraPosition.y = eye.y;
		cbScene.cameraPosition.z = eye.z;
		dc->UpdateSubresource(sceneConstantBuffer.Get(), 0, 0, &cbScene, 0, 0);
	}

	// �萔�o�b�t�@�ݒ�
	ID3D11Buffer* vsConstantBuffers[] =
	{
		skeletonConstantBuffer.Get(),
		sceneConstantBuffer.Get(),
	};
	ID3D11Buffer* psConstantBuffers[] =
	{
		sceneConstantBuffer.Get(),
	};
	dc->VSSetConstantBuffers(6, _countof(vsConstantBuffers), vsConstantBuffers);
	dc->PSSetConstantBuffers(7, _countof(psConstantBuffers), psConstantBuffers);

	// �T���v���X�e�[�g�ݒ�
	ID3D11SamplerState* samplerStates[] =
	{
		rc.renderState->GetSamplerState(SamplerState::LinearWrap)
	};
	dc->PSSetSamplers(0, _countof(samplerStates), samplerStates);

	// �����_�[�X�e�[�g�ݒ�
	dc->OMSetDepthStencilState(rc.renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
	dc->RSSetState(rc.renderState->GetRasterizerState(RasterizerState::SolidCullBack));

	// ���b�V���`��֐�
	auto drawMesh = [&](const Model::Mesh& mesh, Shader* shader)
	{
		// ���_�o�b�t�@�ݒ�
		UINT stride = sizeof(Model::Vertex);
		UINT offset = 0;
		dc->IASetVertexBuffers(0, 1, mesh.vertexBuffer.GetAddressOf(), &stride, &offset);
		dc->IASetIndexBuffer(mesh.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// �X�P���g���p�萔�o�b�t�@�X�V
		CbSkeleton cbSkeleton{};
		if (mesh.bones.size() > 0)
		{
			for (size_t i = 0; i < mesh.bones.size(); ++i)
			{
				const Model::Bone& bone = mesh.bones.at(i);
				DirectX::XMMATRIX WorldTransform = DirectX::XMLoadFloat4x4(&bone.node->worldTransform);
				DirectX::XMMATRIX OffsetTransform = DirectX::XMLoadFloat4x4(&bone.offsetTransform);
				DirectX::XMMATRIX BoneTransform = OffsetTransform * WorldTransform;
				DirectX::XMStoreFloat4x4(&cbSkeleton.boneTransforms[i], BoneTransform);
			}
		}
		else
		{
			cbSkeleton.boneTransforms[0] = mesh.node->worldTransform;
		}
		dc->UpdateSubresource(skeletonConstantBuffer.Get(), 0, 0, &cbSkeleton, 0, 0);

		// �X�V
		shader->Update(rc, mesh);

		// �`��
		dc->DrawIndexed(static_cast<UINT>(mesh.indices.size()), 0, 0);
	};

	DirectX::XMVECTOR CameraPosition = DirectX::XMLoadFloat3(&rc.camera->GetEye());
	DirectX::XMVECTOR CameraFront = DirectX::XMLoadFloat3(&rc.camera->GetFront());

	// �u�����h�X�e�[�g�ݒ�
	dc->OMSetBlendState(rc.renderState->GetBlendState(BlendState::Opaque), nullptr, 0xFFFFFFFF);

	// �s�����`�揈��
	for (DrawInfo& drawInfo : drawInfos)
	{
		Shader* shader = shaders[static_cast<int>(drawInfo.shaderId)].get();
		shader->Begin(rc);

		for (const Model::Mesh& mesh : drawInfo.model->GetMeshes())
		{
			// ���������b�V���o�^
			if (mesh.material->alphaMode == Model::AlphaMode::Blend ||
				(mesh.material->baseColor.w > 0.01f && mesh.material->baseColor.w < 0.99f))
			{
				TransparencyDrawInfo& transparencyDrawInfo = transparencyDrawInfos.emplace_back();
				transparencyDrawInfo.mesh = &mesh;
				// �J�����Ƃ̋������Z�o
				DirectX::XMVECTOR Position = DirectX::XMVectorSet(
					mesh.node->worldTransform._41,
					mesh.node->worldTransform._42,
					mesh.node->worldTransform._43,
					0.0f);
				DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(Position, CameraPosition);
				transparencyDrawInfo.distance = DirectX::XMVectorGetX(DirectX::XMVector3Dot(CameraFront, Vec));

				continue;
			}

			// �`��
			drawMesh(mesh, shader);
		}

		shader->End(rc);
	}
	drawInfos.clear();

	// �u�����h�X�e�[�g�ݒ�
	dc->OMSetBlendState(rc.renderState->GetBlendState(BlendState::Transparency), nullptr, 0xFFFFFFFF);

	// �J�������牓�����Ƀ\�[�g
	std::sort(transparencyDrawInfos.begin(), transparencyDrawInfos.end(),
		[](const TransparencyDrawInfo& lhs, const TransparencyDrawInfo& rhs)
		{
			return lhs.distance > rhs.distance;
		});

	// �������`�揈��
	for (const TransparencyDrawInfo& transparencyDrawInfo : transparencyDrawInfos)
	{
		Shader* shader = shaders[static_cast<int>(transparencyDrawInfo.shaderId)].get();

		shader->Begin(rc);

		drawMesh(*transparencyDrawInfo.mesh, shader);

		shader->End(rc);
	}
	transparencyDrawInfos.clear();

	// �萔�o�b�t�@�ݒ����
	for (ID3D11Buffer*& vsConstantBuffer : vsConstantBuffers) { vsConstantBuffer = nullptr; }
	for (ID3D11Buffer*& psConstantBuffer : psConstantBuffers) { psConstantBuffer = nullptr; }
	dc->VSSetConstantBuffers(6, _countof(vsConstantBuffers), vsConstantBuffers);
	dc->PSSetConstantBuffers(7, _countof(psConstantBuffers), psConstantBuffers);

	// �T���v���X�e�[�g�ݒ����
	for (ID3D11SamplerState*& samplerState : samplerStates) { samplerState = nullptr; }
	dc->PSSetSamplers(0, _countof(samplerStates), samplerStates);
}
