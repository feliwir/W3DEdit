// ************************************************************************
// Project: General Tools & Utilities
// ------------------------------------------------------------------------
// Purpose: defines methods and structures for reading W3D-model meshes
// Module : W3dModelMesh.h
// Author : Level_3D_Vision (C) 2008-2012
// System : ANSI C/C++ - MinGW32 compiler
// ------------------------------------------------------------------------
// History: 03.11.09 -- created by Jerry Brutzki and based upon w3dstruct.h
//          06.09.11 -- changed by Jerry Brutzki
// ************************************************************************
#ifndef CW3DMODELMESH_INCLUDED_H
#define CW3DMODELMESH_INCLUDED_H
// ----------------------------------------------------------- Includes ---
// ------------------------------------------------------------------------
#include "W3dModelBase.h"                      //common W3dModel definition
#include <stdlib.h>                            //common function definition
#include <vector>                              //common vector STLcontainer
using namespace std;

// -------------------------------------------------------- Definitions ---
// ------------------------------------------------------------------------
// ------------------------------------------------------ CW3DModelMesh ---
class CW3DModelMesh : public CW3DModelBase
{
public:
   CW3DModelMesh() {};
  ~CW3DModelMesh() {};

   void   ContentParser ();
   char*  GetHeaderData (const int);
   char*  GetVertexData (const int,const int);
   int    GetVertexBone (const int,const int);
   int    GetVertexSize (const int);
   char*  GetTextureData(const int);
   char*  GetBBoxDetails();
   char*  GetHierLodInfo();
   char*  GetHierLodData(const int);
   int    GetHeaderSize () { return m_HeaderSize;  };
   int    GetTextureSize() { return m_TextureSize; };
   int    GetDetailsSize() { return m_DetailsSize; };
   char*  GetPivotsHead (const int);
   char*  GetPivotsData (const int);
   bool   GetPivotsName (char*);
   int    GetHeaderSkel () { return m_HeaderSkel; };
   int    GetPivotsSize () { return m_PivotsSize; };
   bool   SkeletonInside() { return m_SkeletonInside; };

private:
   void   LoadW3dHierarchyStruct(int,unsigned long);
   void   LoadW3dCompAnimationStruct(unsigned long);
   void   LoadW3dAnimationStruct(unsigned long);
   void   LoadW3dHModelStruct(int,unsigned long);
   void   LoadW3dHLODStruct  (int,unsigned long);
   void   LoadW3dLODArrayStruct(int,unsigned long);
   void   LoadW3dMaterialStruct(unsigned long);
   void   LoadW3dTextureStruct (unsigned long);
   void   LoadW3dMaterialPassStruct(unsigned long);
   void   LoadW3dMaterialNormStruct(unsigned long);
   void   LoadW3dTextureStateCoords(unsigned long);
   void   LoadW3dTextureStageStruct(unsigned long);
   void   LoadW3dMeshStruct(unsigned long);
   void   InitModelContents();
   void   LoadPivotContents();
   void   SaveCurrentOffset(int);

   void   SaveInfoMeshHeader();
   void   SaveInfoVtxWeight ();
   void   SaveInfoTextArray ();
   void   SaveInfoHLodModel ();
   void   SaveInfoBoundBox  ();
   void   SaveInfoSkelHeader();
   void   SaveInfoPivotArray();

private:
   CW3DModelMesh(const CW3DModelMesh&);
   enum  {MAX_TEXT=256,MAX_ARRAY=2,MAX_OBJECT=24,MAX_COLOR=3,MAX_MESH=12};
   enum  {MAX_TEXTURE=32,MAX_BOXNAME=32,MAX_INFO=2,MAX_DETAIL=30};
   enum  {MAX_SIZE=3,NONE_NORMALMAP_REFERENCE=0xFFFF};
   enum  {MESH=0,BONE=1,BBOX=2};
   //---------------------------------------------------------
   //DATA: layout of W3D model structures ....
   //---------------------------------------------------------
   typedef struct
   {
     W3dHLodHeaderStruct       Header;
     long                      Offset;
   } W3dHLODHeaderStruct1;

   typedef struct
   {
     W3dHLodArrayHeaderStruct  Header;
     int                       SubObjectCount;
     W3dHLodSubObjectStruct    SubObject[MAX_OBJECT];
   } W3dHLODArrayStruct;

   typedef struct
   {
     int        LinkMap;
     char       Name[MAX_TEXTURE];
     W3dTextureInfoStruct TextureInfo;
   } W3dTextureStruct;

   typedef struct
   {
     char       Name[W3D_NAME_LEN];
     W3dVertexMaterialStruct  VertexMaterialInfo;
   } W3dMaterialStruct;

   W3dMeshHeader3Struct   m_W3DHeader;
   W3dVectorStruct        m_Vertex;
   W3dVectorStruct        m_Normal;
   W3dVertInfStruct       m_Influences;
   W3dTriStruct           m_Triangles;
   unsigned long          m_ShadeIndices;
   W3dShaderStruct        m_Shaders;

   W3dNormMapHeaderStruct m_NormHeader;
   W3dNormMapEntryStruct  m_NormEntry;
   W3dMaterialInfoStruct  m_MaterialInfo;
   W3dMaterialStruct      m_Materials;
   W3dTextureStruct       m_Textures;
   W3dBoxStruct           m_BoundingBox;

   //NOTE: Structures for TimeFrame animation
   W3dAnimHeaderStruct   m_AnimHeader;
   W3dAnimChannelStruct  m_AnimChannel;
   W3dBitChannelStruct   m_BitChannel;
   std::vector<W3dAnimChannelStruct>  m_AnimChannelList[MAX_ARRAY];
   std::vector<W3dBitChannelStruct>   m_BitChannelList [MAX_ARRAY];

   //NOTE: Animation flavor value is ANIM_FLAVOR_TIMECODED = 0
   W3dCompressedAnimHeaderStruct m_CompAnimHeader;
   struct sTimeCodesData
   {
     uint32    NumTimeCodes; // number of time coded entries
     uint16    Pivot;        // pivot affected by this channel
     uint8     VectorFlag;   // length of each vector or channel type
     uint8     Channel;      // channel type XYZQ or default state.
   } m_TimeCodesData;

   W3dHierarchyStruct     m_Skeleton[MAX_ARRAY];
   W3dPivotStruct         m_Pivots;
   W3dHModelHeaderStruct  m_HModelHeader[MAX_ARRAY];
   W3dHModelNodeStruct    m_HModelNode;
   W3dHLODHeaderStruct1   m_HLodHeader[MAX_ARRAY];
   W3dHLODArrayStruct     m_HLodArray[MAX_ARRAY];
   unsigned long          m_VtxMaterialID;
   unsigned long          m_ShaderID;
   W3dRGBAStruct          m_RGBAcolors;
   W3dTexCoordStruct      m_TextureCoord;
   unsigned long          m_TextureID;
   Vector3i               m_PerFaceTexCoordID;

   std::vector<W3dHModelNodeStruct>   m_HModelNodeList[MAX_ARRAY];
   std::vector<W3dPivotStruct>        m_PivotsList[MAX_ARRAY];
   std::vector<W3dMeshHeader3Struct>  m_HeaderList;
   std::vector<W3dVectorStruct>       m_VertexList;
   std::vector<W3dVectorStruct>       m_NormalList;
   std::vector<W3dVectorStruct>       m_VertexCopy;
   std::vector<W3dVectorStruct>       m_NormalCopy;

   std::vector<W3dVertInfStruct>      m_InfluencesList;
   std::vector<W3dTriStruct>          m_TrianglesList;
   std::vector<unsigned long>         m_ShadeIndicesList;
   std::vector<W3dNormMapEntryStruct> m_NormalMapList;
   std::vector<W3dMaterialStruct>     m_MaterialsList;
   std::vector<W3dShaderStruct>       m_ShadersList;
   std::vector<W3dTextureStruct>      m_TexturesList;
   std::vector<unsigned long>         m_VtxMaterialIDsList;
   std::vector<unsigned long>         m_ShaderIDsList;
   std::vector<W3dRGBAStruct>         m_RGBAcolorList[MAX_COLOR];
   std::vector<W3dTexCoordStruct>     m_TextureCoordsList;
   std::vector<unsigned long>         m_TextureIDsList;
   std::vector<Vector3i>              m_PerFaceTexCoordIDsList;

   //---------------------------------------------------------
   //DATA: layout of W3D display structures ....
   //---------------------------------------------------------
   struct sMeshHead
   {
     char  MeshName [MAX_NAME];
     char  Container[MAX_NAME];
     int   FaceCount;
     int   VertCount;
     long  DataSize;
     long  Offset;
   } m_HeaderInfo;

   struct sVertex
   {
     unsigned short BoneIndex;
     unsigned short BoneExtra;
     unsigned short RestData[MAX_INFO];
   } m_VertexInfo;

   struct sTexture
   {
     char  Name[MAX_TEXTURE];
     int   Link;
   } m_TextureInfo;

   struct sBoundBox
   {
     char  Name[MAX_BOXNAME];
     long  DataSize;
     long  Offset;
     unsigned long Attribute;
     float Extent_X;
     float Extent_Y;
     float Extent_Z;
   } m_BoundBoxInfo;

   struct sPivotsHead
   {
     char  Name[MAX_NAME];
     long  PivotCount;
     long  DataSize;
     long  Offset;
   } m_PivotsHead;

   struct sPivotsInfo
   {
     char  Name[MAX_NAME];
     long  ParentIdx;
     int   BoneIndex;
   } m_PivotsInfo;

   struct sHLodHead
   {
     char  ModelName[MAX_NAME];
     char  SkelName[MAX_NAME];
     int   ModelCount;
     long  Offset;
   } m_HLodHeadInfo;

   struct sHLodItem
   {
     char  MeshName[MAX_DETAIL];
     int   PivotIndex;
   } m_HLodItemInfo;

   std::vector<sVertex>     m_VertexInfoList[MAX_MESH];
   std::vector<sTexture>    m_TextureInfoList;
   std::vector<sMeshHead>   m_HeaderInfoList;
   std::vector<sHLodItem>   m_DetailsInfoList;
   std::vector<sPivotsHead> m_PivotsHeadList;
   std::vector<sPivotsInfo> m_PivotsInfoList;

   char    m_PivotFile[MAX_PATH + 2];
   char    m_PivotName[MAX_PATH + 2];
   bool    m_SkeletonInside;
   bool    m_TextureFound;
   long    m_FileOffset[MAX_SIZE];
   long    m_SizeOffset[MAX_SIZE];
   long    m_DataOffset;
   long    m_AnimOffset;
   long    m_NormOffset;
   int     m_HeaderSize;
   int     m_TextureSize;
   int     m_TextureMaps;
   int     m_DetailsSize;
   int     m_HeaderSkel;
   int     m_PivotsSize;
   int     m_PivotCount;
   int     m_MapIDCount;
   int     m_arraySize;
   int     m_next;
   int     m_mesh;
};

#endif // CW3DMODELMESH_INCLUDED_H
// ************************************************************************
// Program:  W3dModelMesh.h -- End of file
// ************************************************************************

