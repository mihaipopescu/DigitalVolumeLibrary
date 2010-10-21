#include "stdafx.h"
#include "DomainVoxelNode.h"

sDomainVoxelNode::sDomainVoxelNode( struct sDomainVoxelNode* pParent, eChildPosition childPos, WORD cutX, WORD cutY, WORD cutZ )
{
	myParent = pParent;
	memset(myChilds, 0, ECP_ENUMNO * sizeof(sDomainVoxelNode*));

	if( pParent )
		myBox = pParent->myBox;

	switch( childPos )
	{
	case ECP_ROOT:
		myBox.Left = 0;
		myBox.Right = cutX;
		myBox.Bottom = 0;
		myBox.Top = cutY;
		myBox.Back = 0;
		myBox.Front = cutZ;
		break;
	case ECP_XN_YP_ZP:
		myBox.Right = cutX;
		myBox.Bottom = cutY;
		myBox.Back = cutZ;
		break;
	case ECP_XP_YP_ZP:
		myBox.Left = cutX;
		myBox.Bottom = cutY;
		myBox.Back = cutZ;
		break;
	case ECP_XN_YN_ZP:
		myBox.Right = cutX;
		myBox.Top = cutY;
		myBox.Back = cutZ;
		break;
	case ECP_XP_YN_ZP:
		myBox.Left = cutX;
		myBox.Top = cutY;
		myBox.Back = cutZ;
		break;
	case ECP_XN_YP_ZN:
		myBox.Right = cutX;
		myBox.Bottom = cutY;
		myBox.Front = cutZ;
		break;
	case ECP_XP_YP_ZN:
		myBox.Left = cutX;
		myBox.Bottom = cutY;
		myBox.Front = cutZ;
		break;
	case ECP_XN_YN_ZN:
		myBox.Right = cutX;
		myBox.Top = cutY;
		myBox.Front = cutZ;
		break;
	case ECP_XP_YN_ZN:
		myBox.Left = cutX;
		myBox.Top = cutY;
		myBox.Front = cutZ;
		break;
	}

	myMetaData = -1;		// not computed yet
	myClass = -1;			// not classified yet
}

sDomainVoxelNode::~sDomainVoxelNode()
{
	myParent = NULL;
	for(int i=0; i<ECP_ENUMNO; ++i)
		delete myChilds[i];
}

void sDomainVoxelNode::ComputeMetaData(const CVolumeData *pVolume)
{
	myMetaData = 0.0;

	for(int x = myBox.Left; x<myBox.Right; ++x)
		for(int y = myBox.Bottom; y<myBox.Top; ++y)
			for(int z = myBox.Back; z<myBox.Front; ++z)
				myMetaData += pVolume->GetVoxelValueAt(x, y, z); // 0..1

	myMetaData /= (myBox.Front - myBox.Back)*(myBox.Top - myBox.Bottom)*(myBox.Right - myBox.Left); // normalize [0..1]
}

void sDomainVoxelNode::ClassifyDomain(const CSymmetricGroup8 * pSymGroup)
{
	int vec[ECP_ENUMNO];
	for(int i=0; i<ECP_ENUMNO; ++i)
	{
		vec[i]=1;
		for(int j=0; j<ECP_ENUMNO; ++j)
		{
			if( i==j ) continue;
			if( myChilds[i]->myMetaData > myChilds[j]->myMetaData )
				vec[i]++;
			else
				if( myChilds[i]->myMetaData == myChilds[j]->myMetaData )
				{
					myChilds[j]->myMetaData += EPSILON; // +eps
					vec[j]++;
				}
		}
	}
}
