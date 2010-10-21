//-----------------------------------------------------------------------------
// File: Volume.h
//
// Desc: This class holds information about the volume data containing
//		 8bits / voxel density data that has to be rendered.
//-----------------------------------------------------------------------------
#ifndef _VOLUME_H_
#define _VOLUME_H_

#include "CObject.h"
#include "../Volume/VolumeData.h"


class CVolume : public CObject, public CVolumeData
{
public:
	CVolume						( );
	CVolume						( const CVolumeData &v );
	CVolume						( UINT Width, UINT Height, UINT Depth );
	virtual ~CVolume			( );
	virtual void Init			( );
	virtual void Release		( );

	HRESULT UpdateVolumeTexture	( );

	virtual HRESULT OnResetDevice( LPDIRECT3DDEVICE9 pD3DDevice );
	virtual void OnLostDevice	( );
	virtual void Render			( );

	void DrawTransferFunction	( UINT left, UINT top, UINT right, UINT bottom );

public:
	LPDIRECT3DVOLUMETEXTURE9	m_pVolumeTexture;
	LPDIRECT3DTEXTURE9			m_pTransferFunction;

	D3DXMATRIX					m_mtxView;
	D3DXMATRIX					m_mtxProj;

	int							m_vEdgeList[24];
	float						m_fIsoValue;
};

#endif