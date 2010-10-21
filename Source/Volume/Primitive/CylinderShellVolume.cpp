#include "StdAfx.h"
#include "CylinderShellVolume.h"

CCylinderShellVolume::CCylinderShellVolume()
{
	m_fTopSmallRadius = m_fBottomSmallRadius = m_fTopLargeRadius = m_fBottomLargeRadius = 2*PI;
	m_fCx = m_fCy = m_fCz = 0.f;
	m_fHeight = 1.f;
}

CCylinderShellVolume::~CCylinderShellVolume()
{
}

void CCylinderShellVolume::Create(	float fCx, float fCy, float fCz, float height, float fTopSmallRadius, float fTopLargeRadius, 
									float fBottomSmallRadius, float fBottomLargeRadius, unsigned char density )
{
	m_fCx = fCx;
	m_fCy = fCy;
	m_fCz = fCz;
	m_fHeight = height;
	m_fTopSmallRadius = fTopSmallRadius;
	m_fTopLargeRadius = fTopLargeRadius;
	m_fBottomSmallRadius = fBottomSmallRadius;	
	m_fBottomLargeRadius = fBottomLargeRadius;

	for(float h = 0.0f; h <= m_fHeight; h += 1.0f)
	{
		float fCurrentCy = m_fCy - height/2 + h;		

		float fSmallRadius = m_fBottomSmallRadius + (m_fHeight - h) * (m_fTopSmallRadius - m_fBottomSmallRadius) / m_fHeight;
		float fLargeRadius = m_fBottomLargeRadius + (m_fHeight - h) * (m_fTopLargeRadius - m_fBottomLargeRadius) / m_fHeight;
		
		for(float r = fSmallRadius; r <= fLargeRadius; r += 1.0f)
		{
			float fStep = (float)asin( 1.0f / r) * 0.4f;			
			for(float phi = 0; phi < PI * 0.5f; phi += fStep)	
			{
				for(unsigned char i = 0; i < 4; i++)
				{
					int x, y, z;
					x = (int)( m_fCx + (i & 1 ? -1: 1) * r * cos(phi) );
					y = (int)( fCurrentCy );
					z = (int)( m_fCz + (i & 2 ? -1: 1) * r * sin(phi) );
					if( x > (int)m_iWidth || y > (int)m_iHeight || z > (int)m_iDepth || x < 0 || y < 0 || z < 0)
						continue;
					GetVoxelAt(x, y, z) = density;
				}
			}
		}
	}	
}