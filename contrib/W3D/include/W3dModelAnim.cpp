// ************************************************************************
// Project: General Tools & Utilities
// ------------------------------------------------------------------------
// Purpose: provides methods and structures for reading W3D-model meshes
// Module : W3dModelAnim.cpp
// Author : Level_3D_Vision (C) 2008-2012
// System : ANSI C/C++ - MinGW32 compiler
// ------------------------------------------------------------------------
// History: 13.04.10 -- created by Jerry Brutzki
//          27.12.11 -- changed by Jerry Brutzki
// ************************************************************************
// ----------------------------------------------------------- Includes ---
// ------------------------------------------------------------------------
#include "W3dModelAnim.h"                      //common W3dModel definition

// --------------------------------------------------------- Parameters ---
// ------------------------------------------------------------------------
const char CHANNEL_TYPE[][3] = {"X ","Y ","Z ","XR","YR","ZR","Q "};
const  int CHANNEL_SIZE = 7;
const char LINE[][61] =
     {"**********************************************************\n",
      "==========================================================\n",
      "----------------------------------------------------------\n"};

// ----------------------------------------------------- Implementation ---
// ------------------------------------------------------------------------
void CW3DModelAnim::ContentDelete ()
// ------------------------------------------------------------------------
{
   m_AnimChannelList[0].clear();  m_AnimChannelList[1].clear();
   m_TimeCodesAnimList.clear();   m_TimeCodesInfoList.clear();
   m_PivotsList[0].clear();       m_PivotsList[1].clear();

   m_TimeCodesList.clear();
   m_TimeFrameList.clear();
   m_PivotsHeadList.clear();
   m_PivotsInfoList.clear();
   m_ExportAnimList.clear();
   m_VectorMatrixList.clear();

   m_FramesSize = m_PivotsSize = m_FramesList = 0;
   m_AnimType = ANIM_UNKNOWN;
   m_Count = m_next = 0;
   m_AnimExport = false;
   m_SkelExport = false;
}

// ------------------------------------------------------------------------
void CW3DModelAnim::ContentParser ()
// ------------------------------------------------------------------------
{
   int   HierarchyLoaded = 0;

   memset(m_PivotFile,0,MAX_PATH);
   memset(m_PivotName,0,MAX_PATH);
   ContentDelete();

   //NOTE: Applies to embedded + stand-alone animations ...
   while (m_Offset < m_FileSize)
   {
      ReadDataChunk();
      m_Offset += (MAX_CHUNK + m_FileData.ChunkSize);

      switch (m_FileData.ChunkType)
      {
      case W3D_CHUNK_MESH:
         ReadDataChunk(m_FileData.ChunkSize);
         break;

      case W3D_CHUNK_HIERARCHY:
         if (HierarchyLoaded == 0)
         {
            m_FileOffset = (m_Offset - (MAX_CHUNK + m_FileData.ChunkSize));
            m_SizeOffset = (MAX_CHUNK + m_FileData.ChunkSize);
            LoadW3dHierarchyStruct(0,m_FileData.ChunkSize);
            HierarchyLoaded = 1;
            m_SkelExport = true;
         }
         else
         {
            LoadW3dHierarchyStruct(1,m_FileData.ChunkSize);
         }
         break;

      case W3D_CHUNK_ANIMATION:
         LoadW3dAnimationStruct(m_FileData.ChunkSize);
         break;
      case W3D_CHUNK_COMPRESSED_ANIMATION:
         LoadW3dCompAnimationStruct(m_FileData.ChunkSize);
         break;
      default:
         ReadDataChunk(m_FileData.ChunkSize);
         break;
      }
   }

   if (strlen(m_PivotName) > 0) {
      m_UserPath = strrchr(m_DataFile,'\\');
      if (m_UserPath != 0) {
         *m_UserPath = 0;
          strcat(m_DataFile,m_PivotName);
      }

      strncpy(m_PivotFile,m_DataFile,MAX_PATH);
      if (CloseModelFile()) {
         if (OpenModelFile(m_PivotFile))
         {
            ObtainPivotContent();
         }
      }
   }

   m_PivotsSize = static_cast<int>(m_PivotsInfoList.size());
   m_FramesSize = static_cast<int>(m_TimeCodesAnimList.size());
   m_FramesList = static_cast<int>(m_AnimChannelList[0].size());
   m_MatrixSize = static_cast<int>(m_VectorMatrixList.size());
   m_MatrixList = 0;
}

// ------------------------------------------------------------------------
void CW3DModelAnim::ObtainPivotContent ()
// ------------------------------------------------------------------------
{
   int  HierarchyLoaded = 0;
   while (m_Offset < m_FileSize)
   {
      ReadDataChunk();
      m_Offset += (MAX_CHUNK + m_FileData.ChunkSize);

      switch (m_FileData.ChunkType)
      {
      case W3D_CHUNK_HIERARCHY:
         if (HierarchyLoaded == 0)
         {
            LoadW3dHierarchyStruct(0,m_FileData.ChunkSize);
            HierarchyLoaded = 1;
         }
         else
         {
            LoadW3dHierarchyStruct(1,m_FileData.ChunkSize);
         }
         break;
      default:
         ReadDataChunk(m_FileData.ChunkSize);
         break;
      }
   }
}

// ------------------------------------------------------------------------
void CW3DModelAnim::LoadW3dHierarchyStruct (int idx, unsigned long DataOffset)
// ------------------------------------------------------------------------
{
   m_next = (idx < MAX_ARRAY) ? idx : 1;
   m_InputSize = 0;

   while (m_InputSize < DataOffset)
   {
      ReadDataChunk();
      m_InputSize += (MAX_CHUNK + m_FileData.ChunkSize);

      switch (m_FileData.ChunkType)
      {
      case W3D_CHUNK_HIERARCHY_HEADER:
         memset(&m_Skeleton[m_next],0,sizeof(W3dHierarchyStruct));
         fread(&m_Skeleton[m_next],sizeof(W3dHierarchyStruct),MAX_RECORD,m_Reader);
         SaveInfoSkelHeader();
         break;
      case W3D_CHUNK_PIVOTS:
         for (m_Count = 0; m_Count < m_Skeleton[m_next].NumPivots; ++m_Count)
         {
            memset(&m_Pivots,0,sizeof(W3dPivotStruct));
            fread(&m_Pivots,sizeof(W3dPivotStruct),MAX_RECORD,m_Reader);
            m_PivotsList[m_next].push_back(m_Pivots);
         }
         SaveInfoPivotArray();
         break;
      default:
         ReadDataChunk(m_FileData.ChunkSize);
         break;
      }
   }
}

// ------------------------------------------------------------------------
void CW3DModelAnim::LoadW3dAnimationStruct (unsigned long DataOffset)
// ------------------------------------------------------------------------
{
   m_InputSize = 0;
   while (m_InputSize < DataOffset)
   {
      ReadDataChunk();
      m_InputSize += (MAX_CHUNK + m_FileData.ChunkSize);

      switch (m_FileData.ChunkType)
      {
      case W3D_CHUNK_ANIMATION_HEADER:
         memset(&m_AnimHeader,0,sizeof(W3dAnimHeaderStruct));
         fread(&m_AnimHeader,sizeof(W3dAnimHeaderStruct),MAX_RECORD,m_Reader);

         memset(m_PivotName,0,MAX_PATH);
         if (m_PivotsList[0].size() == 0 && m_PivotsList[1].size() == 0)
         {
            sprintf(m_PivotName,"\\%s.w3d",m_AnimHeader.HierarchyName);
         }
         break;

      case W3D_CHUNK_ANIMATION_CHANNEL:
         memset(&m_AnimChannel,0,sizeof(W3dAnimChannelStruct));
         fread(&m_AnimChannel,sizeof(W3dAnimChannelStruct),MAX_RECORD,m_Reader);
         m_AnimChannelList[0].push_back(m_AnimChannel);

         memset(&m_TimeCodesInfo,0,sizeof(sTimeCodesInfo));
         m_TimeCodesInfo.Offset = m_FileData.ChunkSize;
         m_TimeCodesInfo.Size = (m_AnimChannel.VectorLen * MAX_LONG);
         m_TimeCodesInfo.Type = ANIM_CHANNEL;

         m_TimeCodesInfo.Index[0] = static_cast<int>(m_TimeFrameList.size());
         m_TimeCodesInfo.Index[1] = LoadW3dTimeAnimationFrames();
         m_TimeCodesInfoList.push_back(m_TimeCodesInfo);
         break;

      case W3D_CHUNK_BIT_CHANNEL:
         //memset(&m_BitChannel,0,sizeof(W3dBitChannelStruct));
         //fread(&m_BitChannel,sizeof(W3dBitChannelStruct),MAX_RECORD,m_Reader);
         //m_BitChannelList[0].push_back(m_BitChannel);
         break;
      default:
         ReadDataChunk(m_FileData.ChunkSize);
         break;
      }
   }

   if (m_AnimChannelList[0].size() > 0) {
      m_AnimType = ANIM_TIMEFRAMES;
      CreateCFrameMatrix();
   }
}

// ------------------------------------------------------------------------
int CW3DModelAnim::LoadW3dTimeAnimationFrames ()
// ------------------------------------------------------------------------
{
   m_VectorSize = (m_AnimChannel.VectorLen * MAX_LONG);
   m_ChannelCnt = (static_cast<int>(m_AnimChannel.LastFrame) + 1);

   for (int next = 0; next < m_ChannelCnt; ++ next)
   {
      memset(&m_TimeFrames,0,sizeof(sTimeFrames));
      fread(&m_TimeFrames,m_VectorSize,MAX_RECORD,m_Reader);
      m_TimeFrameList.push_back(m_TimeFrames);
   }

   return static_cast<int>(m_TimeFrameList.size());
}

// ------------------------------------------------------------------------
void CW3DModelAnim::CreateCFrameMatrix ()
// ------------------------------------------------------------------------
{
   m_MatrixList = 0;
}

// ------------------------------------------------------------------------
bool CW3DModelAnim::IsTypeTimeFrames()
// ------------------------------------------------------------------------
{
   return (m_AnimType == ANIM_TIMEFRAMES) ? true : false;
}

// ------------------------------------------------------------------------
bool CW3DModelAnim::IsTypeCompressed()
// ------------------------------------------------------------------------
{
   return (m_AnimType == ANIM_COMPRESSED) ? true : false;
}

// ------------------------------------------------------------------------
int  CW3DModelAnim::GetFramesSize ()
// ------------------------------------------------------------------------
{
   return (m_FramesList == 0) ? m_FramesSize : m_FramesList;
}

// ------------------------------------------------------------------------
int  CW3DModelAnim::GetMatrixSize ()
// ------------------------------------------------------------------------
{
   return (m_MatrixList == 0) ? m_MatrixSize : m_MatrixList;
}

// ------------------------------------------------------------------------
void CW3DModelAnim::LoadW3dCompAnimationStruct (unsigned long DataOffset)
// ------------------------------------------------------------------------
{
   m_InputSize = 0;
   while (m_InputSize < DataOffset)
   {
      ReadDataChunk();
      m_InputSize += (MAX_CHUNK + m_FileData.ChunkSize);

      switch (m_FileData.ChunkType)
      {
      case W3D_CHUNK_COMPRESSED_ANIMATION_HEADER:
         memset(&m_CompAnimHeader,0,sizeof(W3dCompressedAnimHeaderStruct));
         fread(&m_CompAnimHeader,sizeof(W3dCompressedAnimHeaderStruct),MAX_RECORD,m_Reader);

         memset(m_PivotName,0,MAX_PATH);
         if (m_PivotsList[0].size() == 0 && m_PivotsList[1].size() == 0)
         {
            sprintf(m_PivotName,"\\%s.w3d",m_CompAnimHeader.HierarchyName);
         }
         break;

      case W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL:
         memset(&m_TimeCodesData,0,sizeof(sTimeCodesData));
         fread(&m_TimeCodesData,sizeof(sTimeCodesData),MAX_RECORD,m_Reader);
         m_TimeCodesAnimList.push_back(m_TimeCodesData);

         memset(&m_TimeCodesInfo,0,sizeof(sTimeCodesInfo));
         m_TimeCodesInfo.Offset = m_FileData.ChunkSize;
         m_TimeCodesInfo.Size = (MAX_LONG + (m_TimeCodesData.VectorFlag * MAX_LONG));
         m_TimeCodesInfo.Type = ANIM_CHANNEL;

         m_TimeCodesInfo.Index[0] = static_cast<int>(m_TimeCodesList.size());
         m_TimeCodesInfo.Index[1] = LoadW3dCompAnimationValues();
         m_TimeCodesInfoList.push_back(m_TimeCodesInfo);
         break;

      case W3D_CHUNK_COMPRESSED_BIT_CHANNEL:
         memset(&m_TimeCodesData,0,sizeof(sTimeCodesData));
         fread(&m_TimeCodesData,sizeof(sTimeCodesData),MAX_RECORD,m_Reader);
         m_TimeCodesAnimList.push_back(m_TimeCodesData);

         memset(&m_TimeCodesInfo,0,sizeof(sTimeCodesInfo));
         m_TimeCodesInfo.Offset = m_FileData.ChunkSize;
         m_TimeCodesInfo.Size = (MAX_LONG + (m_TimeCodesData.VectorFlag * MAX_LONG));
         m_TimeCodesInfo.Type = BIT_CHANNEL;

         m_TimeCodesInfo.Index[0] = static_cast<int>(m_TimeCodesList.size());
         m_TimeCodesInfo.Index[1] = LoadW3dCompAnimationBitVal();
         m_TimeCodesInfoList.push_back(m_TimeCodesInfo);
         break;
      default:
         ReadDataChunk(m_FileData.ChunkSize);
         break;
      }
   }

   if (m_TimeCodesAnimList.size() > 0) {
      m_AnimType = ANIM_COMPRESSED;
      CreateVectorMatrix();
   }
}

// ------------------------------------------------------------------------
int CW3DModelAnim::LoadW3dCompAnimationValues ()
// ------------------------------------------------------------------------
{
   m_VectorSize = (MAX_LONG + (m_TimeCodesData.VectorFlag * MAX_LONG));
   m_ChannelCnt = static_cast<int>(m_TimeCodesData.NumTimeCodes);

   for (int next = 0; next < m_ChannelCnt; ++ next)
   {
      memset(&m_TimeCodes,0,sizeof(sTimeCodes));
      fread(&m_TimeCodes,m_VectorSize,MAX_RECORD,m_Reader);
      m_TimeCodesList.push_back(m_TimeCodes);
   }

   return static_cast<int>(m_TimeCodesList.size());
}

// ------------------------------------------------------------------------
int CW3DModelAnim::LoadW3dCompAnimationBitVal ()
// ------------------------------------------------------------------------
{
   m_VectorSize = MAX_LONG;
   m_ChannelCnt = static_cast<int>(m_TimeCodesData.NumTimeCodes);

   for (int next = 0; next < m_ChannelCnt; ++ next)
   {
      memset(&m_TimeCodes,0,sizeof(sTimeCodes));
      fread(&m_TimeCodes,m_VectorSize,MAX_RECORD,m_Reader);
      m_TimeCodesList.push_back(m_TimeCodes);
   }

   return static_cast<int>(m_TimeCodesList.size());
}

// ------------------------------------------------------------------------
void CW3DModelAnim::CreateVectorMatrix()
// ------------------------------------------------------------------------
{
   int  PreviousPivot = 0;
   memset(&m_VectorMatrix,0,sizeof(sVectorMatrix));
   strcpy(m_VectorMatrix.Frame,"F/B  ");
   m_ShowIdx = 0;

   m_FramesSize = static_cast<int>(m_TimeCodesAnimList.size());
   for (int idx = 0; idx < m_FramesSize; ++ idx)
   {
      if (m_TimeCodesAnimList[idx].Pivot != PreviousPivot)
      {
         sprintf(&m_VectorMatrix.Value[m_ShowIdx],"[%02d] ",m_TimeCodesAnimList[idx].Pivot);
         PreviousPivot = m_TimeCodesAnimList[idx].Pivot;
         m_ShowIdx += NEXT_ITEM;
      }
   }
   m_VectorMatrixList.push_back(m_VectorMatrix);

   m_MaxFrames = static_cast<int>(m_CompAnimHeader.NumFrames);
   m_arraySize = m_TimeCodesInfoList.size();
   m_ShowFrames = false;

   for (int next = 0; next < m_MaxFrames; ++ next)
   {
      memset(&m_VectorMatrix,0,sizeof(sVectorMatrix));
      sprintf(m_VectorMatrix.Frame,"%04d",next);
      m_ShowIdx = -NEXT_ITEM;

      m_TimeFrames.Value = static_cast<unsigned long>(next);
      PreviousPivot = 0;
      for (int scan = 0; scan < m_arraySize; ++ scan)
      {
         if (m_TimeCodesAnimList[scan].Pivot != PreviousPivot)
         {
            PreviousPivot = m_TimeCodesAnimList[scan].Pivot;
            m_ShowFrames = false;
            m_ShowIdx += NEXT_ITEM;
         }

         if (m_TimeCodesAnimList[scan].Pivot == PreviousPivot)
         {
            if (!m_ShowFrames)
            {
               m_valueSize = m_TimeCodesInfoList[scan].Index[1];
               for (int idx = m_TimeCodesInfoList[scan].Index[0]; idx < m_valueSize; ++ idx)
               {
                   if (m_TimeCodesList[idx].Number == m_TimeFrames.Value) {
                      strcpy(&m_VectorMatrix.Value[m_ShowIdx],"  *  ");
                      m_ShowFrames = true; break;
                   }
               }
            }
         }

         if (!m_ShowFrames) {
            strcpy(&m_VectorMatrix.Value[m_ShowIdx],"     ");
         }
      }

      m_VectorMatrixList.push_back(m_VectorMatrix);
   }
}

// ---------------------------------------------------- Import AnimData ---
// ------------------------------------------------------------------------
void CW3DModelAnim::SaveInfoSkelHeader ()
// ------------------------------------------------------------------------
{
   if (strlen(m_Skeleton[0].Name) > 0)
   {
      memset(&m_PivotsHead,0,sizeof(sPivotsHead));
      strcpy(m_PivotsHead.Name,m_Skeleton[0].Name);
      m_PivotsHead.PivotCount = m_Skeleton[0].NumPivots;
      m_PivotsHead.DataSize = m_SizeOffset;
      m_PivotsHead.Offset = m_FileOffset;
      m_PivotsHeadList.push_back(m_PivotsHead);
   }

   if (strlen(m_Skeleton[1].Name) > 0)
   {
      memset(&m_PivotsHead,0,sizeof(sPivotsHead));
      strcpy(m_PivotsHead.Name,m_Skeleton[1].Name);
      m_PivotsHead.PivotCount = m_Skeleton[1].NumPivots;
      m_PivotsHeadList.push_back(m_PivotsHead);
   }

   //NOTE: There is only one Skeleton per BFME-Model ....
   m_HeaderSkel = static_cast<int>(m_PivotsHeadList.size());
   if (m_HeaderSkel > 1) {
      m_HeaderSkel = 1;
   }
}

// ------------------------------------------------------------------------
void CW3DModelAnim::SaveInfoPivotArray ()
// ------------------------------------------------------------------------
{
   m_PivotCount = m_PivotsList[0].size();

   for (int idx = 0; idx < m_PivotCount; ++idx)
   {
      memset(&m_PivotsInfo,0,sizeof(sPivotsInfo));
      strcpy(m_PivotsInfo.Name,m_PivotsList[0][idx].Name);
      m_PivotsInfo.ParentIdx = m_PivotsList[0][idx].ParentIdx;
      m_PivotsInfo.BoneIndex = idx;
      m_PivotsInfoList.push_back(m_PivotsInfo);
   }
}

// ------------------------------------------------------------------------
void CW3DModelAnim::GetHeaderData (char* headerData)
// ------------------------------------------------------------------------
{
   memset(m_showData,0,MAX_OUTPUT);

   switch (m_AnimType)
   {
   case ANIM_COMPRESSED:
      if (strlen(m_CompAnimHeader.Name) > 0)
      {
         sprintf(m_showData,"%s;%s;%ld;ANIM_FLAVOR_TIMECODED;",
                 m_CompAnimHeader.Name,m_CompAnimHeader.HierarchyName,
                 m_CompAnimHeader.NumFrames);
         strcpy(headerData,m_showData);
      }
      break;
   case ANIM_TIMEFRAMES:
      if (strlen(m_AnimHeader.Name) > 0)
      {
         sprintf(m_showData,"%s;%s;%ld;ANIM_TIMEFRAMES;",
                 m_AnimHeader.Name,m_AnimHeader.HierarchyName,
                 m_AnimHeader.NumFrames);
         strcpy(headerData,m_showData);
      }
      break;
   }
}

// ------------------------------------------------------------------------
void CW3DModelAnim::GetFramesData (char* framesData, const int idx)
// ------------------------------------------------------------------------
{
   memset(m_showData,0,MAX_OUTPUT);

   switch (m_AnimType)
   {
   case ANIM_COMPRESSED:
      if (idx <= m_FramesSize)
      {
         sprintf(m_showData,"%ld;%d;%s;",m_TimeCodesAnimList[idx].NumTimeCodes,
                 m_TimeCodesAnimList[idx].Pivot,CHANNEL_TYPE[m_TimeCodesAnimList[idx].Channel]);
         strcpy(framesData,m_showData);
      }
      break;
   case ANIM_TIMEFRAMES:
      if (idx <= m_FramesList)
      {
         sprintf(m_showData,"%d;%d;%s;",m_AnimChannelList[0][idx].LastFrame,
                 m_AnimChannelList[0][idx].Pivot,CHANNEL_TYPE[m_AnimChannelList[0][idx].Flags]);
         strcpy(framesData,m_showData);
      }
      break;
   }
}

// ------------------------------------------------------------------------
void CW3DModelAnim::GetMatrixData (char* matrixData, const int idx)
// ------------------------------------------------------------------------
{
   memset(m_AnimaShow,0,MAX_LINE);

   switch (m_AnimType)
   {
   case ANIM_COMPRESSED:
      if (idx <= m_MatrixSize)
      {
         sprintf(m_AnimaShow,"%s;%s;",m_VectorMatrixList[idx].Frame,
                 m_VectorMatrixList[idx].Value);
         strcpy(matrixData,m_AnimaShow);
      }
      break;
   case ANIM_TIMEFRAMES:
      if (idx <= m_MatrixList)
      {
         sprintf(m_AnimaShow,"%d;%d;",m_AnimChannelList[0][idx].LastFrame,
                 m_AnimChannelList[0][idx].Pivot);
         strcpy(matrixData,m_AnimaShow);
      }
      break;
   }
}

// ------------------------------------------------------------------------
void CW3DModelAnim::GetPivotsHead (char* pivotsHead, const int idx)
// ------------------------------------------------------------------------
{
   memset(m_showData,0,MAX_OUTPUT);
   if (idx <= m_HeaderSkel)
   {
      sprintf(m_showData,"%s;%ld;", m_PivotsHeadList[idx].Name,
              m_PivotsHeadList[idx].PivotCount);
      strcpy(pivotsHead,m_showData);
   }
}

// ------------------------------------------------------------------------
void CW3DModelAnim::GetPivotsData (char* pivotsData, const int idx)
// ------------------------------------------------------------------------
{
   memset(m_showData,0,MAX_OUTPUT);
   if (idx <= m_PivotsSize)
   {
      sprintf(m_showData,"%s;%d;",m_PivotsInfoList[idx].Name,
              m_PivotsInfoList[idx].BoneIndex);
      strcpy(pivotsData,m_showData);
   }
}

// ------------------------------------------------------------------------
bool CW3DModelAnim::GetPivotsName (char* skeletonName)
// ------------------------------------------------------------------------
{
   //NOTE: This removes the previously inserted backslash ...
   strcpy (skeletonName,&m_PivotName[1]);
   return (strlen(m_PivotName) == 0) ? false : true;
}

// ------------------------------------------------------------------------
void CW3DModelAnim::SetHeaderData (const char* userName,const char* userSkel)
// ------------------------------------------------------------------------
{
   if (strlen(userName) > 0) {
      strncpy(m_CompAnimHeader.Name,userName,W3D_NAME_LEN);
      strncpy(m_AnimHeader.Name,userName,W3D_NAME_LEN);
   }

   if (strlen(userSkel) > 0) {
      strncpy(m_CompAnimHeader.HierarchyName,userSkel,W3D_NAME_LEN);
      strncpy(m_AnimHeader.HierarchyName,userSkel,W3D_NAME_LEN);

      memset (m_PivotsHeadList[0].Name,0,sizeof(m_PivotsHeadList[0].Name));
      strncpy(m_PivotsHeadList[0].Name,userSkel,W3D_NAME_LEN);
   }
}

// ------------------------------------------------------------------------
void CW3DModelAnim::SetFramesData (const char* Pivot,int idx)
// ------------------------------------------------------------------------
{
   if (idx <= GetFramesSize())
   {
      for (int scan = 0; scan < m_PivotsSize; ++ scan)
      {
         if (!strcmp(m_PivotsInfoList[scan].Name,Pivot)) {
            m_PivotIndex = m_PivotsInfoList[scan].BoneIndex;
            break;
         }
      }

      switch (m_AnimType)
      {
      case ANIM_COMPRESSED:
          m_TimeCodesAnimList[idx].Pivot = m_PivotIndex;
          m_ExportAnimList.push_back(idx);
          break;
      case ANIM_TIMEFRAMES:
          m_AnimChannel.Pivot = m_PivotIndex;
          m_ExportAnimList.push_back(idx);
          break;
      }
   }
}

// ---------------------------------------------------- Export AnimData ---
// ------------------------------------------------------------------------
bool CW3DModelAnim::SaveAnimOutput (const char* OutputAnim)
// ------------------------------------------------------------------------
{
   if (strlen(OutputAnim) > 0)
   {
      memset (m_ModelAnim,0,sizeof(m_ModelAnim));
      //memset (m_ModelSkel,0,sizeof(m_ModelSkel));
      strncpy(m_ModelAnim,OutputAnim,MAX_PATH);

      m_TypeOption = strrchr(m_ModelAnim,'.');
      if (m_TypeOption != 0)
      {
         m_DataFormat = (!strncmp(m_TypeOption,".w3d",MAX_LENGTH)) ? true
                      : (!strncmp(m_TypeOption,".W3D",MAX_LENGTH)) ? true : false;

         switch (m_AnimType)
         {
         case ANIM_TIMEFRAMES:
            m_AnimExport = (m_DataFormat) ? SaveDataFrameAnims() : SaveUserFrameAnims();
            break;
         case ANIM_COMPRESSED:
            m_AnimExport = (m_DataFormat) ? SaveDataModelAnims() : SaveUserModelAnims();
            break;
         }

      }
   }

   return m_AnimExport;
}

// ------------------------------------------------------------------------
bool CW3DModelAnim::SaveDataModelAnims ()
// ------------------------------------------------------------------------
{
   m_ModelSave = fopen(m_ModelAnim,"wb");
   if (m_ModelSave == 0) {
      return false;
   }

   m_FileData.ChunkType = W3D_CHUNK_COMPRESSED_ANIMATION;
   m_FileData.ChunkSize = 0;
   fwrite(&m_FileData,sizeof(W3dChunkHeader),MAX_RECORD,m_ModelSave);
   m_FileData.ChunkType = W3D_CHUNK_COMPRESSED_ANIMATION_HEADER;
   m_FileData.ChunkSize = sizeof(W3dCompressedAnimHeaderStruct);
   fwrite(&m_FileData,sizeof(W3dChunkHeader),MAX_RECORD,m_ModelSave);
   fwrite(&m_CompAnimHeader,sizeof(W3dCompressedAnimHeaderStruct),MAX_RECORD,m_ModelSave);

   //NOTE: Output extracted animation or updated animation ....
   m_ExportSize = m_ExportAnimList.size();
   if (m_ExportSize == 0)
   {
      for (int idx = 0; idx < m_FramesSize; ++ idx)
      {
         m_FileData.ChunkType = (m_TimeCodesInfoList[idx].Type == ANIM_CHANNEL) ?
                    W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL : W3D_CHUNK_COMPRESSED_BIT_CHANNEL;

         m_FileData.ChunkSize = m_TimeCodesInfoList[idx].Offset;
         fwrite(&m_FileData,sizeof(W3dChunkHeader),MAX_RECORD,m_ModelSave);
         fwrite(&m_TimeCodesAnimList[idx],sizeof(sTimeCodesData),MAX_RECORD,m_ModelSave);

         m_valueSize  = m_TimeCodesInfoList[m_next].Index[1];
         m_SizeOffset = m_TimeCodesInfoList[m_next].Size;
         for (int item = m_TimeCodesInfoList[m_next].Index[0]; item < m_valueSize; ++ item)
         {
            memset(&m_TimeCodes,0,sizeof(sTimeCodes));
            memcpy(&m_TimeCodes,&m_TimeCodesList[item],m_SizeOffset);
            fwrite(&m_TimeCodes,m_SizeOffset,MAX_RECORD,m_ModelSave);
         }
      }
   }
   else
   {
      for (int idx = 0; idx < m_ExportSize; ++ idx)
      {
         m_next = m_ExportAnimList[idx];
         m_FileData.ChunkType = (m_TimeCodesInfoList[m_next].Type == ANIM_CHANNEL) ?
                    W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL : W3D_CHUNK_COMPRESSED_BIT_CHANNEL;

         m_FileData.ChunkSize = m_TimeCodesInfoList[m_next].Offset;
         fwrite(&m_FileData,sizeof(W3dChunkHeader),MAX_RECORD,m_ModelSave);
         fwrite(&m_TimeCodesAnimList[m_next],sizeof(sTimeCodesData),MAX_RECORD,m_ModelSave);

         m_SizeOffset = m_TimeCodesInfoList[idx].Size;
         m_valueSize = m_TimeCodesInfoList[idx].Index[1];
         for (int next = m_TimeCodesInfoList[idx].Index[0]; next < m_valueSize; ++ next)
         {
            memset(&m_TimeCodes,0,sizeof(sTimeCodes));
            memcpy(&m_TimeCodes,&m_TimeCodesList[next],m_SizeOffset);
            fwrite(&m_TimeCodes,m_SizeOffset,MAX_RECORD,m_ModelSave);
         }
      }
   }

   if (m_ModelSave != 0) {
      fclose(m_ModelSave);
   }

   UpdateHeaderValue(W3D_CHUNK_COMPRESSED_ANIMATION);
   if (m_SkelExport) {
      SaveDataModelBones();
   }

   return true;
}

// ------------------------------------------------------------------------
void CW3DModelAnim::UpdateHeaderValue (int userAnimationType)
// ------------------------------------------------------------------------
{
   m_ModelSave = fopen(m_ModelAnim, "rb+");
   if (m_ModelSave != 0)
   {
      fseek(m_ModelSave,0,SEEK_END);
      m_FileOffset = ftell(m_ModelSave);
      fseek(m_ModelSave,0,SEEK_SET);

      m_FileData.ChunkType = userAnimationType;
      m_FileData.ChunkSize = (m_FileOffset - MAX_CHUNK);
      //NOTE: This re-instates the left-most 0x80 value ....
      m_FileData.ChunkSize ^= 0x80000000;

      fwrite(&m_FileData,sizeof(W3dChunkHeader),MAX_RECORD,m_ModelSave);
      fclose(m_ModelSave);
   }
}

// ------------------------------------------------------------------------
bool CW3DModelAnim::SaveDataModelBones ()
// ------------------------------------------------------------------------
{
   long  DataSize = m_PivotsHeadList[0].DataSize;
   unsigned char* WorkData = new unsigned char[DataSize];
   char* scanName = 0;

   if (CW3DModelBase::OpenModelFile(m_DataFile))
   {
      fseek(m_Reader,m_PivotsHeadList[0].Offset,SEEK_SET);
      fread (&WorkData[0],DataSize,MAX_RECORD,m_Reader);

      memset (m_PivotFile,0,sizeof(m_PivotFile));
      strncpy(m_PivotFile,m_ModelAnim,MAX_PATH);
      scanName = strrchr(m_PivotFile,'\\');
      if (scanName != 0) {
         *scanName = 0;
      }

      memset (m_ModelSkel,0,sizeof(m_ModelSkel));
      sprintf(m_ModelSkel,"%s\\%s.w3d",m_PivotFile,m_PivotsHeadList[0].Name);
      memcpy(&WorkData[NAME_OFFSET],m_PivotsHeadList[0].Name,W3D_NAME_LEN);

      m_ModelSave = fopen(m_ModelSkel, "wb");
      if (m_ModelSave != 0)
      {
         fwrite(&WorkData[0],DataSize,MAX_RECORD,m_ModelSave);
         fclose(m_ModelSave);
      }
      CW3DModelBase::CloseModelFile();
   }

   delete[] WorkData;  WorkData = 0;
   return true;
}

// ------------------------------------------------------------------------
bool CW3DModelAnim::SaveUserModelAnims ()
// ------------------------------------------------------------------------
{
   m_ModelSave = fopen(m_ModelAnim,"w");
   if (m_ModelSave == 0) {
      return false;
   }

   fputs(LINE[0],m_ModelSave); fputs(LINE[1],m_ModelSave);
   memset(m_ModelData,0,MAX_DATA);
   sprintf(m_ModelData,"Name: %s   Skeleton: %s\n",m_CompAnimHeader.Name,
           m_CompAnimHeader.HierarchyName);
   fputs (m_ModelData,m_ModelSave);

   memset(m_ModelData,0,MAX_DATA);
   sprintf(m_ModelData,"MaxFrames: %ld   FrameRate: %d\n",m_CompAnimHeader.NumFrames,
           m_CompAnimHeader.FrameRate);
   fputs (m_ModelData,m_ModelSave);
   fputs(LINE[1],m_ModelSave);

   OutputVectorMatrix();
   fputs(LINE[1],m_ModelSave);

   for (int idx = 0; idx < m_FramesSize; ++ idx)
   {
      switch (m_TimeCodesInfoList[idx].Type)
      {
      case ANIM_CHANNEL:
         memset(m_ModelData,0,MAX_DATA);
         strcpy(m_ModelData,"Type: W3D_COMPRESSED_ANIMATION_CHANNEL\n");
         fputs (m_ModelData,m_ModelSave);

         memset(m_ModelData,0,MAX_DATA);
         m_next = IdentifyPivotIndex(m_TimeCodesAnimList[idx].Pivot);
         sprintf(m_ModelData,"Bone: [%d] %s   Length: %ld  %s[%d]\n",
                 m_PivotsInfoList[m_next].BoneIndex,
               ((m_next >= 0) ? m_PivotsInfoList[m_next].Name : "???"),
                 m_TimeCodesAnimList[idx].NumTimeCodes,
                 CHANNEL_TYPE[m_TimeCodesAnimList[idx].Channel],
                 m_TimeCodesAnimList[idx].VectorFlag);
         fputs (m_ModelData,m_ModelSave);
         OutputAnimChannel(idx);
         break;

      case BIT_CHANNEL:
         memset(m_ModelData,0,MAX_DATA);
         strcpy(m_ModelData,"Type: W3D_COMPRESSED_BIT_CHANNEL\n");
         fputs (m_ModelData,m_ModelSave);

         memset(m_ModelData,0,MAX_DATA);
         m_next = IdentifyPivotIndex(m_TimeCodesAnimList[idx].Pivot);
         sprintf(m_ModelData,"Bone: [%d] %s   Length: %ld  %s[%d]\n",
                 m_PivotsInfoList[m_next].BoneIndex,
               ((m_next >= 0) ? m_PivotsInfoList[m_next].Name : "???"),
                 m_TimeCodesAnimList[idx].NumTimeCodes,
                 CHANNEL_TYPE[m_TimeCodesAnimList[idx].VectorFlag],
                 m_TimeCodesAnimList[idx].Channel);
         fputs (m_ModelData,m_ModelSave);
         OutputBitsChannel(idx);
         break;
      }
      fputs(LINE[2],m_ModelSave);
   }

   if (m_ModelSave != 0) {
      fclose(m_ModelSave);
   }
   return true;
}

// ------------------------------------------------------------------------
void CW3DModelAnim::OutputVectorMatrix ()
// ------------------------------------------------------------------------
{
   m_MaxFrames = static_cast<int>(m_VectorMatrixList.size());
   for (int next = 0; next < m_MaxFrames; ++ next)
   {
      memset(m_AnimaShow,0,sizeof(m_AnimaShow));
      strcpy(m_AnimaShow,m_VectorMatrixList[next].Frame);
      sprintf(&m_AnimaShow[NEXT_ITEM],"%s\n",m_VectorMatrixList[next].Value);
      fputs (m_AnimaShow,m_ModelSave);
   }
}

// ------------------------------------------------------------------------
void CW3DModelAnim::OutputAnimChannel (int idx)
// ------------------------------------------------------------------------
{
   fputs(LINE[2],m_ModelSave);
   m_valueSize = m_TimeCodesInfoList[idx].Index[1];

   for (int next = m_TimeCodesInfoList[idx].Index[0]; next < m_valueSize; ++ next)
   {
      memset(m_ModelData,0,MAX_DATA);
      switch (m_TimeCodesAnimList[idx].VectorFlag)
      {
      case 1:
         sprintf(m_ModelData,"Vector: %lu  %lu\n",m_TimeCodesList[next].Number,
                 m_TimeCodesList[next].Vector[0]);
         break;
      case 2:
         sprintf(m_ModelData,"Vector: %lu  %lu  %lu\n",m_TimeCodesList[next].Number,
                 m_TimeCodesList[next].Vector[0],m_TimeCodesList[next].Vector[1]);
         break;
      case 3:
         sprintf(m_ModelData,"Vector: %lu  %lu  %lu  %lu\n",m_TimeCodesList[next].Number,
                 m_TimeCodesList[next].Vector[0],m_TimeCodesList[next].Vector[1],
                 m_TimeCodesList[next].Vector[2]);
         break;
      case 4:
         sprintf(m_ModelData,"Vector: %lu  %lu  %lu  %lu  %lu\n",m_TimeCodesList[next].Number,
                 m_TimeCodesList[next].Vector[0],m_TimeCodesList[next].Vector[1],
                 m_TimeCodesList[next].Vector[2],m_TimeCodesList[next].Vector[3]);
         break;
      }
      fputs (m_ModelData,m_ModelSave);
   }
}

// ------------------------------------------------------------------------
void CW3DModelAnim::OutputBitsChannel (int idx)
// ------------------------------------------------------------------------
{
   fputs(LINE[2],m_ModelSave);
   m_valueSize = m_TimeCodesInfoList[idx].Index[1];

   for (int next = m_TimeCodesInfoList[idx].Index[0]; next < m_valueSize; ++ next)
   {
      memset(m_ModelData,0,MAX_DATA);
      sprintf(m_ModelData,"Vector: %lu\n",m_TimeCodesList[next].Number);
      fputs (m_ModelData,m_ModelSave);
   }
}

// ------------------------------------------------------------------------
int  CW3DModelAnim::IdentifyPivotIndex (int verifyPivot)
// ------------------------------------------------------------------------
{
   int pivotIndex = -1;
   for (int scan = 0; scan < m_PivotsSize; ++ scan)
   {
      if (verifyPivot == m_PivotsInfoList[scan].BoneIndex) {
         pivotIndex = scan;
         break;
      }
   }
   return pivotIndex;
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
bool CW3DModelAnim::SaveDataFrameAnims()
// ------------------------------------------------------------------------
{
   m_ModelSave = fopen(m_ModelAnim,"wb");
   if (m_ModelSave == 0) {
      return false;
   }

   m_FileData.ChunkType = W3D_CHUNK_ANIMATION;
   m_FileData.ChunkSize = 0;
   fwrite(&m_FileData,sizeof(W3dChunkHeader),MAX_RECORD,m_ModelSave);
   m_FileData.ChunkType = W3D_CHUNK_ANIMATION_HEADER;
   m_FileData.ChunkSize = sizeof(W3dAnimHeaderStruct);
   fwrite(&m_FileData,sizeof(W3dChunkHeader),MAX_RECORD,m_ModelSave);
   fwrite(&m_AnimHeader,sizeof(W3dAnimHeaderStruct),MAX_RECORD,m_ModelSave);

   //NOTE: Output extracted animation or updated animation ....
   m_ExportSize = m_ExportAnimList.size();
   if (m_ExportSize == 0)
   {
      for (int idx = 0; idx < m_FramesList; ++ idx)
      {
         m_FileData.ChunkType = (m_TimeCodesInfoList[idx].Type == ANIM_CHANNEL) ?
                    W3D_CHUNK_ANIMATION_CHANNEL : W3D_CHUNK_BIT_CHANNEL;

         m_FileData.ChunkSize = m_TimeCodesInfoList[idx].Offset;
         fwrite(&m_FileData,sizeof(W3dChunkHeader),MAX_RECORD,m_ModelSave);
         fwrite(&m_AnimChannelList[0][idx],sizeof(W3dAnimChannelStruct),MAX_RECORD,m_ModelSave);

         m_valueSize  = m_TimeCodesInfoList[m_next].Index[1];
         m_SizeOffset = m_TimeCodesInfoList[m_next].Size;
         for (int item = m_TimeCodesInfoList[m_next].Index[0]; item < m_valueSize; ++ item)
         {
            memset(&m_TimeCodes,0,sizeof(sTimeCodes));
            memcpy(&m_TimeCodes,&m_TimeCodesList[item],m_SizeOffset);
            fwrite(&m_TimeCodes,m_SizeOffset,MAX_RECORD,m_ModelSave);
         }
      }
   }
   else
   {
      for (int idx = 0; idx < m_ExportSize; ++ idx)
      {
         m_next = m_ExportAnimList[idx];
         m_FileData.ChunkType = (m_TimeCodesInfoList[m_next].Type == ANIM_CHANNEL) ?
                    W3D_CHUNK_ANIMATION_CHANNEL : W3D_CHUNK_BIT_CHANNEL;

         m_FileData.ChunkSize = m_TimeCodesInfoList[m_next].Offset;
         fwrite(&m_FileData,sizeof(W3dChunkHeader),MAX_RECORD,m_ModelSave);
         fwrite(&m_AnimChannelList[0][m_next],sizeof(W3dAnimChannelStruct),MAX_RECORD,m_ModelSave);

         m_SizeOffset = m_TimeCodesInfoList[m_next].Size;
         m_valueSize = m_TimeCodesInfoList[m_next].Index[1];
         for (int item = m_TimeCodesInfoList[m_next].Index[0]; item < m_valueSize; ++ item)
         {
            memset(&m_TimeFrames,0,sizeof(sTimeFrames));
            memcpy(&m_TimeFrames,&m_TimeFrameList[item],m_SizeOffset);
            fwrite(&m_TimeFrames,m_SizeOffset,MAX_RECORD,m_ModelSave);
         }
      }
   }

   if (m_ModelSave != 0) {
      fclose(m_ModelSave);
   }

   UpdateHeaderValue(W3D_CHUNK_ANIMATION);
   if (m_SkelExport) {
      SaveDataModelBones();
   }

   return true;
}

// ------------------------------------------------------------------------
bool CW3DModelAnim::SaveUserFrameAnims()
// ------------------------------------------------------------------------
{
   m_ModelSave = fopen(m_ModelAnim,"w");
   if (m_ModelSave == 0) {
      return false;
   }

   fputs(LINE[0],m_ModelSave); fputs(LINE[1],m_ModelSave);
   memset(m_ModelData,0,MAX_DATA);
   sprintf(m_ModelData,"Name: %s   Skeleton: %s\n",m_AnimHeader.Name,
           m_AnimHeader.HierarchyName);
   fputs (m_ModelData,m_ModelSave);

   memset(m_ModelData,0,MAX_DATA);
   sprintf(m_ModelData,"MaxFrames: %ld   FrameRate: %ld\n",m_AnimHeader.NumFrames,
           m_AnimHeader.FrameRate);
   fputs (m_ModelData,m_ModelSave);
   fputs(LINE[1],m_ModelSave);

   for (int idx = 0; idx < m_FramesList; ++ idx)
   {
      memset(m_ModelData,0,MAX_DATA);
      strcpy(m_ModelData,"Type: W3D_CHUNK_ANIMATION_CHANNEL\n");
      fputs (m_ModelData,m_ModelSave);

      memset(m_ModelData,0,MAX_DATA);
      m_next = IdentifyPivotIndex(m_AnimChannelList[0][idx].Pivot);
      sprintf(m_ModelData,"Bone: [%d] %s   Length: %d-%d  %s[%d]\n",
              m_PivotsInfoList[m_next].BoneIndex,
            ((m_next >= 0) ? m_PivotsInfoList[m_next].Name : "???"),
              m_AnimChannelList[0][idx].FirstFrame,
              m_AnimChannelList[0][idx].LastFrame,
              CHANNEL_TYPE[m_AnimChannelList[0][idx].Flags],
              m_AnimChannelList[0][idx].VectorLen);
      fputs (m_ModelData,m_ModelSave);
      fputs(LINE[2],m_ModelSave);
   }

   if (m_ModelSave != 0) {
      fclose(m_ModelSave);
   }
   return true;
}

// ------------------------------------------------------------------------
void CW3DModelAnim::OutputCFrameMatrix()
// ------------------------------------------------------------------------
{
}

// ************************************************************************
// Module:  W3dModelAnim.cpp -- End of File
// ************************************************************************

