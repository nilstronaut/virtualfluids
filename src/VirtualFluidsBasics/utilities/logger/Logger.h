#ifndef Logger_H
#define Logger_H

#include <VirtualFluidsDefinitions.h>

#include <string>
#include <memory>
#include <ostream>
#include <vector>
#include "../../../core/PointerDefinitions.h"

namespace logging 
{
    class __declspec(dllexport) Logger
    {
    protected:
        Logger(std::ostream* stream);

    public:
        virtual ~Logger();

        enum Level
        {
            HIGH = 3,
            INTERMEDIATE = 2,
            LOW = 1,
            WARNING = 0,
            ERROR = -1
        };

        static void setStream(std::ostream* stream);
        static void addStream(std::ostream* stream);
        static void resetStreams();

        static void setDebugLevel(const Level &level = Level::HIGH);
        static void enablePrintedRankNumbers(bool printRankNumbers);

        virtual Logger& operator<<(const Level &level) = 0;
        virtual Logger& operator<<(const std::string &log) = 0;
        virtual Logger& operator<<(const int &log) = 0;
        virtual Logger& operator<<(const float &log) = 0;
        virtual Logger& operator<<(const double &log) = 0;

    protected:
        void addStreamToList(std::ostream* stream);
        void resetStreamList();

        std::vector<std::ostream*> streams;

        static Level globalLogLevel;
        static Level localLogLevel;
        static bool printRankNumber;

    };

    extern __declspec(dllimport) std::shared_ptr<Logger> out;
}



#endif
