#pragma once
#include "CObject.h"
#include "VolumeSlicer.h"


class CViewportManager : public CRenderer, public CSingleton<CViewportManager>
{
public:

	struct sViewport
	{
		LPDIRECT3DTEXTURE9 pTex;
		LPDIRECT3DSURFACE9 pRT;
		D3DXMATRIX mView;
		sScreenVertexTex pVertices[4];
		CVolumeSlicer* pSlicer;
		char* szName;
	};

protected:
	CViewportManager(void);
	~CViewportManager(void);
	friend class CSingleton<CViewportManager>;

public:
	HRESULT OnResetDevice( LPDIRECT3DDEVICE9 pD3DDevice );
	void Create( CVolume *pVolume, HSHADER hShader );
	
	void TranslateSlicePlanes( float dx, float dy, float dz );
	void RotateSlicePlanes( float Yaw, float Pitch, float Roll );

	const sViewport* SetActiveViewport( EProjection viewport );
	const sViewport* GetViewport( EProjection viewport );

	void Update( const D3DXMATRIX *pView );
	void Render();

private:
	CVolume* m_pVolume;
	HSHADER m_hShader;
	sViewport m_pViewports[EP_ENUMNO];
	static char* m_szViewportName[EP_ENUMNO];

	float m_fScreenWidth;
	float m_fScreenHeight;
};
