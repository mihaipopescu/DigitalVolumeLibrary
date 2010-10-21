#include "stdafx.h"
#include "ViewportManager.h"
#include "DXUT.h"


//char * CViewportManager::m_szViewportName[] = { "Top", "Front", "Left", "Perspective",};
char * CViewportManager::m_szViewportName[] = { "XOZ", "XOY", "YOZ", "XYZ" };

CViewportManager* CSingleton<CViewportManager>::m_pInstance = NULL;

CViewportManager::CViewportManager(void)
{
	memset( m_pViewports, 0x00, sizeof(sViewport) * EP_ENUMNO );
	m_pVolume = NULL;
}

CViewportManager::~CViewportManager(void)
{
	for(int i=EP_FIRST; i<EP_ENUMNO; ++i)
	{
		SAFE_RELEASE( m_pViewports[i].pTex );
		SAFE_RELEASE( m_pViewports[i].pRT );
		SAFE_DELETE( m_pViewports[i].pSlicer );
	}
}

void CViewportManager::Create( CVolume *pVolume, HSHADER hShader )
{
	m_pVolume = pVolume;
	m_hShader = hShader;
}

HRESULT CViewportManager::OnResetDevice( LPDIRECT3DDEVICE9 pD3DDevice )
{
	HRESULT hr = CRenderer::OnResetDevice( pD3DDevice );	

	D3DPRESENT_PARAMETERS pp = DXUTGetDeviceSettings().d3d9.pp;
	m_fScreenWidth = float(pp.BackBufferWidth);
	m_fScreenHeight = float(pp.BackBufferHeight);
	
	for( int i = EP_FIRST; i < EP_ENUMNO; ++i )
	{
		SAFE_RELEASE( m_pViewports[i].pTex );
		V_RETURN( D3DXCreateTexture(m_pD3DDevice, pp.BackBufferWidth/2, pp.BackBufferHeight/2, 1, D3DUSAGE_RENDERTARGET, pp.BackBufferFormat, D3DPOOL_DEFAULT, &m_pViewports[i].pTex) );

		SAFE_RELEASE( m_pViewports[i].pRT );
		m_pViewports[i].pTex->GetSurfaceLevel( 0, &m_pViewports[i].pRT );

		m_pViewports[i].szName = m_szViewportName[i];

		m_pViewports[i].pSlicer = new CVolumeSlicer( m_pVolume, (EProjection)i );
		m_pViewports[i].pSlicer->m_hShader = m_hShader;
		m_pViewports[i].pSlicer->OnResetDevice( pD3DDevice );
	}

	sViewport* pv = &m_pViewports[ EP_Top ];
	pv->pVertices[0] = sScreenVertexTex( 0.f, 0.f, 0.f, 0.f );
	pv->pVertices[1] = sScreenVertexTex( m_fScreenWidth/2, 0.f, 1.f, 0.f );
	pv->pVertices[2] = sScreenVertexTex( m_fScreenWidth/2, m_fScreenHeight/2, 1.f, 1.f );
	pv->pVertices[3] = sScreenVertexTex( 0.f, m_fScreenHeight/2, 0.f, 1.f );
	D3DXMatrixIdentity( &pv->mView );

	pv = &m_pViewports[ EP_Front ];
	pv->pVertices[0] = sScreenVertexTex( m_fScreenWidth/2, 0.f, 0.f, 0.f );
	pv->pVertices[1] = sScreenVertexTex( m_fScreenWidth, 0.f, 1.f, 0.f );
	pv->pVertices[2] = sScreenVertexTex( m_fScreenWidth, m_fScreenHeight/2, 1.f, 1.f );
	pv->pVertices[3] = sScreenVertexTex( m_fScreenWidth/2, m_fScreenHeight/2, 0.f, 1.f );
	D3DXMatrixIdentity( &pv->mView );

	pv = &m_pViewports[ EP_Left ];
	pv->pVertices[0] = sScreenVertexTex( 0.f, m_fScreenHeight/2, 0.f, 0.f );
	pv->pVertices[1] = sScreenVertexTex( m_fScreenWidth/2, m_fScreenHeight/2, 1.f, 0.f );
	pv->pVertices[2] = sScreenVertexTex( m_fScreenWidth/2, m_fScreenHeight, 1.f, 1.f );
	pv->pVertices[3] = sScreenVertexTex( 0.f, m_fScreenHeight, 0.f, 1.f );
	D3DXMatrixIdentity( &pv->mView );

	pv = &m_pViewports[ EP_Perspective ];
	pv->pVertices[0] = sScreenVertexTex( m_fScreenWidth/2, m_fScreenHeight/2, 0.f, 0.f );
	pv->pVertices[1] = sScreenVertexTex( m_fScreenWidth, m_fScreenHeight/2, 1.f, 0.f );
	pv->pVertices[2] = sScreenVertexTex( m_fScreenWidth, m_fScreenHeight, 1.f, 1.f );
	pv->pVertices[3] = sScreenVertexTex( m_fScreenWidth/2, m_fScreenHeight, 0.f, 1.f );
	D3DXMatrixIdentity( &pv->mView );

	return hr;
}

const CViewportManager::sViewport* CViewportManager::SetActiveViewport( EProjection viewport )
{
	m_pD3DDevice->SetRenderTarget( 0, m_pViewports[viewport].pRT );
	m_pD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, -1, 1.0f, 0 );
	return &m_pViewports[ viewport ];
}

const CViewportManager::sViewport* CViewportManager::GetViewport( EProjection viewport )
{
	return &m_pViewports[ viewport ];
}

void CViewportManager::TranslateSlicePlanes( float dx, float dy, float dz )
{
	for( int i = EP_FIRST; i < EP_ENUMNO; ++i )
	{
		m_pViewports[i].pSlicer->TranslateSlicePlane( dx, dy, dz );
	}
}

void CViewportManager::RotateSlicePlanes( float Yaw, float Pitch, float Roll )
{
	for( int i = EP_FIRST; i < EP_ENUMNO; ++i )
	{
		m_pViewports[i].pSlicer->RotateSlicePlane( Yaw, Pitch, Roll );
	}
}

void CViewportManager::Render()
{
	LPDIRECT3DSURFACE9 pBB = NULL;
	m_pD3DDevice->GetRenderTarget( 0, &pBB);  // shoud be BackBuffer

	m_pD3DDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

	D3DXMATRIX matOldView = m_pVolume->m_mtxView;

	for( int i = EP_FIRST; i < EP_ENUMNO; ++i )
	{
		const sViewport *v = SetActiveViewport( (EProjection)i );

		// change view matrix
		switch( i )
		{
		case EP_Top:
			D3DXMatrixLookAtLH( &m_pVolume->m_mtxView, 
								&D3DXVECTOR3( 0.0f, matOldView._43, 0.0f ), 
								&D3DXVECTOR3( 0.0f, 0.0f, 0.0f ),
								&D3DXVECTOR3( 1.0f, 0.0f, 0.0f ) );
			break;
		case EP_Front:
			D3DXMatrixLookAtLH( &m_pVolume->m_mtxView, 
								&D3DXVECTOR3( 0.0f, 0.0f, -matOldView._43 ), 
								&D3DXVECTOR3( 0.0f, 0.0f, 0.0f ),
								&D3DXVECTOR3( 0.0f, 1.0f, 0.0f ) );
			break;
		case EP_Left:
			D3DXMatrixLookAtLH( &m_pVolume->m_mtxView, 
								&D3DXVECTOR3( matOldView._43 ,0.0f, 0.0f ), 
								&D3DXVECTOR3( 0.0f, 0.0f, 0.0f ),
								&D3DXVECTOR3( 0.0f, 1.0f, 0.0f ) );
			break;

		case EP_Perspective:
			m_pVolume->m_mtxView = matOldView;
			break;
		}

		v->pSlicer->RenderObject();
	}
	
	m_pD3DDevice->SetFVF( sScreenVertexTex::FVF_FLAGS );

	// render to back buffer
	m_pD3DDevice->SetRenderTarget( 0, pBB );
	m_pD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, -1, 1.0f, 0 );

	// render each viewport
	for(int i=EP_FIRST; i<=EP_LAST; ++i)
	{
		sViewport& v = m_pViewports[ i ];
		m_pD3DDevice->SetTexture( 0, v.pTex );
		m_pD3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 2, v.pVertices, sizeof( sScreenVertexTex ) );
	}

	// done with the back buffer
	SAFE_RELEASE( pBB );
}
