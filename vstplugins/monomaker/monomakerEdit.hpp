#ifndef __MONOMAKEREDIT_H
#define __MONOMAKEREDIT_H

#include "monomaker.hpp"

class MonomakerEdit : public Monomaker
{
public:
	MonomakerEdit(audioMasterCallback audioMaster);
	~MonomakerEdit();

	virtual void setParameter(long index, float value);
	
	long dispatcher (long opCode, long index, long value, void *ptr, float opt);
};

#endif
