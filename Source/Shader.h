#pragma once

#include "RenderContext.h"
#include "Model.h"

class Shader
{
public:
	Shader() {}
	virtual ~Shader() {}

	// �J�n����
	virtual void Begin(const RenderContext& rc) = 0;

	// �X�V����
	virtual void Update(const RenderContext& rc, const Model::Mesh& mesh) = 0;

	// �I������
	virtual void End(const RenderContext& rc) = 0;
};
