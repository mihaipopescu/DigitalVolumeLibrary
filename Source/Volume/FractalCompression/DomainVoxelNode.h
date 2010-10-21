#ifndef _DOMAIN_VOXEL_NODE_
#define _DOMAIN_VOXEL_NODE_

#include "SymmetricGroup8.h"
#include "../VolumeData.h"


#define EPSILON 1e-16


struct sBox {
	WORD Left;
	WORD Top;
	WORD Right;
	WORD Bottom;
	WORD Front;
	WORD Back;
};

struct sDomainVoxelNode
{

	enum eChildPosition
	{
		ECP_ROOT = -1,
		ECP_XN_YP_ZP,
		ECP_XP_YP_ZP,
		ECP_XN_YN_ZP,
		ECP_XP_YN_ZP,
		ECP_XN_YP_ZN,
		ECP_XP_YP_ZN,
		ECP_XN_YN_ZN,
		ECP_XP_YN_ZN,
		ECP_ENUMNO
	};

	sDomainVoxelNode( struct sDomainVoxelNode* pParent, eChildPosition childPos, WORD cutX, WORD cutY, WORD cutZ );
	~sDomainVoxelNode();

public:
	void ComputeMetaData(const CVolumeData *pVolume);
	void ClassifyDomain(const CSymmetricGroup8 * pSymGroup);

public:
	double myMetaData;

	DWORD myClass; // 1..10080 = 8!/4

	sBox myBox;

	struct sDomainVoxelNode *myParent;
	struct sDomainVoxelNode *myChilds[ ECP_ENUMNO ];

};

#endif // _DOMAIN_VOXEL_NODE_