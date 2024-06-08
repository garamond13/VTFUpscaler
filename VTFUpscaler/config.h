#pragma once

#include "pch.h"

// Workaround for string literal as template-argument.
template <size_t n>
struct Char_array
{
	constexpr Char_array(const char(&str)[n]) :
		val(std::to_array(str))
	{}

	const std::array<char, n> val;
};

template<typename T, Char_array str>
struct Cfg_var
{
	T val;
	static constexpr const char* key = str.val.data();
};

class Config
{
public:
	void read();
	Cfg_var<int, "denoize_filter"> m_denoize_fileter;
	Cfg_var<int, "denoize_radius"> m_denoize_radius;
	Cfg_var<float, "denoize_sigma_spatial"> m_denoize_sigma_spatial;
	Cfg_var<float, "denoize_sigma_intensity"> m_denoize_sigma_intensity;
	Cfg_var<int, "scale_filter"> m_scale_filter;
	Cfg_var<int, "scale_factor"> m_scale_factor;
	Cfg_var<int, "kernel_function"> m_kernel_function;
	Cfg_var<float, "kernel_radius"> m_kernel_radius;
	Cfg_var<float, "kernel_blur"> m_kernel_blur;
	Cfg_var<float, "kernel_param1"> m_kernel_param1;
	Cfg_var<float, "kernel_param2"> m_kernel_param2;
	Cfg_var<float, "antiringing_amount"> m_antiringing_amount;
	Cfg_var<int, "sharpen_filter"> m_sharpen_filter;
	Cfg_var<float, "sharpen_amount"> m_sharpen_amount;
	Cfg_var<int, "unsharp_radius"> m_unsharp_radius;
	Cfg_var<float, "unsharp_sigma"> m_unsharp_sigma;
	Cfg_var<int, "grain_filter"> m_grain_filter;
	Cfg_var<float, "grain_amount"> m_grain_amount;
	Cfg_var<float, "grain_size"> m_grain_size;
	Cfg_var<int, "mipmap_filter"> m_mipmap_filter;
	Cfg_var<bool, "save_uncompressed"> m_save_uncompressed;
private:
	std::filesystem::path get_path();
};