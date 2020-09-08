/*
	Copyright 2020 Myles Trevino
	Licensed under the Apache License, Version 2.0
	https://www.apache.org/licenses/LICENSE-2.0
*/


#include <iostream>
#include <regex>

#include "Constants.hpp"
#include "Utilities.hpp"
#include "Generator.hpp"
#include "Viewer.hpp"
#include "Exporter.hpp"


void print_documentation()
{
	std::cout<<"\nTo configure Resonance's model generation settings, enter: 'configure "
		"<dft window duration> <dft sample interval> <harmonic smoothing> <temporal smoothing> "
		"<height multiplier>'. For example: 'configure 30 1 25 2 .3'."

		"\n\nDFT window duration specifies the DFT window in milliseconds. Larger durations "
		"display a wider range of frequencies, increasing the depth of the model and making it "
		"smoother along its width."
		
		"\n\nSample interval specifies the interval between when DFTs samples are taken in "
		"milliseconds. Larger intervals skip more audio data, decreasing the width of the "
		"model and making it rougher along its width."

		"\n\nHarmonic smoothing specifies the number of harmonically adjacent frequencies to "
		"sample in each direction for each point of the model. The higher this is, the smoother "
		"the model is along its depth."
		
		"\n\nTemporal smoothing specifies number of temporally adjacent DFTs to sample in each "
		"direction for each point of the model. The higher this is, the smoother the model will "
		"be along its width. This can help with maintaining a smooth result when using short DFT "
		"windows or large sample intervals."

		"\n\nHeight multiplier scales the height of the model relative to its default value."

		"\n\n---"

		"\n\nTo preview model generation for an audio file, enter: 'view <file name>'. For "
		"example: 'view shadowplay.flac'."
		
		"\n\nThe file name must only contain alphanumeric characters, dashes, and periods "
		"(no spaces). The supported audio file types are: FLAC, MP3, and WAV. Be careful "
		"with the length of the audio file. Files more than a couple seconds long can be very "
		"intensive depending on the configuration."

		"\n\nIn the Viewer, navigate using the 'W', 'A', 'S', and 'D' keys and the mouse. Hold "
		"'Shift' to move faster. Press 'L' to toggle mouse locking. Press the 'F' key to "
		"toggle wireframe rendering. Use the left and right arrow keys to change the light "
		"direction. Use the scrollwheel to change the FOV. Press the 'Esc' key to close the "
		"Viewer."

		"\n\n---"

		"\n\nWhen you're ready, you can export the model by entering: 'export <file name> "
		"<format> <orientation>'. For example: 'export shadowplay.flac ply z-up'."

		"\n\nThe file name must only contain alphanumeric characters, dashes, and periods "
		"(no spaces). The supported audio file types are: FLAC, MP3, and WAV."
		
		"\n\nFormat must be one of: 'ply', 'obj', or 'stl'."
		
		"\n\nOrientation can be either 'z-up' or 'y-up'."

		"\n\nExported models will be saved within the 'Exports' folder. STL is not "
		"recommended for large exports. Exporting as OBJ will generate a corresponding MTL "
		"file."

		"\n\n---"
		
		"\n\nTo exit, enter 'exit'."
		
		"\n\nFor detailed documentation, visit laventh.com.\n";
}


void print_startup_message()
{
	std::cout<<LV::Constants::program_name<<" "<<LV::Constants::program_version<<
		"\nCopyright 2020 Myles Trevino"
		"\nlaventh.com"

		"\n\nLicensed under the Apache License, Version 2.0"
		"\nhttps://www.apache.org/licenses/LICENSE-2.0"

		"\n\nEnter 'help' for documentation.\n";
}


void validate_command_parameters(const std::string& command, int required, size_t given)
{
	if(given != required) throw std::runtime_error{"'"+command+"' requires "
		+std::to_string(required)+" parameters but "+std::to_string(given)+" were given."};
}


void validate_name(const std::string& name)
{
	if(!std::regex_match(name, std::regex{"^[a-zA-Z0-9-.]+.(flac|mp3|wav)$"}))
		throw std::runtime_error{"Invalid audio file name. The file name must "
			"consist only of alphanumeric characters, dashes, and periods, and be "
			"either FLAC, MP3, or WAV."};
}


int main(int arguments_count, const char* arguments[])
{
	// Initialize.
	LV::Utilities::platform_initialization(arguments[0]);
	print_startup_message();

	while(true)
	{
		try
		{
			// Prompt for input.
			std::cout<<"\n> ";
			std::string input;
			std::getline(std::cin, input);

			// Get the tokens and command name.
			std::vector<std::string> tokens{LV::Utilities::split(input)};
			if(tokens.empty()) throw std::runtime_error{"No command entered."};

			std::string command_name{tokens[0]};
			tokens.erase(tokens.begin());

			// Parse and execute the command.
			if(command_name == "configure")
			{
				validate_command_parameters(command_name, 5, tokens.size());
				LV::Generator::configure(std::stof(tokens[0]), std::stof(tokens[1]),
					std::stof(tokens[2]), std::stof(tokens[3]), std::stof(tokens[4]));
			}

			else if(command_name == "view")
			{
				validate_command_parameters(command_name, 1, tokens.size());
				validate_name(tokens[0]);
				LV::Viewer::view(tokens[0]);
			}

			else if(command_name == "export")
			{
				validate_command_parameters(command_name, 3, tokens.size());
				validate_name(tokens[0]);
				LV::Exporter::export_model(tokens[0], tokens[1], tokens[2]);
			}

			else if(command_name == "exit")
			{
				std::cout<<"Exiting...\n";	
				break;
			}

			else if(command_name == "help") print_documentation();
			else throw std::runtime_error{"Unrecognized command."};
		}
		catch(std::exception& error){ std::cout<<"ERROR: "<<error.what()<<'\n'; }
		catch(...){ std::cout<<"ERROR: Unhandled exception.\n"; }
	}
}
