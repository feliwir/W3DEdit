// ************************************************************************
// Project: General Tools & Utilities
// ------------------------------------------------------------------------
// Purpose: defines methods and structures for reading W3D-model meshes
// Module : W3dModelAnim.h
// Author : Level_3D_Vision (C) 2008-2012
// System : ANSI C/C++ - MinGW32 compiler
// ------------------------------------------------------------------------
// History: 13.04.10 -- created by Jerry Brutzki
//          27.12.11 -- changed by Jerry Brutzki
// ************************************************************************
#ifndef CW3DMODELANIM_INCLUDED_H
#define CW3DMODELANIM_INCLUDED_H
// ----------------------------------------------------------- Includes ---
// ------------------------------------------------------------------------
#include "W3dModelBase.h"                      //common W3dModel definition
#include <stdlib.h>                            //common function definition
#include <vector>                              //common vector STLcontainer
using namespace std;

// -------------------------------------------------------- Definitions ---
// ------------------------------------------------------------------------
// ------------------------------------------------------ CW3DModelAnim ---
class CW3DModelAnim : public CW3DModelBase
{
public:
   CW3DModelAnim() {};
  ~CW3DModelAnim() {};

   void  ContentDelete ();
   void  ContentParser ();
   void  GetHeaderData (char*);
   void  GetFramesData (char*,const int);
   void  GetMatrixData (char*,const int);
   void  GetPivotsHead (char*,const int);
   void  GetPivotsData (char*,const int);
   bool  GetPivotsName (char*);
   void  SetHeaderData (const char*,const char*);
   void  SetFramesData (const char*,int);
   bool  SaveAnimOutput(const char*);
   void  ClrExportList () { m_ExportAnimList.clear(); };
   int   GetPivotsSize () { return m_PivotsSize; };
   int   GetFramesSize ();
   int   GetMatrixSize ();
   bool  IsTypeTimeFrames();
   bool  IsTypeCompressed();

private:
   void  LoadW3dHierarchyStruct(int,unsigned long);
   void  LoadW3dAnimationStruct(unsigned long);
   void  LoadW3dCompAnimationStruct(unsigned long);
   int   LoadW3dCompAnimationValues();
   int   LoadW3dCompAnimationBitVal();
   int   LoadW3dTimeAnimationFrames();
   void  ObtainPivotContent();
   void  SaveInfoSkelHeader();
   void  SaveInfoPivotArray();
   bool  SaveDataFrameAnims();
   bool  SaveUserFrameAnims();
   bool  SaveDataModelAnims();
   bool  SaveDataModelBones();
   bool  SaveUserModelAnims();
   //void  SaveSkelModelAnims();

   void  CreateVectorMatrix();
   void  CreateCFrameMatrix();
   void  OutputVectorMatrix();
   void  OutputCFrameMatrix();
   void  OutputAnimChannel (int);
   void  OutputBitsChannel (int);
   void  UpdateHeaderValue (int);
   int   IdentifyPivotIndex(int);

private:
   CW3DModelAnim(const CW3DModelAnim&);
   enum  {ANIM_CHANNEL=1,BIT_CHANNEL=2,MAX_LENGTH=4,MAX_DATA=128};
   enum  {ANIM_UNKNOWN=0,ANIM_TIMEFRAMES=1,ANIM_COMPRESSED=2};
   enum  {SIZE_LONG=4,ANIM_OFFSET=4,MAX_ARRAY=2,MAX_TEXT=16};
   enum  {TYPE_SKEL=1,TYPE_MESH=2,TYPE_ANIM=3,TYPE_LONG=4};
   enum  {NAME_OFFSET=20,MAX_LINE=512,NEXT_ITEM=4};

   //---------------------------------------------------------
   //DATA: layout of W3D mesh + animation structures
   //---------------------------------------------------------
   W3dHierarchyStruct    m_Skeleton[MAX_ARRAY];
   W3dPivotStruct        m_Pivots;
   std::vector<W3dPivotStruct>    m_PivotsList[MAX_ARRAY];

   //NOTE: Structures for TimeFrame animation
   W3dAnimHeaderStruct   m_AnimHeader;
   W3dAnimChannelStruct  m_AnimChannel;
   W3dBitChannelStruct   m_BitChannel;
   std::vector<W3dAnimChannelStruct>  m_AnimChannelList[MAX_ARRAY];
   std::vector<W3dBitChannelStruct>   m_BitChannelList [MAX_ARRAY];

   //NOTE: Animation flavor value is ANIM_FLAVOR_TIMECODED = 0
   W3dCompressedAnimHeaderStruct  m_CompAnimHeader;
   struct sTimeCodesData
   {
     uint32    NumTimeCodes;    // number of time coded entries
     uint16    Pivot;           // pivot affected by this channel
     uint8     VectorFlag;      // length of each vector or channel type
     uint8     Channel;         // channel type XYZQ or default state.
   } m_TimeCodesData;
   std::vector<sTimeCodesData>  m_TimeCodesAnimList;

   struct sTimeCodesInfo
   {
     long  Offset;
     long  Size;
     int   Type;
     int   Index[2];
   } m_TimeCodesInfo;
   std::vector<sTimeCodesInfo>  m_TimeCodesInfoList;

   struct sTimeCodes
   {
     unsigned long Number;
     unsigned long Vector[MAX_LENGTH];
   } m_TimeCodes;
   std::vector<sTimeCodes>      m_TimeCodesList;

   struct sTimeFrames
   {
     unsigned long Value;
   } m_TimeFrames;
   std::vector<sTimeFrames>     m_TimeFrameList;

   struct sVectorMatrix
   {
     char Frame[6];
     char Value[MAX_LINE + 2];
   } m_VectorMatrix;
   std::vector<sVectorMatrix>   m_VectorMatrixList;

   struct sPivotsHead
   {
     char  Name[MAX_TEXT];
     long  PivotCount;
     long  DataSize;
     long  Offset;
   } m_PivotsHead;

   struct sPivotsInfo
   {
     char  Name[MAX_TEXT];
     long  ParentIdx;
     int   BoneIndex;
   } m_PivotsInfo;

   std::vector<sPivotsHead>  m_PivotsHeadList;
   std::vector<sPivotsInfo>  m_PivotsInfoList;
   std::vector<int>          m_ExportAnimList;

   char    m_ModelAnim[MAX_PATH + 2];
   char    m_PivotFile[MAX_PATH + 2];
   char    m_PivotName[MAX_PATH + 2];
   char    m_ModelSkel[MAX_PATH + 2];
   char    m_ModelData[MAX_DATA + 2];
   char    m_AnimaShow[MAX_LINE + 2];
   char*   m_TypeOption;
   long    m_SizeOffset;
   long    m_DataOffset;
   long    m_DataImport;
   long    m_FileOffset;
   long    m_VectorSize;
   long    m_ChannelCnt;
   FILE*   m_ModelSave;
   bool    m_AnimExport;
   bool    m_DataFormat;
   bool    m_SkelExport;
   bool    m_ShowFrames;
   int     m_FramesSize;
   int     m_FramesList;
   int     m_MatrixSize;
   int     m_MatrixList;
   int     m_PivotsSize;
   int     m_PivotCount;
   int     m_HeaderSkel;
   int     m_ExportSize;
   int     m_PivotIndex;
   int     m_ChannelTyp;
   int     m_arraySize;
   int     m_valueSize;
   int     m_MaxFrames;
   int     m_FrameIdx;
   int     m_AnimType;
   int     m_ShowIdx;
   int     m_next;
};

#endif // CW3DMODELANIM_INCLUDED_H
// ************************************************************************
// Module:  W3dModelAnim.h -- End of file
// ************************************************************************

