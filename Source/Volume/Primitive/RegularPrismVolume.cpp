#include "StdAfx.h"
#include "RegularPrismVolume.h"


CRegularPrismVolume::CRegularPrismVolume(void)
{
	m_fCx = m_fCy = m_fCz = m_fTopRadius = m_fBottomRadius = m_fHeight = 0.0f;
	m_numEdges = 0;
}

CRegularPrismVolume::~CRegularPrismVolume(void)
{
}

void CRegularPrismVolume::Create(float fCx, float fCy, float fCz, float height, float fTopRadius, float fBottomRadius, int numEdges, unsigned char density)
{
	m_fCx = fCx;
	m_fCy = fCy;
	m_fCz = fCz;
	m_fHeight = height;
	m_fTopRadius = fTopRadius;
	m_fBottomRadius = fBottomRadius;
	m_numEdges = numEdges;

	struct Vec3{ float x,y,z; } *v = new Vec3[numEdges];
	float alpha = 2 * PI / m_numEdges;
	float *sines = new float[m_numEdges];
	float *cosines = new float[m_numEdges];
	for(int i = 0; i < m_numEdges; ++i)
	{
		sines[i] = sin( i * alpha );
		cosines[i] = cos( i * alpha);
	}

	for(float h = 0.0f; h <= m_fHeight; h += 1.0f)
	{
		GetVoxelAt((int)m_fCx, (int)m_fCy, (int)m_fCz) = density;	// center desnsity at height h

		float fRadius = m_fBottomRadius + (m_fHeight - h) * (m_fTopRadius - m_fBottomRadius) / m_fHeight;
		float fCurrentCy = m_fCy - height/2 + h;
		for(float r = fRadius; r >= 0.1f; r -= 1.0f)
		{

			for(int i = 0; i < m_numEdges; ++i)
			{
				v[i].x = m_fCx + r * sines[i];
				v[i].y = fCurrentCy;
				v[i].z = m_fCz + r * cosines[i];
			}

			for(int i = 0; i < m_numEdges; ++i)
			{				
				int iplus1 = (i + 1) % m_numEdges;				
				float fZDist = v[iplus1].z - v[i].z;
				float fXDist = v[iplus1].x - v[i].x;
				float fZStep = fZDist > 0.0f ? 0.1f : -0.1f;
				float fXStep = fXDist > 0.0f ? 0.1f : -0.1f;

				if( fabs(fZDist) < 0.1f)
				{
					if( fabs(fXDist) < 0.1f)
					{
						// the line segment is basicly a voxel in this case
						if( m_fCx > m_iWidth || fCurrentCy > m_iHeight || m_fCz > m_iDepth || m_fCx < 0 || fCurrentCy < 0 || m_fCz < 0)
							continue;

						GetVoxelAt((int)m_fCx, (int)fCurrentCy, (int)m_fCz) = density;
					}
					else
					{		
						// interpolate over x
						for(float xt = v[i].x; fabs(xt - v[iplus1].x) >= 0.1f; xt += fXStep)
						{
							int x, y, z;
							x = (int)( xt );						
							y = (int)( v[i].y );
							z = (int)( v[i].z + (xt - v[i].x) * fZDist / fXDist );						
							if( x > m_iWidth || y > m_iHeight || z > m_iDepth || x < 0 || y < 0 || z < 0)
								continue;

							GetVoxelAt(x, y, z) = density;
						}					
					}
				}
				else
				{
					// interpolate over z
					for(float zt = v[i].z; fabs(zt - v[iplus1].z) >= 0.1f; zt += fZStep)
					{
						int x, y, z;
						x = (int)( v[i].x + (zt - v[i].z) * fXDist / fZDist );
						y = (int)( v[i].y );
						z = (int)( zt );
						if( x > m_iWidth || y > m_iHeight || z > m_iDepth || x < 0 || y < 0 || z < 0)
							continue;

						GetVoxelAt(x, y, z) = density;
					}
				}
			}
		}
	}

	delete[] v;
	delete[] sines;
	delete[] cosines;
}