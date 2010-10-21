#include "StdAfx.h"
#include "TransferFunction.h"

namespace vdb {
namespace math {

using namespace vdb::types;

cCubic::cCubic(sVector4D<float> *a, sVector4D<float> *b, sVector4D<float> *c, sVector4D<float> *d)
{
	this->a = *a;
	this->b = *b;
	this->c = *c;
	this->d = *d;
}

void cCubic::ComputeCubicSpline(int n, const sTransferControlPoint *v, cCubic *out)
{
	sVector4D<float> *gamma = new sVector4D<float>[n+1];
	sVector4D<float> *delta = new sVector4D<float>[n+1];
	sVector4D<float> *D     = new sVector4D<float>[n+1];
	int i;

    /* We need to solve the equation
     * taken from: http://mathworld.wolfram.com/CubicSpline.html
       [2 1       ] [D[0]]   [3(v[1] - v[0])  ]
       |1 4 1     | |D[1]|   |3(v[2] - v[0])  |
       |  1 4 1   | | .  | = |      .         |
       |    ..... | | .  |   |      .         |
       |     1 4 1| | .  |   |3(v[n] - v[n-2])|
       [       1 2] [D[n]]   [3(v[n] - v[n-1])]

	    by decomposing the matrix into upper triangular and lower matrices
       and then back sustitution.  See Spath "Spline Algorithms for Curves
       and Surfaces" pp 19--21. The D[i] are the derivatives at the knots.
	*/

	//this builds the coefficients of the left matrix
	gamma[0].x = 1.f / 2.f;
	gamma[0].y = 1.f / 2.f;
	gamma[0].z = 1.f / 2.f;
	gamma[0].w = 1.f / 2.f;
  
    for (i = 1; i < n; i++)
    {
		sVector4D<float> d(4.f, 4.f, 4.f, 4.f);
		d -= gamma[i-1];
		gamma[i] = sVector4D<float>(1.f/d.x, 1.f/d.y, 1.f/d.z, 1.f/d.w);
    }
	
	sVector4D<float> d(2.f, 2.f, 2.f, 2.f);
	d -= gamma[n-1];
	gamma[n] = sVector4D<float>(1.f/d.x, 1.f/d.y, 1.f/d.z, 1.f/d.w);

	delta[0] = (v[1].Color - v[0].Color) * 3;
	delta[0] = delta[0] * gamma[0];

	for (i = 1; i < n; i++)
    {
		delta[i] = (v[i+1].Color - v[i].Color) * 3 - delta[i-1];
		delta[i] = delta[i] * gamma[i];
    }
	delta[n] = (v[n].Color - v[n - 1].Color) * 3 - delta[n - 1];
	delta[n] = delta[n] * gamma[n];

    D[n] = delta[n];
    for (i = n - 1; i >= 0; i--)
    {
		D[i] = gamma[i] * D[i + 1];
		D[i] = delta[i] - D[i];
    }

    // now compute the coefficients of the cubics 
    for (i = 0; i < n; i++)
    {
		sVector4D<float> a = v[i].Color;
		sVector4D<float> b = D[i];
		sVector4D<float> c = (v[i + 1].Color - v[i].Color)*3 - D[i]*2 - D[i + 1];
		sVector4D<float> d = (v[i].Color - v[i + 1].Color)*2 + D[i] + D[i + 1];
        out[i] = cCubic(&a, &b, &c, &d);
    }

	delete [] gamma;
	delete [] delta;
	delete [] D;
}


void CreateTransferFunction_CubicSpline(int cn, sTransferControlPoint* vColorKnots, int an, sTransferControlPoint* vAlphaKnots, COLOR_ARGB *pTransferArray)
{
	//initialize the cubic spline for the transfer function
	sVector4D<float> transferFunc[256];
	
	cCubic *colorCubic = new cCubic[cn - 1];
	cCubic::ComputeCubicSpline(cn-1, vColorKnots, colorCubic);

	int i;
    int numTF = 0;
    for (i = 0; i < cn - 1; i++)
    {
        int steps = vColorKnots[i + 1].IsoValue - vColorKnots[i].IsoValue;

        for (int j = 0; j < steps; j++)
        {
            float k = (float)j / (float)steps;

            transferFunc[numTF++] = colorCubic[i].GetPointOnSpline(k);
        }
    }

	delete []colorCubic;

	cCubic *alphaCubic = new cCubic[an - 1];
	cCubic::ComputeCubicSpline(an-1, vAlphaKnots, alphaCubic);

    numTF = 0;
    for (i = 0; i < an - 1; i++)
    {
        int steps = vAlphaKnots[i + 1].IsoValue - vAlphaKnots[i].IsoValue;

        for (int j = 0; j < steps; j++)
        {
            float k = (float)j / (float)steps;

            transferFunc[numTF++].w = alphaCubic[i].GetPointOnSpline(k).w;
        }
    }

	delete []alphaCubic;

	for (i = 0; i < 256; i++)
	{
		sVector4D<float>& color = transferFunc[i] * 255.0f;

		if(color.w<0) color.w = 0.0f;

		pTransferArray[i] = COLOR_ARGB((byte)color.w, (byte)color.x, (byte)color.y, (byte)color.z );
	}
}



void CreateTransferFunction_Palette(sTransferControlPoint* vPaletteKnots, COLOR_ARGB *pTransferArray)
{
	sVector4D<float> transferFunc[256];

	for (int i = 0; i < 256; i++)
	{
		sVector4D<float>& color = vPaletteKnots[i].Color * 255.f;
		pTransferArray[i] = COLOR_ARGB((byte)color.w, (byte)color.x, (byte)color.y, (byte)color.z );
	}
}

}
}