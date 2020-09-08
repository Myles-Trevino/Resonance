/*
	Copyright 2020 Myles Trevino
	Licensed under the Apache License, Version 2.0
	https://www.apache.org/licenses/LICENSE-2.0
*/


#pragma once

#include <string>
#include <vector>


namespace LV::Decoder
{
	void load_track_information(const std::string& file);

	void initialize_resampler_and_decoder();

	void load_samples();

	void destroy();


	// Getters.
	const std::vector<float>& get_data();
}