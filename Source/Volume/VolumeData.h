#ifndef _VOLUME_DATA_H_
#define _VOLUME_DATA_H_

#include "../Math/TransferFunction.h"
#include "../Math/Vector3D.h"
#include "../Math/types.h"

#define MINDENSITY 0
#define MAXDENSITY 255
#define VOLUME_SIZE (m_iWidth*m_iHeight*m_iDepth)
#define _I(x, y, z) ((z)*m_iWidth*m_iHeight + (y)*m_iWidth + (x))
#define clamp(val, vmin, vmax) min((vmax), max((vmin), (val)))

using namespace vdb::types;
using namespace vdb::math;

class CVolumeData
{
public:
	CVolumeData(void);
	CVolumeData					( const CVolumeData& v );
	CVolumeData					( UINT Width, UINT Height, UINT Depth );
	virtual ~CVolumeData(void);

	virtual void Init			( );
	virtual void Release		( );
	void ClearVoxelData			( ) { memset(m_pVoxelArray, 0, GetVoxelCount() ); }
	long GetVoxelCount			( ) const { return m_iWidth * m_iHeight * m_iDepth; }
	voxel& GetVoxelAt			( UINT x, UINT y, UINT z ){ return m_pVoxelArray[_I(x, y, z)]; }
	voxel GetVoxelValueAt		( UINT x, UINT y, UINT z ) const { return m_pVoxelArray[_I(x, y, z)]; }


public:
	HRESULT LoadFromFile		( LPCWSTR wszHDRFileName );


protected:
	HRESULT SaveRawToFile		( LPCWSTR wszFileName );
	void ComputeGradients		( );
	HRESULT SaveGradients		( LPCWSTR wszGRDFileName );
	HRESULT LoadGradients		( LPCWSTR wszGRDFileName );
	float SampleVolume			( UINT x, UINT y, UINT z ) const { return float(m_pVoxelArray[_I(x,y,z)])/255.f; }
	sVector3D<float> SampleNxNxN		( sVector3D<float> *gradients, UINT x, UINT y, UINT z, UINT n );


public:
	// operators only usable on volumes of the same size
	CVolumeData operator+(const CVolumeData &v);
	CVolumeData operator*(const CVolumeData &v);
	CVolumeData operator-(const CVolumeData &v);
	CVolumeData operator~();


protected:
	int							m_nColors;
	sTransferControlPoint*		m_pColorKnots;
	int							m_nAlpha;
	sTransferControlPoint*		m_pAlphaKnots;

	unsigned char				*m_pVoxelArray;
	sVector3D<float>			*m_pGradients;
	COLOR_ARGB					m_pTransferArray[256];

public:
	UINT						m_iWidth;
	UINT						m_iHeight;
	UINT						m_iDepth;

};

#endif // _VOLUME_DATA_H_