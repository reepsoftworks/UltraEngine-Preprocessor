//========= Copyright Reep Softworks, All rights reserved. ============//
// 
// https://github.com/reepsoftworks/UltraEngine-Preprocessor
// This code is free to use for those who wish to develop and ship their 
// commerical or non-commerical game projects using Ultra Engine.
// Do not remove this notice.
// 
//=====================================================================//
#include "UltraEngine.h"
using namespace UltraEngine;

const static WString componentspath = "Components";
const static WString outfile = "RegisterComponents.h";
static std::vector<WString> components;
static bool generate = false;

const String GenerationTimestamp()
{
	auto now = std::chrono::system_clock::now();
	std::time_t end_time = std::chrono::system_clock::to_time_t(now);
	return String(std::ctime(&end_time));
}

void LoadFiles(const WString& path, std::vector<WString>& files)
{
	if (FileType(path) == 2)
	{
		auto dir = LoadDir(path);
		for (const auto& file : dir)
		{
			auto filepath = path;
			if (not filepath.empty()) filepath += L"/";
			filepath += file;
			switch (FileType(filepath))
			{
			case 1:
				if (ExtractExt(file).Lower() == "hpp" or ExtractExt(file).Lower() == "h")
				{
					files.push_back(filepath);

					// Check to see if the component has a json file.
					// If not, generate one since we're cool like that. B)
					auto jsonpath = ExtractDir(filepath) + "/" + StripExt(file) + ".json";
					if (FileType(jsonpath) != 1)
					{
						Print("Writing file \"" + FixPath(jsonpath) + "\"");

						auto j3 = table();
						j3["component"] = {};
						j3["component"]["properties"] = {};
						j3["component"]["properties"][0]; // Write nothing here.
						if (!SaveTable(j3, jsonpath))
						{
							Print("Error: Failed to write file \"" + FixPath(jsonpath) + "\"");
						}
					}
				}
				break;
			case 2:
				LoadFiles(filepath, files);
				break;
			}
		}
	}
	else
	{
		// If there's no component folder, abort the process.
		Print("Error: Failed to locate Components Directory!");
	}
}

static int WriteHeader()
{
	auto stream = CreateBufferStream();
	if (stream == NULL)
	{
		Print("Error: Failed to write file \"" + FixPath(outfile) + "\"");
		return 1;
	}

	stream->WriteLine("// This file is generated by the pre-processor on " + GenerationTimestamp() + "// Please do not do not modify this file!");
	stream->WriteLine("#pragma once");
	stream->WriteLine("#include \"UltraEngine.h\"");
	stream->WriteLine("using namespace UltraEngine;");
	stream->WriteLine("");

	for (const auto& p : components)
	{
		stream->WriteLine("#include \"" + FixPath(p).ToString() + "\"");
	}

	stream->WriteLine("\nstatic void RegisterComponents()\n{");

	for (const auto& p : components)
	{
		auto name = StripAll(p);
		stream->WriteLine("\tRegisterComponent<" + name.ToString() + ">();");
	}
	stream->WriteLine("}");

	if (!stream->data->Save(outfile))
	{
		Print("Error: Failed to write file \"" + FixPath(outfile) + "\"");
		return 1;
	}

	stream = NULL;

	return 0;
}

int main(int argc, const char* argv[])
{
	auto cl = ParseCommandLine(argc, argv);
	Print("Ultra Engine Pre-Processor - (" + WString(__DATE__) + ")");

	if (FileType("Source") == 2)
	{
		ChangeDir("Source");
		LoadFiles(componentspath, components);
	}
	else
	{
		// If the application is in a diffrent folder (eg Tools), allow the operator to set the target of the project and try again.
		if (cl["path"].is_string())
		{
			String workpath = cl["path"];
			if (FileType(FixPath(workpath + "/Source")) == 2)
			{
				ChangeDir(workpath + "/Source");
				LoadFiles(componentspath, components);
			}
		}

		Print("Error: Failed to locate Source Directory!");
		return 1;
	}

	// Check to see if the operator wants to force this.
	if (cl["forcegen"].is_boolean())
	{
		const bool b = cl["forcegen"];
		generate = b;

		Print("Alert: Force Generating!");
	}

	int rtn = 0;
	auto tm = Millisecs();

	// Check to see if we need to regenerate the header based on updated or new files since.
	if (!generate)
	{
		uint64_t headertime = FileTime(outfile);
		if (headertime > 0)
		{
			for (const auto& p : components)
			{
				if (FileTime(p) > headertime)
				{
					Print("File \"" + FixPath(p) + "\" is newer than \"" + FixPath(outfile) + ", regenerating...");
					generate = true;
					break;
				}
			}

			if (!generate)
			{
				Print("Components are up-to-date!");
			}
		}
		else
		{
			generate = true;
		}
	}

	// Generate the header.
	if (generate)
	{
		rtn = WriteHeader();
		if (rtn == 0) 
			Print("Pre-processor completed successfully - Time Elapsed: " + String(Millisecs() - tm) + " milliseconds.");
		else
			Print("Pre-processor FAILED - Time Elapsed: " + String(Millisecs() - tm) + " milliseconds.");
	}

    return rtn;
}