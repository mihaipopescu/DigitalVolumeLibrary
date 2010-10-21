#include "StdAfx.h"
#include "MengerSpongeVolume.h"
#include <list>


CMengerSpongeVolume::CMengerSpongeVolume()
{	
	m_nIterations = 0;
}

CMengerSpongeVolume::~CMengerSpongeVolume(void)
{
}

void CMengerSpongeVolume::Create( UINT nIterations )
{
	// with 6 iterations Sierpinsky cube have over 64 millions cubes !
	if( nIterations >= 6 )
		return;

	m_nIterations = nIterations;
	
	MengerSpongeRecursive();
}

bool operator == (const BOX& b1, const BOX& b2)
{
	return	b1.Back == b2.Back && 
			b1.Bottom == b2.Bottom &&
			b1.Front == b2.Front &&
			b1.Left == b2.Left &&
			b1.Right == b2.Right &&
			b1.Top == b2.Top;
}

void CMengerSpongeVolume::MengerSpongeRecursive()
{
	static std::list<BOX> boxes;
	
	ClearVoxelData();

	BOX box;
	box.Left = 0;
	box.Right = m_iWidth;
	box.Bottom = 0;
	box.Top = m_iHeight;
	box.Front = 0;
	box.Back = m_iDepth;

	boxes.clear();
	boxes.push_back(box);

	if( m_nIterations > 0 )
	{
		std::list<BOX> cubes;
		std::list<BOX>::iterator it;
		for( it = boxes.begin(); it != boxes.end(); it++)
		{
			BOX box = *it;

			// split curret box into 27 cubes
			float cx = float(box.Right - box.Left) / 3;
			float cy = float(box.Top - box.Bottom) / 3;
			float cz = float(box.Back - box.Front) / 3;

			for(int i=0;i<3;i++)
				for(int j=0;j<3;j++)
					for(int k=0;k<3;k++)
					{
						// skip inside cubes
						if(	i==1 && j==1 || j==1 && k==1 ||	i==1 && k==1)
							continue;

						BOX cube;
						cube.Left = UINT(box.Left + i * cx);
						cube.Right = UINT(box.Left + (i+1) * cx);
						cube.Bottom = UINT(box.Bottom + j * cx);
						cube.Top = UINT(box.Bottom + (j+1) * cx);
						cube.Front = UINT(box.Front + k * cz);
						cube.Back = UINT(box.Front + (k+1) * cz);

						cubes.push_back(cube);
					}
		}

		boxes = cubes;

		// decrement left iteration and iterate...
		--m_nIterations;
		MengerSpongeRecursive();
	}
	else
	{
		int idx = 0;
		std::list<BOX>::iterator it;
		for( it = boxes.begin(); it != boxes.end(); it++)
		{
			BOX box = *it;
			for(UINT z=box.Front;z<box.Back;z++)
				for(UINT y=box.Bottom;y<box.Top;y++)
					for(UINT x=box.Left;x<box.Right;x++)
					{
						if( z < m_iDepth && y < m_iHeight && x < m_iWidth )
							GetVoxelAt(x,y,z) = MAXDENSITY;
					}
					idx++;
		}

		boxes.clear();
	}
}