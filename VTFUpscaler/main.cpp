#include "pch.h"
#include "global.h"
#include "gpu_upscaler.h"

namespace
{
	template<typename T>
	constexpr T g_max_image_size = 4096;

	constexpr std::array g_version = { 1, 2, 0 };
	Gpu_upscaler g_gpu_upscaler;

	void upscale_vtf(const std::filesystem::path& path)
	{
		VTFLib::CVTFFile vtf_src;
		if (vtf_src.Load(path.string().c_str()) == vlFalse) {
			std::cerr << "FAILED: " << path << '\n';
			return;
		}
		g_src_width = vtf_src.GetWidth();
		g_src_height = vtf_src.GetHeight();
		g_vtf_create_options.uiVersion[0] = vtf_src.GetMajorVersion();
		g_vtf_create_options.uiVersion[1] = vtf_src.GetMinorVersion();
		g_vtf_create_options.ImageFormat = vtf_src.GetFormat();
		g_vtf_create_options.uiFlags = vtf_src.GetFlags();

		// Get source data as RGBA8888.
		auto src_data = std::make_unique_for_overwrite<uint8_t[]>(vtf_src.ComputeImageSize(g_src_width, g_src_height, 1, IMAGE_FORMAT_RGBA8888));
		if (vtf_src.ConvertToRGBA8888(vtf_src.GetData(0, 0, 0, 0), src_data.get(), g_src_width, g_src_height, g_vtf_create_options.ImageFormat) == vlFalse) {
			std::cerr << "FAILED: " << path << '\n';
			return;
		}

		if (g_config.m_scale_filter.val) {
			g_dst_width = g_src_width * g_config.m_scale_factor.val;
			g_dst_height = g_src_height * g_config.m_scale_factor.val;

			// Limit scale so that we don't exceed 4k resolution, or stretch image.
			if (g_dst_width > g_max_image_size<int> || g_dst_height > g_max_image_size<int>) {
				const int new_scale = std::min(g_max_image_size<double> / static_cast<double>(g_src_width), g_max_image_size<double> / static_cast<double>(g_src_height));
				g_dst_width = g_src_width * new_scale;
				g_dst_height = g_src_height * new_scale;
			}
		}
		else {
			g_dst_width = g_src_width;
			g_dst_height = g_src_height;
		}

		// Upscale and save in place new vtf file.
		try {
			g_gpu_upscaler.upscale(src_data.get(), path.string().c_str());
		}
		catch (const std::runtime_error& e) {
			std::cerr << e.what() << '\n';
		}

	}
}

int main(int argc, char* argv[])
{
	std::cout << "VTFUpscaler " << g_version[0] << '.' << g_version[1] << '.' << g_version[2] << '\n';

	// Check do we have the right DLL version.
	if (vlGetVersion() != VL_VERSION) {
		std::cerr << "ERROR: Wrong VTFLib.dll version.\n";
		system("pause");
		return 1;
	}

	try {
		g_config.read();
	}
	catch (const std::runtime_error& e) {
		std::cerr << e.what() << '\n';
		system("pause");
		return 1;
	}

	if (!argv[1]) {
		std::cerr << "USAGE: VTFUpscaler file.vtf or VTFUpscaler directory.\n";
		system("pause");
		return 0;
	}
	const std::filesystem::path path(argv[1]);
	
	try {
		g_gpu_upscaler.init();
	}
	catch (const std::runtime_error& e) {
		std::cerr << e.what() << '\n';
		system("pause");
		return 1;
	}

	vlImageCreateDefaultCreateStructure(&g_vtf_create_options);
	g_vtf_create_options.MipmapFilter = static_cast<VTFMipmapFilter>(g_config.m_mipmap_filter.val);

	// Process single file or directory including subdirectories.
	if (std::filesystem::is_directory(path)) {

		// First save paths to all .vtf filles we need to process.
		std::vector<std::filesystem::path> paths;
		for (const auto& path : std::filesystem::recursive_directory_iterator(path))
			if (path.path().extension() == L".vtf")
				paths.push_back(path.path());

		std::cout << std::fixed << std::showpoint << std::setprecision(2);
		for (int i = 0; i < paths.size(); ++i) {
			upscale_vtf(paths[i]);
			std::cout << '\r' << "       "; // Clear the line with 7 spaces.
			std::cout << '\r' << static_cast<float>(i + 1) / static_cast<float>(paths.size()) * 100.0f << '%'; // Show progress.
		}
		std::cout << '\n';
	}
	else if (path.extension() == L".vtf")
		upscale_vtf(path);
	else {
		std::cerr << "ERROR: Only directories or .vtf files are supported.\n";
		system("pause");
		return 1;
	}

	std::cout << "Finished.\n";
	system("pause");
	return 0;
}