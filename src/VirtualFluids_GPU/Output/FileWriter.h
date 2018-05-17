#ifndef FILE_WRITER_H
#define FILE_WRITER_H

#include <memory>

#include <VirtualFluidsDefinitions.h>

#include "DataWriter.h"

class Parameter;

class FileWriter : public DataWriter
{
public:
	VF_PUBLIC FileWriter() {}

	void VF_PUBLIC writeInit(std::shared_ptr<Parameter> para) override;
	void VF_PUBLIC writeTimestep(std::shared_ptr<Parameter> para, unsigned int timestep) override;
    void VF_PUBLIC writeParticle(Parameter* para, unsigned int t);
private:
	
	FileWriter(const FileWriter& fileWriter) {};
};
#endif