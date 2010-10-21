#pragma once
#include "../VolumeData.h"

class CSphereShellVolume :
	public CVolumeData
{
public:
	CSphereShellVolume(void);
	CSphereShellVolume(unsigned int width, unsigned int height, unsigned int depth):CVolumeData(width, height, depth){}
	~CSphereShellVolume(void);
	void Create(float fCx, float fCy, float fCz, float fSmallRadius, float fLargeRadius, unsigned char density = MAXDENSITY);

private:
	float m_fSmallRadius, m_fLargeRadius;	// in voxels
	float m_fCx, m_fCy, m_fCz;	// center position inside the volume
};
