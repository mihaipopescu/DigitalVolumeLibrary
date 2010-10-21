#pragma once
#include "../VolumeData.h"

class CSphereVolume 
	: public CVolumeData
{
public:
	CSphereVolume(void);
	CSphereVolume(unsigned int width, unsigned int height, unsigned int depth):CVolumeData(width, height, depth){}
	~CSphereVolume(void);

	void Create(float fCx, float fCy, float fCz, float fRadius, unsigned char density = MAXDENSITY);

protected:
	
	float m_fRadius;	// in voxels
	float m_fCx, m_fCy, m_fCz;	// center position inside the volume
};
