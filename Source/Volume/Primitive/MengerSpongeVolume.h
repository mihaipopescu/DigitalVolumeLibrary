#pragma once
#include "../VolumeData.h"

class CMengerSpongeVolume :
	public CVolumeData
{
public:
	CMengerSpongeVolume();
	CMengerSpongeVolume( UINT iLength ) : CVolumeData( iLength, iLength, iLength ) { }
	virtual ~CMengerSpongeVolume(void);

public:
	void Create( UINT nIterations );

private:
	void MengerSpongeRecursive();
	UINT m_nIterations;
};
