#pragma once
#include "../VolumeData.h"

class CThorusVolume :
	public CVolumeData
{
public:
	CThorusVolume(void);
	CThorusVolume(unsigned int width, unsigned int height, unsigned int depth):CVolumeData(width, height, depth){}
	~CThorusVolume(void);
	void Create(float fCx, float fCy, float fCz, float fSmallRadius, float fLargeRadius, unsigned char density = MAXDENSITY);

protected:
	float m_fSmallRadius, m_fLargeRadius; // radii of interior and exterior circles (2) in voxels
	float m_fCx, m_fCy, m_fCz; // center position inside the volume	
};
