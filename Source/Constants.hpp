/*
	Copyright 2020 Myles Trevino
	Licensed under the Apache License, Version 2.0
	https://www.apache.org/licenses/LICENSE-2.0
*/


#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>


namespace LV::Constants
{
	// General.
	const std::string program_name{"Laventh Resonance"};
	const std::string program_version{"2020-9-20"};
	const std::string resources_directory{"Resources"};
	constexpr bool opengl_logging{false};

	// Generator.
	const std::string generated_data_directory{"Configurations/"};
	const std::string generated_data_file_name_extension{".lrc"};
	constexpr int dft_noise_floor{90}; // Decibels.
	constexpr float bottom{-50.f};

	// Viewer.
	constexpr int samples{8};
	constexpr glm::fvec3 clear_color{.9f, .9f, .9f};
	constexpr glm::fvec2 default_camera_axes{-glm::radians(90.f), -glm::radians(88.f)};
	constexpr float default_camera_fov{glm::radians(100.f)};
	constexpr bool smooth_camera{true};
	constexpr int shadow_resolution{8192};
	constexpr glm::fvec2 initial_light_rotation{0.f, glm::radians(60.f)};
	constexpr glm::fvec3 dft_color{.7f, .7f, .7f};
	constexpr glm::fvec3 base_color{.07f, .07f, .07f};
	constexpr glm::fvec3 wireframe_color{1.f, 1.f, 1.f};

	// Exporter.
	const std::string exports_directory{"Exports/"};
	const std::vector<std::string> supported_formats{"ply", "obj", "stl"};
	const std::string material_name{"Material"};
	constexpr glm::fvec3 material_color{.5f, .5f, .5f};
}
