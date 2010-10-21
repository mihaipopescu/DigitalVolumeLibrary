#include "StdAfx.h"
#include "ThorusVolume.h"

CThorusVolume::CThorusVolume(void)
{
}

CThorusVolume::~CThorusVolume(void)
{
}

void CThorusVolume::Create(float fCx, float fCy, float fCz, float fSmallRadius, float fLargeRadius, unsigned char density)
{
	m_fCx = fCx;
	m_fCy = fCy;
	m_fCz = fCz;	
	m_fSmallRadius = fSmallRadius;
	m_fLargeRadius = fLargeRadius;	
	float fThorusRadius = (m_fLargeRadius - m_fSmallRadius) * 0.5f;
	float fThorusStep = (float)asin( 1.0 / fThorusRadius ) * 0.5f;
	for(float theta = -PI * 0.5f; theta <= 0 ; theta += fThorusStep)
	{
		float fSmallR = m_fSmallRadius + fThorusRadius * ( 1.0f - cos(theta) );
		float fLargeR = m_fLargeRadius - fThorusRadius * ( 1.0f - cos(theta) );
		float fRelativeHeight = fThorusRadius * sin(theta);		
		for(float r = fSmallR; r <= fLargeR; r += 1.0f)
		{
			float fStep = (float)asin( 1.0 / r) * 0.4f;			
			for(float phi = 0; phi < PI * 0.5f; phi += fStep)	
			{
				float fRCosPhi = r * cos( phi );
				float fRSinPhi = r * sin( phi );
				for(int i = 0; i < 8; i++)
				{
					int x, y, z;
					x = (int)( m_fCx + (i & 1 ? -fRCosPhi : fRCosPhi)  );
					y = (int)( m_fCy + (i & 2 ? -fRelativeHeight : fRelativeHeight) );
					z = (int)( m_fCz + (i & 4 ? -fRSinPhi : fRSinPhi) );
					if( x > m_iWidth || y > m_iHeight || z > m_iDepth || x < 0 || y < 0 || z < 0)
						continue;
					GetVoxelAt(x, y, z) = density;
				}
			}
		}
	}	
}
