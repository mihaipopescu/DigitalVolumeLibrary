#pragma once
#include "WTypes.h"
#include "math.h"


#ifndef V
#define V(x)           { hr = (x); }
#endif
#ifndef V_RETURN
#define V_RETURN(x)    { hr = (x); if( FAILED(hr) ) { return hr; } }
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if (p) { delete (p);     (p)=NULL; } }
#endif    
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p);   (p)=NULL; } }
#endif    
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

#define PI 3.141592654f

namespace vdb
{
	namespace types
	{
		typedef unsigned char voxel;

		union COLOR_ARGB
		{
			COLOR_ARGB(byte a=0, byte r=0, byte g=0, byte b=0) : A(a), R(r), G(g), B(b) {}
			unsigned long ARGB;
			struct
			{
				byte A;
				byte R;
				byte G;
				byte B;
			};
		};

		struct BOX {
			UINT Left;
			UINT Top;
			UINT Right;
			UINT Bottom;
			UINT Front;
			UINT Back;
		};

	}


}