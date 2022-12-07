#ifndef FILE_WRITER_H
#define FILE_WRITER_H

#include <memory>
#include <vector>
#include <string>



#include "DataWriter.h"

class Parameter;
class CudaMemoryManager;
struct PN27;

class FileWriter : public DataWriter
{
public:
	void writeInit(std::shared_ptr<Parameter> para, std::shared_ptr<CudaMemoryManager> cudaMemoryManager) override;
	void writeTimestep(std::shared_ptr<Parameter> para, unsigned int timestep) override;

private:
	void writeTimestep(std::shared_ptr<Parameter> para, unsigned int timestep, int level) override;
    std::vector<std::string> writeUnstrucuredGridLT(std::shared_ptr<Parameter> para, int level,
                                                         std::vector<std::string> &fname);
	std::vector<std::string> writeUnstrucuredGridMedianLT(std::shared_ptr<Parameter> para, int level, std::vector<std::string >& fname);
	bool isPeriodicCell(std::shared_ptr<Parameter> para, int level, unsigned int number2, unsigned int number1, unsigned int number3, unsigned int number5);

    std::string writeCollectionFile( std::shared_ptr<Parameter> para, unsigned int timestep );

    std::string writeCollectionFileMedian( std::shared_ptr<Parameter> para, unsigned int timestep );

	std::vector<std::string> getNodeDataNames(std::shared_ptr<Parameter> para);
	std::vector<std::string> getMedianNodeDataNames(std::shared_ptr<Parameter> para);

    std::vector< std::string > fileNamesForCollectionFile;
    std::vector< std::string > fileNamesForCollectionFileMedian;
};
#endif