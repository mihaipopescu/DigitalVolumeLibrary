#include "StdAfx.h"
#include "Volume.h"
#include "../Math/Vector3D.h"

CVolume::CVolume(void)
{
	CVolumeData::Init();
	Init();
}

CVolume::CVolume( UINT iWidth, UINT iHeight, UINT iDepth ) : CVolumeData( iWidth, iHeight, iDepth )
{
	Init();
}

CVolume::CVolume( const CVolumeData &v ) : CVolumeData( v )
{
	Init();
}

CVolume::~CVolume(void)
{
	Release();
}

void CVolume::Init()
{
	m_pVolumeTexture = NULL;
	m_pMesh = NULL;
	memset( m_vEdgeList, 0, sizeof(m_vEdgeList) );
	m_fIsoValue = 75.f;

	m_pD3DDevice = NULL;
	m_pTransferFunction = NULL;
	
	D3DXMatrixIdentity(&m_mtxView);
	D3DXMatrixIdentity(&m_mtxProj);
}

void CVolume::Release()
{
	CVolumeData::Release();
	SAFE_RELEASE( m_pVolumeTexture );
	SAFE_DELETE( m_pMesh );
	SAFE_RELEASE( m_pTransferFunction );

	Init();
}


HRESULT CVolume::UpdateVolumeTexture()
{
	HRESULT hr;

	// set volume texture data
	D3DLOCKED_BOX lb;

	V_RETURN(m_pVolumeTexture->LockBox(0, &lb, NULL, D3DLOCK_DISCARD));

	long i = 0;

	if(m_pGradients)
	{
		for (UINT z = 0; z < m_iDepth; z++)
		{
			BYTE *plane = static_cast<BYTE *>(lb.pBits) + z*lb.SlicePitch;

			for (UINT y = 0; y < m_iHeight; y++)
			{
				D3DCOLOR *scanline = reinterpret_cast<D3DCOLOR *>(plane + y*lb.RowPitch);
				for (UINT x = 0; x < m_iWidth; x++)
				{
					sVector3D<float> *v = &m_pGradients[_I(x,y,z)];
					scanline[x] = D3DCOLOR_ARGB( m_pVoxelArray[i], UCHAR(v->x * 255.f), UCHAR(v->y * 255.f), UCHAR(v->z * 255.f) );

					i++;
				}
			}
		}
	}
	else
	{
		for (UINT z = 0; z < m_iDepth; z++)
		{
			BYTE *plane = static_cast<BYTE *>(lb.pBits) + z*lb.SlicePitch;

			for (UINT y = 0; y < m_iHeight; y++)
			{
				D3DCOLOR *scanline = reinterpret_cast<D3DCOLOR *>(plane + y*lb.RowPitch);
				for (UINT x = 0; x < m_iWidth; x++)
				{
					scanline[x] = D3DCOLOR_ARGB( m_pVoxelArray[i], m_pVoxelArray[i], m_pVoxelArray[i], m_pVoxelArray[i] );

					i++;
				}
			}
		}
	}


	V_RETURN( m_pVolumeTexture->UnlockBox(0));

	return S_OK;
}

HRESULT CVolume::OnResetDevice( LPDIRECT3DDEVICE9 pD3DDevice )
{
	HRESULT hr;

	CObject::OnResetDevice( pD3DDevice );

	// create volume bounding box
	SAFE_DELETE( m_pMesh );
	m_pMesh = new CMesh;

	if( m_pMesh->AddVertex( 8 ) < 0 ) return E_FAIL;

	CVertex * pVertex = &m_pMesh->m_pVertex[0];

	*pVertex++ = CVertex( -0.5, -0.5, -0.5, 0, 1, 1 );
	*pVertex++ = CVertex( -0.5, +0.5, -0.5, 0, 0, 1 );
	*pVertex++ = CVertex( +0.5, +0.5, -0.5, 1, 0, 1 );
	*pVertex++ = CVertex( +0.5, -0.5, -0.5, 1, 1, 1 );

	*pVertex++ = CVertex( -0.5, -0.5, +0.5, 0, 1, 0 );
	*pVertex++ = CVertex( -0.5, +0.5, +0.5, 0, 0, 0 );
	*pVertex++ = CVertex( +0.5, +0.5, +0.5, 1, 0, 0 );
	*pVertex++ = CVertex( +0.5, -0.5, +0.5, 1, 1, 0 );


	//Add the indices as a strip (with one degenerate) ;)
	if ( m_pMesh->AddIndex( 36 ) < 0 ) return E_FAIL;

	USHORT idx[36] = { 0, 1, 2, 0, 2, 3, 7, 6, 5, 7, 5, 4, 4, 5, 1, 4, 1, 0, 3, 2, 6, 3, 6, 7, 1, 5, 6, 1, 6, 2, 4, 0, 3, 4, 3, 7 };
	memcpy( m_pMesh->m_pIndex, idx, 36 * sizeof(USHORT) );


	// Adding box edges
	int i=0;
	// edges for top & bottom squares
	for( ; i<8 ; ++i )
	{
		m_vEdgeList[2*i] = i;
		m_vEdgeList[2*i+1] = i+1;
		if( 0 == (i+1)%4 )
			m_vEdgeList[2*i+1] -= 4;
	}
	// edges for vertical lines
	for(int j=0; j<4; ++j, ++i)
	{
		m_vEdgeList[2*i] = j;
		m_vEdgeList[2*i+1] = j + 4;
	}
	

	V_RETURN( m_pMesh->BuildBuffers( m_pD3DDevice, true, false ) );
	
	// create transfer function texture
	if( !m_pColorKnots && !m_pAlphaKnots )
	{
		m_pAlphaKnots = new sTransferControlPoint[2];
		m_pAlphaKnots[0] = sTransferControlPoint(1.f, 0);
		m_pAlphaKnots[1] = sTransferControlPoint(0.f, 255);
		m_nAlpha = 2;
		m_pColorKnots = new sTransferControlPoint[2];
		m_pColorKnots[0] = sTransferControlPoint(0,0,0,0);
		m_pColorKnots[1] = sTransferControlPoint(1.f, 1.f, 1.f, 255);
		m_nColors = 2;
	}

	// create transfer function
	SAFE_RELEASE( m_pTransferFunction );

	if( m_pColorKnots && m_pAlphaKnots )
	{
		vdb::math::CreateTransferFunction_CubicSpline(m_nColors, m_pColorKnots, m_nAlpha, m_pAlphaKnots, m_pTransferArray);
	}
	else if( m_pColorKnots )
	{
		vdb::math::CreateTransferFunction_Palette(m_pColorKnots, m_pTransferArray);
	}

	V_RETURN( D3DXCreateTexture(pD3DDevice, 256, 1, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTransferFunction) );

	D3DLOCKED_RECT rect;
	V_RETURN( m_pTransferFunction->LockRect(0, &rect, NULL, D3DLOCK_DISCARD) );

	memcpy( rect.pBits, m_pTransferArray, 256 * sizeof(D3DCOLOR) );

	V_RETURN( m_pTransferFunction->UnlockRect(0) );


	// Create the volume texture
	SAFE_RELEASE( m_pVolumeTexture );
	
	if( m_iWidth && m_iHeight && m_iDepth )
	{
		V_RETURN( pD3DDevice->CreateVolumeTexture(m_iWidth, m_iHeight, m_iDepth, 1, D3DUSAGE_DYNAMIC, 
			D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pVolumeTexture, NULL) );

		V_RETURN( UpdateVolumeTexture() );
	}

	return D3D_OK;
}

void CVolume::OnLostDevice()
{
	CObject::OnLostDevice();
	SAFE_RELEASE( m_pTransferFunction );
}

void CVolume::Render()
{
	m_pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, 8, 0, 12 );
}

void CVolume::DrawTransferFunction(UINT left, UINT top, UINT right, UINT bottom)
{
	sScreenVertexTex vertices[4];
	
	vertices[0] = sScreenVertexTex( ( float )left,  ( float )top,    0, 1 );
	vertices[1] = sScreenVertexTex( ( float )right, ( float )top,    0, 0 );
	vertices[2] = sScreenVertexTex( ( float )right, ( float )bottom, 1, 0 );
	vertices[3] = sScreenVertexTex( ( float )left,  ( float )bottom, 1, 1 );
 

	DWORD fvf;
    m_pD3DDevice->GetFVF( &fvf );
	m_pD3DDevice->SetFVF( sScreenVertexTex::FVF_FLAGS );
	
	m_pD3DDevice->SetTexture( 0, m_pTransferFunction );
    
	m_pD3DDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
    m_pD3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 2, vertices, sizeof( sScreenVertexTex ) );

    // Restore the vertex decl
    m_pD3DDevice->SetFVF( fvf );
}