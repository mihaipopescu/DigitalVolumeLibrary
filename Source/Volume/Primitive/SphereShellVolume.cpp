#include "StdAfx.h"
#include "SphereShellVolume.h"


CSphereShellVolume::CSphereShellVolume(void)
{
}

CSphereShellVolume::~CSphereShellVolume(void)
{
}

void CSphereShellVolume::Create(float fCx, float fCy, float fCz, float fSmallRadius, float fLargeRadius, unsigned char density)
{	
	if(m_fSmallRadius > m_fLargeRadius)
		return;

	m_fCx = fCx;
	m_fCy = fCy;
	m_fCz = fCz;
	m_fSmallRadius = fSmallRadius;
	m_fLargeRadius = fLargeRadius;

	for(float r = m_fSmallRadius; r < m_fLargeRadius; r += 1.0f)
	{
		float fStep = (float)asinf( 1.0 / r) / 2.0f;
		for(float theta = 0.0f; theta < PI * 0.5f; theta += fStep)		
		{
			float fRSinTheta = r * sin(theta);			
			float fRCosTheta = r * cos(theta);
			for(float phi = 0; phi < PI * 0.5f; phi += fStep)	
			{
				float fRSinThetaCosPhi = fRSinTheta * cos(phi);
				float fRSinThetaSinPhi = fRSinTheta * sin(phi);
				for(int i = 0; i < 8; ++i)
				{
					int x, y, z;
					x = (int)( m_fCx + ( i & 1 ? fRSinThetaCosPhi : -fRSinThetaCosPhi ) );
					y = (int)( m_fCy + ( i & 2 ? fRSinThetaSinPhi : -fRSinThetaSinPhi ) );
					z = (int)( m_fCz + ( i & 4 ? fRCosTheta : -fRCosTheta ) );
					if( x < m_iWidth && y < m_iHeight && z < m_iDepth && x >= 0 && y >= 0 && z >= 0)
						GetVoxelAt(x, y, z) = density;
				}
			}
		}
	}	
}