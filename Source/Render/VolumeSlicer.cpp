#include "stdafx.h"
#include "VolumeSlicer.h"


CVolumeSlicer::CVolumeSlicer( const CVolume *pVolume, EProjection eType )
{
	m_pVolume = pVolume;
	m_pTransferFnTex = NULL;
	m_fxShader = NULL;
	m_eType = eType;
}

CVolumeSlicer::~CVolumeSlicer(void)
{
	Release();
}

void CVolumeSlicer::Release()
{
	CObject::Release();
	SAFE_RELEASE( m_pTransferFnTex );
}

HRESULT CVolumeSlicer::OnResetDevice( LPDIRECT3DDEVICE9 pD3DDevice )
{
	HRESULT hr = D3D_OK;

	V( CObject::OnResetDevice( pD3DDevice ) );

	SAFE_DELETE( m_pMesh );
	m_pMesh = new CMesh;

	if( m_pMesh->AddVertex( 4 ) < 0 ) return E_FAIL;
	CVertex * pVertex = &m_pMesh->m_pVertex[0];

	switch( m_eType )
	{
	case EP_Perspective:
	case EP_Front:
		*pVertex++ = CVertex( -0.5, -0.5, 0.0f, 0.0f, 0.0f, 0.5f );
		*pVertex++ = CVertex( -0.5, +0.5, 0.0f, 0.0f, 1.0f, 0.5f );
		*pVertex++ = CVertex( +0.5, +0.5, 0.0f, 1.0f, 1.0f, 0.5f );
		*pVertex++ = CVertex( +0.5, -0.5, 0.0f, 1.0f, 0.0f, 0.5f );
		break;
	case EP_Top:
		*pVertex++ = CVertex( -0.5, 0.0f, -0.5, 0.0f, 0.5f, 0.0f );
		*pVertex++ = CVertex( -0.5, 0.0f, +0.5, 0.0f, 0.5f, 1.0f );
		*pVertex++ = CVertex( +0.5, 0.0f, +0.5, 1.0f, 0.5f, 1.0f );
		*pVertex++ = CVertex( +0.5, 0.0f, -0.5, 1.0f, 0.5f, 0.0f );
		break;
	case EP_Left:
		*pVertex++ = CVertex( 0.0f, -0.5, -0.5, 0.5f, 0.0f, 0.0f );
		*pVertex++ = CVertex( 0.0f, +0.5, -0.5, 0.5f, 1.0f, 0.0f );
		*pVertex++ = CVertex( 0.0f, +0.5, +0.5, 0.5f, 1.0f, 1.0f );
		*pVertex++ = CVertex( 0.0f, -0.5, +0.5, 0.5f, 0.0f, 1.0f );
		break;
	}

	if( m_pMesh->AddIndex( 4 ) < 0 ) return E_FAIL;

	m_pMesh->m_pIndex[0] = 0;
	m_pMesh->m_pIndex[1] = 1;
	m_pMesh->m_pIndex[2] = 3;
	m_pMesh->m_pIndex[3] = 2;


	V_RETURN( m_pMesh->BuildBuffers( m_pD3DDevice, true, false ) );
	

	SAFE_RELEASE( m_pTransferFnTex );
	m_pTransferFnTex = m_pVolume->m_pTransferFunction;
	m_pTransferFnTex->AddRef();

	// init shader variables
	if( m_hShader != HSHADER_INVALID )
	{
		m_fxShader = CShaderManager::GetInstance()->GetShader( m_hShader );
		if( !m_fxShader ) return E_FAIL;

		m_fxShader->SetTexture( "Volume", m_pVolume->m_pVolumeTexture );

		float maxSize = (float)max(m_pVolume->m_iWidth, max(m_pVolume->m_iHeight, m_pVolume->m_iDepth));
		D3DXVECTOR4 ratios( m_pVolume->m_iWidth / maxSize, m_pVolume->m_iHeight / maxSize, m_pVolume->m_iDepth / maxSize, 1.0f);
		m_fxShader->SetFloatArray( "ScaleFactor", (const FLOAT*)&ratios, 4 );
		m_fxShader->SetTexture( "Transfer", m_pTransferFnTex );

		m_fxShader->SetTechnique( "TransferFn1D" );
	}

	return hr;
}

void CVolumeSlicer::OnLostDevice()
{
	CObject::OnLostDevice();
	SAFE_RELEASE( m_pTransferFnTex );
}

HRESULT CVolumeSlicer::BeginRender()
{
	HRESULT hr = D3D_OK;
	V_RETURN( CObject::BeginRender() );

	if( m_fxShader )
	{
		// Update the effect's variables.  Instead of using strings, it would 
		// be more efficient to cache a handle to the parameter by calling 
		// ID3DXEffect::GetParameterByName
		m_fxShader->SetMatrix( "matWorld", &m_mtxWorld );
		m_fxShader->SetMatrix( "matView", &m_pVolume->m_mtxView );
		m_fxShader->SetMatrix( "matProj", &m_pVolume->m_mtxProj );
		m_fxShader->SetMatrix( "matTex", &m_mtxTexture );

		m_fxShader->SetFloat( "IsoValue", m_pVolume->m_fIsoValue );

	}
	else
	{
		m_pD3DDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
		m_pD3DDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

		// enable alpha blending
		m_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		m_pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		m_pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	}

	return hr;
}

void CVolumeSlicer::Render()
{
	m_pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP, 0, 0, 4, 0, 2 );
}

HRESULT CVolumeSlicer::EndRender()
{
	HRESULT hr = D3D_OK;
	V_RETURN( CObject::EndRender() );

	if( !m_fxShader )
	{
		m_pD3DDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
		m_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	}

	return hr;
}

void CVolumeSlicer::TranslateSlicePlane( float dx, float dy, float dz )
{
	D3DXMATRIX mt;
	
	switch( m_eType )
	{
	case EP_Top:
		D3DXMatrixTranslation( &mt, 0, dy, 0 );
		break;

	case EP_Left:
		D3DXMatrixTranslation( &mt, dx, 0, 0 );
		break;

	case EP_Front:
		D3DXMatrixTranslation( &mt, 0, 0, dz );
		break;

	case EP_Perspective:
		D3DXMatrixTranslation( &mt, dx, dy, dz );
		break;
	}

	D3DXMatrixMultiply( &m_mtxWorld, &m_mtxWorld, &mt );
	D3DXMatrixMultiply( &m_mtxTexture, &m_mtxTexture, &mt );
}

void CVolumeSlicer::RotateSlicePlane(float Yaw, float Pitch, float Roll)
{
	if( m_eType != EP_Perspective )
		return;

	D3DXMATRIX mr;
	D3DXMatrixRotationYawPitchRoll( &mr, Yaw, Pitch, Roll );

	D3DXMatrixMultiply( &m_mtxWorld, &m_mtxWorld, &mr );

	D3DXMATRIX mt;
	D3DXMatrixTranslation( &mt, -0.5f, -0.5f, -0.5f );
	D3DXMatrixMultiply( &mr, &mt, &mr );
	D3DXMatrixTranslation( &mt, 0.5f, 0.5f, 0.5f );
	D3DXMatrixMultiply( &mr, &mr, &mt );

	D3DXMatrixMultiply( &m_mtxTexture, &m_mtxTexture, &mr );
}