//-----------------------------------------------------------------------------
// File: ShaderManager.h
//
// Desc: This class is responsable for managing shaders used in the application
//			
//-----------------------------------------------------------------------------

#ifndef _SHADERMANAGER_H_
#define _SHADERMANAGER_H_

#include "CObject.h"


#define MAX_EFFECTS 10u


class CShaderManager : public CRenderer, public CSingleton<CShaderManager>
{
protected:
	CShaderManager(void);
	virtual ~CShaderManager(void);

public:
	HRESULT OnResetDevice( LPDIRECT3DDEVICE9 pD3DDevice );
	void OnLostDevice();

	HSHADER LoadShader( LPCWSTR wcsEffectFile );
	bool LoadShaderForObject( LPCWSTR wcsEffectFile, CObject *pObject );

	void UnloadShader( HSHADER hShader );
	void UnloadAllShaders();

	LPD3DXEFFECT GetShader( HSHADER hShader );
	void SetObject( CObject *pObject ) { m_pObject = pObject; }

	void Render();

private:
	LPD3DXEFFECT m_vEffects[ MAX_EFFECTS ];
	CObject *m_pObject;



	friend class CSingleton<CShaderManager>;
};


#endif