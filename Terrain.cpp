/*--------------------------------------------------------------------------------

	Terrain.cpp

	Provides all functionality for Terrain generation/manipulation

  
	History:

	Created by Scott Wakeling

      15.01.02 +Tga save [greg zhebrakoff]
         17.01.02 +File Dialog [scott wakeling]
	  
	  15.01.02 +Blur function [greg zhebrakoff]
	
--------------------------------------------------------------------------------*/

//--------------
//	Includes
//--------------
#include "Terrain.h"
extern CLogFunc g_LogFunc;
extern HWND g_hWnd;

//--------------------------------------
//
//	CLASS: CFaultLine implementation
//
//--------------------------------------
CFaultLine::CFaultLine()
{
	vFaultBase.x = 0.f;
	vFaultBase.y = 0.f;
	vFaultBase.x = 0.f;

	vFaultEnd.x = 1.f;
	vFaultEnd.y = 1.f;
	vFaultEnd.x = 0.f;
}

CFaultLine::CFaultLine( CVector v1, CVector v2 )
{
	vFaultBase = v1;
	vFaultEnd = v2;
}

CFaultLine::~CFaultLine()
{

}

//----------------------------------------------------------------------------------
//	Test the locality of a point in relation to this fault line
//
//	If the return value < 0, the point is to the left of the fault line
//	If the return value > 0, the point is to the right of the fault line
//	If the return value == 0, the point is on the fault line itself
//----------------------------------------------------------------------------------
FLOAT CFaultLine::TestPoint( CVector vTest )
{
	CVector vPoint = vTest - vFaultBase;
			
	CVector vFaultCrossPoint = vPoint ^ ( vFaultEnd - vFaultBase );
	
	return vFaultCrossPoint.z;
}

//------------------------------------
//
//	CLASS: CTerrain implementation
//
//------------------------------------
CTerrain::CTerrain()
{
	m_iMaxHeight = 255;
	m_iMinHeight = 0;
	m_iTileSq = 256;
	
	memset( (void*)&m_lpstrFilename, 0, sizeof(TCHAR) * MAX_PATH );	
	sprintf( m_lpstrFilename, TEXT( "fractal01" ) );

	ClearGrid( m_iMinHeight + ( ( m_iMaxHeight - m_iMinHeight ) / 2 ) );
	
	//------------------------------------------------------------------------------
	//	Seed the random generator for when we're not using the logistic function
	//------------------------------------------------------------------------------
	srand( (unsigned)time( NULL ) );
}

CTerrain::~CTerrain()
{

}

//---------------------------------------
//	Clear the grid with a given value
//---------------------------------------
void CTerrain::ClearGrid( int iValue )
{
	for ( int iX = 0; iX < m_iTileSq; iX++ )
	{
		for ( int iY = 0; iY < m_iTileSq; iY++ )
		{
			m_abGrid[iX][iY] = iValue;
		}
	}

	InvalidateRect( NULL, NULL, TRUE );
}

//-------------------------
//	Grid value accessor
//-------------------------
BYTE& CTerrain::Grid( int iXPos, int iYPos )
{
	return m_abGrid[iXPos][iYPos];
}

//-----------------------------------------------------------------
//	Draw the terrain tile at the specified client coords
//
//	Pass -1 for both values if you wish the tile to be centered
//-----------------------------------------------------------------
void CTerrain::Draw( HWND hWnd, HDC hdc, int iClientX, int iClientY )
{
	//-----------------------------------
	//	Get current window dimensions
	//-----------------------------------
	RECT rectClient;
	
	GetClientRect( hWnd, &rectClient );

	INT iOriginX;
	INT iOriginY;

	if ( iClientX == -1 && iClientY == -1 )
	{
		//-------------------------------------------------------
		//	Draw the terrain tile in the centre of the window
		//-------------------------------------------------------
		iOriginX = ( ( rectClient.right - rectClient.left ) - m_iTileSq ) / 2;
		iOriginY = ( ( rectClient.bottom - rectClient.top ) - m_iTileSq ) / 2;
	}
	else
	{
		//----------------------------------------------------------
		//	Draw the terrain tile at the client coords specified
		//----------------------------------------------------------
		iOriginX = iClientX < 0 ? 0 : iClientX;
		iOriginY = iClientY < 0 ? 0 : iClientY;

		iOriginX = iClientX + m_iTileSq > ( rectClient.right - rectClient.left ) 
					? iClientX - ( ( iClientX + m_iTileSq ) - ( rectClient.right - rectClient.left ) ) : iOriginX;
		iOriginY = iClientY + m_iTileSq > ( rectClient.bottom - rectClient.top ) 
					? iClientY - ( ( iClientY + m_iTileSq ) - ( rectClient.bottom - rectClient.top ) ) : iOriginY;
	}

	//----------------------
	//	Draw the terrain
	//----------------------
	for ( INT iXidx = 0; iXidx < m_iTileSq; iXidx++ )
	{
		for ( INT iYidx = 0; iYidx < m_iTileSq; iYidx++ )
		{
			BYTE bGrey = m_abGrid[iXidx][iYidx];
			SetPixel( hdc, iXidx + iOriginX, iYidx + iOriginY, COLOUR( bGrey, bGrey, bGrey ) );
		}
	}

}

//----------------------------------------------------------
//	Picks a point along one length of the terrain tile
//
//	May use computer generated randoms, or those of a
//	logistic function
//----------------------------------------------------------
FLOAT CTerrain::PickPoint( CLogFunc* pLogFunc )
{
	FLOAT fResult;

	if ( pLogFunc != NULL )
	{
		fResult = pLogFunc->Iterate() * ((FLOAT)m_iTileSq - 1.f);
	}
	else
	{
		fResult = (FLOAT)(rand() % m_iTileSq);
	}

	return fResult;
}

//------------------------------------------------------------------------------------
//	Generate contents for the terrain tile with a fault line formation fractal
//
//	If iFixedFaultDepth != 0, each iteration will interpolate the fault depth
//	between iDepthInit and iDepthEnd, else it will use iFixedFaultDepth for
//	every fault line
//
//	bUseLogisticFunc	-	Use the logisitic function to generate random numbers
//------------------------------------------------------------------------------------
void CTerrain::GenerateFaultLines( int iIterations, int iDepthInit, int iDepthEnd, int iFixedFaultDepth, bool bUseLogisticFunc, bool bRetainAllValues, HWND hWnd )
{ 
	int iFaultDepth;
	double adRetainGrid[256][256];

	for ( int i = 0; i < 256; i++ )
	{
		for ( int j = 0; j < 256; j++ )
		{
			adRetainGrid[i][j] = (double)Grid(i,j);
		}
	}

	//------------------------------------------------------------------------------
	//	Seed the random generator for when we're not using the logistic function
	//------------------------------------------------------------------------------
	srand( (unsigned)time( NULL ) );

	for ( int iFaultIDX = 0; iFaultIDX < iIterations; iFaultIDX++ )
	{
		//-----------------------------------------------------
		//	Generate two points to describe this fault line
		//-----------------------------------------------------
		FLOAT x1, y1, x2, y2;

		if ( bUseLogisticFunc )
		{
			x1 = PickPoint( &g_LogFunc );
			y1 = PickPoint( &g_LogFunc );
			x2 = PickPoint( &g_LogFunc );
			y2 = PickPoint( &g_LogFunc );
		}
		else
		{
			x1 = PickPoint( NULL );
			y1 = PickPoint( NULL );
			x2 = PickPoint( NULL );
			y2 = PickPoint( NULL );
		}

		CFaultLine faultLine( CVector(x1, y1, 0.f), CVector(x2, y2, 0.f) );
		
		//----------------------------------------------------------------------------------
		//	Use a fixed fault depth, or, linearly interpolate between the desired values
		//----------------------------------------------------------------------------------
		iFaultDepth = iFixedFaultDepth != 0 ? iFixedFaultDepth : iDepthInit + ( (int)( (FLOAT)iFaultIDX / (FLOAT)iIterations ) * ( iDepthEnd - iDepthInit ) );

		for ( int iXPos = 0; iXPos < m_iTileSq; iXPos++ )
		{
			for ( int iYPos = 0; iYPos < m_iTileSq; iYPos++ )
			{
				CVector vGridLoc((FLOAT)iXPos, (FLOAT)iYPos, 0.f);

				if ( faultLine.TestPoint( vGridLoc ) < 0.f )
				{
					//----------------------------------------------------
					//	Grid position is to the left of the fault line
					//----------------------------------------------------
					if ( bRetainAllValues )
					{
						adRetainGrid[iXPos][iYPos] += (double)iFaultDepth;
					}
					else
					{
						if ( m_abGrid[iXPos][iYPos] + iFaultDepth < m_iMaxHeight )
						{
							m_abGrid[iXPos][iYPos] += iFaultDepth;
						}
					}
				}
				else
				{
					//---------------------------------------------------------------
					//	Grid position is to the right of the fault line, or on it
					//---------------------------------------------------------------
					if ( bRetainAllValues )
					{
						adRetainGrid[iXPos][iYPos] -= (double)iFaultDepth;
					}
					else
					{
						if ( m_abGrid[iXPos][iYPos] - iFaultDepth > m_iMinHeight )
						{
							m_abGrid[iXPos][iYPos] -= iFaultDepth;
						}
					}
				}
			}				
		}

		int iProgress = (int)(((FLOAT)iFaultIDX / (FLOAT)iIterations) * 100.f);

		SendMessage( GetDlgItem( hWnd, IDC_PROGRESS ), WM_USER+2, (WPARAM)iProgress, 0 );
	}

	//	If we retained all values, we need to quantize the retained value grid
	//	to fill our BYTE values
	//----------------------------------------------------------------------------
	if ( bRetainAllValues )
	{
		//	Get min and max retained values
		//-------------------------------------
		double dMIN = 65536;
		double dMAX = 0;
		double dRange, dRatio;
		for ( int iXPos = 0; iXPos < m_iTileSq; iXPos++ )
		{
			for ( int iYPos = 0; iYPos < m_iTileSq; iYPos++ )
			{
				if ( adRetainGrid[iXPos][iYPos] > 65536 )
				{
					exit(0);
				}

				if ( adRetainGrid[iXPos][iYPos] < dMIN )
				{
					dMIN = adRetainGrid[iXPos][iYPos];
				}
				if ( adRetainGrid[iXPos][iYPos] > dMAX )
				{
					dMAX = adRetainGrid[iXPos][iYPos];
				}
			}
		}
		
		dRange = dMAX - dMIN;
		dRatio = dRange / (double)(m_iMaxHeight - m_iMinHeight);

		for ( iXPos = 0; iXPos < m_iTileSq; iXPos++ )
		{
			for ( int iYPos = 0; iYPos < m_iTileSq; iYPos++ )
			{
				double dValue = (adRetainGrid[iXPos][iYPos] - dMIN) / dRatio;

				m_abGrid[iXPos][iYPos] = (BYTE)dValue;
			}
		}
	}

	InvalidateRect( NULL, NULL, TRUE );
}

//-----------------------------------------------------------------------
//	Calculates the fractal dimension for this terrain tile
//
//	Note:	Use as many elements from the results array as are needed
//-----------------------------------------------------------------------
FLOAT CTerrain::CalcFractalDimension()
{
	INT aiResults[32][2];
	int iResultIDX = 0;

	for ( int i = 0; i < 32; i++ )
	{
		aiResults[i][0] = aiResults[i][1] = -1;
	}

	INT iBBoxSq = m_iTileSq;

	//	Fit bouding boxes from the tile size itself, right down to 1
	//------------------------------------------------------------------
	while ( iBBoxSq >= 1 )
	{
		if ( iResultIDX == 0 )
		{
			//	The first bounding box needs one to bound the terrain tile,
			//	no more, no less
			//-----------------------------------------------------------------
			aiResults[iResultIDX][0] = iBBoxSq;
			aiResults[iResultIDX][1] = 1;
		}
		else
		{
			//	Calculate the number of cubes of dimension iBBoxSq units needed
			//	to entirely bound the terrain tile
			//---------------------------------------------------------------------
			int iEpsn = m_iTileSq / iBBoxSq;
						
			//	Count the number of blocks needed to cover each vertical extrusion
			//------------------------------------------------------------------------
			int iBlockCount = 0;

			for ( int iXPatch = 0; iXPatch < iEpsn; iXPatch++ )
			{
				for ( int iYPatch = 0; iYPatch < iEpsn; iYPatch++ )
				{
					int iHeightToReach = PatchMaxHeight( iXPatch * iBBoxSq, iBBoxSq, iYPatch * iBBoxSq , iBBoxSq );

					//	Conservative block count
					//------------------------------
					int iStackSize = ( iHeightToReach + 1 ) / iBBoxSq;
					iBlockCount += iStackSize;
					
					// If this many blocks don't clear the max height, add one more
					//-----------------------------------------------------------------
					if ( iStackSize * iBBoxSq < ( iHeightToReach + 1 ) )
					{
						iBlockCount++;
					}
				}
			}

			aiResults[iResultIDX][0] = iBBoxSq;
			aiResults[iResultIDX][1] = iBlockCount;
		}

		iBBoxSq /= 2;
		iResultIDX++;	
	}

	//	Use results to calculate the fractal dimension of this terrain tile
	//-------------------------------------------------------------------------
	FLOAT afResults[32][2];
	
	for ( int iOutput = 0; iOutput < iResultIDX; iOutput++ )
	{
		FLOAT fLogOneOverCubeDim = (FLOAT)log10( (double)(1.f / (FLOAT)aiResults[iOutput][0]) );
		FLOAT fLogNumCubes = (FLOAT)log10( (double)aiResults[iOutput][1] );

		afResults[iOutput][0] = fLogOneOverCubeDim;		// log( 1 / cube dim. )
		afResults[iOutput][1] = fLogNumCubes;			// log( no. of cubes )
	}

	//	Approximate gradient for a quick display
	//
	//	Note: Should only be used for display of extreme results, a proper
	//	stats package should (and was) used for data analysis
	//------------------------------------------------------------------------
	FLOAT fFracDim;

	//	(y2-y1)/(x2-x1)
	//---------------------
	fFracDim = (FLOAT)(afResults[1][1] - afResults[0][1]) / (FLOAT)(afResults[1][0] - afResults[0][0]);

	for ( iOutput = 1; iOutput < iResultIDX - 1; iOutput++ )
	{
		FLOAT fThisGradient = (FLOAT)(afResults[iOutput+1][1] - afResults[iOutput][1]) / (FLOAT)(afResults[iOutput+1][0] - afResults[iOutput][0]);

		fFracDim = (fFracDim + fThisGradient) / 2;
	}

	return fFracDim;
}

//-------------------------------------------------------------------------------------
//	Returns the greatest height encountered in the given patch of the terrain tile
//
//	Note:	Provide params as inclusive values
//-------------------------------------------------------------------------------------
INT CTerrain::PatchMaxHeight( int iStartX, int iWidth, int iStartY, int iHeight )
{
	INT iMax = 0;

	for ( int iX = iStartX; iX < iStartX + iWidth; iX++ )
	{
		for ( int iY = iStartY; iY < iStartY + iHeight; iY++ )
		{
			iMax = Grid( iX, iY ) > (BYTE)iMax ? Grid( iX, iY ) : iMax;
		}	
	}
	
	return iMax;
}

FLOAT CTerrain::GetAvgHeight()
{
	int iHeight = Grid(0,0);
	
	for ( int iX = 0; iX < 256; iX++ )
	{
		for ( int iY = 0; iY < 256; iY++ )
		{
			iHeight = (INT)((FLOAT)( iHeight + Grid( iX, iY ) ) / 2.f);
		}
	}
	
	return (FLOAT)iHeight;
}

int& CTerrain::MaxHeight()
{
	return m_iMaxHeight;
}

int& CTerrain::MinHeight()
{
	return m_iMinHeight;
}

void CTerrain::SetFilename( LPSTR szNewFilename )
{
	strcpy( &m_lpstrFilename[0], szNewFilename );
}

LPSTR CTerrain::GetFilename()
{
	return m_lpstrFilename;
}

void CTerrain::Save()
{
	FILE* file;
	int width = GRID_X;
	int height = GRID_Y;
	TCHAR acBuffer[MAX_PATH];

	BYTE head[18]=         // header of tga file 
	{
      0,                   // id length
      0,                   // colormap type
	  2,                   // image type     (2=Uncompressed RGB - no colormap)
      0,                   // colormap index [1/2 bytes]
      0,                   // colormap index [2/2 bytes]
	  0,                   // colormap length [1/2 bytes]
	  0,                   // colormap length [2/2 bytes]
	  0,                   // colormap size
	  0,                   // x origin [1/2 bytes]
	  0,                   // x origin [2/2 bytes]
	  0,                   // y origin [1/2 bytes]
	  0,                   // y origin [2/2 bytes]
      width%256,           // width [1/2]
	  (width>>8)%256,      // width [2/2]
      height%256,          // height [1/2]
	  (height>>8)%256,     // height [2/2]
      24,                  // pixel size
	  0,                   // attrib. [alway 0 for 24bit]
	};
	
	if ((file = fopen(m_lpstrFilename, "wb"))==NULL) MessageBox(NULL,"Failed","TGA",ERROR);                                    

    fwrite(&head,18,1,file);
	
	for (int y=height-1;y>=0;y--)
		for (int x=0;x<width;x++)
		{
		  fwrite(&m_abGrid[x][y],sizeof(BYTE),1,file);			// as Red		
		  fwrite(&m_abGrid[x][y],sizeof(BYTE),1,file);			// as Green		
		  fwrite(&m_abGrid[x][y],sizeof(BYTE),1,file);			// as Blue		
		}

	sprintf( acBuffer, "Fractal Terrain Generator - [%s]", m_lpstrFilename );
	SetWindowText( g_hWnd, acBuffer );

	fclose( file );
}

void CTerrain::Blur( int iBlurFactor )
{
	unsigned int i, j;

	for ( int k = 0; k < iBlurFactor; k++ )
	{
		// Horizontal
		for ( j = 0; j < GRID_Y; j++ )
		{
			for ( i = 1; i < GRID_X-1; i += 2 )
			{
				m_abGrid[i][j]=( m_abGrid[i-1][j] + m_abGrid[i + 1][j] ) / 2;
			}
		}

		// Vertical
		for ( j = 1; j < GRID_Y-1; j += 2 )
		{
			for ( i = 0; i < GRID_X; i++ )
			{
				m_abGrid[i][j] = ( m_abGrid[i][j - 1] + m_abGrid[i][j + 1] ) / 2;
			}
		}

		// Horizontal + 1
		for ( j = 0; j < GRID_Y; j++ )
		{
			for ( i = 2; i < GRID_X; i += 2 )
			{
				m_abGrid[i][j] = ( m_abGrid[i - 1][j] + m_abGrid[i + 1][j] ) / 2;
			}
		}

		// Vertical + 1
		for ( j = 2; j < GRID_Y; j += 2 )
		{
			for ( i = 0;i < GRID_X; i++ )
			{
				m_abGrid[i][j] = ( m_abGrid[i][j - 1] + m_abGrid[i][j + 1] ) / 2;
			}
		}
	}

	InvalidateRect( NULL, NULL, TRUE );
}

//------------------------------------
//
//	CLASS: CLogFunc implementation
//
//------------------------------------
CLogFunc::CLogFunc()
{
	m_fM = 4.f;
	m_fLastIterate = m_fSeed = 0.5f;
}

CLogFunc::CLogFunc( float fM, float fSeed )
{
	m_fM = fM;
	m_fLastIterate = m_fSeed = fSeed;
}

CLogFunc::~CLogFunc()
{
}

//----------------------------------------------------------------------
//	Return the next iterate from this logistic function
//
//	Note: If this function was created with a default constructor it
//	will default to chaotic behaviour, seeded by 0.5f
//----------------------------------------------------------------------
float CLogFunc::Iterate()
{
	return m_fLastIterate = (m_fM * m_fLastIterate) * (1.f - m_fLastIterate );
}

void CLogFunc::Reset()
{
	m_fLastIterate = m_fSeed;
}

float& CLogFunc::Seed()
{
	return m_fSeed;
}