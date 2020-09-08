/*
	Copyright 2020 Myles Trevino
	Licensed under the Apache License, Version 2.0
	https://www.apache.org/licenses/LICENSE-2.0
*/


#pragma once

#include <string>


namespace LV::Exporter
{
	void export_model(const std::string& file_name,
		const std::string& format, const std::string& orientation);
}