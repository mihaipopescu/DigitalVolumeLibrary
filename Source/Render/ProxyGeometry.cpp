#include "stdafx.h"
#include "ProxyGeometry.h"

#define EPSILON 1e-5

CProxyGeometry::CProxyGeometry( const CVolume *pVolume )
: CVolumeSlicer( pVolume )
{
	m_vSlicesVertexCount = NULL;
	m_nSlices = 0;
}


CProxyGeometry::~CProxyGeometry( void )
{
	Release();
}


void CProxyGeometry::Release()
{
	CVolumeSlicer::Release();

	m_nSlices = 0;
	SAFE_DELETE_ARRAY( m_vSlicesVertexCount );
}

int VertexSort(void* context, const void* p1, const void* p2)
{
	D3DXVECTOR3 *v1 = (D3DXVECTOR3*)p1;
	D3DXVECTOR3 *v2 = (D3DXVECTOR3*)p2;
	float *s		= (float*)context;

	float a = atan2(v1->y, v1->x);
	float b = atan2(v2->y, v2->x);

	if(a<0.f) a += 2*D3DX_PI;
	if(b<0.f) b += 2*D3DX_PI;

	if(fabsf(a-b)<EPSILON)
		return 0;
	else
		if(a < b)
			return *s < 0.f ? 1 : -1;
		else
			return *s < 0.f ? -1 : 1;
}

bool IntersectPlaneWithSegment(const D3DXPLANE* pPlane, const D3DXVECTOR3 *pS1, const D3DXVECTOR3 *pS2, D3DXVECTOR3 *pResult)
{
	FLOAT t = D3DXPlaneDotCoord(pPlane, pS1);
	D3DXVECTOR3 r = *pS1 - *pS2;
	FLOAT d = D3DXPlaneDotNormal(pPlane, &r);

	if(fabsf(d) < EPSILON)
		return false;

	t /= d;

	if(t >= 0.f && t<=1.f)
	{
		*pResult = D3DXVECTOR3(	pS1->x + (pS2->x - pS1->x)*t,
								pS1->y + (pS2->y - pS1->y)*t,
								pS1->z + (pS2->z - pS1->z)*t);
		return true;
	}

	return false;
}

HRESULT CProxyGeometry::Update( const D3DXMATRIX *pView )
{
	HRESULT hr;
	if(!m_pD3DDevice) return E_INVALIDARG;

	CMesh * pMesh = m_pVolume->m_pMesh;

	// Transform the volume bounding box vertices into view coordinates using the modelview matrix.
	D3DXVECTOR3 * vTransformedVertices = new D3DXVECTOR3[ pMesh->m_nVertexCount ];
	D3DXVec3TransformCoordArray( vTransformedVertices, sizeof(D3DXVECTOR3), reinterpret_cast<D3DXVECTOR3*>( pMesh->m_pVertex ), sizeof(CVertex), pView, pMesh->m_nVertexCount );

	// compute inverse view matrix
	D3DXMATRIX matViewInv;
	D3DXMatrixInverse( &matViewInv, NULL, pView );

	// create afine transform matrix for 3D texture coordinates
	D3DXMATRIX matTex;
	D3DXMatrixTranslation(&matTex, 0.5, 0.5, 0.5);

	// Find the minimum and maximum z coordinates of the transformed vertices. 
	// Compute the number of sampling planes used between these two values using equidistant spacing from the view origin. 
	float minz = 1000.f, maxz = -1000.f;
	for(unsigned int i=0;i<pMesh->m_nVertexCount;i++)
	{
		minz = min(minz, vTransformedVertices[i].z);
		maxz = max(maxz, vTransformedVertices[i].z);
	}

	// The sampling distance is computed from the voxel size and current sampling rate. 
	if( m_nSlices == 0 )
		m_nSlices = max( max( m_pVolume->m_iDepth, m_pVolume->m_iHeight ), m_pVolume->m_iWidth );

	// make sure we have at lease 10 slices
	if( m_nSlices == 0 )
		m_nSlices = 10;

	float dDist = (maxz - minz) / m_nSlices;

	// discard previous mesh
	SAFE_DELETE( m_pMesh );
	m_pMesh = new CMesh;

	SAFE_DELETE_ARRAY( m_vSlicesVertexCount );
	m_vSlicesVertexCount = new int[m_nSlices];

	// we are now working on view coordinates
	//  so the slicing planes should always be perpendicular to the view 
	//  thus the normal should always be the versor of Z axis
	D3DXVECTOR3 norm(0.f, 0.f, 1.f);

	// the initial position is at the farest vertex to the camera
	D3DXVECTOR3 pos(0.f, 0.f, maxz);

	// for each slice
	for( int iSlice = 0; iSlice < m_nSlices; ++iSlice )
	{
		// generate plane
		D3DXPLANE SlicePlane;
		D3DXPlaneFromPointNormal(&SlicePlane, &pos, &norm);

		// advance the slice plane point along Z Axis (back to front)
		pos -= D3DXVECTOR3(0.f, 0.f, dDist);

		int nSliceVertexCount = 0;
		
		// maximum 6 intersection points
		D3DXVECTOR3 vIntPts[6];

		for(int e = 0; e < 12; ++e )
		{
			D3DXVECTOR3 res;

			if( IntersectPlaneWithSegment( &SlicePlane, &vTransformedVertices[ m_pVolume->m_vEdgeList[2*e] ], &vTransformedVertices[ m_pVolume->m_vEdgeList[2*e+1] ], &res) )
			{
				int j = 0;
				// is intersection point unique ?

				for( ; j<nSliceVertexCount; ++j )
				{
					D3DXVECTOR3 dif;
					D3DXVec3Subtract(&dif, &vIntPts[j], &res);
					if(D3DXVec3Length(&dif) < EPSILON)
						break;
				}

				// yes ? add it to the list
				if( j == nSliceVertexCount )
					vIntPts[ nSliceVertexCount++ ] = res;
			}

			if( nSliceVertexCount == 6 )
				break;
		}

		// We must have minimum 3 intersection points to create a mesh!
		if( nSliceVertexCount < 3 )
		{
			m_vSlicesVertexCount[ iSlice ] = 0;
			continue;
		}

		m_vSlicesVertexCount[ iSlice ] = nSliceVertexCount;

		// compute the center of gravity and bring back the vertices from view to world coordinates
		D3DXVECTOR3 avg(0.f, 0.f, 0.f);
		for( int j=0; j<nSliceVertexCount; ++j)
		{
			avg += vIntPts[j];
			D3DXVec3TransformCoord(&vIntPts[j], &vIntPts[j], &matViewInv);
		}

		// performe the average on points
		D3DXVec3Scale(&avg, &avg, 1.f / nSliceVertexCount);
		D3DXVec3TransformCoord(&avg, &avg, &matViewInv);

		// sort vertices
		{
			// to sort the vertices in the correct winding order
			float side = -pView->_33;
			qsort_s(&vIntPts, nSliceVertexCount, sizeof(D3DXVECTOR3), VertexSort, &side);
		}

		D3DXVECTOR3 texc;

		// add sorted vertices into the mesh
		int iPreviousVertexCount = m_pMesh->m_nVertexCount;
		m_pMesh->AddVertex( nSliceVertexCount + 1 );
		for( int j=0; j<nSliceVertexCount; ++j )
		{
			// transform texture coordinates
			D3DXVec3TransformCoord(&texc, &vIntPts[j], &matTex);
			m_pMesh->m_pVertex[ j + iPreviousVertexCount ] = CVertex(vIntPts[j].x, vIntPts[j].y, vIntPts[j].z, texc.x, texc.y, texc.z);
		}

		D3DXVec3TransformCoord(&texc, &avg, &matTex);
		m_pMesh->m_pVertex[ nSliceVertexCount + iPreviousVertexCount ] = CVertex(avg.x, avg.y, avg.z, texc.x, texc.y, texc.z);

		// add indices prepared for TRIANGLEFAN
		int iPreviousIndexCount = m_pMesh->m_nIndexCount;
		m_pMesh->AddIndex( nSliceVertexCount + 2 );
		m_pMesh->m_pIndex[ iPreviousIndexCount ] = nSliceVertexCount + iPreviousVertexCount;

		for( int j=0; j<nSliceVertexCount; ++j )
			m_pMesh->m_pIndex[ iPreviousIndexCount + j + 1 ] = j + iPreviousVertexCount;
		m_pMesh->m_pIndex[ iPreviousIndexCount + nSliceVertexCount + 1 ] = iPreviousVertexCount;
	}

	// build index/vertex buffers
	V ( m_pMesh->BuildBuffers( m_pD3DDevice, true ) );

	SAFE_DELETE_ARRAY( vTransformedVertices );

	return S_OK;
}

void CProxyGeometry::Render( )
{
	int startIndex = 0;

	// render slices back to front
	for( int iSlice = 0; iSlice < m_nSlices; ++iSlice )
	{
		if( m_vSlicesVertexCount[ iSlice ] == 0 )
			continue;

		m_pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLEFAN, 0, 0, m_vSlicesVertexCount[ iSlice ] + 1, startIndex, m_vSlicesVertexCount[ iSlice ] );

		startIndex += m_vSlicesVertexCount[ iSlice ] + 2;
	}
}