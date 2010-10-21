#include "stdafx.h"
#include "VolumeRaycasting.h"
#include "ShaderManager.h"
#include "DXUT.h"



CVolumeRaycasting::CVolumeRaycasting(CVolume *pVolume)
{
	m_pVolume = pVolume;
	m_fxShader = NULL;
	m_vLightDir = D3DXVECTOR3(0.1f, -.5f, 1.f);

	m_vRoiParams = D3DXVECTOR3(3.f, .15f, .15f);
	m_vRoiCenter = D3DXVECTOR3(0.488f, 0.131f, 0.425f);

	m_pBack = NULL;
	m_pFront = NULL;
}

CVolumeRaycasting::~CVolumeRaycasting(void)
{
	Release();
}

void CVolumeRaycasting::Release()
{
	CObject::Release();

	SAFE_RELEASE( m_pBack );
	SAFE_RELEASE( m_pFront );
	m_pVolume = NULL;
	m_fxShader = NULL;
}


HRESULT CVolumeRaycasting::OnResetDevice(LPDIRECT3DDEVICE9 pD3DDevice)
{
	CObject::OnResetDevice( pD3DDevice );

	m_fxShader = CShaderManager::GetInstance()->GetShader( m_hShader );
	
	if(!m_pVolume || !m_fxShader) return E_FAIL;

	HRESULT hr;
	D3DPRESENT_PARAMETERS pp = DXUTGetDeviceSettings().d3d9.pp;

	SAFE_RELEASE( m_pBack );
	V_RETURN( D3DXCreateTexture(m_pD3DDevice, pp.BackBufferWidth, pp.BackBufferHeight, 1, D3DUSAGE_RENDERTARGET, pp.BackBufferFormat, D3DPOOL_DEFAULT, &m_pBack));

	SAFE_RELEASE( m_pFront );
	V_RETURN( D3DXCreateTexture(m_pD3DDevice, pp.BackBufferWidth, pp.BackBufferHeight, 1, D3DUSAGE_RENDERTARGET, pp.BackBufferFormat, D3DPOOL_DEFAULT, &m_pFront));

	int iWidth = m_pVolume->m_iWidth, iHeight = m_pVolume->m_iHeight, iDepth = m_pVolume->m_iDepth;

	float maxSize = (float)max(iWidth, max(iHeight, iDepth));
	D3DXVECTOR3 stepSize(1.0f  / (iWidth * (maxSize / iWidth)),
						   1.0f / (iHeight * (maxSize / iHeight)),
						   1.0f / (iDepth * (maxSize / iDepth)));


	float mStepScale = 0.5;

	stepSize = stepSize * mStepScale;
	m_fxShader->SetFloatArray("SampleDist", (const FLOAT*)&stepSize, 3);
	m_fxShader->SetFloat("ActualSampleDist", mStepScale);
	m_fxShader->SetInt("Iterations", (int)(maxSize * (1.0f / mStepScale) * 2.0f));

	//calculate the scale factor
	//volumes are not always perfect cubes. so we need to scale our cube
	//by the sizes of the volume. Also, scalar data is not always sampled
	//at equidistant steps. So we also need to scale the cube model by mRatios.
	D3DXVECTOR4 ratios( iWidth / maxSize, iHeight / maxSize, iDepth / maxSize, 1.0f);
	m_fxShader->SetFloatArray("ScaleFactor", (const FLOAT*)&ratios, 4);
	m_fxShader->SetTexture( "Volume", m_pVolume->m_pVolumeTexture );
	m_fxShader->SetTexture( "Transfer", m_pVolume->m_pTransferFunction );


	return D3D_OK;
}

void CVolumeRaycasting::OnLostDevice()
{
	CObject::OnLostDevice();
	Release();
}

void CVolumeRaycasting::Render()
{
	// bail out if no device set
	if(m_pD3DDevice == NULL || m_pVolume == NULL || m_fxShader == NULL) return;
	HRESULT hr;

	m_pVolume->m_hShader = m_hShader;

	// set shader variables
	D3DXMATRIX mat = m_pVolume->m_mtxWorld;
	D3DXMatrixMultiply( &mat, &m_pVolume->m_mtxView, &m_pVolume->m_mtxProj );
	m_fxShader->SetMatrix( "WorldViewProj", &mat );

	LPDIRECT3DSURFACE9 pSurf = NULL; // local surface
	LPDIRECT3DSURFACE9 pBack = NULL;
	m_pD3DDevice->GetRenderTarget(0, &pBack); // get a copy of the backbuffer

	m_fxShader->SetTechnique("RenderPosition");

	//~~ 1. Render Front Position
	m_pFront->GetSurfaceLevel(0, &pSurf);
	m_pD3DDevice->SetRenderTarget(0, pSurf);
	m_pD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
	m_pD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0 );
	m_pVolume->RenderObject();

	// intermediary
	m_pD3DDevice->SetRenderTarget(0, pBack);
	pSurf->Release();

	//~~ 2. Render Back Position
	m_pBack->GetSurfaceLevel(0, &pSurf);
	m_pD3DDevice->SetRenderTarget(0, pSurf);
	m_pD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
	m_pD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0 );
	m_pVolume->RenderObject();

	//~~ 3. Render Raycast
	V( m_fxShader->SetTechnique("Raycasting") );
	m_fxShader->SetTexture("Front", m_pFront);
	m_fxShader->SetTexture("Back", m_pBack);
	m_fxShader->SetFloat("IsoValue", m_pVolume->m_fIsoValue);
	m_fxShader->SetFloatArray("LightDir", (const FLOAT*)m_vLightDir, 3);

	m_fxShader->SetFloatArray("ROI_Params", (const FLOAT*)m_vRoiParams, 3);
	m_fxShader->SetFloatArray("ROI_Center", (const FLOAT*)m_vRoiCenter, 3);

	m_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	m_pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	m_pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	m_pD3DDevice->SetRenderTarget(0, pBack);
	m_pD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
	m_pVolume->RenderObject();

	pSurf->Release();
	pBack->Release();

	m_pVolume->m_hShader = HSHADER_INVALID;

	m_pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}

void CVolumeRaycasting::TranslateROI( float dx, float dy, float dz )
{
	m_vRoiCenter.x += dx;
	m_vRoiCenter.y += dy;
	m_vRoiCenter.z += dz;
}