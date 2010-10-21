#pragma once
#include "../VolumeData.h"

class CCylinderVolume :
	public CVolumeData
{
public:
	CCylinderVolume(void);
	CCylinderVolume(unsigned int width, unsigned int height, unsigned int depth):CVolumeData(width, height, depth){}
	~CCylinderVolume(void);
	void Create(float fCx, float fCy, float fCz, float height, float fTopRadius, float fBottomRadius, unsigned char density = MAXDENSITY);

protected:
	float m_fTopRadius, m_fBottomRadius; // radii of circumscribed circles in voxels
	float m_fCx, m_fCy, m_fCz; // center position inside the volume
	float m_fHeight;	
};
