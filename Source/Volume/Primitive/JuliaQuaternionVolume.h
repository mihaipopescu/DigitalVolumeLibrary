#pragma once
#include "../VolumeData.h"
#include "../../Math/Vector4D.h"

enum EQUATPLANE
{
	EQP_ABC,
	EQP_RBC,
	EQP_RAC,
	EQP_RAB
};

class CJuliaQuaternionVolume :
	public CVolumeData
{
public:
	CJuliaQuaternionVolume(void);
	virtual ~CJuliaQuaternionVolume(void);
	CJuliaQuaternionVolume(UINT iWidth, UINT iHeight, UINT iDepth) : CVolumeData( iWidth, iHeight, iDepth ) { }

	void Create(const vdl::math::sVector4D<float> c, float f4DPlane, EQUATPLANE eqp, int iIterations);
};
