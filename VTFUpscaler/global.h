#pragma once

#include "pch.h"
#include "config.h"
#include "user_shader.h"

inline Config g_config;
inline int g_src_width;
inline int g_src_height;
inline int g_dst_width;
inline int g_dst_height;
inline SVTFCreateOptions g_vtf_create_options;
inline std::vector<User_shader> g_user_shaders;