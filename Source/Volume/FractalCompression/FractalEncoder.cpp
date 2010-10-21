#include "stdafx.h"
#include "FractalEncoder.h"
#include "SymmetricGroup8.h"


CFractalEncoder::CFractalEncoder(const CVolumeData* pVolumeData) : myVolumeData( pVolumeData )
{
	myRootDomain = new sDomainVoxelNode(NULL, sDomainVoxelNode::ECP_ROOT, myVolumeData->m_iWidth, myVolumeData->m_iHeight, myVolumeData->m_iDepth);
}

void CFractalEncoder::Encode()
{
	Partition_Octree( myRootDomain );
}

void CFractalEncoder::Partition_Octree(struct sDomainVoxelNode* node)
{
	for(int pos = 0; pos<sDomainVoxelNode::ECP_ENUMNO; ++pos)
	{
		node->myChilds[pos] = new sDomainVoxelNode(node, (sDomainVoxelNode::eChildPosition)pos, (node->myBox.Right - node->myBox.Left)/2, (node->myBox.Top - node->myBox.Bottom)/2, (node->myBox.Front - node->myBox.Back)/2);
		node->myChilds[pos]->ComputeMetaData( myVolumeData );
	}

	node->ClassifyDomain( &CSymmetricGroup8::GetStaticInstance() );
}
