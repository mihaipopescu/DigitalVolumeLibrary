//-----------------------------------------------------------------------------
// File: CObject.h
//
// Desc: This file houses the various object / mesh related classes.
//
// Copyright (c) 1997-2002 Adam Hoult & Gary Simmons. All rights reserved.
// Modified by Mihai Popescu
//-----------------------------------------------------------------------------

#ifndef _COBJECT_H_
#define _COBJECT_H_

//-----------------------------------------------------------------------------
// CObject Specific Includes
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "d3dx9.h"

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CVertex (Class)
// Desc : Vertex struct used to construct & store vertex components.
//-----------------------------------------------------------------------------
class CVertex
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
    CVertex( float fX = 0.0f, float fY = 0.0f, float fZ = 0.0f, float fU = 0.0f, float fV = 0.0f, float fT = 0.0f) 
        { x = fX; y = fY; z = fZ; u = fU; v = fV; t = fT; }
    
    //-------------------------------------------------------------------------
    // Public Variables for This Class
    //-------------------------------------------------------------------------
    float       x;          // Vertex Position X Component
    float       y;          // Vertex Position Y Component
    float       z;          // Vertex Position Z Component
    
	float u, v, t;

	enum FVF
    {
        FVF_Flags = D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0)
    };
};

//-----------------------------------------------------------------------------
// Name : CScreenVertexTex (struct)
// Desc : Vertex struct used for screen 2D rendering with texture
//-----------------------------------------------------------------------------
struct sScreenVertexTex
{
	sScreenVertexTex( float fx = 0.f, float fy = 0.f, float fu = 0.f, float fv = 0.f )
	{ x = fx; y = fy; z = 0.5f; rhw = 1.0f; u = fu; v = fv; }

	float x, y, z, rhw;
	float u, v;

	enum FVF
	{
		FVF_FLAGS = D3DFVF_XYZRHW | D3DFVF_TEX1
	};
};

//-----------------------------------------------------------------------------
// Name : CScreenVertex (struct)
// Desc : Vertex struct used for screen 2D rendering with texture
//-----------------------------------------------------------------------------
struct sScreenVertex
{
	float x, y, z, rhw;
	DWORD dwColor;

	enum FVF
	{
		FVF_FLAGS = D3DFVF_XYZRHW | D3DFVF_DIFFUSE
	};
};

//-----------------------------------------------------------------------------
// Name : CMesh (Class)
// Desc : Basic mesh class used to store individual mesh data.
//-----------------------------------------------------------------------------
class CMesh
{
public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors for This Class.
	//-------------------------------------------------------------------------
             CMesh( ULONG VertexCount, ULONG IndexCount );
	         CMesh();
	virtual ~CMesh();
	
	ULONG	AddRef();

	//-------------------------------------------------------------------------
	// Public Functions for This Class
	//-------------------------------------------------------------------------
    long        AddVertex    ( ULONG Count = 1 );
    long        AddIndex     ( ULONG Count = 1 );
    HRESULT     BuildBuffers ( LPDIRECT3DDEVICE9 pD3DDevice, bool HardwareTnL, bool ReleaseOriginals = true);

    //-------------------------------------------------------------------------
	// Public Variables for This Class
	//-------------------------------------------------------------------------
	ULONG					m_nRefCount;		// Number of references to this mesh
    ULONG                   m_nVertexCount;     // Number of vertices stored
    CVertex                *m_pVertex;          // Simple temporary vertex array.
    ULONG                   m_nIndexCount;      // Number of indices stored
    USHORT                 *m_pIndex;           // Simple temporary index array
    LPDIRECT3DVERTEXBUFFER9 m_pVertexBuffer;    // Vertex Buffer to be Rendered
    LPDIRECT3DINDEXBUFFER9  m_pIndexBuffer;     // Index Buffer to be Rendered

    D3DXVECTOR3             m_BoundsMin;        // Bounding box minimum extents
    D3DXVECTOR3             m_BoundsMax;        // Bounding box maximum extents

};


class CRenderer
{
public:
	CRenderer() { m_pD3DDevice = NULL; }

	virtual HRESULT OnCreateDevice	( LPDIRECT3DDEVICE9 pD3DDevice );
	virtual HRESULT OnResetDevice	( LPDIRECT3DDEVICE9 pD3DDevice );
	virtual void OnLostDevice		( );
	virtual void Render				( ) = 0;

protected:
	LPDIRECT3DDEVICE9				m_pD3DDevice;
};



template <class T>
class CSingleton
{
public:
	static T* GetInstance() { return m_pInstance ? m_pInstance : m_pInstance = new T(); }
	static void DestroyInstance() { SAFE_DELETE( m_pInstance ); }
	static T* m_pInstance;
};


typedef unsigned int HSHADER;
#define HSHADER_INVALID -1


//-----------------------------------------------------------------------------
// Name : CObject (Abstract Class)
// Desc : Abstract rendering object
//-----------------------------------------------------------------------------
class CObject : public CRenderer
{
public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors for This Class.
	//-------------------------------------------------------------------------
     CObject( CMesh * pMesh );
	 CObject();
	 virtual ~CObject();

	//-------------------------------------------------------------------------
	// Public Variables for This Class
	//-------------------------------------------------------------------------
    D3DXMATRIX  m_mtxWorld;             // World transformation matrix
	D3DXMATRIX  m_mtxTexture;			// Texture transformation matrix
    CMesh *		m_pMesh;                // Object Mesh

public:
	virtual void Release			( );
	virtual HRESULT BeginRender		( );
	virtual HRESULT EndRender		( );
	virtual HRESULT RenderObject	( );

public:
	HSHADER							m_hShader;

private:
	DWORD							m_dwOldFVF;
};


#endif // !_COBJECT_H_