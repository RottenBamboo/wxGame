#pragma once

class GraphicD3D12
{
public:
	GraphicD3D12();
	~GraphicD3D12();
	virtual void Initialize(void) = 0;
	virtual void LoadPipeline(void) = 0;
	virtual void LoadGraphicAssets(void) = 0;
	virtual void OnRender(void) = 0;
	virtual void OnUpdate(void) = 0;

protected:
	uint32_t m_width;
	uint32_t m_height;
	float m_aspect_ratio;
};

