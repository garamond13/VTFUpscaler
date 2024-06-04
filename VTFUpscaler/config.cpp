#include "pch.h"
#include "config.h"

/* Config example

// comment
key=value

// comment
key=value

key=value0,value1,value2
key=value

*/

void Config::read()
{

// Macro helper for reading config.
#undef read
#define read(name)\
	if (key == name ## .key) {\
		strtoval(val, name ## .val);\
		continue;\
	}

	std::ifstream file(get_path());
	if (file.is_open()) {
		std::string line;
		while (std::getline(file, line)) {

			// Skip comments.
			if (line[0] == '/' && line[1] == '/')
				continue;

			const auto pos = line.find('=');
			if (pos != std::string::npos) {
				const auto key = line.substr(0, pos);
				const auto val = line.substr(pos + 1);
				read(m_scale_factor)
				read(m_use_jinc)
				read(m_kernel_function)
				read(m_kernel_radius)
				read(m_kernel_blur)
				read(m_kernel_param1)
				read(m_kernel_param2)
				read(m_antiringing_amount)
				read(m_unsharp_enabeled)
				read(m_unsharp_radius)
				read(m_unsharp_sigma)
				read(m_unsharp_amount)
				read(m_mipmap_filter)
			}
		}
	}
	else
		throw std::runtime_error("ERROR: Couldn't open config.txt.");
}

std::filesystem::path Config::get_path()
{
	wchar_t path[MAX_PATH];
	GetModuleFileNameW(nullptr, path, MAX_PATH);
	return std::filesystem::path(path).parent_path() / L"config.txt";
}