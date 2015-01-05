#include "W3DModelLoader.hpp"
#include <Windows.h>
#include <iostream>
#include <iomanip>
W3DModel* W3DModelLoader::LoadModel(const std::string& filename)
{
    std::ifstream fin(filename,std::ios::binary);
    auto filesize = GetFileSize(fin);
    W3DModel* model = new W3DModel();

    while (static_cast<uint32>(fin.tellg())!=filesize)
    {
        auto chunktype  = Read<uint32>(fin);
        auto chunksize = GetChunkSize(Read<uint32>(fin));

        switch (chunktype)
        {

        case W3D_CHUNK_MESH:
            model->AddMesh(LoadMeshChunk(fin, chunksize));
            break;
        case W3D_CHUNK_HIERARCHY:
            fin.seekg(chunksize, std::ios::cur);
            break;
        case W3D_CHUNK_ANIMATION:
            fin.seekg(chunksize, std::ios::cur);
            break;
        case W3D_CHUNK_COMPRESSED_ANIMATION:
            fin.seekg(chunksize, std::ios::cur);
            break;
        case W3D_CHUNK_HLOD:
            fin.seekg(chunksize, std::ios::cur);
            break;
        case W3D_CHUNK_BOX:
            fin.seekg(chunksize, std::ios::cur);
            break;
        default:
            std::cout << "Unhandled chunk 0x" << std::hex << chunktype << std::endl;
            fin.seekg(chunksize, std::ios::cur);
            break;
        }
    }
    fin.close();
    return  model;
}

W3DModel::Mesh* W3DModelLoader::LoadMeshChunk(std::ifstream& fin, uint32 pchunksize)
{
    auto chunkend = static_cast<uint32>(fin.tellg()) + pchunksize;
    W3DModel::Mesh* mesh = new W3DModel::Mesh();
    auto isTypeMapSection = false;
    auto offsetMapSection = 0;

    while (fin.tellg() < chunkend)
    {
        auto chunktype = Read<uint32>(fin);
        auto chunksize = GetChunkSize(Read<uint32>(fin));

        switch (chunktype)
        {
        case W3D_CHUNK_VERTICES:
            ReadArray(fin, chunksize, mesh->vertices);
            break;
        case W3D_CHUNK_VERTICES_COPY:
            ReadArray(fin, chunksize, mesh->vertices_copy);
            break;
        case W3D_CHUNK_VERTEX_NORMALS:
            ReadArray(fin, chunksize, mesh->normals);
            break;
        case W3D_CHUNK_VERTEX_NORMALS_COPY:
            ReadArray(fin, chunksize, mesh->normals_copy);
            break;
        case W3D_CHUNK_MESH_USER_TEXT:
            mesh->usertext = ReadCString(fin);
            break;
        case W3D_CHUNK_VERTEX_INFLUENCES:
            ReadArray(fin, chunksize, mesh->influences);
            break;
        case W3D_CHUNK_MESH_HEADER3:
            mesh->header = Read<W3dMeshHeader3Struct>(fin);
            break;
        case W3D_CHUNK_TRIANGLES:
            ReadArray(fin, chunksize, mesh->triangles);
            break;
        case W3D_CHUNK_VERTEX_SHADE_INDICES:
            ReadArray(fin, chunksize, mesh->shadeIndices);
            break;
        case W3D_CHUNK_MATERIAL_INFO:
            mesh->materialInfo = Read<W3dMaterialInfoStruct>(fin);
            break;
        case W3D_CHUNK_SHADERS:
            ReadArray(fin, chunksize, mesh->shaders);
            break;
        case W3D_CHUNK_VERTEX_MATERIALS:
            mesh->materials = LoadMaterials(fin, chunksize);
            break;
        case W3D_CHUNK_TEXTURES:
            mesh->textures = LoadTextures(fin, chunksize);
            break;
        case W3D_CHUNK_NORMALMAP_INFO:
            mesh->normalMap = LoadNormalMap(fin, chunksize);
            break;
        case W3D_CHUNK_MATERIAL_PASS:
            mesh->materialpass = LoadMaterialPass(fin, chunksize);
            break;
        default:
            std::cout << "Unhandled chunk 0x" << std::hex << chunktype << std::endl;
            fin.seekg(chunksize, std::ios::cur);
            break;
        }

    }

    return mesh;
}

std::vector<W3dMaterialStruct> W3DModelLoader::LoadMaterials(std::ifstream& fin, uint32 pchunksize)
{
    auto chunkend = static_cast<uint32>(fin.tellg()) + pchunksize;
    std::vector<W3dMaterialStruct> result;

    while (fin.tellg()<chunkend)
    {
        auto chunktype = Read<uint32>(fin);
        auto chunksize = GetChunkSize(Read<uint32>(fin));

        switch (chunktype)
        {
        case W3D_CHUNK_VERTEX_MATERIAL:
            result.push_back(ReadMaterial(fin, chunksize));
            break;
        default:
            std::cout << "Unhandled chunk 0x" << std::hex << chunktype << std::endl;
            fin.seekg(chunksize, std::ios::cur);
            break;
        }
    }

    return result;
}

W3dMaterialStruct W3DModelLoader::ReadMaterial(std::ifstream& fin, uint32 pchunksize)
{
    W3dMaterialStruct material;
    auto chunkend = static_cast<uint32>(fin.tellg()) + pchunksize;
    while (fin.tellg()<chunkend)
    {
        auto chunktype = Read<uint32>(fin);
        auto chunksize = GetChunkSize(Read<uint32>(fin));

        switch (chunktype)
        {
        case W3D_CHUNK_VERTEX_MATERIAL_NAME:
            ReadFixedString<W3D_NAME_LEN>(fin, material.Name);
            break;
        case W3D_CHUNK_VERTEX_MATERIAL_INFO:
            material.VertexMaterialInfo = Read<W3dVertexMaterialStruct>(fin);
            break;
        case W3D_CHUNK_VERTEX_MAPPER_ARGS0:
            material.Arg0 = ReadCString(fin);
            break;
        case W3D_CHUNK_VERTEX_MAPPER_ARGS1:
            material.Arg1 = ReadCString(fin);
            break;
        default:
            std::cout << "Unhandled chunk 0x" << std::hex << chunktype << std::endl;
            fin.seekg(chunksize, std::ios::cur);
            break;
        }
    }

    return material;
}

std::vector<W3dTextureStruct> W3DModelLoader::LoadTextures(std::ifstream& fin, uint32 pchunksize)
{
    auto chunkend = static_cast<uint32>(fin.tellg()) + pchunksize;
    std::vector<W3dTextureStruct> result;

    while (fin.tellg()<chunkend)
    {
        auto chunktype = Read<uint32>(fin);
        auto chunksize = GetChunkSize(Read<uint32>(fin));

        switch (chunktype)
        {
        case W3D_CHUNK_TEXTURE:
            result.push_back(ReadTexture(fin, chunksize));
            break;
        default:
            std::cout << "Unhandled chunk 0x" << std::hex << chunktype << std::endl;
            fin.seekg(chunksize, std::ios::cur);
            break;
        }
    }

    return result;
}

W3dTextureStruct W3DModelLoader::ReadTexture(std::ifstream& fin, uint32 pchunksize)
{
    W3dTextureStruct texture;
    auto chunkend = static_cast<uint32>(fin.tellg()) + pchunksize;
    while (fin.tellg()<chunkend)
    {
        auto chunktype = Read<uint32>(fin);
        auto chunksize = GetChunkSize(Read<uint32>(fin));

        switch (chunktype)
        {
        case W3D_CHUNK_TEXTURE_NAME:
            ReadFixedString<MAX_TEXTURE>(fin, texture.Name);
            break;
        case W3D_CHUNK_TEXTURE_INFO:
            texture.TextureInfo = Read<W3dTextureInfoStruct>(fin);
            break;
        default:
            std::cout << "Unhandled chunk 0x" << std::hex << chunktype << std::endl;
            fin.seekg(chunksize, std::ios::cur);
            break;
        }
    }

    return texture;
}

W3dMaterialPassStruct W3DModelLoader::LoadMaterialPass(std::ifstream& fin, uint32 pchunksize)
{
    W3dMaterialPassStruct matpass = W3dMaterialPassStruct();
    auto chunkend = static_cast<uint32>(fin.tellg()) + pchunksize;
    auto old = 0;
    while (fin.tellg()<chunkend)
    {
        auto chunktype = Read<uint32>(fin);
        auto chunksize = GetChunkSize(Read<uint32>(fin));

        switch (chunktype)
        {
        case W3D_CHUNK_VERTEX_MATERIAL_IDS:
            ReadArray(fin, chunksize, matpass.vmIDs);
            break;
        case W3D_CHUNK_SHADER_IDS:
            ReadArray(fin, chunksize, matpass.shaderIDs);
            break;
        case W3D_CHUNK_DCG:
            ReadArray(fin, chunksize, matpass.dcg);
            break;
        case W3D_CHUNK_DIG:
            ReadArray(fin, chunksize, matpass.dig);
            break;
        case W3D_CHUNK_SCG:
            ReadArray(fin, chunksize, matpass.scg);
            break;
        case W3D_CHUNK_TEXTURE_STAGE:
            matpass.textureStage = ReadTextureStage(fin, chunksize);
            break;
        default:
            std::cout << "Unhandled chunk 0x" << std::hex << chunktype << std::endl;
            fin.seekg(chunksize, std::ios::cur);
            break;
        }
        old = chunktype;
    }

    return matpass;
}

W3dTextureStageStruct W3DModelLoader::ReadTextureStage(std::ifstream& fin, uint32 pchunksize)
{
    W3dTextureStageStruct texstage;
    auto chunkend = static_cast<uint32>(fin.tellg()) + pchunksize;
    auto old = 0;
    while (fin.tellg()<chunkend)
    {
        auto chunktype = Read<uint32>(fin);
        auto chunksize = GetChunkSize(Read<uint32>(fin));

        switch (chunktype)
        {
        case W3D_CHUNK_TEXTURE_IDS:
            ReadArray(fin, chunksize, texstage.txIDs);
            break;
        case W3D_CHUNK_STAGE_TEXCOORDS:
            ReadArray(fin, chunksize, texstage.txCoords);
            break;    
        default:
            std::cout << "Unhandled chunk 0x" << std::hex << chunktype << std::endl;
            fin.seekg(chunksize, std::ios::cur);
            break;
        }
        old = chunktype;
    }

    return texstage;
}

W3dNormalMapStruct W3DModelLoader::LoadNormalMap(std::ifstream& fin, uint32 pchunksize)
{
    W3dNormalMapStruct normalMap = W3dNormalMapStruct();
    auto chunkend = static_cast<uint32>(fin.tellg()) + pchunksize;
    auto old = 0;
    while (fin.tellg()<chunkend)
    {
        auto chunktype = Read<uint32>(fin);
        auto chunksize = GetChunkSize(Read<uint32>(fin));

        switch (chunktype)
        {
        case W3D_CHUNK_NORMALMAP_FLAG1:
            normalMap.isTypeMap = true;
            break;
        case W3D_CHUNK_NORMALMAP_FLAG2:
            if (normalMap.isTypeMap)
            {
                normalMap.header = Read<W3dNormMapHeaderStruct>(fin);
                normalMap.entries = ReadNormalMapEntries(fin, chunksize, normalMap.header.Number);
            }
            break;
        case W3D_CHUNK_NORMALMAP_FLAG3:
            fin.seekg(chunksize, std::ios::cur);
            break;
        default:
            std::cout << "Unhandled chunk 0x" << std::hex << chunktype << std::endl;
            fin.seekg(chunksize, std::ios::cur);
            break;
        }
        old = chunktype;
    }

    return normalMap;
}

W3dNormMapEntryStruct* W3DModelLoader::ReadNormalMapEntries(std::ifstream& fin, uint32 pchunksize,uint8 number)
{
    W3dNormMapEntryStruct* entries = new W3dNormMapEntryStruct[number];
    auto chunkend = static_cast<uint32>(fin.tellg()) + pchunksize;

    while (fin.tellg() < chunkend)
    {
        auto chunktype = Read<uint32>(fin);
        auto chunksize = GetChunkSize(Read<uint32>(fin));

        switch (chunktype)
        {
        case W3D_CHUNK_NORMALMAP_FLAG2:
        {
            auto typeflag = Read<uint32>(fin);

        }
            break;
        }
    }

    return entries;
}