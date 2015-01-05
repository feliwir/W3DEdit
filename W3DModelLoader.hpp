#pragma once
#include <string>
#include <fstream>
#include <memory>
#include "W3DModel.hpp"

class W3D_EDIT W3DModelLoader
{
public:
    static W3DModel* LoadModel(const std::string& filename);
    static void SaveModel(const W3DModel& model, const std::string& filename);

private:
    //Get filesize
    static inline const uint32 GetFileSize(std::ifstream& fin)
    {
        fin.seekg(0, std::ios::end);
        uint32 size = static_cast<uint32>(fin.tellg());
        fin.seekg(0, std::ios::beg);
        return size;
    }

    //Get chunksize
    static inline const uint32 GetChunkSize(uint32 data)
    {
        return data & 0x7FFFFFFF;
    }

    //Read any fixed-size data type
    template <typename T>
    static inline const T Read(std::ifstream& fin)
    {
        T result = T();

        fin.read(reinterpret_cast<char*>(&result), sizeof(T));

        return result;
    }

    //Read an array of a type
    template<typename T>
    static inline void ReadArray(std::ifstream& fin, uint32 chunksize, T*& arr)
    {
        auto num_elements = chunksize / sizeof(T);
        arr = new T[num_elements];

        fin.read((char*)arr, sizeof(T)*num_elements);
    }

    template<size_t N>
    static inline void ReadFixedString(std::ifstream& fin, char* str)
    {
        //set str to 0
        memset(str, 0, N);
        
        //read until nullterminated, but max N characters
        char c;
        auto index = 0;
        while (((c = fin.get()) != '\0') && index<N)
        {
            str[index] = c;
            ++index;
        }
    }

    //Read a nullterminated string
    static inline char* ReadCString(std::ifstream& fin)
    {
        std::string buffer;
        char c;
        while ((c = fin.get()) != '\0') {
            buffer += c;
        }

        char *cstr = new char[buffer.length() + 1];
        strcpy(cstr, buffer.c_str());

        return cstr;
    }

    //Smaller chunks
    static W3dMaterialStruct ReadMaterial(std::ifstream& fin, uint32 pchunksize);
    static W3dTextureStruct ReadTexture(std::ifstream& fin, uint32 pchunksize);
    static W3dTextureStageStruct ReadTextureStage(std::ifstream& fin, uint32 pchunksize);
    static W3dNormMapEntryStruct* ReadNormalMapEntries(std::ifstream& fin, uint32 pchunksize,uint8 number);

    //Load complex chunks
    static W3DModel::Mesh* LoadMeshChunk(std::ifstream& fin, uint32 pchunksize);
    static std::vector<W3dMaterialStruct> LoadMaterials(std::ifstream& fin, uint32 pchunksize);
    static std::vector<W3dTextureStruct> LoadTextures(std::ifstream& fin, uint32 pchunksize);
    static W3dMaterialPassStruct LoadMaterialPass(std::ifstream& fin, uint32 pchunksize);
    static W3dNormalMapStruct   LoadNormalMap(std::ifstream& fin, uint32 pchunksize);
};