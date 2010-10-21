#include "StdAfx.h"
#include "VolumeData.h"
#include "Shlwapi.h"
#include "stdio.h"


CVolumeData::CVolumeData(void)
{
	Init();
}

CVolumeData::~CVolumeData(void)
{
	Release();
}

void CVolumeData::Init( )
{
	m_iWidth = 0;
	m_iHeight = 0;
	m_iDepth = 0;

	m_pVoxelArray = NULL;

	m_nColors = 0;
	m_pColorKnots = NULL;
	m_nAlpha = 0;
	m_pAlphaKnots = NULL;

	m_pGradients = NULL;
}

void CVolumeData::Release( )
{
	SAFE_DELETE_ARRAY( m_pVoxelArray );
	SAFE_DELETE_ARRAY( m_pGradients );
	SAFE_DELETE_ARRAY( m_pColorKnots );
	SAFE_DELETE_ARRAY( m_pAlphaKnots );

	CVolumeData::Init();
}

CVolumeData::CVolumeData(const CVolumeData &volume)
{
	Init();
	m_iWidth = volume.m_iWidth;
	m_iHeight = volume.m_iHeight;
	m_iDepth = volume.m_iDepth;
	unsigned int size = VOLUME_SIZE;
	m_pVoxelArray = new voxel[size];
	memcpy(m_pVoxelArray, volume.m_pVoxelArray, size * sizeof(voxel));
	m_nAlpha = volume.m_nAlpha;
	if(m_nAlpha)
	{
		m_pAlphaKnots = new sTransferControlPoint[volume.m_nAlpha];
		memcpy(m_pAlphaKnots, volume.m_pAlphaKnots, sizeof(sTransferControlPoint) * volume.m_nAlpha);
	}
	m_nColors = volume.m_nColors;
	if(m_nColors)
	{	
		m_pColorKnots = new sTransferControlPoint[volume.m_nColors];
		memcpy(m_pColorKnots, volume.m_pColorKnots, sizeof(sTransferControlPoint) * volume.m_nColors);
	}
}

CVolumeData::CVolumeData( UINT iWidth, UINT iHeight, UINT iDepth )
{
	Init();
	m_iWidth = iWidth;
	m_iHeight = iHeight;
	m_iDepth = iDepth;

	m_pVoxelArray = new voxel[VOLUME_SIZE];
}

HRESULT CVolumeData::SaveRawToFile( LPCWSTR wszFileName )
{
	FILE *fout;
	if( ! ( fout = _wfopen(wszFileName, L"wb") ) )
		return E_ACCESSDENIED;

	if(fwrite(m_pVoxelArray, sizeof(voxel), VOLUME_SIZE, fout) != VOLUME_SIZE)
		return E_FAIL;

	if( fclose(fout) != 0)
		return E_ACCESSDENIED;

	return S_OK;
}

HRESULT CVolumeData::LoadFromFile( LPCWSTR wszHDRFileName )
{
	HRESULT hr = S_OK;
	FILE* fin;
	int i;

	// read header file
	if ( ! (fin = _wfopen(wszHDRFileName, L"rt")) )
		return E_ACCESSDENIED;

	int iWidth, iHeight, iDepth;
	if( fwscanf(fin, L"%d %d %d", &iWidth, &iHeight, &iDepth) != 3)
		return E_FAIL;

	if( fwscanf(fin, L"%d", &m_nColors) != 1)
		return E_FAIL;

	if( m_nColors < 256 )
	{
		m_pColorKnots = new sTransferControlPoint[m_nColors];
		for(i=0;i<m_nColors;i++)
		{
			m_pColorKnots[i].Color.w = 1.f;
			if(fwscanf(fin, L"%f%f%f%d", &m_pColorKnots[i].Color.x, &m_pColorKnots[i].Color.y, &m_pColorKnots[i].Color.z, &m_pColorKnots[i].IsoValue) != 4)
				return E_FAIL;
		}

		if( fwscanf(fin, L"%d", &m_nAlpha) != 1)
			return E_FAIL;

		m_pAlphaKnots = new sTransferControlPoint[m_nAlpha];
		for(i=0;i<m_nAlpha;i++)
		{
			if(fwscanf(fin, L"%f%d", &m_pAlphaKnots[i].Color.w, &m_pAlphaKnots[i].IsoValue) != 2)
				return E_FAIL;
		}
	}
	else
	{
		m_pColorKnots = new sTransferControlPoint[m_nColors];
		for(i=0;i<256;i++)
		{
			int r, g, b;
			if(fwscanf(fin, L"%d%d%d", &r, &g, &b ) != 3)
				return E_FAIL;

			m_pColorKnots[i].Color.x = (float)r / 255.f;
			m_pColorKnots[i].Color.y = (float)g / 255.f;
			m_pColorKnots[i].Color.z = (float)b / 255.f;
			m_pColorKnots[i].Color.w = (float)i / 255.f;
			m_pColorKnots[i].IsoValue = i;
		}
	}


	if( fclose(fin) !=  0)
		return E_ACCESSDENIED;

	long lSize = iDepth * iWidth * iHeight;
	m_iWidth = iWidth;
	m_iHeight = iHeight;
	m_iDepth = iDepth;

	m_pVoxelArray = new voxel[lSize];

	// create raw filename from hdr filename by changing extension
	// NOTE: The raw file must be in the same folder as the header
	WCHAR wszRawFile[MAX_PATH];
	wcscpy(wszRawFile, wszHDRFileName);
	PathRemoveExtension(wszRawFile);
	wcscat(wszRawFile, L".raw");

	// read raw data file
	if( !( fin = _wfopen(wszRawFile, L"rb") ) )
		return E_ACCESSDENIED;

	if( fread(m_pVoxelArray, lSize * sizeof(voxel), sizeof(voxel), fin) != lSize * sizeof(voxel) )
		return E_FAIL;

	if( fclose(fin) !=  0)
		return E_ACCESSDENIED;

	// create grd filename from hdr filename by changing extension
	// NOTE: The grd file must be in the same folder as the header
	wcscpy(wszRawFile, wszHDRFileName);
	PathRemoveExtension(wszRawFile);
	wcscat(wszRawFile, L".grd");

	m_pGradients = new sVector3D<float>[lSize];
	if(FAILED(LoadGradients( wszRawFile )))
	{
		ComputeGradients( );
		V ( SaveGradients( wszRawFile ) );
	}

	return hr;
}

void CVolumeData::ComputeGradients( )
{
	HRESULT hr = S_OK;
	sVector3D<float> *grads = m_pGradients;
	sVector3D<float> v1, v2;

	// Generates gradients central differences scheme
	int index = 0;
	{
		for (UINT z = 0; z < m_iDepth; z++)
		{
			for (UINT y = 0; y < m_iHeight; y++)
			{
				for (UINT x = 0; x < m_iWidth; x++)
				{
					v1.x = x > 0			? SampleVolume(x - 1, y, z) : 0.f;
					v2.x = x < m_iWidth-1	? SampleVolume(x + 1, y, z) : 0.f;
					v1.y = y > 0			? SampleVolume(x, y - 1, z) : 0.f;
					v2.y = y < m_iHeight-1	? SampleVolume(x, y + 1, z) : 0.f;
					v1.z = z > 0			? SampleVolume(x, y, z - 1) : 0.f;
					v2.z = z < m_iDepth-1	? SampleVolume(x, y, z + 1) : 0.f;

					v2 -= v1;
					grads[index++] = v2 * 0.5f;
				}
			}
		}
	}

	// Applies an NxNxN filter to the gradients. 
	index = 0;
	{
		for (UINT z = 0; z < m_iDepth; z++)
		{
			for (UINT y = 0; y < m_iHeight; y++)
			{
				for (UINT x = 0; x < m_iWidth; x++)
				{
					grads[index++] = SampleNxNxN(grads, x, y, z, 3);
				}
			}
		}
	}

}

sVector3D<float> CVolumeData::SampleNxNxN(sVector3D<float> *gradients, UINT x, UINT y, UINT z, UINT n)
{
	n = (n - 1) / 2;

	sVector3D<float> average(0.f, 0.f, 0.f);
	int num = 0;

	for (UINT k = z - n; k <= z + n; k++)
	{
		for (UINT j = y - n; j <= y + n; j++)
		{
			for (UINT i = x - n; i <= x + n; i++)
			{
				if ((x >= 0 && x < m_iWidth) &&
					(y >= 0 && y < m_iHeight) &&
					(z >= 0 && z < m_iDepth))
				{
					average += gradients[_I(x,y,z)];
					num++;
				}
			}
		}
	}

	average.x /= (float)num;
	average.y /= (float)num;
	average.z /= (float)num;

	if (average.x != 0.0f && average.y != 0.0f && average.z != 0.0f)
		average.Normalize();

	return average;
}

HRESULT CVolumeData::SaveGradients(LPCWSTR wszGRDFileName)
{
	FILE* fout;

	// read header file
	if ( ! (fout = _wfopen(wszGRDFileName, L"wb")) )
		return E_ACCESSDENIED;

	long lSize = GetVoxelCount();

	if( fwrite(m_pGradients, sizeof(sVector3D<float>), lSize, fout) != lSize )
		return E_FAIL;

	if( fclose(fout) !=  0)
		return E_ACCESSDENIED;

	return S_OK;
}

HRESULT CVolumeData::LoadGradients(LPCWSTR wszGRDFileName)
{
	FILE* fin;

	// read header file
	if ( !(fin = _wfopen(wszGRDFileName, L"rb")) )
		return E_ACCESSDENIED;

	long lSize = GetVoxelCount();

	if( fread(m_pGradients, lSize * sizeof(sVector3D<float>), sizeof(sVector3D<float>), fin) != lSize )
		return E_FAIL;

	if( fclose(fin) !=  0)
		return E_ACCESSDENIED;

	return S_OK;
}


CVolumeData CVolumeData::operator+(const CVolumeData &v)
{
	CVolumeData result = *this;			
	unsigned int size = m_iWidth * m_iHeight * m_iDepth;
	for(register unsigned int i = 0; i < size; ++i)
		result.m_pVoxelArray[i] = clamp(m_pVoxelArray[i] + v.m_pVoxelArray[i], MINDENSITY, MAXDENSITY);

	return result;
}

CVolumeData CVolumeData::operator*(const CVolumeData &v)
{
	CVolumeData result = *this;			
	unsigned int size = m_iWidth * m_iHeight * m_iDepth;
	for(register unsigned int i = 0; i < size; ++i)
		result.m_pVoxelArray[i] = clamp((m_pVoxelArray[i] * v.m_pVoxelArray[i])*0.00390625f, MINDENSITY, MAXDENSITY);// 1/256 = 0.00390625

	return result;

}

CVolumeData CVolumeData::operator-(const CVolumeData &v)
{
	CVolumeData result = *this;			
	unsigned int size = m_iWidth * m_iHeight * m_iDepth;
	for(register unsigned int i = 0; i < size; ++i)
		result.m_pVoxelArray[i] = clamp(m_pVoxelArray[i] - v.m_pVoxelArray[i], MINDENSITY, MAXDENSITY);

	return result;

}

CVolumeData CVolumeData::operator~()
{
	CVolumeData result = *this;	

	unsigned int size = m_iWidth * m_iHeight * m_iDepth;
	for(register unsigned int i = 0; i < size; ++i)
		result.m_pVoxelArray[i] = clamp(MAXDENSITY - m_pVoxelArray[i], MINDENSITY, MAXDENSITY);

	return result;
}