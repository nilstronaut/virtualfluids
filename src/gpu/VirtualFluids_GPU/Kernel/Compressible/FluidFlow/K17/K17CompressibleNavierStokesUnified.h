#ifndef K17CompressibleNavierStokesUnified_H
#define K17CompressibleNavierStokesUnified_H

#include "Kernel/KernelImp.h"

namespace vf
{
namespace gpu
{


class K17CompressibleNavierStokesUnified : public KernelImp
{
public:
    K17CompressibleNavierStokesUnified(std::shared_ptr<Parameter> para, int level);

    void run();
};

}
}

#endif
