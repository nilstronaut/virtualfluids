//=======================================================================================
// ____          ____    __    ______     __________   __      __       __        __
// \    \       |    |  |  |  |   _   \  |___    ___| |  |    |  |     /  \      |  |
//  \    \      |    |  |  |  |  |_)   |     |  |     |  |    |  |    /    \     |  |
//   \    \     |    |  |  |  |   _   /      |  |     |  |    |  |   /  /\  \    |  |
//    \    \    |    |  |  |  |  | \  \      |  |     |   \__/   |  /  ____  \   |  |____
//     \    \   |    |  |__|  |__|  \__\     |__|      \________/  /__/    \__\  |_______|
//      \    \  |    |   ________________________________________________________________
//       \    \ |    |  |  ______________________________________________________________|
//        \    \|    |  |  |         __          __     __     __     ______      _______
//         \         |  |  |_____   |  |        |  |   |  |   |  |   |   _  \    /  _____)
//          \        |  |   _____|  |  |        |  |   |  |   |  |   |  | \  \   \_______
//           \       |  |  |        |  |_____   |   \_/   |   |  |   |  |_/  /    _____  |
//            \ _____|  |__|        |________|   \_______/    |__|   |______/    (_______/
//
//  This file is part of VirtualFluids. VirtualFluids is free software: you can
//  redistribute it and/or modify it under the terms of the GNU General Public
//  License as published by the Free Software Foundation, either version 3 of
//  the License, or (at your option) any later version.
//
//  VirtualFluids is distributed in the hope that it will be useful, but WITHOUT
//  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
//  for more details.
//
//  SPDX-License-Identifier: GPL-3.0-or-later
//  SPDX-FileCopyrightText: Copyright © VirtualFluids Project contributors, see AUTHORS.md in root folder
//
//! \addtogroup config
//! \ingroup basics
//! \{
//! \author Soeren Peters
//=======================================================================================
#include "ConfigurationFile.h"

#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <basics/utilities/UbException.h>

namespace vf::basics
{

template <>
bool convert_to<bool>(const std::string& value)
{
    return value == "true";
}

void ConfigurationFile::clear()
{
    data.clear();
}
//////////////////////////////////////////////////////////////////////////
void ConfigurationFile::load(const std::string& file)
{
    std::ifstream inFile(file.c_str());

    if (!inFile.good()) {
        const std::string error = "Cannot read configuration file " + file + "! Your current directory is " +
                            std::filesystem::current_path().string() + "\n" +
                            "For further information on how to run VirtualFluids please visit: "
                            "https://irmb.gitlab-pages.rz.tu-bs.de/VirtualFluids/build-and-run.html#run-the-examples";

        throw std::invalid_argument(error);
    }

    while (inFile.good() && !inFile.eof()) {
        std::string line;
        getline(inFile, line);

        // filter out comments
        if (!line.empty()) {
            size_t pos = line.find('#');

            if (pos != std::string::npos) {
                line = line.substr(0, pos);
            }
        }

        // split line into key and value
        if (!line.empty()) {
            size_t pos = line.find('=');

            if (pos != std::string::npos) {
                std::string key = trim(line.substr(0, pos));
                std::string value = trim(line.substr(pos + 1));

                if (!key.empty() && !value.empty()) {
                    data[key] = value;
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
bool ConfigurationFile::contains(const std::string& key) const
{
    return data.find(key) != data.end();
}
//////////////////////////////////////////////////////////////////////////
std::string ConfigurationFile::getValue(const std::string& key) const
{
    std::map<std::string, std::string>::const_iterator iter = data.find(key);

    if (iter != data.end()) {
        return iter->second;
    }
    UB_THROW(UbException(UB_EXARGS, "The parameter \"" + key + "\" is missing!"));
}
//////////////////////////////////////////////////////////////////////////
std::string ConfigurationFile::trim(const std::string& str)
{
    size_t first = str.find_first_not_of(" \t\n\r");

    if (first != std::string::npos) {
        size_t last = str.find_last_not_of(" \t\n\r");

        return str.substr(first, last - first + 1);
    }
    return "";
}
//////////////////////////////////////////////////////////////////////////
void ConfigurationFile::split(std::vector<std::string>& lst, const std::string& input, const std::string& separators,
                              bool remove_empty) const
{
    std::ostringstream word;
    for (size_t n = 0; n < input.size(); ++n) {
        if (std::string::npos == separators.find(input[n]))
            word << input[n];
        else {
            if (!word.str().empty() || !remove_empty)
                lst.push_back(word.str());
            word.str("");
        }
    }
    if (!word.str().empty() || !remove_empty)
        lst.push_back(word.str());
}

//////////////////////////////////////////////////////////////////////////

ConfigurationFile loadConfig(int argc, char* argv[], std::string configPath)
{
    // the config file's default path can be replaced by passing a command line argument

    if (argc > 1) {
        configPath = argv[1];
        VF_LOG_TRACE("Using command line argument for config path: {}", configPath);
    } else {
        VF_LOG_TRACE("Using default config path: {}", configPath);
    }

    vf::basics::ConfigurationFile config;
    config.load(configPath);
    return config;
}
} // namespace vf::basics

//! \}
