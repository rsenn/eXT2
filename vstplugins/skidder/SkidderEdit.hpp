#ifndef __SkidderEdit
#define __SkidderEdit

#include "skidder.hpp"

class SkidderEdit : public Skidder
{
public:
	SkidderEdit(audioMasterCallback audioMaster);
	~SkidderEdit();

	virtual void setParameter(long index, float value);
	
	long dispatcher (long opCode, long index, long value, void *ptr, float opt);	
};

#endif
