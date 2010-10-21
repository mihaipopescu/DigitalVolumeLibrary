#include "stdafx.h"
#include "../Math/types.h"
//-----------------------------------------------------------------------------
// File: CObject.cpp
//
// Desc: This file houses the various object / mesh related classes.
//
// Copyright (c) 1997-2002 Adam Hoult & Gary Simmons. All rights reserved.
// Modified by Mihai Popescu
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CObject Specific Includes
//-----------------------------------------------------------------------------
#include "CObject.h"
#include "ShaderManager.h"

template <class T> T* CSingleton<T>::m_pInstance = NULL;

//-----------------------------------------------------------------------------
// Name : CObject () (Constructor)
// Desc : CObject Class Constructor
//-----------------------------------------------------------------------------
CObject::CObject()
{
	// Reset / Clear all required values
    m_pMesh = NULL;
	m_hShader = HSHADER_INVALID;
    D3DXMatrixIdentity( &m_mtxWorld );
	D3DXMatrixIdentity( &m_mtxTexture );
}

//-----------------------------------------------------------------------------
// Name : CObject () (Alternate Constructor)
// Desc : CObject Class Constructor, sets the internal mesh object
//-----------------------------------------------------------------------------
CObject::CObject( CMesh * pMesh )
{
	// Reset / Clear all required values
    D3DXMatrixIdentity( &m_mtxWorld );
	D3DXMatrixIdentity( &m_mtxTexture );

	// The mesh was referenced
	pMesh->AddRef();

    // Set Mesh
    m_pMesh = pMesh;
}

CObject::~CObject( )
{
	Release();
}

HRESULT CRenderer::OnCreateDevice( LPDIRECT3DDEVICE9 pD3DDevice )
{
	m_pD3DDevice = pD3DDevice;
	return D3D_OK;
}

HRESULT CRenderer::OnResetDevice( LPDIRECT3DDEVICE9 pD3DDevice )
{
	m_pD3DDevice = pD3DDevice;
	return D3D_OK;
}

void CRenderer::OnLostDevice()
{
	m_pD3DDevice = NULL;
}

void CObject::Release()
{
	m_pD3DDevice = NULL;
	SAFE_DELETE( m_pMesh );
}

HRESULT CObject::BeginRender()
{
	HRESULT hr = D3D_OK;
	if( !m_pD3DDevice || !m_pMesh ) return E_FAIL;

	// transforms for fixed function pipeline
	m_pD3DDevice->SetTransform( D3DTS_WORLD, &m_mtxWorld );
	m_pD3DDevice->SetTransform( D3DTS_TEXTURE0, &m_mtxTexture );

	m_pD3DDevice->GetFVF( &m_dwOldFVF );
	m_pD3DDevice->SetFVF( CVertex::FVF_Flags );

	V_RETURN( m_pD3DDevice->SetStreamSource( 0, m_pMesh->m_pVertexBuffer, 0, sizeof(CVertex) ) );
	V_RETURN( m_pD3DDevice->SetIndices( m_pMesh->m_pIndexBuffer ) );

	return hr;
}

HRESULT CObject::EndRender()
{
	HRESULT hr = D3D_OK;

	if( !m_pD3DDevice ) return E_FAIL;

	m_pD3DDevice->SetFVF( m_dwOldFVF );

	return hr;
}

HRESULT CObject::RenderObject()
{
	HRESULT hr = D3D_OK;

	CShaderManager::GetInstance()->SetObject( this );
	V_RETURN( BeginRender() );
	CShaderManager::GetInstance()->Render();
	V_RETURN( EndRender() );

	return hr;
}


//-----------------------------------------------------------------------------
// Name : CMesh () (Constructor)
// Desc : CMesh Class Constructor
//-----------------------------------------------------------------------------
CMesh::CMesh()
{
	// Reset / Clear all required values
    m_pVertex       = NULL;
    m_pIndex        = NULL;
    m_nVertexCount  = 0;
    m_nIndexCount   = 0;
	m_nRefCount		= 1;

    m_pVertexBuffer = NULL;
    m_pIndexBuffer  = NULL;
}

//-----------------------------------------------------------------------------
// Name : CMesh () (Alternate Constructor)
// Desc : CMesh Class Constructor, adds specified number of vertices / indices
//-----------------------------------------------------------------------------
CMesh::CMesh( ULONG VertexCount, ULONG IndexCount )
{
	// Reset / Clear all required values
    m_pVertex       = NULL;
    m_pIndex        = NULL;
    m_nVertexCount  = 0;
    m_nIndexCount   = 0;
	m_nRefCount		= 1;

    m_pVertexBuffer = NULL;
    m_pIndexBuffer  = NULL;

    // Add Vertices & indices if required
    if ( VertexCount > 0 ) AddVertex( VertexCount );
    if ( IndexCount  > 0 ) AddIndex( IndexCount );
}

//-----------------------------------------------------------------------------
// Name : ~CMesh () (Destructor)
// Desc : CMesh Class Destructor
//-----------------------------------------------------------------------------
CMesh::~CMesh()
{
	// release mesh only if RefCount is 0 (there is no other reference to our mesh)
	if ( --m_nRefCount ) return;

	// Release our mesh components
    if ( m_pVertex ) delete []m_pVertex;
    if ( m_pIndex  ) delete []m_pIndex;
    
    if ( m_pVertexBuffer ) m_pVertexBuffer->Release();
    if ( m_pIndexBuffer  ) m_pIndexBuffer->Release();

    // Clear variables
    m_pVertex       = NULL;
    m_pIndex        = NULL;
    m_nVertexCount  = 0;
    m_nIndexCount   = 0;

    m_pVertexBuffer = NULL;
    m_pIndexBuffer  = NULL;
}

__forceinline ULONG CMesh::AddRef()
{
	return ++m_nRefCount;
}

//-----------------------------------------------------------------------------
// Name : AddVertex()
// Desc : Adds a vertex, or multiple vertices, to this mesh.
// Note : Returns the index for the first vertex added, or -1 on failure.
//-----------------------------------------------------------------------------
long CMesh::AddVertex( ULONG Count )
{
    CVertex * pVertexBuffer = NULL;
    
    // Allocate new resized array
    if (!( pVertexBuffer = new CVertex[ m_nVertexCount + Count ] )) return -1;

    // Existing Data?
    if ( m_pVertex )
    {
        // Copy old data into new buffer
        memcpy( pVertexBuffer, m_pVertex, m_nVertexCount * sizeof(CVertex) );

        // Release old buffer
        delete []m_pVertex;

    } // End if

    // Store pointer for new buffer
    m_pVertex = pVertexBuffer;
    m_nVertexCount += Count;

    // Return first vertex
    return m_nVertexCount - Count;
}

//-----------------------------------------------------------------------------
// Name : AddIndex()
// Desc : Adds an index, or multiple indices, to this mesh.
// Note : Returns the index for the first vertex index added, or -1 on failure.
//-----------------------------------------------------------------------------
long CMesh::AddIndex( ULONG Count )
{
    USHORT * pIndexBuffer = NULL;
    
    // Allocate new resized array
    if (!( pIndexBuffer = new USHORT[ m_nIndexCount + Count ] )) return -1;

    // Existing Data?
    if ( m_pIndex )
    {
        // Copy old data into new buffer
        memcpy( pIndexBuffer, m_pIndex, m_nIndexCount * sizeof(USHORT) );

        // Release old buffer
        delete []m_pIndex;

    } // End if

    // Store pointer for new buffer
    m_pIndex = pIndexBuffer;
    m_nIndexCount += Count;

    // Return first index
    return m_nIndexCount - Count;
}

//-----------------------------------------------------------------------------
// Name : BuildBuffers()
// Desc : Instructs the mesh to build a set of index / vertex buffers from the
//        data currently stored within the mesh object.
// Note : By passing in true to the 'ReleaseOriginals' parameter, the original
//        buffers will be destroyed (including vertex / index counts being
//        reset) so make sure you duplicate any data you may require.
//-----------------------------------------------------------------------------
HRESULT CMesh::BuildBuffers( LPDIRECT3DDEVICE9 pD3DDevice, bool HardwareTnL, bool ReleaseOriginals)
{
    HRESULT     hRet    = S_OK;
    CVertex    *pVertex = NULL;
    USHORT     *pIndex  = NULL;
    ULONG       ulUsage = D3DUSAGE_WRITEONLY;

    // Should we use software vertex processing ?
    if ( !HardwareTnL ) ulUsage |= D3DUSAGE_SOFTWAREPROCESSING;

    // Release any previously allocated vertex / index buffers
	SAFE_RELEASE( m_pVertexBuffer );
	
	SAFE_RELEASE( m_pIndexBuffer );

    // Create our vertex buffer
	if( m_pVertexBuffer == NULL )
	{
		hRet = pD3DDevice->CreateVertexBuffer( sizeof(CVertex) * m_nVertexCount, ulUsage, CVertex::FVF_Flags,
												 D3DPOOL_MANAGED, &m_pVertexBuffer, NULL );
		if ( FAILED( hRet ) ) return hRet;

		// Lock the vertex buffer ready to fill data
		hRet = m_pVertexBuffer->Lock( 0, sizeof(CVertex) * m_nVertexCount, (void**)&pVertex, 0 );
		if ( FAILED( hRet ) ) return hRet;

		// Copy over the vertex data
		memcpy( pVertex, m_pVertex, sizeof(CVertex) * m_nVertexCount );

		// We are finished with the vertex buffer
		m_pVertexBuffer->Unlock();
	}

    // Create our index buffer
	if( m_pIndexBuffer == NULL )
	{
		hRet = pD3DDevice->CreateIndexBuffer( sizeof(USHORT) * m_nIndexCount, ulUsage, D3DFMT_INDEX16,
												D3DPOOL_MANAGED, &m_pIndexBuffer, NULL );
		if ( FAILED( hRet ) ) return hRet;

		// Lock the index buffer ready to fill data
		hRet = m_pIndexBuffer->Lock( 0, sizeof(USHORT) * m_nIndexCount, (void**)&pIndex, 0 );
		if ( FAILED( hRet ) ) return hRet;

		// Copy over the index data
		memcpy( pIndex, m_pIndex, sizeof(USHORT) * m_nIndexCount );

		// We are finished with the indexbuffer
		m_pIndexBuffer->Unlock();
	}
    // Calculate the mesh bounding box extents
    m_BoundsMin = D3DXVECTOR3( 999999.0f, 999999.0f, 999999.0f );
    m_BoundsMax = D3DXVECTOR3( -999999.0f, -999999.0f, -999999.0f );
    for ( ULONG i = 0; i < m_nVertexCount; ++i )
    {
        D3DXVECTOR3 * Pos = (D3DXVECTOR3*)&m_pVertex[i];
        if ( Pos->x < m_BoundsMin.x ) m_BoundsMin.x = Pos->x;
        if ( Pos->y < m_BoundsMin.y ) m_BoundsMin.y = Pos->y;
        if ( Pos->z < m_BoundsMin.z ) m_BoundsMin.z = Pos->z;
        if ( Pos->x > m_BoundsMax.x ) m_BoundsMax.x = Pos->x;
        if ( Pos->y > m_BoundsMax.y ) m_BoundsMax.y = Pos->y;
        if ( Pos->z > m_BoundsMax.z ) m_BoundsMax.z = Pos->z;
    
    } // Next Vertex
    
    // Release old data if requested
    if ( ReleaseOriginals )
    {
        // Release our mesh components
        if ( m_pVertex ) delete []m_pVertex;
        if ( m_pIndex  ) delete []m_pIndex;

        // Clear variables
        m_pVertex       = NULL;
        m_pIndex        = NULL;
        //m_nVertexCount  = 0;
        //m_nIndexCount   = 0;

    } // End if ReleaseOriginals

    return S_OK;
}
