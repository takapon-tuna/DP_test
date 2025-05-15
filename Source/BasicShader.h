#pragma once

#include "Shader.h"

class BasicShader : public Shader
{
public:
	BasicShader(ID3D11Device* device);
	~BasicShader() override = default;

	// �J�n����
	void Begin(const RenderContext& rc) override;

	// �X�V����
	void Update(const RenderContext& rc, const Model::Mesh& mesh) override;

	// �I������
	void End(const RenderContext& rc) override;

private:
	struct CbMesh
	{
		DirectX::XMFLOAT4		materialColor;
	};

	Microsoft::WRL::ComPtr<ID3D11VertexShader>		vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>		pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>		inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer>			meshConstantBuffer;
};
