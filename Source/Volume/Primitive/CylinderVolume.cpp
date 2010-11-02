#include "StdAfx.h"
#include "CylinderVolume.h"


CCylinderVolume::CCylinderVolume(void)
{
	m_fTopRadius = m_fBottomRadius = 2 * PI;
	m_fCx = m_fCy = m_fCz = 0.f;
	m_fHeight = 1.f;
}

CCylinderVolume::~CCylinderVolume(void)
{
}

void CCylinderVolume::Create(float fCx, float fCy, float fCz, float height, float fTopRadius, float fBottomRadius, unsigned char density)
{
	m_fCx = fCx;
	m_fCy = fCy;
	m_fCz = fCz;
	ClearVoxelData();
	m_fHeight = height;
	m_fTopRadius = fTopRadius;
	m_fBottomRadius = fBottomRadius;	

	for(float h = 0.0f; h <= m_fHeight; h += 1.0f)
	{
		float fCurrentCy = m_fCy - height/2 + h;
		if( m_fCx < m_iWidth && fCurrentCy < m_iHeight && m_fCz < m_iDepth && m_fCx >= 0 && fCurrentCy >= 0 && m_fCz >= 0)
		{
			GetVoxelAt((int)m_fCx, (int)fCurrentCy, (int)m_fCz) = density;	// center density at height h
		}

		float fRadius = m_fBottomRadius + (m_fHeight - h) * (m_fTopRadius - m_fBottomRadius) / m_fHeight;
		
		for(float r = fRadius; r >= 0.1f; r -= 1.0f)
		{
			float fStep = (float)asinf( 1.0f / r) *0.4f;			
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