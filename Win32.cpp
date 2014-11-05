/*--------------------------------------------------------------------------------

	Win32.cpp

	Provides all Win32 configuration/management functionality

  
	History:

	Created by Scott Wakeling

--------------------------------------------------------------------------------*/

//--------------
//	Includes
//--------------
#include <stdio.h>
#include <windows.h>

#include "resource.h"
#include "Terrain.h"

//-------------
//	Globals
//-------------
char szWinName[] = "CHAOSWindow";
HINSTANCE g_hGlobalInstance;
HWND g_hWnd;
CTerrain terrTile;
CLogFunc g_LogFunc;

//-----------------
//	Definitions
//-----------------
LRESULT CALLBACK WindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
BOOL FAR PASCAL FaultLineDialog( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
BOOL FAR PASCAL SetGridDialog( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
int ProcMenuEvent( HWND hWnd, WPARAM wParam, LPARAM lParam );
int ProcKeyEvent( HWND hWnd, WPARAM wParam, LPARAM lParam );
int ProcMouseEvent( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

//---------------------------------------------------------------
//	Main entry point for the application
//
//	Handles Win32 configuration and initialises the generator
//---------------------------------------------------------------
int WINAPI WinMain(	HINSTANCE hThisAppInstance, 
					HINSTANCE hPreviousAppInstance, 
					LPSTR lpszCmdArguments, 
					int iWindowShowState )
{
	MSG message;
	WNDCLASSEX wndClass;
	g_hGlobalInstance = hThisAppInstance;
	TCHAR acBuffer[MAX_PATH];
	
	wndClass.cbSize			= sizeof(WNDCLASSEX);
	wndClass.hInstance		= g_hGlobalInstance;
	wndClass.lpszClassName	= szWinName;
	wndClass.lpfnWndProc	= WindowProc;
	wndClass.style			= CS_HREDRAW | CS_VREDRAW;
	wndClass.hIcon			= LoadIcon( NULL, IDI_APPLICATION );
	wndClass.hIconSm		= LoadIcon( NULL, IDI_WINLOGO );
	wndClass.hCursor		= LoadCursor( NULL, IDC_ARROW );
	wndClass.lpszMenuName	= MAKEINTRESOURCE( CHAOSMENU );
	wndClass.cbClsExtra		= 0;
	wndClass.cbWndExtra		= 0;
	wndClass.hbrBackground	= (HBRUSH)GetStockObject( WHITE_BRUSH );

	if ( !RegisterClassEx( &wndClass ) )
	{
		return 0;
	}

	sprintf( acBuffer, "Fractal Terrain Generator - [%s]", terrTile.GetFilename() );

	g_hWnd = CreateWindow( szWinName,
							acBuffer,
							WS_OVERLAPPEDWINDOW,
							CW_USEDEFAULT, CW_USEDEFAULT,
							800, 600,
							HWND_DESKTOP,
							NULL,
							g_hGlobalInstance,
							NULL );
	
	ShowWindow( g_hWnd, iWindowShowState );
	UpdateWindow( g_hWnd );

	while ( GetMessage( &message, NULL, 0, 0 ) )
	{
		TranslateMessage( &message );
		DispatchMessage( &message );
	}

	return message.wParam;
}


LRESULT CALLBACK WindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	static HDC	hdc;
	PAINTSTRUCT	ps;

	switch( message )
	{
		case WM_PAINT:
			hdc = BeginPaint( hWnd, &ps );
			terrTile.Draw( hWnd, hdc, -1, -1 );
			EndPaint( hWnd, &ps );
		break;

		case WM_COMMAND:
			ProcMenuEvent( hWnd, wParam, lParam );
		break;

		case WM_KEYDOWN:
			ProcKeyEvent( hWnd, wParam, lParam );
		break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:

		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_RBUTTONDBLCLK:

		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MBUTTONDBLCLK:

		case WM_MOUSEMOVE:
			ProcMouseEvent( hWnd, message, wParam, lParam );
		break;

		case WM_DESTROY:
			PostQuitMessage( 0 );
		break;
		
		case WM_SIZE:
			InvalidateRect( hWnd, NULL, TRUE );
		break;

		default:

		return DefWindowProc( hWnd, message, wParam, lParam );
	}

	return 0;
}

//----------------------------------------
//	Process any meny events that occur
//----------------------------------------
int ProcMenuEvent( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
	switch( LOWORD( wParam ) )
	{
		//-----------------------
		//	File menu options
		//-----------------------
		case CHAOS_FILE_EXIT:
		{
			PostQuitMessage( 0 );
		}
		break;

        case CHAOS_FILE_SAVE:
		{
			terrTile.Save();
		}
		break;

		case CHAOS_FILE_SAVEAS:
		{
            TCHAR szFilenameBuffer[MAX_PATH];

			OPENFILENAME ofn;

			ofn.lStructSize			= sizeof(OPENFILENAME);
			ofn.hwndOwner			= hWnd;
			ofn.hInstance			= NULL;
			ofn.lpstrFilter			= TEXT( "*.tga" );
			ofn.lpstrCustomFilter	= NULL;
			ofn.nMaxCustFilter		= 0;
			ofn.nFilterIndex		= 0;
			ofn.nMaxFile			= MAX_PATH;
			ofn.nMaxFileTitle		= MAX_PATH;
			ofn.lpstrInitialDir		= NULL;
			ofn.lpstrTitle			= TEXT( "Save As" );
			ofn.nFileOffset			= 0;
			ofn.nFileExtension		= 0;
			ofn.lpstrDefExt			= TEXT ( "tga" );
			ofn.lCustData			= 0L;
			ofn.lpfnHook			= NULL;
			ofn.lpTemplateName		= NULL;
			ofn.lpstrFile			= NULL;
			ofn.lpstrFileTitle		= &szFilenameBuffer[0];
			ofn.Flags				= OFN_OVERWRITEPROMPT;
			
			if ( GetSaveFileName( &ofn ) )
			{
				TCHAR szFilename[MAX_PATH];

				if ( strstr( ofn.lpstrFileTitle, ".tga" ) != NULL )
				{
					sprintf( szFilename, "%s", ofn.lpstrFileTitle );
				}
				else
				{
					sprintf( szFilename, "%s%s%s", ofn.lpstrFileTitle, TEXT("."), ofn.lpstrDefExt );
				}
				
				terrTile.SetFilename( szFilename );
				terrTile.Save();
			}
		}
		break;

		//--------------------------
		//	Terrain menu options
		//--------------------------
		case CHAOS_TERRAIN_REFRESH:
		{
			InvalidateRect( hWnd, NULL, TRUE );
		}
		break;

		case CHAOS_TERRAIN_FRACDIM:
		{
			char acBuffer[MAX_PATH];
						
			FLOAT fDimension = terrTile.CalcFractalDimension();

			sprintf( acBuffer, "Fractal Dimension: %.3f", fDimension );
			MessageBox( hWnd, acBuffer, "", MB_OK );
		}
		break;

		case CHAOS_TERRAIN_SETGRID:
		{
			DialogBox( g_hGlobalInstance, MAKEINTRESOURCE(IDD_SETGRID), hWnd, (DLGPROC)SetGridDialog );
		}
		break;

		case CHAOS_TERRAIN_FAULTFORMATION:
		{
			DialogBox( g_hGlobalInstance, MAKEINTRESOURCE(IDD_FAULTDLG), hWnd, (DLGPROC)FaultLineDialog );
		}
		break;
		
		case CHAOS_TERRAIN_BLUR:
		{
            terrTile.Blur( 1 );
		}
		break;

		case CHAOS_TERRAIN_BLURMORE:
		{
            terrTile.Blur( 4 );
		}
		break;
	}
	
	return 1;
}

//--------------------------------------------------------
//	Request settings for fault line terrain generation
//--------------------------------------------------------
BOOL FAR PASCAL FaultLineDialog( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			SendDlgItemMessage( hWnd, IDC_ITERATION_NUM, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)"512" );
			SendDlgItemMessage( hWnd, IDC_FAULTDEPTH_START, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)"10" );
			SendDlgItemMessage( hWnd, IDC_FAULTDEPTH_FINISH, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)"1" );
		}
		break;

		case WM_COMMAND:
			switch(LOWORD(wParam)) 
			{
				case IDOK:
				{
					char acBuffer[64];
					int iIterations, iFaultDepthStart, iFaultDepthFinish, iFixedFaultDepth, iNumChars;
					bool bUseLogisticFunc, bRetainAllValues;

					//------------------------------
					//	Get number of iterations
					//------------------------------
					iNumChars = SendDlgItemMessage( hWnd, IDC_ITERATION_NUM, WM_GETTEXTLENGTH, 0, 0 );
					SendDlgItemMessage( hWnd, IDC_ITERATION_NUM, WM_GETTEXT, 62, (LPARAM)(LPCTSTR)acBuffer );
					iIterations = atoi( &acBuffer[0] );

					//---------------------------
					//	Get fault depth start
					//---------------------------
					iNumChars = SendDlgItemMessage( hWnd, IDC_FAULTDEPTH_START, WM_GETTEXTLENGTH, 0, 0 );
					SendDlgItemMessage( hWnd, IDC_FAULTDEPTH_START, WM_GETTEXT, 62, (LPARAM)(LPCTSTR)acBuffer );
					iFaultDepthStart = atoi( &acBuffer[0] );

					//----------------------------
					//	Get fault depth finish
					//----------------------------
					iNumChars = SendDlgItemMessage( hWnd, IDC_FAULTDEPTH_FINISH, WM_GETTEXTLENGTH, 0, 0 );
					SendDlgItemMessage( hWnd, IDC_FAULTDEPTH_FINISH, WM_GETTEXT, 62, (LPARAM)(LPCTSTR)acBuffer );
					iFaultDepthFinish = atoi( &acBuffer[0] );

					//-----------------------------------------------------------
					//	Determine whether to interpolate the fault line depth
					//-----------------------------------------------------------
					if ( IsDlgButtonChecked( hWnd, IDC_CHK_FIXEDFAULTDEPTH ) == BST_CHECKED )
					{
						iFixedFaultDepth = 0;
					}
					else
					{
						iFixedFaultDepth = iFaultDepthStart;
					}

					bUseLogisticFunc = IsDlgButtonChecked( hWnd, IDC_CHK_USE_LOG_FUNC ) == BST_CHECKED ? true : false;
					
					if ( bUseLogisticFunc )
					{
						if ( IsDlgButtonChecked( hWnd, IDC_CHK_RANDOM_SEED ) == BST_CHECKED )
						{
							g_LogFunc.Seed() = terrTile.GetAvgHeight() / terrTile.MaxHeight();
							g_LogFunc.Reset();
						}
						else
						{
							FLOAT fMax = RAND_MAX;
							FLOAT fRandomSeed = (FLOAT)rand() / fMax;
							g_LogFunc.Seed() = fRandomSeed;
							g_LogFunc.Reset();
						}
					}
					
					bRetainAllValues = IsDlgButtonChecked( hWnd, IDC_CHK_RETAINALL ) == BST_CHECKED ? true : false;

					terrTile.GenerateFaultLines( iIterations, iFaultDepthStart, iFaultDepthFinish, iFixedFaultDepth, bUseLogisticFunc, bRetainAllValues, hWnd );
					EndDialog( hWnd, TRUE );
				}
				break;
				
				case IDCANCEL:      
				{
					EndDialog( hWnd, TRUE );
				}
				break;

				case IDC_CHK_USE_LOG_FUNC:
				{
					if ( IsDlgButtonChecked( hWnd, IDC_CHK_USE_LOG_FUNC ) == BST_CHECKED )
					{
						//	Enable the "seed from height field" check box
						//---------------------------------------------------
						EnableWindow( GetDlgItem( hWnd, IDC_CHK_RANDOM_SEED ), TRUE );
					}
					else
					{
						//	Disable the "seed from height field" check box
						//----------------------------------------------------
						EnableWindow( GetDlgItem( hWnd, IDC_CHK_RANDOM_SEED ), FALSE );
					}
				}
				break;

				default:
				break;
			}

		default:
		break;
	}

	return FALSE;
}

//---------------------------------------
//	Set the grid to a specified value
//---------------------------------------
BOOL FAR PASCAL SetGridDialog( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			SendDlgItemMessage( hWnd, IDC_GRIDVALUE, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)"0" );
		}
		break;

		case WM_COMMAND:
			switch(LOWORD(wParam)) 
			{
				case IDOK:
				{
					char acBuffer[64];
					int iGridValue, iNumChars;
					
					//------------------------------
					//	Get required grid value
					//------------------------------
					iNumChars = SendDlgItemMessage( hWnd, IDC_GRIDVALUE, WM_GETTEXTLENGTH, 0, 0 );
					SendDlgItemMessage( hWnd, IDC_GRIDVALUE, WM_GETTEXT, 62, (LPARAM)(LPCTSTR)acBuffer );
					iGridValue = atoi( &acBuffer[0] );
					
					terrTile.ClearGrid( iGridValue );
					
					EndDialog( hWnd, TRUE );
				}
				break;
				
				case IDCANCEL:      
				{
					EndDialog( hWnd, TRUE );
				}
				break;

				

				break;
			}
		
		default:
		{
		}
		break;
	}

	return FALSE;	
}

//---------------------------------------
//	Process any key events that occur
//---------------------------------------
int ProcKeyEvent( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
	//---------------------------
	// Retrieve key scan code
	//---------------------------
	switch( (int)wParam )
	{
		case VK_UP:
			
		break;

		case VK_LEFT:
			
		break;

		case VK_DOWN:
			
		break;

		case VK_RIGHT:
			
		break;

		default:
		break;
	}

	return 1;
}

//-----------------------------------------
//	Process any mouse events that occur
//-----------------------------------------
int ProcMouseEvent( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	unsigned int iXPos, iYPos;

	//---------------------------
	//	Retrieve mouse coords
	//---------------------------
	switch( message )
	{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:

		case WM_MOUSEMOVE:
			iXPos = LOWORD( lParam );
			iYPos = HIWORD( lParam );
		break;

		default:
		break;
	}

	//-------------------
	//	Process event
	//-------------------
	switch( message )
	{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MOUSEMOVE:

		default:
		break;
	}

	return 1;
}
