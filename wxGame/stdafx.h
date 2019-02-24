#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <Xinput.h>
#include "d3dx12.h"

#include <iostream>
#include <string>
#include <vector>
#include <wrl.h>
#include <shellapi.h>

#ifndef ALIGN
#define ALIGN(x, a)         (((x) + ((a) - 1)) & ~((a) - 1))
#endif
#define ALIGN_256(x)	((x % 256)?(((x / 256) + 1) * 256):(x))