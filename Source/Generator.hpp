/*
	Copyright 2020 Myles Trevino
	Licensed under the Apache License, Version 2.0
	https://www.apache.org/licenses/LICENSE-2.0
*/


#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>


namespace LV
{
	struct Mesh
	{
		std::vector<glm::fvec3> vertices;
		std::vector<unsigned> indices;
	};
}


namespace LV::Generator
{
	void configure(float dft_window_duration, float sample_interval,
		float harmonic_smoothing, float temporal_smoothing,
		float height_multiplier, const std::string& logarithmic);

	void generate(const std::string& file_name);

	// Getters.
	glm::ivec2 get_size();

	float get_height();

	Mesh get_dft_mesh();

	Mesh get_base_mesh();
}
