//-----------------------------------------------------------------------------
// File: CubicSpline.h
//
// Desc: Cubic spline is computed from a set of control points 
//		 and preforms cubic interpolation. Based on the natural cubic spline
//		 code from: http://www.cse.unsw.edu.au/~lambert/splines/source.html
//-----------------------------------------------------------------------------
#ifndef _CUBIC_SPLINE_H_
#define _CUBIC_SPLINE_H_

#include "types.h"
#include "Vector4D.h"

namespace vdl
{
	namespace math
	{
		struct sTransferControlPoint
		{
			sVector4D<float> Color;
			unsigned char IsoValue;

			sTransferControlPoint() : Color(sVector4D<float>(0.f, 0.f, 0.f, 0.f)), IsoValue(0) { }
			sTransferControlPoint(float r, float g, float b, unsigned char iso) : Color(sVector4D<float>(r,g,b,1.f)), IsoValue(iso) { }
			sTransferControlPoint(float r, float g, float b, float a, unsigned char iso) : Color(sVector4D<float>(r,g,b,a)), IsoValue(iso) { }
			sTransferControlPoint(float alpha, unsigned char iso) : Color(sVector4D<float>(0.f, 0.f, 0.f, alpha)), IsoValue(iso) { }
		};

		class cCubic
		{
		private:
			sVector4D<float> a, b, c, d; // a + b*s + c*s^2 +d*s^3 

		public:
			cCubic(){ }
			cCubic(sVector4D<float> *a, sVector4D<float> *b, sVector4D<float> *c, sVector4D<float> *d);

			sVector4D<float> GetPointOnSpline(float s) { return (((d * s) + c) * s + b) * s + a; }

			static void ComputeCubicSpline(int n, const sTransferControlPoint *v, cCubic *out);

		};

		void CreateTransferFunction_CubicSpline(int cn, sTransferControlPoint* vColorKnots, int an, sTransferControlPoint* vAlphaKnots, vdl::types::COLOR_ARGB *pTransferArray);
		void CreateTransferFunction_Palette(sTransferControlPoint* vPaletteKnots, vdl::types::COLOR_ARGB *pTransferArray);
	}
}

#endif // _CUBIC_SPLINE_H_