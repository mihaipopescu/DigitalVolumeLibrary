//-----------------------------------------------------------------------------
// File: VolumeSlicer.h
//
// Desc: This class slices a volume with a user-defined plane
//			
//-----------------------------------------------------------------------------
#ifndef _VOLUMESLICER_H_
#define _VOLUMESLICER_H_
#include "CObject.h"
#include "Volume.h"
#include "ShaderManager.h"


enum EProjection
{
	EP_FIRST,
	EP_Top = EP_FIRST,
	EP_Front,
	EP_Left,
	EP_Perspective,
	EP_LAST = EP_Perspective,
	EP_ENUMNO
};


class CVolumeSlicer : public CObject
{
public:
	CVolumeSlicer			( const CVolume* pVolume, EProjection eType = EP_Perspective );
	virtual ~CVolumeSlicer	( void );
	virtual void Release	( );

public:
	virtual HRESULT OnResetDevice	( LPDIRECT3DDEVICE9 pD3DDevice );
	virtual void OnLostDevice		( );

	virtual HRESULT BeginRender		( );
	virtual void Render				( );
	virtual HRESULT EndRender		( );

	// Transform slice plane
	void TranslateSlicePlane		( float dx, float dy, float dz );
	void RotateSlicePlane			( float Yaw, float Pitch, float Roll );

protected:
	const CVolume *				m_pVolume;
	EProjection					m_eType;

	LPDIRECT3DTEXTURE9			m_pTransferFnTex;
	LPD3DXEFFECT				m_fxShader;
};


#endif