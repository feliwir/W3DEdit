// ************************************************************************
// Project: General Tools & Utilities
// ------------------------------------------------------------------------
// Purpose: provides methods and structures for reading W3D-model meshes
// Module : W3dModelMesh.cpp
// Author : Level_3D_Vision (C) 2008-2012
// System : ANSI C/C++ - MinGW32 compiler
// ------------------------------------------------------------------------
// History: 03.11.09 -- created by Jerry Brutzki and based upon loader.c
//                      from the Open Source Code of w3dview ....
//          06.09.11 -- changed by Jerry Brutzki
// ************************************************************************
// ----------------------------------------------------------- Includes ---
// ------------------------------------------------------------------------
#include "W3dModelMesh.h"                      //common W3dModel definition

// ----------------------------------------------------- Implementation ---
// ------------------------------------------------------------------------
void CW3DModelMesh::InitModelContents ()
// ------------------------------------------------------------------------
{
   m_VertexInfoList[0].clear();    m_VertexInfoList[1].clear();
   m_VertexInfoList[2].clear();    m_VertexInfoList[3].clear();
   m_VertexInfoList[4].clear();    m_VertexInfoList[5].clear();
   m_VertexInfoList[6].clear();    m_VertexInfoList[7].clear();
   m_VertexInfoList[8].clear();    m_VertexInfoList[9].clear();
   m_VertexInfoList[10].clear();   m_VertexInfoList[11].clear();
   m_TextureInfoList.clear();      m_DetailsInfoList.clear();
   m_HeaderInfoList.clear();

   m_AnimChannelList[0].clear();   m_AnimChannelList[1].clear();
   m_BitChannelList[0].clear();    m_BitChannelList[1].clear();
   m_HModelNodeList[0].clear();    m_HModelNodeList[1].clear();
   m_PivotsList[0].clear();        m_PivotsList[1].clear();
   m_VertexList.clear();           m_VertexCopy.clear();
   m_NormalList.clear();           m_NormalCopy.clear();

   m_HeaderList.clear();           m_InfluencesList.clear();
   m_TrianglesList.clear();        m_ShadeIndicesList.clear();
   m_NormalMapList.clear();        m_MaterialsList.clear();
   m_ShadersList.clear();          m_TexturesList.clear();
   m_VtxMaterialIDsList.clear();   m_ShaderIDsList.clear();
   m_RGBAcolorList[0].clear();     m_RGBAcolorList[1].clear();
   m_RGBAcolorList[2].clear();     m_TextureCoordsList.clear();
   m_TextureIDsList.clear();       m_PerFaceTexCoordIDsList.clear();
   m_PivotsHeadList.clear();       m_PivotsInfoList.clear();

   m_HeaderSkel = m_PivotsSize = 0;
   m_PivotCount = m_MapIDCount = 0;
   m_NormOffset = 0;
   m_next = 0;
   m_mesh = 0;
}

// ------------------------------------------------------------------------
void CW3DModelMesh::ContentParser ()
// ------------------------------------------------------------------------
{
   int  HierarchyLoaded = 0;
   int  HModelLoaded = 0;
   int  HLODLoaded = 0;

   memset(m_PivotFile,0,MAX_PATH);
   memset(m_PivotName,0,MAX_PATH);
   InitModelContents();
   m_SkeletonInside = false;

   while (m_Offset < m_FileSize)
   {
      ReadDataChunk();
      m_DataOffset = m_Offset;
      m_Offset += (MAX_CHUNK + m_FileData.ChunkSize);
   //================================================== DEBUG_OUTPUT ===
   printf("\n---------------------------------------------------------------------\n");
   printf("### MAIN_PARSER: Offset [%X] Size [%X] Type [%X]\n",(int)m_DataOffset,
         (int)m_FileData.ChunkSize, (int)m_FileData.ChunkType);
   printf("---------------------------------------------------------------------\n");
   //===================================================================

      switch (m_FileData.ChunkType)
      {
      case W3D_CHUNK_MESH:
         SaveCurrentOffset(MESH);
         LoadW3dMeshStruct(m_FileData.ChunkSize);
         break;

      case W3D_CHUNK_HIERARCHY:
         if (HierarchyLoaded == 0)
         {
            SaveCurrentOffset(BONE);
            LoadW3dHierarchyStruct(0,m_FileData.ChunkSize);
            m_SkeletonInside = true;
            HierarchyLoaded = 1;
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

      case W3D_CHUNK_BOX:
         SaveCurrentOffset(BBOX);
         memset(&m_BoundingBox,0,sizeof(W3dBoxStruct));
         fread(&m_BoundingBox,sizeof(W3dBoxStruct),MAX_RECORD,m_Reader);
   //================================================== DEBUG_OUTPUT ===
   //printf("### W3D_CHUNK_BOX: [%X] Name [%s]\n",
   //      (int)m_FileData.ChunkSize,m_BoundingBox.Name);

   //printf("### W3D_CHUNK_BOX: [%X]\n",(int)m_BoundingBox.Version);
   //printf("### W3D_CHUNK_BOX: [%X]\n",(int)m_BoundingBox.Attributes);
   //printf("### W3D_CHUNK_BOX: [%X %X %X]\n",(int)m_BoundingBox.Color.R,
   //              (int)m_BoundingBox.Color.G,(int)m_BoundingBox.Color.B);
   //printf("### W3D_CHUNK_BOX: [%f %f %f]\n",m_BoundingBox.Center.X,
   //                  m_BoundingBox.Center.Y,m_BoundingBox.Center.Z);
   //printf("### W3D_CHUNK_BOX: [%f %f %f]\n",m_BoundingBox.Extent.X,
   //                  m_BoundingBox.Extent.Y,m_BoundingBox.Extent.Z);
   //===================================================================
         SaveInfoBoundBox();
         break;
      case W3D_CHUNK_HMODEL:
         if (HModelLoaded == 0)
         {
            LoadW3dHModelStruct(0,m_FileData.ChunkSize);
            HModelLoaded = 1;
         }
         else
         {
            LoadW3dHModelStruct(1,m_FileData.ChunkSize);
         }
         break;
      case W3D_CHUNK_HLOD:
         if (HLODLoaded == 0)
         {
            LoadW3dHLODStruct(0,m_FileData.ChunkSize);
            HLODLoaded = 1;
         }
         else
         {
            LoadW3dHLODStruct(1,m_FileData.ChunkSize);
         }
         SaveInfoHLodModel();
         break;

      default:
         ReadDataChunk(m_FileData.ChunkSize);
         break;
      }
   }

   m_HeaderSize = static_cast<int>(m_HeaderInfoList.size());
   m_TextureSize= static_cast<int>(m_TextureInfoList.size());
   m_TextureMaps= static_cast<int>(m_NormalMapList.size());
   m_DetailsSize= static_cast<int>(m_DetailsInfoList.size());

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
            LoadPivotContents();
         }
      }
   }
}

// ------------------------------------------------------------------------
void CW3DModelMesh::SaveCurrentOffset (int idx)
// ------------------------------------------------------------------------
{
   if (idx < MAX_SIZE)
   {
      m_FileOffset[idx] = (m_Offset - (MAX_CHUNK + m_FileData.ChunkSize));
      m_SizeOffset[idx] = (MAX_CHUNK + m_FileData.ChunkSize);
   }
}

// ------------------------------------------------------------------------
void CW3DModelMesh::LoadPivotContents ()
// ------------------------------------------------------------------------
{
   int  HierarchyLoaded = 0;
   m_Offset = 0;

   //===================================================== DEBUG_OUTPUT ===
   printf("\n---------------------------------------------------------------------\n");
   printf("### MAIN_PARSER: Size [%X] Name [%s]\n",(int)m_FileSize,m_PivotFile);
   printf("---------------------------------------------------------------------\n");
   //======================================================================

   while (m_Offset < m_FileSize)
   {
      ReadDataChunk();
      m_Offset += (MAX_CHUNK + m_FileData.ChunkSize);

      //===================================================== DEBUG_OUTPUT ===
      printf("\n---------------------------------------------------------------------\n");
      printf("### READ_PIVOTS: Offset [%X] Size [%X] Type [%X]\n",(int)m_Offset,
            (int)m_FileData.ChunkSize, (int)m_FileData.ChunkType);
      printf("---------------------------------------------------------------------\n");
     //======================================================================

      switch (m_FileData.ChunkType)
      {
      case W3D_CHUNK_HIERARCHY:
         if (HierarchyLoaded == 0)
         {
            SaveCurrentOffset(BONE);
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
void CW3DModelMesh::LoadW3dMeshStruct (unsigned long DataOffset)
// ------------------------------------------------------------------------
{
   m_TextureFound = false;
   m_InputSize = 0;

   while (m_InputSize < DataOffset)
   {
      ReadDataChunk();
      m_InputSize += (MAX_CHUNK + m_FileData.ChunkSize);

      switch (m_FileData.ChunkType)
      {
      case W3D_CHUNK_MESH:
         m_InputSize = DataOffset;
         break;
      case W3D_CHUNK_MESH_HEADER3:
         memset(&m_W3DHeader,0,sizeof(W3dMeshHeader3Struct));
         fread(&m_W3DHeader,sizeof(W3dMeshHeader3Struct),MAX_RECORD,m_Reader);
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_MESH_HEADER3: [%X] Name [%s.%s]\n",(int)m_InputSize,
         m_W3DHeader.ContainerName,m_W3DHeader.MeshName);
   printf("---------------------------------------------\n");

   //printf("Vers [%X]  Attr [%X]\n", (int)m_W3DHeader.Version,(int)m_W3DHeader.Attributes);
   //printf("Tris [%X]  Vert [%X]\n", (int)m_W3DHeader.NumTris,(int)m_W3DHeader.NumVertices);
   //printf("Matl [%X]  Stage[%X]\n", (int)m_W3DHeader.NumMaterials,(int)m_W3DHeader.NumDamageStages);
   //printf("Sort [%X]  PreVs[%X]\n", (int)m_W3DHeader.SortLevel,(int)m_W3DHeader.PrelitVersion);
   //printf("Cnts [%X]  VtxCH[%X]\n", (int)m_W3DHeader.FutureCounts[1],(int)m_W3DHeader.VertexChannels);
   //printf("Face [%X]\n", (int)m_W3DHeader.FaceChannels);

   //printf("Min_XYZ [%f  %f  %f]\n",m_W3DHeader.Min.X,m_W3DHeader.Min.Y,m_W3DHeader.Min.Z);
   //printf("Max_XYZ [%f  %f  %f]\n",m_W3DHeader.Max.X,m_W3DHeader.Max.Y,m_W3DHeader.Max.Z);
   //printf("Sph_XYZ [%f  %f  %f]\n",m_W3DHeader.SphCenter.X,
   //        m_W3DHeader.SphCenter.Y,m_W3DHeader.SphCenter.Z);
   //printf("SphRadius [%f]\n",m_W3DHeader.SphRadius);
   //===================================================================
         SaveInfoMeshHeader();
         break;
      case W3D_CHUNK_VERTICES:
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_VERTICES: [%X] Type [%X] Vtx [%X]\n",(int)m_InputSize,
         (int)m_FileData.ChunkType, (int)m_W3DHeader.NumVertices);
   //===================================================================
         for (m_Count = 0; m_Count < m_W3DHeader.NumVertices; ++m_Count)
         {
            memset(&m_Vertex,0,sizeof(W3dVectorStruct));
            fread(&m_Vertex,sizeof(W3dVectorStruct),MAX_RECORD,m_Reader);
            m_VertexList.push_back(m_Vertex);
   //================================================== DEBUG_OUTPUT ===
   printf("VertexList:  X[%f], Y[%f], Z[%f]\n",m_Vertex.X,m_Vertex.Y,m_Vertex.Z);
   //===================================================================
         }
         break;
      case W3D_CHUNK_VERTICES_COPY:
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_VERTICES_COPY: [%X] Type [%X] Vtx [%X]\n",(int)m_InputSize,
         (int)m_FileData.ChunkType, (int)m_W3DHeader.NumVertices);
   //===================================================================
         for (m_Count = 0; m_Count < m_W3DHeader.NumVertices; ++m_Count)
         {
            memset(&m_Vertex,0,sizeof(W3dVectorStruct));
            fread(&m_Vertex,sizeof(W3dVectorStruct),MAX_RECORD,m_Reader);
            m_VertexCopy.push_back(m_Vertex);
         }
         ReadDataChunk(m_FileData.ChunkSize);
         break;
      case W3D_CHUNK_VERTEX_NORMALS:
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_VERTEX_NORMALS: [%X] Type [%X] Vtx [%X]\n",(int)m_InputSize,
         (int)m_FileData.ChunkType, (int)m_W3DHeader.NumVertices);
   //===================================================================
         for (m_Count = 0; m_Count < m_W3DHeader.NumVertices; ++m_Count)
         {
            memset(&m_Normal,0,sizeof(W3dVectorStruct));
            fread(&m_Normal,sizeof(W3dVectorStruct),MAX_RECORD,m_Reader);
            m_NormalList.push_back(m_Normal);
   //================================================== DEBUG_OUTPUT ===
   printf("m_NormalList:  X[%f], Y[%f], Z[%f]\n",m_Normal.X,m_Normal.Y,m_Normal.Z);
   //===================================================================
         }
         break;
      case W3D_CHUNK_VERTEX_NORMALS_COPY:
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_VERTEX_NORMALS_COPY: [%X] Type [%X] Vtx [%X]\n",(int)m_InputSize,
         (int)m_FileData.ChunkType, (int)m_W3DHeader.NumVertices);
   //===================================================================
         for (m_Count = 0; m_Count < m_W3DHeader.NumVertices; ++m_Count)
         {
            memset(&m_Normal,0,sizeof(W3dVectorStruct));
            fread(&m_Normal,sizeof(W3dVectorStruct),MAX_RECORD,m_Reader);
            m_NormalCopy.push_back(m_Normal);
         }
         ReadDataChunk(m_FileData.ChunkSize);
         break;
      case W3D_CHUNK_VERTEX_INFLUENCES:
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_VERTEX_INFLUENCES: [%X] Type [%X] Vtx [%X]\n",
         (int)m_InputSize,(int)m_FileData.ChunkType, (int)m_W3DHeader.NumVertices);
   //===================================================================
         for (m_Count = 0; m_Count < m_W3DHeader.NumVertices; ++m_Count)
         {
            memset(&m_Influences,0,sizeof(W3dVertInfStruct));
            fread(&m_Influences,sizeof(W3dVertInfStruct),MAX_RECORD,m_Reader);
            m_InfluencesList.push_back(m_Influences);
            SaveInfoVtxWeight();
   //================================================== DEBUG_OUTPUT ===
   printf("Vertex_Weights: [%d] [%X] extra[%d]\n",(int)m_Influences.BoneIdx,
         (int)m_Influences.BoneIdx,(int)m_Influences.XtraIdx);
   //===================================================================
         }
         break;
      case W3D_CHUNK_TRIANGLES:
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_TRIANGLES: [%X] Type [%X] Tris [%X]\n",
         (int)m_InputSize,(int)m_FileData.ChunkType, (int)m_W3DHeader.NumTris);
   //===================================================================

         for (m_Count = 0; m_Count < m_W3DHeader.NumTris; ++m_Count)
         {
            memset(&m_Triangles,0,sizeof(W3dTriStruct));
            fread(&m_Triangles,sizeof(W3dTriStruct),MAX_RECORD,m_Reader);
            m_TrianglesList.push_back(m_Triangles);
   //================================================== DEBUG_OUTPUT ===
   printf("m_Triangles: [%d %d %d] [%X] XYZ[%f  %f  %f] [%f]\n",
         (int)m_Triangles.Vindex[0],(int)m_Triangles.Vindex[1],
         (int)m_Triangles.Vindex[2],(int)m_Triangles.Attributes,
         m_Triangles.Normal.X,m_Triangles.Normal.Y,m_Triangles.Normal.Z,m_Triangles.Dist);
   //===================================================================
         }
         break;
      case W3D_CHUNK_VERTEX_SHADE_INDICES:
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_VERTEX_SHADE_INDICES: [%X] Type [%X] Vtx [%X]\n",
         (int)m_InputSize,(int)m_FileData.ChunkType, (int)m_W3DHeader.NumVertices);
   //===================================================================
         for (m_Count = 0; m_Count < m_W3DHeader.NumVertices; ++m_Count)
         {
            memset(&m_ShadeIndices,0,sizeof(uint32));
            fread(&m_ShadeIndices,sizeof(uint32),MAX_RECORD,m_Reader);
            m_ShadeIndicesList.push_back(m_ShadeIndices);
   //================================================== DEBUG_OUTPUT ===
   printf("Vertex_ShadeIdx: [%d] [%X]\n", (int)m_ShadeIndices, (int)m_ShadeIndices);
   //===================================================================
         }
         break;

      case W3D_CHUNK_MATERIAL_INFO:
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_MATERIAL_INFO: [%X] Type [%X]\n",
         (int)m_InputSize,(int)m_FileData.ChunkType);
   //===================================================================
         fread(&m_MaterialInfo,sizeof(W3dMaterialInfoStruct),MAX_RECORD,m_Reader);
   //================================================== DEBUG_OUTPUT ===
   printf("PassCount + VertexMaterialCount: [%X]  [%X]\n",
         (int)m_MaterialInfo.PassCount,(int)m_MaterialInfo.VertexMaterialCount);
   printf("ShaderCount + TextureCount: [%X]  [%X]\n",
         (int)m_MaterialInfo.ShaderCount,(int)m_MaterialInfo.TextureCount);
   //===================================================================
         break;
      case W3D_CHUNK_VERTEX_MATERIALS:
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_VERTEX_MATERIALS: [%X] Type [%X] Cnts [%X]\n",(int)m_InputSize,
         (int)m_FileData.ChunkType, (int)m_MaterialInfo.VertexMaterialCount);
   //===================================================================
         for (m_Count = 0; m_Count < m_MaterialInfo.VertexMaterialCount; ++m_Count)
         {
            ReadDataChunk();
            if (m_FileData.ChunkType == W3D_CHUNK_VERTEX_MATERIAL)
            {
               LoadW3dMaterialStruct(m_FileData.ChunkSize);
            }
            else
            {
               ReadDataChunk(m_FileData.ChunkSize);
            }
         }
         break;

      case W3D_CHUNK_SHADERS:
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_SHADERS: [%X] Type [%X] Cnts [%X]\n",(int)m_InputSize,
         (int)m_FileData.ChunkType, (int)m_MaterialInfo.ShaderCount);
   //===================================================================
         for (m_Count = 0; m_Count < m_MaterialInfo.ShaderCount; ++m_Count)
         {
            memset(&m_Shaders,0,sizeof(W3dShaderStruct));
            fread(&m_Shaders,sizeof(W3dShaderStruct),MAX_RECORD,m_Reader);
            m_ShadersList.push_back(m_Shaders);
         }
   //================================================== DEBUG_OUTPUT ===
   if (m_MaterialInfo.ShaderCount == 1)
   {
      printf("DepthCompare: %X\n",(int)m_Shaders.DepthCompare);
      printf("DepthMask   : %X\n",(int)m_Shaders.DepthMask);
      printf("ColorMask   : %X\n",(int)m_Shaders.ColorMask);
      printf("DestBlend   : %X\n",(int)m_Shaders.DestBlend);
      printf("FogFunc     : %X\n",(int)m_Shaders.FogFunc);
      printf("PriGradient : %X\n",(int)m_Shaders.PriGradient);
      printf("SecGradient : %X\n",(int)m_Shaders.SecGradient);
      printf("SrcBlend    : %X\n",(int)m_Shaders.SrcBlend);
      printf("Texturing   : %X\n",(int)m_Shaders.Texturing);
      printf("DetailColorFunc: %X\n",(int)m_Shaders.DetailColorFunc);
      printf("DetailAlphaFunc: %X\n",(int)m_Shaders.DetailAlphaFunc);
      printf("ShaderPreset: %X\n",(int)m_Shaders.ShaderPreset);
      printf("AlphaTest   : %X\n",(int)m_Shaders.AlphaTest);
      printf("PostDetailColorFunc: %X\n",(int)m_Shaders.PostDetailColorFunc);
      printf("PostDetailAlphaFunc: %X\n",(int)m_Shaders.PostDetailAlphaFunc);
      printf("pad[1]      : %X\n",(int)m_Shaders.pad[1]);
   }
   //===================================================================
         break;

      case W3D_CHUNK_TEXTURES:
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_TEXTURES: [%X] Type [%X] Cnts [%X]\n",(int)m_InputSize,
         (int)m_FileData.ChunkType, (int)m_MaterialInfo.TextureCount);
   //===================================================================
         for (m_Count = 0; m_Count < m_MaterialInfo.TextureCount; ++m_Count)
         {
            ReadDataChunk();
            if (m_FileData.ChunkType == W3D_CHUNK_TEXTURE)
            {
               LoadW3dTextureStruct(m_FileData.ChunkSize);
            }
            else
            {
               ReadDataChunk(m_FileData.ChunkSize);
            }
         }
         break;
      case W3D_CHUNK_MATERIAL_PASS:
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_MATERIAL_PASS: [%X] Type [%X]\n\n",
         (int)m_InputSize,(int)m_FileData.ChunkType);
   //===================================================================
         LoadW3dMaterialPassStruct(m_FileData.ChunkSize);
         break;

   //////////////////////////////////////////////////////////////////////
   //BFME2: New Texture/Material structure ....
   //////////////////////////////////////////////////////////////////////
     case W3D_CHUNK_NORMALMAP_INFO:
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_NORMALMAP_INFO: [%X] Type [%X] Size [%X]\n",
         (int)m_InputSize,(int)m_FileData.ChunkType,(int)m_FileData.ChunkSize);
   //===================================================================
         m_NormOffset = m_FileData.ChunkSize;
         break;
     case W3D_CHUNK_NORMALMAP_FLAG1:
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_NORMALMAP_FLAG1: [%X] Type [%X]\n",
         (int)m_InputSize,(int)m_FileData.ChunkType);
   //===================================================================
         break;
     case W3D_CHUNK_NORMALMAP_FLAG2:
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_NORMALMAP_FLAG2: [%X] Type [%X]\n",
         (int)m_InputSize,(int)m_FileData.ChunkType);
   //===================================================================
         memset(&m_NormHeader,0,sizeof(W3dNormMapHeaderStruct));
         fread(&m_NormHeader,m_FileData.ChunkSize,MAX_RECORD,m_Reader);
         LoadW3dMaterialNormStruct(m_NormOffset);
         LoadW3dTextureStateCoords(m_InputSize);
         break;

      case W3D_CHUNK_MESH_USER_TEXT:
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_MESH_USER_TEXT: [%X] Size [%X]\n",(int)m_InputSize,
         (int)m_FileData.ChunkSize);
   //===================================================================
         ReadDataChunk(m_FileData.ChunkSize);
         break;
      default:
   //================================================== DEBUG_OUTPUT ===
   if (m_FileData.ChunkSize > 0) {
      printf("### MESH_DEFAULT >>>>: Size [%X] DataOffset [%X]\n\n",
            (int)m_FileData.ChunkSize,(int)DataOffset);
   }
   //===================================================================
         ReadDataChunk(m_FileData.ChunkSize);
         break;
      }
   }

   //NOTE: There are meshes without textures in BFME2 ....
   if (!m_TextureFound) {
      memset(&m_Textures,0,sizeof(W3dTextureStruct));
      m_Textures.LinkMap = NONE_NORMALMAP_REFERENCE;
      m_Textures.Name[0] = '?';
      SaveInfoTextArray();
   }

   //NOTE: Synchronizes the request for vertex influences ....
   m_mesh += (m_mesh < MAX_MESH) ? 1 : 0;

   //NOTE: The FilePointer is off by 8 bytes so reset to next mesh ...
   if (m_Offset < m_FileSize) {
      fseek(m_Reader,m_Offset,SEEK_SET);
   }
}

// ---------------------------------------------------------- HIERARCHY ---
// ------------------------------------------------------------------------
void CW3DModelMesh::LoadW3dHierarchyStruct (int idx, unsigned long DataOffset)
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
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_HIERARCHY_HEADER: [%X] Type [%X] Name [%s] Cnts [%X]\n",
         (int)m_InputSize,(int)m_FileData.ChunkType,m_Skeleton[m_next].Name,
         (int)m_Skeleton[m_next].NumPivots);
   //===================================================================
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

// ---------------------------------------------------------- ANIMATION ---
// ------------------------------------------------------------------------
void CW3DModelMesh::LoadW3dAnimationStruct (unsigned long DataOffset)
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
         break;
      case W3D_CHUNK_ANIMATION_CHANNEL:
         for (m_Count = 0;m_Count < (m_AnimHeader.NumFrames); ++m_Count)
         {
            memset(&m_AnimChannel,0,sizeof(W3dAnimChannelStruct));
            fread(&m_AnimChannel,sizeof(W3dAnimChannelStruct),MAX_RECORD,m_Reader);
            m_AnimChannelList[0].push_back(m_AnimChannel);
         }
         break;
      case W3D_CHUNK_BIT_CHANNEL:
         for (m_Count = 0;m_Count < (m_AnimHeader.NumFrames); ++m_Count)
         {
            memset(&m_BitChannel,0,sizeof(W3dBitChannelStruct));
            fread(&m_BitChannel,sizeof(W3dBitChannelStruct),MAX_RECORD,m_Reader);
            m_BitChannelList[0].push_back(m_BitChannel);
         }
         break;
      default:
         ReadDataChunk(m_FileData.ChunkSize);
         break;
      }
   }
}

// ------------------------------------------------------------------------
void CW3DModelMesh::LoadW3dCompAnimationStruct (unsigned long DataOffset)
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
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_COMPRESSED_ANIMATION_HEADER: [%X] Type [%X] Name [%s.%s] Cnts [%X]\n",
         (int)m_InputSize,(int)m_FileData.ChunkType,m_CompAnimHeader.Name,
          m_CompAnimHeader.HierarchyName,(int)m_CompAnimHeader.NumFrames);
   //===================================================================
         break;

      case W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL:
         for (m_Count = 0;m_Count < (m_CompAnimHeader.NumFrames); ++m_Count)
         {
            memset(&m_AnimChannel,0,sizeof(W3dAnimChannelStruct));
            fread(&m_AnimChannel,sizeof(W3dAnimChannelStruct),MAX_RECORD,m_Reader);
            m_AnimChannelList[1].push_back(m_AnimChannel);
         }
         break;
      case W3D_CHUNK_COMPRESSED_BIT_CHANNEL:
         for (m_Count = 0;m_Count < (m_CompAnimHeader.NumFrames); ++m_Count)
         {
            memset(&m_BitChannel,0,sizeof(W3dBitChannelStruct));
            fread(&m_BitChannel,sizeof(W3dBitChannelStruct),MAX_RECORD,m_Reader);
            m_BitChannelList[1].push_back(m_BitChannel);
         }
         break;
      default:
         ReadDataChunk(m_FileData.ChunkSize);
         break;
      }
   }
}

// ----------------------------------------------------------- HLODDATA ---
// ------------------------------------------------------------------------
void CW3DModelMesh::LoadW3dHModelStruct (int idx, unsigned long DataOffset)
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
      case W3D_CHUNK_HMODEL_HEADER:
         memset(&m_HModelHeader[m_next],0,sizeof(W3dHModelHeaderStruct));
         fread(&m_HModelHeader[m_next],sizeof(W3dHModelHeaderStruct),MAX_RECORD,m_Reader);
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_HMODEL_HEADER: [%X] Type [%X] Name [%s] Cnts [%X]\n",
         (int)m_InputSize,(int)m_FileData.ChunkType,
          m_HModelHeader[m_next].HierarchyName,(int)m_HModelHeader[m_next].NumConnections);
   //===================================================================
         break;
      case W3D_CHUNK_NODE:
      case W3D_CHUNK_SKIN_NODE:
      case W3D_CHUNK_COLLISION_NODE:
         memset(&m_HModelNode,0,sizeof(W3dHModelNodeStruct));
         fread(&m_HModelNode,sizeof(W3dHModelNodeStruct),MAX_RECORD,m_Reader);
         m_HModelNodeList[m_next].push_back(m_HModelNode);
         break;
      default:
         ReadDataChunk(m_FileData.ChunkSize);
         break;
      }
   }
}

// ------------------------------------------------------------------------
void CW3DModelMesh::LoadW3dHLODStruct (int idx, unsigned long DataOffset)
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
      case W3D_CHUNK_HLOD_HEADER:
         memset(&m_HLodHeader[m_next].Header,0,sizeof(W3dHLodHeaderStruct));
         fread(&m_HLodHeader[m_next].Header,sizeof(W3dHLodHeaderStruct),MAX_RECORD,m_Reader);
         m_HLodHeader[m_next].Offset = m_DataOffset;
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_HLOD_HEADER: [%X] Type [%X] Name [%s] Cnts [%X]\n",
         (int)m_InputSize,(int)m_FileData.ChunkType,m_HLodHeader[m_next].Header.HierarchyName,
         (int)m_HLodHeader[m_next].Header.LodCount);
   //===================================================================

         memset(m_PivotName,0,MAX_PATH);
         if (m_PivotsList[0].size() == 0 && m_PivotsList[1].size() == 0)
         {
            sprintf(m_PivotName,"\\%s.w3d",m_HLodHeader[m_next].Header.HierarchyName);
         }
         break;
      case W3D_CHUNK_HLOD_LOD_ARRAY:
         LoadW3dLODArrayStruct(m_next, m_FileData.ChunkSize);
         break;
      default:
         ReadDataChunk(m_FileData.ChunkSize);
         break;
      }
   }
}

// ------------------------------------------------------------------------
void CW3DModelMesh::LoadW3dLODArrayStruct (int idx, unsigned long DataOffset)
// ------------------------------------------------------------------------
{
   m_next = (idx < MAX_ARRAY) ? idx : 1;
   int ItemCount = 0;
   m_InputSize = 0;

   while (m_InputSize < DataOffset)
   {
      ReadDataChunk();
      m_InputSize += (MAX_CHUNK + m_FileData.ChunkSize);

      switch (m_FileData.ChunkType)
      {
      case W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER:
         memset(&m_HLodArray[m_next],0,sizeof(W3dHLodArrayHeaderStruct));
         fread(&m_HLodArray[m_next],sizeof(W3dHLodArrayHeaderStruct),MAX_RECORD,m_Reader);
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_HLOD_HEADER: [%X] Type [%X] Cnts [%X]\n",(int)m_InputSize,
         (int)m_FileData.ChunkType, (int)m_HLodArray[m_next].Header.ModelCount);
   //===================================================================
         break;

      case W3D_CHUNK_HLOD_SUB_OBJECT:
         fread(&m_HLodArray[m_next].SubObject[ItemCount],sizeof(W3dHLodSubObjectStruct),MAX_RECORD,m_Reader);
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_HLOD_SUB_OBJECT: [%X] Type [%X] Name [%s]\n",(int)m_InputSize,
         (int)m_FileData.ChunkType, m_HLodArray[m_next].SubObject[ItemCount].Name);
   //===================================================================
         if (ItemCount < MAX_OBJECT) {
            ++ ItemCount;
         }
         m_HLodArray[m_next].SubObjectCount = ItemCount;
         break;

      default:
         ReadDataChunk(m_FileData.ChunkSize);
         break;
      }
   }
}

// ----------------------------------------------------------- MATERIAL ---
// ------------------------------------------------------------------------
void CW3DModelMesh::LoadW3dMaterialNormStruct (unsigned long DataOffset)
// ------------------------------------------------------------------------
{
   //================================================== DEBUG_OUTPUT ===
   printf("### LoadW3dMaterialNormStruct: [%X] Size [%X] [%s]\n\n",
         (int)m_InputSize,(int)DataOffset, m_NormHeader.TypeName);
   //===================================================================

   m_InputData = 0;
   while (m_InputData < DataOffset)
   {
      ReadDataChunk();
      m_InputData += (MAX_CHUNK + m_FileData.ChunkSize);

      switch (m_FileData.ChunkType)
      {
      case W3D_CHUNK_NORMALMAP_FLAG3:
    //================================================== DEBUG_OUTPUT ===
    printf("### W3D_CHUNK_NORMALMAP_FLAG3: [%X] Type [%X] Size [%X]\n",
          (int)m_InputData,(int)m_FileData.ChunkType, (int)m_FileData.ChunkSize);
    //===================================================================

          memset(&m_NormEntry,0,sizeof(W3dNormMapEntryStruct));
          fread(&m_NormEntry,MAX_CHUNK,MAX_RECORD,m_Reader);
          fread(&m_NormEntry.InfoName[0],m_NormEntry.TypeSize,MAX_RECORD,m_Reader);

          switch (m_NormEntry.TypeFlag)
          {
          case W3D_NORMTYPE_TEXTURE:
               fread(&m_NormEntry.ItemSize,MAX_LONG,MAX_RECORD,m_Reader);
               fread(&m_NormEntry.ItemName[0],m_NormEntry.ItemSize,MAX_RECORD,m_Reader);

               if (strcmp(m_NormEntry.InfoName,"DiffuseTexture") == 0)
               {
                  memset(&m_Textures,0,sizeof(W3dTextureStruct));
                  strcpy(m_Textures.Name,m_NormEntry.ItemName);
                  m_Textures.LinkMap = m_MapIDCount;
                  m_TextureFound = true;
                  SaveInfoTextArray();
               }
               else
               if (strcmp(m_NormEntry.InfoName,"NormalMap") == 0)
               {
                  m_NormalMapList.push_back(m_NormEntry);
                  ++ m_MapIDCount;
               }
               break;
          case W3D_NORMTYPE_BUMP:
               ReadDataChunk(MAX_LONG);
               break;
          case W3D_NORMTYPE_COLORS:
               ReadDataChunk();  ReadDataChunk();
               break;
          case W3D_NORMTYPE_ALPHA:
               ReadDataChunk(1);
               break;
          }

    //================================================== DEBUG_OUTPUT ===
    printf("### [%X] [%X] Info [%s = %s]\n\n",(int)m_NormEntry.TypeSize,
          (int)m_NormEntry.ItemSize,m_NormEntry.InfoName,m_NormEntry.ItemName);
    //===================================================================
          break;
      }
   }
}

// ------------------------------------------------------------------------
void CW3DModelMesh::LoadW3dTextureStateCoords (unsigned long DataOffset)
// ------------------------------------------------------------------------
{
   //================================================== DEBUG_OUTPUT ===
   //printf("### LoadW3dTextureStateCoords: Offsets [%X] [%X]\n\n",
   //      (int)m_InputSize,(int)DataOffset);
   //===================================================================
   unsigned int TextureCoordCount = 0;
   m_InputData = DataOffset;

   //NOTE: Some meshes have per-vertex diffuse color values
   ReadDataChunk();
   m_InputData += MAX_CHUNK;
   if (m_FileData.ChunkType == W3D_CHUNK_DCG )
   {
       ReadDataChunk(m_FileData.ChunkSize);
       m_InputData += m_FileData.ChunkSize;
   }

   //NOTE: Advance offset to W3D_CHUNK_STAGE_TEXCOORDS
   ReadDataChunk(MAX_LONG);
   m_InputData += MAX_LONG;
   ReadDataChunk();
   m_InputData += (MAX_CHUNK + m_FileData.ChunkSize);
   if (m_FileData.ChunkType == W3D_VERTEX_CHANNEL_TEXCOORD)
   {
      ReadDataChunk();
      m_InputData += (MAX_CHUNK + m_FileData.ChunkSize);
   }

    //================================================== DEBUG_OUTPUT ===
    printf("### LoadW3dTextureStateCoords: Type [%X] Size [%X]\n",
          (int)m_FileData.ChunkType, (int)m_FileData.ChunkSize);
    //===================================================================

   if (m_FileData.ChunkType == W3D_CHUNK_STAGE_TEXCOORDS)
   {
      TextureCoordCount = (m_FileData.ChunkSize / sizeof(W3dTexCoordStruct));
    //================================================== DEBUG_OUTPUT ===
    printf("### W3D_CHUNK_STAGE_TEXCOORDS: [%X] Type [%X] Cnts [%X]\n",(int)m_InputSize,
          (int)m_FileData.ChunkType, (int)TextureCoordCount);
    //===================================================================
      for (m_Count = 0; m_Count < TextureCoordCount; ++m_Count)
      {
         memset(&m_TextureCoord,0,sizeof(W3dTexCoordStruct));
         fread(&m_TextureCoord,sizeof(W3dTexCoordStruct),MAX_RECORD,m_Reader);
         m_TextureCoordsList.push_back(m_TextureCoord);
       //================================================== DEBUG_OUTPUT ===
       //printf("m_TextureCoord.U + V: [%f %f]\n",m_TextureCoord.U,m_TextureCoord.V);
       //===================================================================
      }
   }
}

// ------------------------------------------------------------------------
void CW3DModelMesh::LoadW3dMaterialStruct (unsigned long DataOffset)
// ------------------------------------------------------------------------
{
   memset(&m_Materials,0,sizeof(W3dMaterialStruct));
   m_InputSize = 0;

   while (m_InputSize < DataOffset)
   {
      ReadDataChunk();
      m_InputSize += (MAX_CHUNK + m_FileData.ChunkSize);

      switch (m_FileData.ChunkType)
      {
      case W3D_CHUNK_VERTEX_MATERIAL_NAME:
         fread(m_Materials.Name,m_FileData.ChunkSize,MAX_RECORD,m_Reader);
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_VERTEX_MATERIAL_NAME: [%X] Type [%X] Name [%s]\n",(int)m_InputSize,
         (int)m_FileData.ChunkType, m_Materials.Name);
   //===================================================================
         break;

      case W3D_CHUNK_VERTEX_MAPPER_ARGS0:
         ReadDataChunk(m_FileData.ChunkSize);
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_VERTEX_MAPPER_ARGS0: [%X] Type [%X] Arg0 ...\n",
         (int)m_InputSize,(int)m_FileData.ChunkType);  //m_Materials.MapperArgs0
   //===================================================================
         break;
      case W3D_CHUNK_VERTEX_MAPPER_ARGS1:
         ReadDataChunk(m_FileData.ChunkSize);
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_VERTEX_MAPPER_ARGS1: [%X] Type [%X] Arg1 ...\n",
         (int)m_InputSize,(int)m_FileData.ChunkType);  //m_Materials.MapperArgs1
   //===================================================================
         break;

      case W3D_CHUNK_VERTEX_MATERIAL_INFO:
         fread(&m_Materials.VertexMaterialInfo,sizeof(m_Materials.VertexMaterialInfo),MAX_RECORD,m_Reader);
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_VERTEX_MATERIAL_INFO: [%X] Type [%X] Size [%X]\n",(int)m_InputSize,
         (int)m_FileData.ChunkType, (int)sizeof(m_Materials.VertexMaterialInfo));
   //===================================================================

   //===================================================================
   printf("Attributes  : %X\n",(int)m_Materials.VertexMaterialInfo.Attributes);
   printf("Ambient_RGB : %X %X %X\n",(int)m_Materials.VertexMaterialInfo.Ambient.R,
                                     (int)m_Materials.VertexMaterialInfo.Ambient.G,
                                     (int)m_Materials.VertexMaterialInfo.Ambient.B);

   printf("Diffuse_RGB : %X %X %X\n",(int)m_Materials.VertexMaterialInfo.Diffuse.R,
                                     (int)m_Materials.VertexMaterialInfo.Diffuse.G,
                                     (int)m_Materials.VertexMaterialInfo.Diffuse.B);

   printf("Specular_RGB: %X %X %X\n",(int)m_Materials.VertexMaterialInfo.Specular.R,
                                     (int)m_Materials.VertexMaterialInfo.Specular.G,
                                     (int)m_Materials.VertexMaterialInfo.Specular.B);

   printf("Emissive_RGB: %X %X %X\n",(int)m_Materials.VertexMaterialInfo.Emissive.R,
                                     (int)m_Materials.VertexMaterialInfo.Emissive.G,
                                     (int)m_Materials.VertexMaterialInfo.Emissive.B);

   printf("Shininess   : %f\n",m_Materials.VertexMaterialInfo.Shininess);
   printf("Opacity     : %f\n",m_Materials.VertexMaterialInfo.Opacity);
   printf("Translucency: %f\n",m_Materials.VertexMaterialInfo.Translucency);
   printf("\n");
   //===================================================================
         break;
      default:
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_MATERIAL_DEFAULT >>>>: [%X] Size [%X]\n",(int)m_InputSize,
         (int)m_FileData.ChunkSize);
   //===================================================================
         ReadDataChunk(m_FileData.ChunkSize);
         break;
      }
   }

   if (strlen(m_Materials.Name) > 0) {
      m_MaterialsList.push_back(m_Materials);
   }
}

// ------------------------------------------------------------------------
void CW3DModelMesh::LoadW3dTextureStruct (unsigned long DataOffset)
// ------------------------------------------------------------------------
{
   memset(&m_Textures,0,sizeof(W3dTextureStruct));
   m_Textures.LinkMap = NONE_NORMALMAP_REFERENCE;
   m_InputSize = 0;

   while (m_InputSize < DataOffset)
   {
      ReadDataChunk();
      m_InputSize += (MAX_CHUNK + m_FileData.ChunkSize);

      switch (m_FileData.ChunkType)
      {
      case W3D_CHUNK_TEXTURE_NAME:
         fread(m_Textures.Name,m_FileData.ChunkSize,MAX_RECORD,m_Reader);
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_TEXTURE_NAME: [%X] Type [%X] Name [%s]\n",(int)m_InputSize,
         (int)m_FileData.ChunkType, m_Textures.Name);
   //===================================================================
         m_TextureFound = true;
         SaveInfoTextArray();
         break;
      case W3D_CHUNK_TEXTURE_INFO:
         fread(&m_Textures.TextureInfo,sizeof(W3dTextureInfoStruct),MAX_RECORD,m_Reader);
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_TEXTURE_INFO: [%X] Type [%X] Cnts [%X]\n",(int)m_InputSize,
         (int)m_FileData.ChunkType, (int)m_Textures.TextureInfo.Attributes);
   //===================================================================
   //================================================== DEBUG_OUTPUT ===
   printf("Attributes: [%X]\n",(int)m_Textures.TextureInfo.Attributes);
   printf("AnimType  : [%X]\n",(int)m_Textures.TextureInfo.AnimType);
   printf("FrameCount: [%X]\n",(int)m_Textures.TextureInfo.FrameCount);
   printf("FrameRate : [%f]\n", m_Textures.TextureInfo.FrameRate);
   printf("\n");
   //===================================================================
         break;
      default:
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_TEXTURE_DEFAULT >>>>: [%X] Size [%X]\n",(int)m_InputSize,
         (int)m_FileData.ChunkSize);
   //===================================================================
         ReadDataChunk(m_FileData.ChunkSize);
         break;
      }
   }

   //NOTE: There are meshes without textures in BFME2 ....
   if (strlen(m_Textures.Name) == 0) {
      m_Textures.Name[0] = '?';
   }
   m_TexturesList.push_back(m_Textures);
}

// ------------------------------------------------------------------------
void CW3DModelMesh::LoadW3dMaterialPassStruct (unsigned long DataOffset)
// ------------------------------------------------------------------------
{
   unsigned int VertexMaterialIDCount = 0;
   unsigned int ShaderIDCount = 0;
   unsigned int ColorArray1 = 0;
   unsigned int ColorArray2 = 0;
   unsigned int ColorArray3 = 0;
   m_InputSize = 0;

   while (m_InputSize < DataOffset)
   {
      ReadDataChunk();
      m_InputSize += (MAX_CHUNK + m_FileData.ChunkSize);

      switch (m_FileData.ChunkType)
      {
      case W3D_CHUNK_VERTEX_MATERIAL_IDS:
         VertexMaterialIDCount = (m_FileData.ChunkSize / sizeof(uint32));
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_VERTEX_MATERIAL_IDS: [%X] Type [%X] Cnts [%X]\n",(int)m_InputSize,
         (int)m_FileData.ChunkType, (int)VertexMaterialIDCount);
   //===================================================================

         for (m_Count = 0; m_Count < VertexMaterialIDCount; ++m_Count)
         {
            fread(&m_VtxMaterialID,sizeof(uint32),MAX_RECORD,m_Reader);
            m_VtxMaterialIDsList.push_back(m_VtxMaterialID);
   //================================================== DEBUG_OUTPUT ===
   printf("m_VtxMaterialID: [%X]\n",(int)m_VtxMaterialID);
   //===================================================================
         }
         break;
      case W3D_CHUNK_SHADER_IDS:
         ShaderIDCount = (m_FileData.ChunkSize / sizeof(uint32));
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_SHADER_IDS: [%X] Type [%X] Cnts [%X]\n",(int)m_InputSize,
         (int)m_FileData.ChunkType, (int)ShaderIDCount);
   //===================================================================

         for (m_Count = 0; m_Count < ShaderIDCount; ++m_Count)
         {
            fread(&m_ShaderID,sizeof(uint32),MAX_RECORD,m_Reader);
            m_ShaderIDsList.push_back(m_ShaderID);
   //================================================== DEBUG_OUTPUT ===
   printf("m_ShaderID: [%X]\n",(int)m_ShaderID);
   //===================================================================
         }
         break;

      case W3D_CHUNK_DCG:
         ColorArray1 = (m_FileData.ChunkSize / sizeof(W3dRGBAStruct));
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_DCG: [%X] Type [%X] Cnts [%X]\n",(int)m_InputSize,
         (int)m_FileData.ChunkType, (int)ColorArray1);
   //===================================================================
         for (m_Count = 0; m_Count < ColorArray1; ++m_Count)
         {
            memset(&m_RGBAcolors,0,sizeof(W3dRGBAStruct));
            fread(&m_RGBAcolors,sizeof(W3dRGBAStruct),MAX_RECORD,m_Reader);
            m_RGBAcolorList[0].push_back(m_RGBAcolors);
         }
         break;
      case W3D_CHUNK_DIG:
         ColorArray2 = (m_FileData.ChunkSize / sizeof(W3dRGBAStruct));
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_DIG: [%X] Type [%X] Cnts [%X]\n",(int)m_InputSize,
         (int)m_FileData.ChunkType, (int)ColorArray2);
   //===================================================================
         for (m_Count = 0; m_Count < ColorArray2; ++m_Count)
         {
            memset(&m_RGBAcolors,0,sizeof(W3dRGBAStruct));
            fread(&m_RGBAcolors,sizeof(W3dRGBAStruct),MAX_RECORD,m_Reader);
            m_RGBAcolorList[1].push_back(m_RGBAcolors);
         }
         break;
      case W3D_CHUNK_SCG:
         ColorArray3 = (m_FileData.ChunkSize / sizeof(W3dRGBAStruct));
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_SCG: [%X] Type [%X] Cnts [%X]\n",(int)m_InputSize,
         (int)m_FileData.ChunkType, (int)ColorArray3);
   //===================================================================
         for (m_Count = 0; m_Count < ColorArray3; ++m_Count)
         {
            memset(&m_RGBAcolors,0,sizeof(W3dRGBAStruct));
            fread(&m_RGBAcolors,sizeof(W3dRGBAStruct),MAX_RECORD,m_Reader);
            m_RGBAcolorList[2].push_back(m_RGBAcolors);
         }
         break;
      case W3D_CHUNK_TEXTURE_STAGE:
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_TEXTURE_STAGE: [%X] Type [%X] Size [%X]\n",(int)m_InputSize,
         (int)m_FileData.ChunkType,(int)m_FileData.ChunkSize);
   //===================================================================
         LoadW3dTextureStageStruct(m_FileData.ChunkSize);
         break;
      default:
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_MATERIALPASS_DEFAULT >>>>: [%X] Size [%X]\n",(int)m_InputSize,
         (int)m_FileData.ChunkSize);
   //===================================================================
         ReadDataChunk(m_FileData.ChunkSize);
         break;
      }
   }
}

// ------------------------------------------------------------------------
void CW3DModelMesh::LoadW3dTextureStageStruct (unsigned long DataOffset)
// ------------------------------------------------------------------------
{
   //### W3D_CHUNK_TEXTURE_IDS: [C] Type [49] Cnts [1]
   //### W3D_CHUNK_STAGE_TEXCOORDS: [F84] Type [4A] Cnts [1EE]

   unsigned int TextureCoordCount = 0;
   unsigned int TexCoordIDCount = 0;
   unsigned int TextureIDCount = 0;
   m_InputSize = 0;

   while (m_InputSize < DataOffset)
   {
      ReadDataChunk();
      m_InputSize += (MAX_CHUNK + m_FileData.ChunkSize);

      switch (m_FileData.ChunkType)
      {
      case W3D_CHUNK_TEXTURE_IDS:
         TextureIDCount = (m_FileData.ChunkSize / sizeof(uint32));
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_TEXTURE_IDS: [%X] Type [%X] Cnts [%X]\n",(int)m_InputSize,
         (int)m_FileData.ChunkType, (int)TextureIDCount);
   //===================================================================
         for (m_Count = 0; m_Count < TextureIDCount; ++m_Count)
         {
            fread(&m_TextureID,sizeof(uint32),MAX_RECORD,m_Reader);
            m_TextureIDsList.push_back(m_TextureID);
   //================================================== DEBUG_OUTPUT ===
   printf("m_TextureID: [%X]\n",(int)m_TextureID);
   //===================================================================
         }
         break;

      case W3D_CHUNK_PER_FACE_TEXCOORD_IDS:
         TexCoordIDCount = (m_FileData.ChunkSize / sizeof(uint32));
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_PER_FACE_TEXCOORD_IDS: [%X] Type [%X] Cnts [%X]\n",(int)m_InputSize,
         (int)m_FileData.ChunkType, (int)TexCoordIDCount);
   //===================================================================
         for (m_Count = 0; m_Count < TexCoordIDCount; ++m_Count)
         {
            memset(&m_PerFaceTexCoordID,0,sizeof(Vector3i));
            fread(&m_PerFaceTexCoordID,sizeof(Vector3i),MAX_RECORD,m_Reader);
            m_PerFaceTexCoordIDsList.push_back(m_PerFaceTexCoordID);
         }
         break;

      case W3D_CHUNK_STAGE_TEXCOORDS:
         TextureCoordCount = (m_FileData.ChunkSize / sizeof(W3dTexCoordStruct));
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_CHUNK_STAGE_TEXCOORDS: [%X] Type [%X] Cnts [%X]\n",(int)m_InputSize,
         (int)m_FileData.ChunkType, (int)TextureCoordCount);
   //===================================================================
         for (m_Count = 0; m_Count < TextureCoordCount; ++m_Count)
         {
            memset(&m_TextureCoord,0,sizeof(W3dTexCoordStruct));
            fread(&m_TextureCoord,sizeof(W3dTexCoordStruct),MAX_RECORD,m_Reader);
            m_TextureCoordsList.push_back(m_TextureCoord);
   //================================================== DEBUG_OUTPUT ===
   //printf("m_TextureCoord.U + V: [%f %f]\n",m_TextureCoord.U,m_TextureCoord.V);
   //===================================================================
         }
         break;
      default:
   //================================================== DEBUG_OUTPUT ===
   printf("### W3D_TEXTURESTAGE_DEFAULT >>>>: [%X] Size [%X]\n",(int)m_InputSize,
         (int)m_FileData.ChunkSize);
   //===================================================================
         ReadDataChunk(m_FileData.ChunkSize);
         break;
      }
   }
}

// ----------------------------------------------------------- SAVEDATA ---
// ------------------------------------------------------------------------
void CW3DModelMesh::SaveInfoMeshHeader ()
// ------------------------------------------------------------------------
{
   memset(&m_HeaderInfo,0,sizeof(sMeshHead));
   strcpy(m_HeaderInfo.MeshName, m_W3DHeader.MeshName);
   strcpy(m_HeaderInfo.Container,m_W3DHeader.ContainerName);
   m_HeaderInfo.FaceCount = m_W3DHeader.NumTris;
   m_HeaderInfo.VertCount = m_W3DHeader.NumVertices;

   m_HeaderInfo.DataSize = m_SizeOffset[MESH];
   m_HeaderInfo.Offset = m_DataOffset;
   m_HeaderInfoList.push_back(m_HeaderInfo);
}

// ------------------------------------------------------------------------
void CW3DModelMesh::SaveInfoVtxWeight ()
// ------------------------------------------------------------------------
{
   memset(&m_VertexInfo,0,sizeof(sVertex));
   m_VertexInfo.BoneIndex = m_Influences.BoneIdx;
   m_VertexInfoList[m_mesh].push_back(m_VertexInfo);
}

// ------------------------------------------------------------------------
void CW3DModelMesh::SaveInfoTextArray ()
// ------------------------------------------------------------------------
{
   memset(&m_TextureInfo, 0,sizeof(sTexture));
   strcpy(m_TextureInfo.Name,m_Textures.Name);
   m_TextureInfo.Link = m_Textures.LinkMap;
   m_TextureInfoList.push_back(m_TextureInfo);
}

// ------------------------------------------------------------------------
void CW3DModelMesh::SaveInfoBoundBox ()
// ------------------------------------------------------------------------
{
   memset(&m_BoundBoxInfo,0,sizeof(sBoundBox));
   strcpy(m_BoundBoxInfo.Name,m_BoundingBox.Name);
   m_BoundBoxInfo.DataSize = m_SizeOffset[BBOX];
   m_BoundBoxInfo.Offset = m_DataOffset;

   //===================================================== DEBUG_OUTPUT ===
   printf("\n---------------------------------------------------------------------\n");
   printf("### SAVEINFOBOUNDBOX:   m_SizeOffset [%X] m_DataOffset [%X]\n",
         (int)m_SizeOffset[BBOX],(int)m_DataOffset);
   printf("---------------------------------------------------------------------\n");
   //======================================================================

   m_BoundBoxInfo.Attribute = m_BoundingBox.Attributes;
   m_BoundBoxInfo.Extent_X = m_BoundingBox.Extent.X;
   m_BoundBoxInfo.Extent_Y = m_BoundingBox.Extent.Y;
   m_BoundBoxInfo.Extent_Z = m_BoundingBox.Extent.Z;
}

// ------------------------------------------------------------------------
void CW3DModelMesh::SaveInfoHLodModel ()
// ------------------------------------------------------------------------
{
   memset(&m_HLodHeadInfo,0,sizeof(sHLodHead));
   strcpy(m_HLodHeadInfo.ModelName,m_HLodHeader[0].Header.Name);
   strcpy(m_HLodHeadInfo.SkelName,m_HLodHeader[0].Header.HierarchyName);
   m_HLodHeadInfo.ModelCount = static_cast<int>(m_HLodHeader[0].Header.LodCount);
   m_HLodHeadInfo.Offset = m_HLodHeader[0].Offset;

   m_arraySize = m_HLodArray[0].SubObjectCount;
   for (int idx = 0; idx < m_arraySize; ++ idx)
   {
      memset(&m_HLodItemInfo,0,sizeof(sHLodItem));
      strcpy(m_HLodItemInfo.MeshName,m_HLodArray[0].SubObject[idx].Name);
      m_HLodItemInfo.PivotIndex = m_HLodArray[0].SubObject[idx].BoneIndex;
      m_DetailsInfoList.push_back(m_HLodItemInfo);
   }
}

// ------------------------------------------------------------------------
void CW3DModelMesh::SaveInfoSkelHeader ()
// ------------------------------------------------------------------------
{
   if (strlen(m_Skeleton[0].Name) > 0)
   {
      memset(&m_PivotsHead,0,sizeof(sPivotsHead));
      strcpy(m_PivotsHead.Name,m_Skeleton[0].Name);
      m_PivotsHead.PivotCount = m_Skeleton[0].NumPivots;
      m_PivotsHead.DataSize = m_SizeOffset[BONE];
      m_PivotsHead.Offset   = m_FileOffset[BONE];
      m_PivotsHeadList.push_back(m_PivotsHead);
   }

   if (strlen(m_Skeleton[1].Name) > 0)
   {
      memset(&m_PivotsHead,0,sizeof(sPivotsHead));
      strcpy(m_PivotsHead.Name,m_Skeleton[1].Name);
      m_PivotsHead.PivotCount = m_Skeleton[1].NumPivots;
      m_PivotsHead.DataSize = m_SizeOffset[BONE];
      m_PivotsHead.Offset   = m_FileOffset[BONE];
      m_PivotsHeadList.push_back(m_PivotsHead);
   }

   //NOTE: There is only one Skeleton per BFME-Model ....
   m_HeaderSkel = static_cast<int>(m_PivotsHeadList.size());
   if (m_HeaderSkel > 1) {
      m_HeaderSkel = 1;
   }
}

// ------------------------------------------------------------------------
void CW3DModelMesh::SaveInfoPivotArray ()
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

   m_PivotsSize = static_cast<int>(m_PivotsInfoList.size());
   m_PivotCount = -1;
}

// ----------------------------------------------------------- USERDATA ---
// ------------------------------------------------------------------------
char* CW3DModelMesh::GetHeaderData (const int idx)
// ------------------------------------------------------------------------
{
   memset(m_showData, 0, MAX_OUTPUT);
   if (idx < m_HeaderSize)
   {
      sprintf(m_showData,"%s;%s;%ld;%d;%ld;", m_HeaderInfoList[idx].Container,
              m_HeaderInfoList[idx].MeshName, m_HeaderInfoList[idx].Offset,
              m_HeaderInfoList[idx].VertCount,m_HeaderInfoList[idx].DataSize);
   }
   return m_showData;
}

// ------------------------------------------------------------------------
char* CW3DModelMesh::GetVertexData (const int user, const int idx)
// ------------------------------------------------------------------------
{
   memset(m_showData,0,MAX_OUTPUT);
   if (user < MAX_MESH) {
      if (idx < (int)m_VertexInfoList[user].size())
      {
         sprintf(m_showData,"%d",(int)m_VertexInfoList[user][idx].BoneIndex);
      }
   }
   return m_showData;
}

// ------------------------------------------------------------------------
int  CW3DModelMesh::GetVertexBone (const int user, const int idx)
// ------------------------------------------------------------------------
{
   if (user < MAX_MESH) {
      if (idx < (int)m_VertexInfoList[user].size())
      {
         return (int)m_VertexInfoList[user][idx].BoneIndex;
      }
   }
   return -1;
}

// ------------------------------------------------------------------------
int CW3DModelMesh::GetVertexSize (const int idx)
// ------------------------------------------------------------------------
{
   return (idx < MAX_MESH) ? (int)m_VertexInfoList[idx].size() : 0;
}

// ------------------------------------------------------------------------
char* CW3DModelMesh::GetTextureData (const int idx)
// ------------------------------------------------------------------------
{
   memset(m_showData,0,MAX_OUTPUT);
   if (idx < m_TextureSize)
   {
      if (m_TextureInfoList[idx].Link == NONE_NORMALMAP_REFERENCE)
      {
         strcpy(m_showData,m_TextureInfoList[idx].Name);
      }
      else
      {
         sprintf(m_showData,"%s;%s",m_TextureInfoList[idx].Name,
                 m_NormalMapList[m_TextureInfoList[idx].Link].ItemName);
      }
   }
   return m_showData;
}

// ------------------------------------------------------------------------
char* CW3DModelMesh::GetBBoxDetails ()
// ------------------------------------------------------------------------
{
   memset(m_showData, 0, MAX_OUTPUT);
   if (m_BoundBoxInfo.Name[0] != 0 && m_BoundBoxInfo.Name[1] != 0)
   {
      sprintf(m_showData,"%s;%ld;%ld;%ld;%.4f %.4f %.4f;",m_BoundBoxInfo.Name,
              m_BoundBoxInfo.DataSize, m_BoundBoxInfo.Offset,
              m_BoundBoxInfo.Attribute,m_BoundBoxInfo.Extent_X,
              m_BoundBoxInfo.Extent_Y, m_BoundBoxInfo.Extent_Z);
   }
   return m_showData;
}

// ------------------------------------------------------------------------
char* CW3DModelMesh::GetHierLodInfo ()
// ------------------------------------------------------------------------
{
   memset(m_showData, 0, MAX_OUTPUT);
   if (m_HLodHeadInfo.ModelName[0] != 0 && m_HLodHeadInfo.ModelName[1] != 0)
   {
      sprintf(m_showData,"%s;%s;%ld;",m_HLodHeadInfo.ModelName,
              m_HLodHeadInfo.SkelName,m_HLodHeadInfo.Offset);
   }
   return m_showData;
}

// ------------------------------------------------------------------------
char* CW3DModelMesh::GetHierLodData (const int idx)
// ------------------------------------------------------------------------
{
   memset(m_showData, 0, MAX_OUTPUT);
   if (idx < m_DetailsSize)
   {
      sprintf(m_showData,"%s;%d;",m_DetailsInfoList[idx].MeshName,
              m_DetailsInfoList[idx].PivotIndex);
   }
   return m_showData;
}

// ------------------------------------------------------------------------
bool CW3DModelMesh::GetPivotsName (char* skeletonName)
// ------------------------------------------------------------------------
{
   //NOTE: This removes the previously inserted backslash ...
   strcpy (skeletonName,&m_PivotName[1]);
   return (strlen(m_PivotName) == 0) ? false : true;
}

// ------------------------------------------------------------------------
char* CW3DModelMesh::GetPivotsHead (const int idx)
// ------------------------------------------------------------------------
{
   memset(m_showData,0,MAX_OUTPUT);
   if (idx <= m_HeaderSkel)
   {
      sprintf(m_showData,"%s;%ld;%ld;%ld;",m_PivotsHeadList[idx].Name,
          m_PivotsHeadList[idx].PivotCount,m_PivotsHeadList[idx].DataSize,
          m_PivotsHeadList[idx].Offset);
   }
   return m_showData;
}

// ------------------------------------------------------------------------
char* CW3DModelMesh::GetPivotsData (const int idx)
// ------------------------------------------------------------------------
{
   memset(m_showData,0,MAX_OUTPUT);
   if (idx <= m_PivotsSize)
   {
      //m_PivotCount += 1;
      sprintf(m_showData,"%s;%d;",m_PivotsInfoList[idx].Name,
              m_PivotsInfoList[idx].BoneIndex);
   }
   return m_showData;
}

// ************************************************************************
// Module:  W3dModelMesh.cpp -- End of File
// ************************************************************************

