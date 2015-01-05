#pragma once
#include <vector>
#include "W3dModelFile.h"

#ifdef W3D_EXPORT
    #define W3D_EDIT   __declspec( dllexport )
#else
    #define W3D_EDIT   __declspec( dllimport )
#endif

#pragma warning(disable,4251)

class W3D_EDIT W3DModel
{
public:
    struct W3D_EDIT Mesh
    {
        W3dMeshHeader3Struct header;
        W3dVectorStruct* vertices;
        //used in some BFME models
        W3dVectorStruct* vertices_copy;
        W3dVectorStruct* normals;
        //used in some BFME models
        W3dVectorStruct* normals_copy;
        W3dVertInfStruct* influences;
        W3dTriStruct*           triangles;
        unsigned long*          shadeIndices;
        W3dShaderStruct*        shaders;

        W3dNormalMapStruct              normalMap;
        W3dMaterialInfoStruct           materialInfo;
        std::vector<W3dMaterialStruct>  materials;
        std::vector<W3dTextureStruct>   textures;
        W3dMaterialPassStruct           materialpass;
        W3dBoxStruct                    boundingBox;

        char*       usertext;
    };

public:
    void AddMesh(Mesh*);
private:
    std::vector<Mesh*> m_meshes;
};