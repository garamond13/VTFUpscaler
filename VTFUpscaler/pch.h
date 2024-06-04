#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <filesystem>
#include <fstream>
#include <array>
#include <iostream>
#include <memory>

#include "..\\lib\\VTFLib.h"
#pragma comment(lib, "..\\lib\\x64\\VTFLib.lib")

// DirectX
//

// window.h is being included!
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <d3d11_4.h>
#pragma comment(lib, "d3d11.lib")

#include <dxgi1_6.h>
#include <wrl/client.h>

//

#include "vtfu_assert.h"
#include "strtoval.h"