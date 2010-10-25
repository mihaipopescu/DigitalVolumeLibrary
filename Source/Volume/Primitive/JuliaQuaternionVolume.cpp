#include "StdAfx.h"
#include "JuliaQuaternionVolume.h"

CJuliaQuaternionVolume::CJuliaQuaternionVolume(void)
{
}

CJuliaQuaternionVolume::~CJuliaQuaternionVolume(void)
{
}

void CJuliaQuaternionVolume::Create(const vdl::math::sVector4D<float> c, float f4DPlane, EQUATPLANE eqp, int iIterations)
{
	ClearVoxelData();

	int s = 0;
	for(UINT z = 0; z < m_iDepth; z++)
	{
		for(UINT y = 0; y < m_iHeight; y++)
		{
			for(UINT x = 0; x < m_iWidth; x++)
			{
				vdl::math::sVector4D<float> q;
				switch(eqp)
				{
				case EQP_ABC:
					q.x = f4DPlane;
					q.y = x * 2.f/(m_iWidth - 1) - 1;
					q.z = y * 2.f/(m_iHeight - 1) - 1;
					q.w = z * 2.f/(m_iDepth - 1) - 1;
					break;
				case EQP_RBC:
					q.x = x * 2.f/(m_iWidth - 1) - 1;
					q.y = f4DPlane;
					q.z = y * 2.f/(m_iHeight - 1) - 1;
					q.w = z * 2.f/(m_iDepth - 1) - 1;
					break;
				case EQP_RAC:
					q.x = x * 2.f/(m_iWidth - 1) - 1;
					q.y = y * 2.f/(m_iHeight - 1) - 1;
					q.z = f4DPlane;
					q.w = z * 2.f/(m_iDepth - 1) - 1;
					break;
				case EQP_RAB:
					q.x = x * 2.f/(m_iWidth - 1) - 1;
					q.y = y * 2.f/(m_iHeight - 1) - 1;
					q.z = z * 2.f/(m_iDepth - 1) - 1;
					q.w = f4DPlane;
					break;
				}

				int it = 0;
				for( ; it < iIterations; it++)
				{
					// z[n+1] = z[n]*z[n] + c
					q = q*q + c;

					// bailout condition
					if(q.Norm() > 16.f)
						break;
				}

				it = (int)(8.f * it / iIterations); // 0 .. 8
				GetVoxelAt(x,y,z) =  (1 << it) - 1;

				s++;
			}
		}
	}
}