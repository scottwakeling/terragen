/*--------------------------------------------------------------------------------

	Terrain.h

	Provides all definitions for Terrain generation/manipulation

  
	History:

	Created by Scott Wakeling

--------------------------------------------------------------------------------*/

#ifndef _TERRAIN_H
#define _TERRAIN_H

//-------------
//	Includes
//-------------
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#include "resource.h"

//-----------------
//	Definitions
//-----------------
#define COLOUR(r,g,b) ((COLORREF)((((0)&0xff)<<24)|(((b)&0xff)<<16)|(((g)&0xff)<<8)|((r)&0xff)))
#define GRID_X 256
#define GRID_Y 256
//----------------------------------
//	A simple mathematical vector
//----------------------------------
class CVector
{
public:
	FLOAT x, y, z;

	//----------------------------------
	//	Construction and Destruction
	//----------------------------------
	CVector() : x(0.0),
				y(0.0),
				z(0,0)
	{
	};
	CVector( FLOAT fInx, FLOAT fIny, FLOAT fInz ) : x(fInx),
													y(fIny),
													z(fInz)
	{
	};
	
	//-----------------------
	//	CVector Interface
	//-----------------------
	//...

	//------------------------------------------------------------------------------
	//	Operators
	//------------------------------------------------------------------------------
		
	//	Cross
	CVector operator^( const CVector& vec ) const
	{
		return CVector( (y * vec.z - z * vec.y), (z * vec.x - x * vec.z), (x * vec.y - y * vec.x) );
	}

	//	Subtraction
	CVector operator-( const CVector vec ) const
	{
		return CVector( x - vec.x, y - vec.y, z - vec.z );
	}

};

//--------------------------------------------------------------------
//	A fault line class for use in the fault line generation fractal
//--------------------------------------------------------------------
class CFaultLine
{
public:
	//----------------------------------
	//	Construction and Destruction
	//----------------------------------
	CFaultLine();
	CFaultLine( CVector v1, CVector v2 );
	virtual ~CFaultLine();

	//--------------------------
	//	CFaultLine Interface
	//--------------------------
	FLOAT TestPoint( CVector vTest );

private:
	CVector vFaultBase;
	CVector vFaultEnd;
};

//----------------------------------------------------------------------
//	A wrapper class for deriving iterates from the logistic function
//----------------------------------------------------------------------
class CLogFunc
{
public:
	//----------------------------------
	//	Construction and Destruction
	//----------------------------------
	CLogFunc();
	CLogFunc( float fM, float fSeed );
	virtual ~CLogFunc();

	//------------------------
	//	CLogFunc Interface
	//------------------------
	float& Seed();
	float& M();

	float Iterate();
	void Reset();
	
private:
	float m_fM;
	float m_fSeed;
	float m_fLastIterate;
};

//--------------------------------------------------------------------------------------
//	A terrain tile, used for generating the heightmaps and interrogating the results
//--------------------------------------------------------------------------------------
class CTerrain
{
public:
	//----------------------------------
	//	Construction and Destruction
	//----------------------------------
	CTerrain();
	virtual ~CTerrain();

	//------------------------
	//	CTerrain Interface
	//------------------------
	BYTE& Grid( int iXPos, int iYPos );
	int& MaxHeight();
	int& MinHeight();

	void ClearGrid( int iValue );
	void Draw( HWND hWnd, HDC hdc, int iClientX, int iClientY );
	FLOAT PickPoint( CLogFunc* pLogFunc );
	void GenerateFaultLines( int iIterations, int iDepthInit, int iDepthEnd, int iFixedFaultDepth, bool bUseLogisticFunc, bool bRetainAllValues, HWND hWnd );
	FLOAT CalcFractalDimension();
	INT PatchMaxHeight( int iStartX, int iWidth, int iStartY, int iHeight );
	FLOAT GetAvgHeight();
	
	void SetFilename( LPSTR szNewFilename );
	LPSTR GetFilename();

	void Save();
	void Blur( int iBlurFactor );

private:
	int m_iMaxHeight;
	int m_iMinHeight;
	int m_iTileSq;
	BYTE m_abGrid[256][256];
	TCHAR m_lpstrFilename[MAX_PATH];
};

#endif