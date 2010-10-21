#pragma once
#include "Volume.h"


class CVolumeRaycasting : public CObject
{
public:
	CVolumeRaycasting( CVolume *pVolume );
	~CVolumeRaycasting(void);

public:
	virtual HRESULT OnResetDevice	( LPDIRECT3DDEVICE9 pD3DDevice );
	virtual void OnLostDevice		( );
	virtual void Release			( );
	virtual void Render				( );

	void TranslateROI				( float dx, float dy, float dz );

public:
	D3DXVECTOR3					m_vLightDir;
	D3DXVECTOR3					m_vRoiParams;
	D3DXVECTOR3					m_vRoiCenter;

protected:
	CVolume*					m_pVolume;
	LPD3DXEFFECT				m_fxShader;

private:
	LPDIRECT3DTEXTURE9			m_pBack;
	LPDIRECT3DTEXTURE9			m_pFront;
};
