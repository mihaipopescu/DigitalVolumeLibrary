//--------------------------------------------------------------------------------------
// File: VolumeRenderingApp.cpp
//
// Author:	Mihai Popescu
// Date:	06/15/2009
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "DXUTgui.h"
#include "DXUTcamera.h"
#include "DXUTsettingsdlg.h"
#include "SDKmisc.h"
#include "../Math/TransferFunction.h"
#include "../Render/Volume.h"
#include "../Render/ProxyGeometry.h"
#include "../Render/VolumeSlicer.h"
#include "../Render/VolumeRaycasting.h"
#include "../Render/ShaderManager.h"
#include "../Render/ViewportManager.h"

#include "../Volume/Primitive/HoneycombVolume.h"
#include "../Volume/Primitive/SphereVolume.h"



//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
ID3DXFont*                  g_pFont = NULL;         // Font for drawing text
ID3DXSprite*                g_pTextSprite = NULL;   // Sprite for batching draw text calls
bool                        g_bRenderText = true;
CModelViewerCamera          g_Camera;               // A model viewing camera
CDXUTDialogResourceManager  g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg             g_SettingsDlg;          // Device settings dialog
CDXUTDialog                 g_HUD;                  // dialog for standard controls
DWORD                       g_dwLastFPSCheck = 0;        // The time index of the last frame rate check
DWORD                       g_dwFrameCount = 0;        // How many frames have elapsed since the last check
DWORD                       g_dwFrameRate = 0;        // How many frames rendered during the PREVIOUS interval
float						g_fAspectRatio;
float						g_fNearPlane = 0.1f;
float						g_fIsoValue = 0.f;
CVolume*					g_pVolume;
CProxyGeometry*				g_pProxyGeometry;
CVolumeRaycasting*			g_pVolumeRaycasting;

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           3
#define IDC_CHANGEDEVICE        4
#define IDC_RENDERMETHOD		5
#define IDC_ISOVALUE			6



enum ERenderMethod
{
	ERM_RENDER_PROXYGEOMETRY,
	ERM_RENDER_RAYCASTING,
	ERM_RENDER_SLICER,
	ERM_ENUNMO
} g_RenderMethod = ERM_RENDER_PROXYGEOMETRY;	// the default render method


const char *g_szRenderMethods[ERM_ENUNMO] = { "ProxyGeometry", "RayCasting", "Slicer" };

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed,
                                  void* pUserContext );
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );
HRESULT CALLBACK OnD3D9CreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                 void* pUserContext );
HRESULT CALLBACK OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
void CALLBACK OnD3D9FrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext );
void CALLBACK MouseProc( bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown, bool bSideButton1Down,
                         bool bSideButton2Down, int nMouseWheelDelta, int xPos, int yPos, void* pUserContext );
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );
void CALLBACK OnLostDevice( void* pUserContext );
void CALLBACK OnDestroyDevice( void* pUserContext );


void RenderText();
void ReleaseObjects();


//--------------------------------------------------------------------------------------
// Rejects any D3D9 devices that aren't acceptable to the app by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D9DeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat,
                                      bool bWindowed, void* pUserContext )
{
	// No fallback, so need ps2.0
    if( pCaps->PixelShaderVersion < D3DPS_VERSION( 2, 0 ) )
        return false;

    // Typically want to skip back buffer formats that don't support alpha blending
    IDirect3D9* pD3D = DXUTGetD3D9Object();
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                                         AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
                                         D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
        return false;

    return true;
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D10 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    assert( DXUT_D3D9_DEVICE == pDeviceSettings->ver );

    HRESULT hr;
    IDirect3D9* pD3D = DXUTGetD3D9Object();
    D3DCAPS9 caps;

    V( pD3D->GetDeviceCaps( pDeviceSettings->d3d9.AdapterOrdinal,
                            pDeviceSettings->d3d9.DeviceType,
                            &caps ) );

    // If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW 
    // then switch to SWVP.
    if( ( caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) == 0 ||
        caps.VertexShaderVersion < D3DVS_VERSION( 1, 1 ) )
    {
        pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }

    // Debugging vertex shaders requires either REF or software vertex processing 
    // and debugging pixel shaders requires REF.  
#ifdef DEBUG_VS
    if( pDeviceSettings->d3d9.DeviceType != D3DDEVTYPE_REF )
    {
        pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
        pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
        pDeviceSettings->d3d9.BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }
#endif
#ifdef DEBUG_PS
    pDeviceSettings->d3d9.DeviceType = D3DDEVTYPE_REF;
#endif
    // For the first device created if its a REF device, optionally display a warning dialog box
    static bool s_bFirstTime = true;
    if( s_bFirstTime )
    {
        s_bFirstTime = false;
        if( pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_REF )
           DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver );
    }

    return true;
}

//--------------------------------------------------------------------------------------
// Create any D3D9 resources that will live through a device reset (D3DPOOL_MANAGED)
// and aren't tied to the back buffer size
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D9CreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext )
{
	HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D9CreateDevice( pd3dDevice ) );
    V_RETURN( g_SettingsDlg.OnD3D9CreateDevice( pd3dDevice ) );
	
	V_RETURN( D3DXCreateFont( pd3dDevice, 15, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET,
				OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                L"Arial", &g_pFont ) );

	V_RETURN( D3DXCreateSprite( pd3dDevice, &g_pTextSprite ) ); 

	D3DXVECTOR3 vecEye( 0.0f, 0.0f, -5.0f );
    D3DXVECTOR3 vecAt ( 0.0f, 0.0f,  0.0f );
    g_Camera.SetViewParams( &vecEye, &vecAt );
	g_Camera.SetRadius(3.f, 0.5f, 10.f);

	CShaderManager::GetInstance()->OnCreateDevice( pd3dDevice );

	// create Volume

	//CJuliaQuaternionVolume v1(128, 128, 128);
	//v1.Create(D3DXQUATERNION(-0.08,0.0,-0.8,-0.03), 0.2f, EQP_ABC, 16);

	CHoneycombVolume v1(256);
	v1.Create(7, MAXDENSITY, 0, 0, 0);

	CSphereVolume v2(128, 128, 128);
	v2.Create(64.f, 64.f, 64.f, 50.f);

	CVolumeData vd;
	vd.LoadFromFile( L"data/bonsai.hdr" );
	
	g_pVolume = new CVolume( vd + v1 );


	// create proxy geometry for that volume
	g_pProxyGeometry = new CProxyGeometry( g_pVolume );
	CShaderManager::GetInstance()->LoadShaderForObject( L"shaders/tf1d.fx", g_pProxyGeometry );

	CViewportManager::GetInstance()->Create( g_pVolume, g_pProxyGeometry->m_hShader );
	
	g_pVolumeRaycasting = new CVolumeRaycasting( g_pVolume );
	CShaderManager::GetInstance()->LoadShaderForObject( L"shaders/RayCasting.fx", g_pVolumeRaycasting );


	// Setup our D3D Device initial states
    pd3dDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
    pd3dDevice->SetRenderState( D3DRS_DITHERENABLE,  TRUE );
    pd3dDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
	pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW);
    pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

	
	// texture filtering
	pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);


    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D9 resources that won't live through a device reset (D3DPOOL_DEFAULT) 
// or that are tied to the back buffer size 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                    void* pUserContext )
{
	HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D9ResetDevice() );
    V_RETURN( g_SettingsDlg.OnD3D9ResetDevice() );
	
	CShaderManager::GetInstance()->OnResetDevice( pd3dDevice );

	if( g_pFont )
		V_RETURN( g_pFont->OnResetDevice() );

	// Create a sprite to help batch calls when drawing many lines of text
	if( g_pTextSprite )
		g_pTextSprite->OnResetDevice();

	if( g_pVolume )
		g_pVolume->OnResetDevice( pd3dDevice );

	if( g_pProxyGeometry )
		g_pProxyGeometry->OnResetDevice( pd3dDevice );

	if( g_pVolumeRaycasting )
		g_pVolumeRaycasting->OnResetDevice( pd3dDevice );

	CViewportManager::GetInstance()->OnResetDevice( pd3dDevice );

	// Setup the camera's projection parameters
    g_fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
    g_Camera.SetProjParams( D3DX_PI / 4, g_fAspectRatio, g_fNearPlane, 1000.0f );
    g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
	g_Camera.SetButtonMasks( MOUSE_LEFT_BUTTON, MOUSE_WHEEL, MOUSE_RIGHT_BUTTON );

	if( g_pProxyGeometry )
		g_pProxyGeometry->Update( g_Camera.GetViewMatrix() );

	g_HUD.SetLocation( pBackBufferSurfaceDesc->Width - 170, 10 );
    g_HUD.SetSize( 170, 170 );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	IDirect3DDevice9* pd3dDevice = DXUTGetD3D9Device();

	g_Camera.FrameMove( fElapsedTime );

	// Compute the frame rate based on a 1/4 second update cycle
    if( ( GetTickCount() - g_dwLastFPSCheck ) >= 250 )
    {
        g_dwFrameRate = g_dwFrameCount * 4;
        g_dwFrameCount = 0;
        g_dwLastFPSCheck = GetTickCount();
    }

    ++g_dwFrameCount;


   // Pause animatation if the user is rotating around
    if( !IsIconic( DXUTGetHWND() ) )
    {
        if( g_Camera.IsBeingDragged() && !DXUTIsTimePaused() )
            DXUTPause( true, false );
        if( !g_Camera.IsBeingDragged() && DXUTIsTimePaused() )
            DXUTPause( false, false );
    }

	pd3dDevice->SetTransform( D3DTS_WORLD, g_Camera.GetWorldMatrix() );
	pd3dDevice->SetTransform( D3DTS_VIEW, g_Camera.GetViewMatrix() );
	pd3dDevice->SetTransform( D3DTS_PROJECTION, g_Camera.GetProjMatrix() );

	if( g_pVolume )
	{
		g_pVolume->m_mtxWorld = *g_Camera.GetWorldMatrix();
		g_pVolume->m_mtxView = *g_Camera.GetViewMatrix();
		g_pVolume->m_mtxProj = *g_Camera.GetProjMatrix();
	}

	static bool sbDragStart = false;
	if(sbDragStart && !g_Camera.IsBeingDragged())
	{
		if( g_pProxyGeometry )
			g_pProxyGeometry->Update( g_Camera.GetViewMatrix() );

		sbDragStart = false;
	}

	if(!sbDragStart && g_Camera.IsBeingDragged())
		sbDragStart = true;
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D9 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9FrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    // If the settings dialog is being shown, then
    // render it instead of rendering the app's scene
    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.OnRender( fElapsedTime );
        return;
    }

	HRESULT hr;

	// Clear the render target and the zbuffer 
	V( pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, -1, 1.0f, 0 ) );

	// Render the scene
	if( SUCCEEDED( pd3dDevice->BeginScene() ) )
	{
		// render depending on the method
		switch( g_RenderMethod )
		{
		case ERM_RENDER_PROXYGEOMETRY:
			if( g_pProxyGeometry == NULL )
				break;
			g_pProxyGeometry->RenderObject();	
			break;
		case ERM_RENDER_RAYCASTING:
			if( g_pVolumeRaycasting )
				g_pVolumeRaycasting->Render();
			break;
		case ERM_RENDER_SLICER:
			CViewportManager::GetInstance()->Render();
			break;
		}
	
		// render objects boundaries
		pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );

		g_pVolume->RenderObject();

		// draw legend
		g_pVolume->DrawTransferFunction(10, 100, 50, 400);

		// draw text
		RenderText();

		// render HUD
		V( g_HUD.OnRender( fElapsedTime ) );

		V( pd3dDevice->EndScene() );
	}

}


//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText()
{
    if( !g_bRenderText )
        return;

    // The helper object simply helps keep track of text position, and color
    // and then it calls pFont->DrawText( g_pSprite, strMsg, -1, &rc, DT_NOCLIP, g_clr );
    // If NULL is passed in as the sprite object, then it will work however the 
    // pFont->DrawText() will not be batched together.  Batching calls will improves performance.
    CDXUTTextHelper txtHelper( g_pFont, g_pTextSprite, 15 );

    // Output statistics
    txtHelper.Begin();
    txtHelper.SetInsertionPos( 5, 5 );
    txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 0.0f, 0.0f, 1.0f ) );
    txtHelper.DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
    txtHelper.DrawTextLine( DXUTGetDeviceStats() );

	const D3DSURFACE_DESC* pd3dsdBackBuffer = DXUTGetD3D9BackBufferSurfaceDesc();
	static const UINT iMaxStringSize = 1024;
    WCHAR str[iMaxStringSize];
    StringCchPrintf( str, iMaxStringSize, L"Final Composed Image (%dx%d) @ %dfps.\n",
                     pd3dsdBackBuffer->Width,
                     pd3dsdBackBuffer->Height,
                     g_dwFrameRate);

    txtHelper.DrawTextLine( str );

    /*if( !g_bRenderUI )
	    txtHelper.DrawTextLine( L"Drag with LEFT mouse button  : Rotate volume." );
	    txtHelper.DrawTextLine( L"Drag with RIGHT mouse button : Rotate view of scene." );
        txtHelper.DrawFormattedTextLine( L"Press 'U' to show UI" );*/

	// Debug info to know where camera/model are
	D3DXVECTOR3 cvecEye = *g_Camera.GetEyePt();
	D3DXMATRIX mPos = *g_Camera.GetViewMatrix();
	txtHelper.DrawFormattedTextLine( L"camera: eye=(%0.1f,%0.1f,%0.1f) distance=%0.1f", cvecEye.x,cvecEye.y,cvecEye.z,mPos._43); 

	if( g_pVolumeRaycasting )
		txtHelper.DrawFormattedTextLine( L"ROI: center=(%0.3f,%0.3f,%0.3f)", g_pVolumeRaycasting->m_vRoiCenter.x, g_pVolumeRaycasting->m_vRoiCenter.y, g_pVolumeRaycasting->m_vRoiCenter.z );

	if( g_RenderMethod == ERM_RENDER_SLICER )
	{
		for(int vp=EP_FIRST; vp<=EP_LAST; ++vp)
		{
			// set active viewport
			const CViewportManager::sViewport* v = CViewportManager::GetInstance()->GetViewport( (EProjection)vp );

			RECT r;
			r.left = v->pVertices[0].x;
			r.top = v->pVertices[2].y - 20;
			r.right = v->pVertices[1].x;
			r.bottom = v->pVertices[2].y - 5;
			wchar_t wszName[32];
			mbstowcs( wszName, v->szName, 32 );
			txtHelper.DrawTextLine( r, DT_NOCLIP | DT_CENTER, wszName );
		}
	}

	txtHelper.End();
}


//--------------------------------------------------------------------------------------
// Handle messages to the application 
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                          bool* pbNoFurtherProcessing, void* pUserContext )
{
	// Always allow dialog resource manager calls to handle global messages
    // so GUI state is updated correctly
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

	 // Pass all remaining windows messages to camera so it can respond to user input
    g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

    return 0;
}


//--------------------------------------------------------------------------------------
// As a convenience, DXUT inspects the incoming windows messages for
// keystroke messages and decodes the message parameters to pass relevant keyboard
// messages to the application.  The framework does not remove the underlying keystroke 
// messages, which are still passed to the application's MsgProc callback.
//--------------------------------------------------------------------------------------
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
	// move offsets
	float dx = 0.f, dy = 0.f, dz = 0.f;
	float ax = 0.f, ay = 0.f, az = 0.f;
	static const float fd = 0.001f;

	switch( nChar )
	{
	case 'A':
		dx = -fd;
		break;
	case 'D':
		dx = fd;
		break;
	case 'W':
		dy = fd;
		break;
	case 'S':
		dy = -fd;
		break;
	case 'Q':
		dz = -fd;
		break;
	case 'E':
		dz = fd;
		break;
	case 'G':
		ax = -fd;
		break;
	case 'J':
		ax = fd;
		break;
	case 'Y':
		ay = fd;
		break;
	case 'H':
		ay = -fd;
		break;
	case 'T':
		az = -fd;
		break;
	case 'U':
		az = fd;
		break;
	}



	if( dx + dy + dz != 0.f )
	{
		if( g_pVolumeRaycasting )
			g_pVolumeRaycasting->TranslateROI( dx, dy, dz );

		CViewportManager::GetInstance()->TranslateSlicePlanes( dx, dy, dz );
	}

	if( ax + ay + az != 0.f )
	{
		CViewportManager::GetInstance()->RotateSlicePlanes( ax, ay, az );
	}
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN:
            DXUTToggleFullScreen(); break;
        case IDC_TOGGLEREF:
            DXUTToggleREF(); break;
        case IDC_CHANGEDEVICE:
            g_SettingsDlg.SetActive( !g_SettingsDlg.IsActive() ); break;
		case IDC_ISOVALUE:
			{
				CDXUTSlider* pSlider = ( CDXUTSlider* )pControl;
				if( g_pVolume )
				{
					g_fIsoValue = (float)pSlider->GetValue()/2;
					g_pVolume->m_fIsoValue = g_fIsoValue;
				}
			}
			break;
		case IDC_RENDERMETHOD:
			{
				CDXUTComboBox *c = (CDXUTComboBox*)pControl;
				int idx = c->GetSelectedIndex();
				g_RenderMethod = (ERenderMethod) idx;
			}

    }
}


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9ResetDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9LostDevice( void* pUserContext )
{
	g_DialogResourceManager.OnD3D9LostDevice();
    g_SettingsDlg.OnD3D9LostDevice();
	
	CShaderManager::GetInstance()->OnLostDevice();
	CViewportManager::GetInstance()->OnLostDevice();

	if( g_pFont )
        g_pFont->OnLostDevice();

	if( g_pVolume )
		g_pVolume->OnLostDevice();

	if( g_pProxyGeometry )
		g_pProxyGeometry->OnLostDevice();

	if( g_pVolumeRaycasting )
		g_pVolumeRaycasting->OnLostDevice();

    SAFE_RELEASE( g_pTextSprite );
}


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9CreateDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9DestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9DestroyDevice();
    g_SettingsDlg.OnD3D9DestroyDevice();

	ReleaseObjects();
}


//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
    // Initialize dialogs
    g_SettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );

    g_HUD.SetCallback( OnGUIEvent ); int iY = 10;
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, iY, 125, 22 );
    g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 35, iY += 24, 125, 22 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 35, iY += 24, 125, 22, VK_F2 );
	
	g_HUD.AddSlider( IDC_ISOVALUE , 35, iY += 24, 125, 22, 0, 255, 150);
	g_HUD.AddComboBox( IDC_RENDERMETHOD, 35, iY += 24, 125, 22, L'M' );
	for(int i=0;i<ERM_ENUNMO;++i)
	{
		WCHAR met[32];
		mbstowcs_s(NULL, met, 32, g_szRenderMethods[i], 32);
		g_HUD.GetComboBox( IDC_RENDERMETHOD )->AddItem( met, ( void* )i );
	}
}


//--------------------------------------------------------------------------------------
// Releases all objects / resources created by the application
//--------------------------------------------------------------------------------------
void ReleaseObjects()
{
    SAFE_RELEASE( g_pFont );
    SAFE_RELEASE( g_pTextSprite );
	SAFE_DELETE( g_pVolume );
	SAFE_DELETE( g_pProxyGeometry );
	SAFE_DELETE( g_pVolumeRaycasting );

	CShaderManager::DestroyInstance();
	CViewportManager::DestroyInstance();
}


//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
INT WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	//_crtBreakAlloc = 785;
#endif


    // Set the callback functions
    DXUTSetCallbackD3D9DeviceAcceptable( IsD3D9DeviceAcceptable );
    DXUTSetCallbackD3D9DeviceCreated( OnD3D9CreateDevice );
    DXUTSetCallbackD3D9DeviceReset( OnD3D9ResetDevice );
    DXUTSetCallbackD3D9FrameRender( OnD3D9FrameRender );
    DXUTSetCallbackD3D9DeviceLost( OnD3D9LostDevice );
    DXUTSetCallbackD3D9DeviceDestroyed( OnD3D9DestroyDevice );
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );
    DXUTSetCallbackMsgProc( MsgProc );
	DXUTSetCallbackKeyboard( KeyboardProc );
    DXUTSetCallbackFrameMove( OnFrameMove );
	DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );

    // TODO: Perform any application-level initialization here
	InitApp();

    // Initialize DXUT and create the desired Win32 window and Direct3D device for the application
    DXUTInit( true, true ); // Parse the command line and show msgboxes
    DXUTSetHotkeyHandling( true, true, true );  // handle the default hotkeys
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
    DXUTCreateWindow( L"Volume Rendering" );
    DXUTCreateDevice( true, 800, 600 );

    // Start the render loop
    g_dwLastFPSCheck = GetTickCount();
    DXUTMainLoop();

    // Perform any application-level cleanup
	ReleaseObjects();

    return DXUTGetExitCode();
}

