<<<<<<< HEAD
/*-----------------------------------------------------------------------------

 1999, Steinberg Soft und Hardware GmbH, All Rights Reserved

-----------------------------------------------------------------------------*/
#ifndef __BITCRUSHEREDIT_H
#define __BITCRUSHEREDIT_H

#include "Bitcrusher.hpp"
#include <string.h>

class BitcrusherEdit : public Bitcrusher
{
public:
    BitcrusherEdit (audioMasterCallback audioMaster);
    ~BitcrusherEdit ();

    virtual void setParameter (long index, float value);

    long dispatcher (long opCode, long index, long value, void *ptr, float opt);
};

#endif
=======
/*-----------------------------------------------------------------------------

 1999, Steinberg Soft und Hardware GmbH, All Rights Reserved

-----------------------------------------------------------------------------*/
#ifndef __BITCRUSHEREDIT_H
#define __BITCRUSHEREDIT_H

#include "Bitcrusher.hpp"
#include <string.h>

class BitcrusherEdit : public Bitcrusher
{
public:
    BitcrusherEdit (audioMasterCallback audioMaster);
    ~BitcrusherEdit ();

    virtual void setParameter (long index, float value);

    long dispatcher (long opCode, long index, long value, void *ptr, float opt);
};

#endif
>>>>>>> b32feae3968ea26b82a00fee5a6b1c8375c0568a
