#ifndef TransmitterType_h__
#define TransmitterType_h__

#include "basics/transmitter/TbTransmitter.h"
#include "basics/transmitter/TbTransmitterLocal.h"
#include "basics/container/CbVector.h"
#include "D3Q27System.h"
#include <PointerDefinitions.h>


typedef TbTransmitter< CbVector< LBMReal > > VectorTransmitter;
typedef VectorTransmitter::value_type  vector_type;
typedef SPtr< TbTransmitter< CbVector< LBMReal > > > VectorTransmitterPtr;

#endif // TransmitterType_h__

