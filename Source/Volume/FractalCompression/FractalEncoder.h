#ifndef _FRACTAL_ENCODER_
#define _FRACTAL_ENCODER_

#include "DomainVoxelNode.h"
#include "../VolumeData.h"


class CFractalEncoder
{
public:
	CFractalEncoder(const CVolumeData* pVolumeData);

	void Encode();

private:
	void Partition_Octree( struct sDomainVoxelNode* node );

private:
	const CVolumeData* myVolumeData;
	sDomainVoxelNode* myRootDomain;
};

#endif // s_FRACTAL_ENCODER_