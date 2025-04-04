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
//! \addtogroup gpu_Restart Restart
//! \ingroup gpu_core core
//! \{
//! \author Martin Schoenherr
//=======================================================================================
#ifndef RestartObject_H
#define RestartObject_H

#include <memory>
#include <string>
#include <vector>

#include <basics/DataTypes.h>

class Parameter;

class RestartObject
{
public:
    virtual ~RestartObject() = default;

    void deserialize(const std::string &filename, std::shared_ptr<Parameter>& para);
    void serialize(const std::string &filename, const std::shared_ptr<Parameter>& para);

    std::vector<std::vector<real>> fs;

    virtual void serialize_internal(const std::string &filename)   = 0;
    virtual void deserialize_internal(const std::string &filename) = 0;

    void delete_restart_file(const std::string &filename);

private:
    void clear(const std::shared_ptr<Parameter>& para);
    virtual std::string getFileExtension() const = 0;
};

class ASCIIRestartObject : public RestartObject
{
private:
    void serialize_internal(const std::string& filename) override;
    void deserialize_internal(const std::string& filename) override;
    std::string getFileExtension() const override;
};

class BinaryRestartObject : public RestartObject
{
private:
    void serialize_internal(const std::string& filename) override;
    void deserialize_internal(const std::string& filename) override;
    std::string getFileExtension() const override;
};

#endif

//! \}
