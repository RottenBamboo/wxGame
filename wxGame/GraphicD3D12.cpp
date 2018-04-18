#include"stdafx.h"
#include "GraphicD3D12.h"

GraphicD3D12::GraphicD3D12():
m_width(GetSystemMetrics(SM_CXSCREEN)),
m_height(GetSystemMetrics(SM_CYSCREEN)),
m_aspect_ratio((float)m_width / (float)m_height)
{
}


GraphicD3D12::~GraphicD3D12()
{
}
