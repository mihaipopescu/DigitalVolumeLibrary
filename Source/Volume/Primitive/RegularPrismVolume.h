#pragma once
#include "../VolumeData.h"

class CRegularPrismVolume 
	: public CVolumeData
{
public:
		CRegularPrismVolume(void);
		CRegularPrismVolume(unsigned int width, unsigned int height, unsigned int depth):CVolumeData(width, height, depth){}
		~CRegularPrismVolume(void);
		void Create(float fCx, float fCy, float fCz, float height, float fTopRadius, float fBottomRadius, int numEdges, unsigned char density = MAXDENSITY);

protected:
		float m_fTopRadius, m_fBottomRadius; // radii of circumscribed circles in voxels
		float m_fCx, m_fCy, m_fCz; // center position inside the volume
		float m_fHeight;
		int m_numEdges;
};