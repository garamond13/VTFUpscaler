#pragma once

#include "pch.h"

inline constexpr int g_max_user_shaders = 1000;

enum HOOK_ : int
{
	HOOK_NONE,
	HOOK_MAIN,
	HOOK_DENOISE = HOOK_MAIN + g_max_user_shaders,
	HOOK_SCALE = HOOK_DENOISE + g_max_user_shaders,
	HOOK_SHARPEN = HOOK_SCALE + g_max_user_shaders,
	HOOK_GRAIN = HOOK_SHARPEN + g_max_user_shaders
};

struct User_shader_pass
{
	Microsoft::WRL::ComPtr<ID3DBlob> m_data;
	const int* m_width;
	const int* m_heigth;
	int m_save;
	std::vector<int> m_binds;
};

class User_shader
{
public:
	User_shader() = delete;
	User_shader(const std::filesystem::path& path);
	static constexpr int m_api_v = 1;
	std::vector<User_shader_pass> m_passes;
	int m_hooked;
};