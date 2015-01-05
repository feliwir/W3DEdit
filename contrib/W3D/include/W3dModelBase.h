// ************************************************************************
// Project: General Tools & Utilities
// ------------------------------------------------------------------------
// Purpose: Defines general methods and structures for W3D-models
// Module : W3dModelBase.h
// Author : Level_3D_Vision (C) 2008-2012
// System : ANSI C/C++ - MinGW32 compiler
// ------------------------------------------------------------------------
// History: 03.11.09 -- created by Jerry Brutzki
//          04.04.11 -- changed by Jerry Brutzki
// ************************************************************************
#ifndef CW3DMODELBASE_INCLUDED_H
#define CW3DMODELBASE_INCLUDED_H
// ----------------------------------------------------------- Includes ---
// ------------------------------------------------------------------------
#include "W3dModelFile.h"                      //common W3dModel definition
#include <stdlib.h>                            //common function definition
#include <stdio.h>                             //common file-IO definitions
#include <string.h>                            //common string  definitions

// -------------------------------------------------------- Definitions ---
// ------------------------------------------------------------------------
// ------------------------------------------------------ CW3DModelBase ---
class CW3DModelBase
{
public:
   CW3DModelBase() {};
   virtual ~CW3DModelBase() {};
   bool   OpenModelFile (char*);
   bool   CloseModelFile();

protected:
   enum  {MAX_NAME=256,MAX_RECORD=1,MAX_OUTPUT=128};
   enum  {MAX_DATA=1024,MAX_LONG=4,MAX_CHUNK=8};

   void   ReadDataChunk(const long);
   void   ReadDataChunk();

   //Basic W3D data structure (W3dModelFile)
   W3dChunkHeader   m_FileData;
   union uNumber
   {
     unsigned char  Chunk[MAX_LONG];
     unsigned long  Value;
   } m_Number;

   unsigned char    m_UserData[MAX_DATA];
   char             m_DataFile[MAX_NAME + 2];
   char             m_showData[MAX_OUTPUT + 2];
   unsigned long    m_InputData;
   unsigned long    m_InputSize;
   unsigned long    m_Count;
   char*            m_UserPath;
   FILE*            m_Reader;
   long             m_FileSize;
   long             m_Offset;
};

#endif // CW3DMODELBASE_INCLUDED_H
// ************************************************************************
// Module:  W3dModelBase.h -- End of file
// ************************************************************************

