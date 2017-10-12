#include "LoggerImp.h"

#include "mpi.h"

#include <memory>
#include <iostream>

logging::LoggerImp::LoggerImp(std::ostream &stream) : logging::Logger(stream)
{

}

logging::LoggerImp::~LoggerImp()
{

}

logging::Logger& logging::LoggerImp::operator<<(const Level &level)
{
    localLogLevel = level;
    return *this;
}

bool logging::LoggerImp::isLocalLogLevelHighEnough()
{
    return localLogLevel >= globalLogLevel;
}

logging::Logger& logging::LoggerImp::operator<<(const std::string &message)
{
    return this->log(message);
}

logging::Logger& logging::LoggerImp::operator<<(const int &message)
{
    return this->log(std::to_string(message));
}

logging::Logger& logging::LoggerImp::operator<<(const float &message)
{
    return this->log(std::to_string(message));
}

logging::Logger& logging::LoggerImp::operator<<(const double &message)
{
    return this->log(std::to_string(message));
}


logging::Logger& logging::LoggerImp::log(const std::string &message)
{
    if (isLocalLogLevelHighEnough())
        stream << getRankString() + message;
    return *this;
}




std::string logging::LoggerImp::getRankString()
{
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    return printRankNumber ? "[" + std::to_string(rank) + "] " : "";
}

