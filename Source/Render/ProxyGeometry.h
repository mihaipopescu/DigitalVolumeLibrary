//-----------------------------------------------------------------------------
// File: ProxyGeometry.h
//
// Desc: This class contains the proxy geometry generated for the volume
//			
//-----------------------------------------------------------------------------

#ifndef _PROXYGEOMETRY_H_
#define _PROXYGEOMETRY_H_

#include "CObject.h"
#include "Volume.h"
#include "ShaderManager.h"
#include "VolumeSlicer.h"


class CProxyGeometry : public CVolumeSlicer
{
public:
	CProxyGeometry			( const CVolume *pVolume );
	virtual ~CProxyGeometry	( );
	virtual void Release	( );

	HRESULT Update					( const D3DXMATRIX *pView );

	virtual void Render				( );

private:
	int	*						m_vSlicesVertexCount;
	int							m_nSlices;
};


#endif

