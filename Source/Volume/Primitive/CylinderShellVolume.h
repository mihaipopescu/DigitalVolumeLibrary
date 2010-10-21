#pragma once
#include "../VolumeData.h"

class CCylinderShellVolume :
	public CVolumeData
{
public:
	CCylinderShellVolume(void);
	CCylinderShellVolume(unsigned int width, unsigned int height, unsigned int depth):CVolumeData(width, height, depth){}
	~CCylinderShellVolume(void);
	void Create(float fCx, float fCy, float fCz, float height, float fTopSmallRadius, float fTopLargeRadius, float fBottomSmallRadius, float fBottomLargeRadius, unsigned char density = MAXDENSITY);

protected:
	float m_fTopSmallRadius, m_fBottomSmallRadius, m_fTopLargeRadius, m_fBottomLargeRadius; // radii of circles for the top and bottom of the tube
	float m_fCx, m_fCy, m_fCz; // center position inside the volume
	float m_fHeight;
};
