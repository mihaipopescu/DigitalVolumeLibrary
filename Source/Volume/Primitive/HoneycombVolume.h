#pragma once
#include "../VolumeData.h"

class CHoneycombVolume :
	public CVolumeData
{
public:
	CHoneycombVolume(void);
	CHoneycombVolume( UINT iLength ) : CVolumeData( iLength, iLength, iLength ) { }
	virtual ~CHoneycombVolume(void);
	
	void Create(UINT iSpan, UCHAR opacity, int xx, int yy, int zz);
};
