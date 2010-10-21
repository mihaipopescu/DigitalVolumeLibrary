#include "StdAfx.h"
#include "HoneycombVolume.h"


#define IN_RANGE_X(x,rx) ( (rx = (x)) >= 0 && (x)<m_iWidth )
#define IN_RANGE_Y(y,ry) ( (ry = (y)) >= 0 && (y)<m_iHeight )


CHoneycombVolume::CHoneycombVolume(void)
{
}

CHoneycombVolume::~CHoneycombVolume(void)
{
}


void CHoneycombVolume::Create(UINT iSpan, UCHAR opacity, int xx, int yy, int zz)
{
	ClearVoxelData();

	//The span of the cell must be an even number to produce good results!
	if( iSpan % 2 == 0 )
		return;

	POINT p;

	// for each z slice
	for(UINT z=0; z<m_iDepth; z++)
	{
		p.x = 0;
		p.y = (iSpan + 1) >> 1;

		for(UINT x=iSpan, cx=0; x<m_iWidth; x += (3*iSpan - 1)>>1, p.x++, cx++)
		{
			for(UINT y=p.y, cy=0; y<m_iHeight; y += iSpan + 1, cy++)
			{
				int a = iSpan;
				bool bWireframe = cx!=xx || cy!=yy || (z/iSpan)!=zz;

				do
				{
					int s = (a-1)>>1;
					for(int ud=-s;ud<=s;ud++)
					{
						UINT rx, ry;
						if( bWireframe && (z % iSpan) && (abs(ud)!=s) )
							continue;

						unsigned char o = opacity;

						if(IN_RANGE_X(x + ud, rx))
						{
							if(IN_RANGE_Y(y + s + 1, ry))
								GetVoxelAt(rx, ry, z) = o;

							if(IN_RANGE_Y(y - s - 1, ry))
								GetVoxelAt(rx, ry, z) = o;
						}
						if(IN_RANGE_Y(y + ud, ry))
						{
							if(IN_RANGE_X(x + (abs(ud) - a), rx))
								GetVoxelAt(rx, ry, z) = o;
		
							if(IN_RANGE_X(x - (abs(ud) - a), rx))
								GetVoxelAt(rx, ry, z) = o;
						}
					}
					a--;
				} while( !bWireframe && (a>=0)   );
			}
			
			if(p.x % 2)
				p.y += (iSpan + 1)>>1;
			else
				p.y -= (iSpan + 1)>>1;
		}
	}
}