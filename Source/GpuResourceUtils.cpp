#include <filesystem>
#include <wrl.h>
#include <DirectXTex.h>
#include "Misc.h"
#include "GpuResourceUtils.h"

// 頂点シェーダー読み込み
HRESULT GpuResourceUtils::LoadVertexShader(
	ID3D11Device* device,
	const char* filename,
	const D3D11_INPUT_ELEMENT_DESC inputElementDescs[],
	UINT inputElementCount,
	ID3D11InputLayout** inputLayout,
	ID3D11VertexShader** vertexShader)
{
	// ファイルを開く
	FILE* fp = nullptr;
	fopen_s(&fp, filename, "rb");
	_ASSERT_EXPR_A(fp, "Vertex Shader File not found");

	// ファイルのサイズを求める
	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// メモリ上に頂点シェーダーデータを格納する領域を用意する
	std::unique_ptr<u_char[]> data = std::make_unique<u_char[]>(size);
	fread(data.get(), size, 1, fp);
	fclose(fp);

	// 頂点シェーダー生成
	HRESULT hr = device->CreateVertexShader(data.get(), size, nullptr, vertexShader);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	// 入力レイアウト
	if (inputLayout != nullptr)
	{
		hr = device->CreateInputLayout(inputElementDescs, inputElementCount, data.get(), size, inputLayout);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	return hr;
}

// ピクセルシェーダー読み込み
HRESULT GpuResourceUtils::LoadPixelShader(
	ID3D11Device* device,
	const char* filename,
	ID3D11PixelShader** pixelShader)
{
	// ファイルを開く
	FILE* fp = nullptr;
	fopen_s(&fp, filename, "rb");
	_ASSERT_EXPR_A(fp, "Pixel Shader File not found");

	// ファイルのサイズを求める
	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// メモリ上に頂点シェーダーデータを格納する領域を用意する
	std::unique_ptr<u_char[]> data = std::make_unique<u_char[]>(size);
	fread(data.get(), size, 1, fp);
	fclose(fp);

	// ピクセルシェーダー生成
	HRESULT hr = device->CreatePixelShader(data.get(), size, nullptr, pixelShader);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	return hr;
}

// テクスチャ読み込み
HRESULT GpuResourceUtils::LoadTexture(
	ID3D11Device* device,
	const char* filename,
	ID3D11ShaderResourceView** shaderResourceView,
	D3D11_TEXTURE2D_DESC* texture2dDesc)
{
	// 拡張子を取得
	std::filesystem::path filepath(filename);
	std::string extension = filepath.extension().string();
	std::transform(extension.begin(), extension.end(), extension.begin(), tolower);	// 小文字化

	// ワイド文字に変換
	std::wstring wfilename = filepath.wstring();

	// フォーマット毎に画像読み込み処理
	HRESULT hr;
	DirectX::TexMetadata metadata;
	DirectX::ScratchImage scratch_image;
	if (extension == ".tga")
	{
		hr = DirectX::GetMetadataFromTGAFile(wfilename.c_str(), metadata);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		hr = DirectX::LoadFromTGAFile(wfilename.c_str(), &metadata, scratch_image);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}
	else if (extension == ".dds")
	{
		hr = DirectX::GetMetadataFromDDSFile(wfilename.c_str(), DirectX::DDS_FLAGS_NONE, metadata);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		hr = DirectX::LoadFromDDSFile(wfilename.c_str(), DirectX::DDS_FLAGS_NONE, &metadata, scratch_image);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}
	else if (extension == ".hdr")
	{
		hr = DirectX::GetMetadataFromHDRFile(wfilename.c_str(), metadata);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		hr = DirectX::LoadFromHDRFile(wfilename.c_str(), &metadata, scratch_image);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}
	else
	{
		hr = DirectX::GetMetadataFromWICFile(wfilename.c_str(), DirectX::WIC_FLAGS_NONE, metadata);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		hr = DirectX::LoadFromWICFile(wfilename.c_str(), DirectX::WIC_FLAGS_NONE, &metadata, scratch_image);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	// シェーダーリソースビュー作成
	hr = DirectX::CreateShaderResourceView(device, scratch_image.GetImages(), scratch_image.GetImageCount(),
		metadata, shaderResourceView);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	// テクスチャ情報取得
	if (texture2dDesc != nullptr)
	{
		Microsoft::WRL::ComPtr<ID3D11Resource> resource;
		(*shaderResourceView)->GetResource(resource.GetAddressOf());

		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
		hr = resource->QueryInterface<ID3D11Texture2D>(texture2d.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		texture2d->GetDesc(texture2dDesc);
	}
	return hr;
}

// テクスチャ読み込み
HRESULT GpuResourceUtils::LoadTexture(
	ID3D11Device* device,
	const void* data,
	size_t size,
	ID3D11ShaderResourceView** shaderResourceView,
	D3D11_TEXTURE2D_DESC* texture2dDesc)
{
	// フォーマット毎に画像読み込み処理
	HRESULT hr = E_FAIL;
	DirectX::TexMetadata metadata;
	DirectX::ScratchImage scratch_image;

	// .tga
	{
		hr = DirectX::GetMetadataFromTGAMemory(data, size, metadata);
		if (SUCCEEDED(hr))
		{
			hr = DirectX::LoadFromTGAMemory(data, size, &metadata, scratch_image);
		}
	}
	// .dds
	if (FAILED(hr))
	{
		hr = DirectX::GetMetadataFromDDSMemory(data, size, DirectX::DDS_FLAGS_NONE, metadata);
		if (SUCCEEDED(hr))
		{
			hr = DirectX::LoadFromDDSMemory(data, size, DirectX::DDS_FLAGS_NONE, &metadata, scratch_image);
		}
	}
	// .hdr
	if (FAILED(hr))
	{
		hr = DirectX::GetMetadataFromHDRMemory(data, size, metadata);
		if (SUCCEEDED(hr))
		{
			hr = DirectX::LoadFromHDRMemory(data, size, &metadata, scratch_image);
		}
	}
	if (FAILED(hr))
	{
		hr = DirectX::GetMetadataFromWICMemory(data, size, DirectX::WIC_FLAGS_NONE, metadata);
		if (SUCCEEDED(hr))
		{
			hr = DirectX::LoadFromWICMemory(data, size, DirectX::WIC_FLAGS_NONE, &metadata, scratch_image);
		}
	}
	if (FAILED(hr))
	{
		return hr;
	}

	// シェーダーリソースビュー作成
	hr = DirectX::CreateShaderResourceView(device, scratch_image.GetImages(), scratch_image.GetImageCount(),
		metadata, shaderResourceView);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	// テクスチャ情報取得
	if (texture2dDesc != nullptr)
	{
		Microsoft::WRL::ComPtr<ID3D11Resource> resource;
		(*shaderResourceView)->GetResource(resource.GetAddressOf());

		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
		hr = resource->QueryInterface<ID3D11Texture2D>(texture2d.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		texture2d->GetDesc(texture2dDesc);
	}
	return hr;
}

// ダミーテクスチャ作成
HRESULT GpuResourceUtils::CreateDummyTexture(
	ID3D11Device* device,
	UINT color,
	ID3D11ShaderResourceView** shaderResourceView,
	D3D11_TEXTURE2D_DESC* texture2dDesc)
{
	D3D11_TEXTURE2D_DESC desc = { 0 };
	desc.Width = 1;
	desc.Height = 1;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA data{};
	data.pSysMem = &color;
	data.SysMemPitch = desc.Width;

	Microsoft::WRL::ComPtr<ID3D11Texture2D>	texture;
	HRESULT hr = device->CreateTexture2D(&desc, &data, texture.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	hr = device->CreateShaderResourceView(texture.Get(), nullptr, shaderResourceView);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	// テクスチャ情報取得
	if (texture2dDesc != nullptr)
	{
		Microsoft::WRL::ComPtr<ID3D11Resource> resource;
		(*shaderResourceView)->GetResource(resource.GetAddressOf());

		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
		hr = resource->QueryInterface<ID3D11Texture2D>(texture2d.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		texture2d->GetDesc(texture2dDesc);
	}

	return hr;
}

// 定数バッファ作成
HRESULT GpuResourceUtils::CreateConstantBuffer(
	ID3D11Device* device,
	UINT bufferSize,
	ID3D11Buffer** constantBuffer)
{
	D3D11_BUFFER_DESC desc{};
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.ByteWidth = bufferSize;
	desc.StructureByteStride = 0;

	HRESULT hr = device->CreateBuffer(&desc, 0, constantBuffer);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	return hr;
}
