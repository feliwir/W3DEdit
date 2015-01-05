// ************************************************************************
// Project: General Tools & Utilities
// ------------------------------------------------------------------------
// Purpose: provides general methods and structures for W3D-models
// Module : W3dModelBase.cpp
// Author : Level_3D_Vision (C) 2008-2012
// System : ANSI C/C++ - MinGW32 compiler
// ------------------------------------------------------------------------
// History: 03.11.09 -- created by Jerry Brutzki
// ************************************************************************
// ----------------------------------------------------------- Includes ---
// ------------------------------------------------------------------------
#include "W3dModelBase.h"                      //common W3dModel definition
#include <stdlib.h>                            //common function definition

// ----------------------------------------------------- Implementation ---
// ------------------------------------------------------------------------
bool CW3DModelBase::OpenModelFile (char* modelFile)
// ------------------------------------------------------------------------
{
   m_FileSize = 0;  m_Offset = 0;
   if (strlen(modelFile) > 0) {
      m_Reader = fopen(modelFile,"rb");
   }

   if (m_Reader != 0) {
      strncpy(m_DataFile,modelFile,MAX_NAME);
      fseek(m_Reader,0, SEEK_END);
      m_FileSize = ftell(m_Reader);
      fseek(m_Reader,0, SEEK_SET);
   }

   return (m_FileSize != 0) ? true : false;
}

// ------------------------------------------------------------------------
bool CW3DModelBase::CloseModelFile ()
// ------------------------------------------------------------------------
{
   if (m_Reader != 0) {
      fclose(m_Reader); m_Reader = 0;
      return true;
   }

   return false;
}

// ------------------------------------------------------------------------
void CW3DModelBase::ReadDataChunk ()
// ------------------------------------------------------------------------
{
   memset(&m_FileData,0,sizeof(W3dChunkHeader));
   fread (&m_FileData,MAX_CHUNK,MAX_RECORD,m_Reader);

   //NOTE: This eliminates the left-most 0x80 value ....
   m_FileData.ChunkSize &= 0x7FFFFFFF;
}

// ------------------------------------------------------------------------
void CW3DModelBase::ReadDataChunk (const long dataSize)
// ------------------------------------------------------------------------
{
   unsigned char* UserData = 0;
   if (dataSize > 0) {
      UserData = new unsigned char[dataSize];
      fread(UserData,dataSize,MAX_RECORD,m_Reader);
      delete UserData;
   }
}

// ************************************************************************
// Module:  W3dModelBase.cpp -- End of File
// ************************************************************************

