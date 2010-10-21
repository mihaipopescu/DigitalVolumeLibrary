#include "stdafx.h"
#include "DXUT.h"
#include "ShaderManager.h"

//singleton instance
CShaderManager* CSingleton<CShaderManager>::m_pInstance = NULL;

CShaderManager::CShaderManager()
{
	memset( m_vEffects, NULL, MAX_EFFECTS * sizeof(LPD3DXEFFECT) );
	m_pObject = NULL;
}

CShaderManager::~CShaderManager(void)
{
	UnloadAllShaders();
}

HRESULT CShaderManager::OnResetDevice( LPDIRECT3DDEVICE9 pD3DDevice )
{
	HRESULT hr = CRenderer::OnResetDevice( pD3DDevice );

	for( HSHADER hs = 0; hs < MAX_EFFECTS; ++hs )
		if( m_vEffects[ hs ] != NULL )
		{
			hr = m_vEffects[ hs ]->OnResetDevice();
			if( hr != D3D_OK ) break;
		}

	return hr;
}

void CShaderManager::OnLostDevice()
{
	CRenderer::OnLostDevice();

	for( HSHADER hs = 0; hs < MAX_EFFECTS; ++hs )
		if( m_vEffects[ hs ] != NULL )
			m_vEffects[ hs ]->OnLostDevice();
}


HSHADER CShaderManager::LoadShader( LPCWSTR wcsEffectFile )
{
	// find an empty slot
	HSHADER hs = 0;
	for( ; hs < MAX_EFFECTS && m_vEffects[ hs ] != NULL; ++hs );

	// return invalid handle if no slot found or no device
	if( hs == MAX_EFFECTS || m_pD3DDevice == NULL ) 
		return HSHADER_INVALID;

	LPD3DXBUFFER pBufferErrors = NULL;
	HRESULT hr = D3DXCreateEffectFromFile( m_pD3DDevice, wcsEffectFile, NULL, NULL, 0, NULL, &m_vEffects[ hs ], &pBufferErrors );

	// If there are errors, notify the users
	if( FAILED( hr ) && pBufferErrors )
	{
		WCHAR wsz[256];
		MultiByteToWideChar( CP_ACP, 0, ( LPSTR )pBufferErrors->GetBufferPointer(), -1, wsz, 256 );
		wsz[ 255 ] = 0;
		DXUTTrace( __FILE__, ( DWORD )__LINE__, E_FAIL, wsz, true );
	}

	if( hr != D3D_OK )
		return HSHADER_INVALID;

	return hs;
}

bool CShaderManager::LoadShaderForObject( LPCWSTR wcsEffectFile, CObject *pObject )
{
	if( pObject != NULL )
	{
		HSHADER hs = LoadShader( wcsEffectFile );
		if( hs == HSHADER_INVALID )
			return false;
		else
		{
			pObject->m_hShader = hs;
			return true;
		}
	}
	else
		return false;
}

void CShaderManager::UnloadShader( HSHADER hShader )
{
	LPD3DXEFFECT fxShader = GetShader( hShader );
	if( fxShader == NULL ) return;

	fxShader->Release();
	m_vEffects[ hShader ] = NULL;
}

void CShaderManager::UnloadAllShaders( )
{
	for( HSHADER hs=0; hs < MAX_EFFECTS; ++hs )
		UnloadShader( hs );
}

LPD3DXEFFECT CShaderManager::GetShader( HSHADER hShader )
{
	// bail out on out of range or no shader loaded at that handle
	if( hShader >= MAX_EFFECTS || m_vEffects[ hShader ] == NULL )
		return NULL;

	return m_vEffects[ hShader ];
}

void CShaderManager::Render( )
{
	if( m_pD3DDevice == NULL || m_pObject == NULL )
		return;

	LPD3DXEFFECT fxShader = GetShader( m_pObject->m_hShader );

	if( fxShader == NULL )
	{
		m_pObject->Render();
		return;
	}

	UINT uiNumPasses = 0;

	fxShader->Begin( &uiNumPasses, 0 );

	for( UINT iPass = 0; iPass < uiNumPasses; ++iPass )
	{
		fxShader->BeginPass( iPass );

		m_pObject->Render();

		fxShader->EndPass();
	}

	fxShader->End();
}