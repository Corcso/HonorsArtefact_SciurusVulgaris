#include "PCH.h"
#include "PointMesh.h"
#include "VulkanUtility.h"
#include "Graphics.h"

// Include ASSIMP headers, (Kulling and assimp team, 2025) v6.0.2
#include <assimp/Importer.hpp>   
#include <assimp/Exporter.hpp>    
#include <assimp/scene.h>         
#include <assimp/postprocess.h>    

void PointMesh::CopyPointsToVRAM()
{
    // VERTEX BUFFER
    
    // Staging Vertex buffer
    VkBuffer stagingVertexBuffer;
    //VkDeviceMemory stagingVertexBufferMemory;
    VulkanMemoryAllocator::VulkanMemoryBlock stagingVertexBufferMemory;

    VkDeviceSize bufferSize = sizeof(points[0]) * points.size();
    VulkanUtility::CreateBufferAndAssignMemory(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingVertexBuffer, &stagingVertexBufferMemory, VulkanMemoryAllocator::VulkanMemoryMapUsage::INSTANT);

    // Map GPU memory to CPU memory
    VulkanUtility::MapCopyBlockToGPU(stagingVertexBufferMemory, points.data(), bufferSize);

    VulkanUtility::CreateBufferAndAssignMemory(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &pointBuffer, &pointBufferMemory);

    // See below
    VulkanUtility::CopyBufferData(stagingVertexBuffer, pointBuffer, bufferSize);

    // Dont need the stager anymore
    VulkanUtility::DestroyBuffer(stagingVertexBuffer);
    //VulkanUtility::FreeGPUMemory(stagingVertexBufferMemory);
    VulkanUtility::FreeGPUMemoryBlock(stagingVertexBufferMemory);

    // INDEX BUFFER
    //bufferSize = sizeof(indices[0]) * indices.size();

    //VkBuffer stagingIndexBuffer;
    //VulkanMemoryAllocator::VulkanMemoryBlock stagingIndexBufferMemory;
    ////VkDeviceMemory stagingIndexBufferMemory;
    //VulkanUtility::CreateBufferAndAssignMemory(bufferSize,
    //    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    //    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    //    &stagingIndexBuffer, &stagingIndexBufferMemory, VulkanMemoryAllocator::VulkanMemoryMapUsage::INSTANT);

    //VulkanUtility::MapCopyBlockToGPU(stagingIndexBufferMemory, indices.data(), bufferSize);

    //VulkanUtility::CreateBufferAndAssignMemory(bufferSize,
    //    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    //    &indexBuffer, &indexBufferMemory);

    //VulkanUtility::CopyBufferData(stagingIndexBuffer, indexBuffer, bufferSize);

    //VulkanUtility::DestroyBuffer(stagingIndexBuffer);
    //VulkanUtility::FreeGPUMemoryBlock(stagingIndexBufferMemory);
    //VulkanUtility::FreeGPUMemory(stagingIndexBufferMemory);

    isDataOnGPU = true;
}

void PointMesh::LoadFromFileOBJMTL(std::string pathOBJ, std::string pathMTL)
{
    // Read from the text file
    std::ifstream OBJFile(pathOBJ);

    std::string line;

    // Use a while loop together with the getline() function to read the file line by line
    while (std::getline(OBJFile, line)) {
        if (line.length() < 3) continue;
        if (line[0] == 'v') {
            std::string thisLinePoints[3];
            int currentPointIndex = 0;
            std::string currentPoint = "";

            for (int c = 2; c < line.length(); ++c) {
                if (line[c] == ' ') {
                    thisLinePoints[currentPointIndex] = currentPoint;
                    currentPoint = "";
                    currentPointIndex++;
                }
                else{
                    currentPoint += line[c];
                }
            }
            thisLinePoints[currentPointIndex] = currentPoint;

            points.push_back({
                   HMM_V3(std::stof(thisLinePoints[0]), std::stof(thisLinePoints[1]), std::stof(thisLinePoints[2])), HMM_V3(0, 0, 0)
                });
        }
    }

    // Close the file
    OBJFile.close();

    // Read from the text file
    std::ifstream MTLFile(pathMTL);

    int index = 0;
    // Use a while loop together with the getline() function to read the file line by line
    while (std::getline(MTLFile, line)) {
        if (line.length() < 3) continue;
        if (line[0] == 'K' && line[1] == 'd') {
            std::string thisLinePoints[3];
            int currentPointIndex = 0;
            std::string currentPoint = "";

            for (int c = 3; c < line.length(); ++c) {
                if (line[c] == ' ') {
                    thisLinePoints[currentPointIndex] = currentPoint;
                    currentPoint = "";
                    currentPointIndex++;
                }
                else {
                    currentPoint += line[c];
                }
            }
            thisLinePoints[currentPointIndex] = currentPoint;

            points[index].color = HMM_V3(std::stof(thisLinePoints[0]), std::stof(thisLinePoints[1]), std::stof(thisLinePoints[2]));
            index++;
        }
    }

    /*for (int i = 0; i < points.size(); ++i) {
        indices.push_back(i);
    }*/

    // Close the file
    MTLFile.close();
}

void PointMesh::LoadFromFile(std::string path)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(path, 0);
    //uint64_t currentIndex = 0;
    for (int mesh = 0; mesh < scene->mNumMeshes; mesh++) {
        if (scene->mMeshes[mesh]->mNumVertices > 0) {
            for (int v = 0; v < scene->mMeshes[mesh]->mNumVertices; v++) {
                // Check for color
                HMM_Vec3 color = HMM_V3(1, 1, 1);
                if (scene->mMeshes[mesh]->mColors[0] != nullptr) {
                    color = HMM_V3(scene->mMeshes[mesh]->mColors[0][v].r, scene->mMeshes[mesh]->mColors[0][v].g, scene->mMeshes[mesh]->mColors[0][v].b);
                }
                
                points.push_back(
                    {
                        HMM_V3(scene->mMeshes[mesh]->mVertices[v].x, scene->mMeshes[mesh]->mVertices[v].y, scene->mMeshes[mesh]->mVertices[v].z),
                        color,
                        HMM_V3(scene->mMeshes[mesh]->mNormals[v].x, scene->mMeshes[mesh]->mNormals[v].y, scene->mMeshes[mesh]->mNormals[v].z)
                    });

                // Points have no faces just push back 0 -> numVertices
                //indices.push_back(currentIndex);
                //currentIndex++;
            }
        }
       
    }

    // Scene deleted from heap when importer leaves scope
}

void PointMesh::SaveToFile(std::string path)
{
    Assimp::Exporter exporter;

    aiScene sceneData;
    sceneData.mNumMaterials = 1;
    sceneData.mMaterials = new aiMaterial * [1] { new aiMaterial }; // deleted: Version.cpp:158
    sceneData.mNumMeshes = 1;
    sceneData.mMeshes = new aiMesh*[1];
    sceneData.mMeshes[0] = new aiMesh;
    sceneData.mMeshes[0]->mPrimitiveTypes = aiPrimitiveType_POINT;
    sceneData.mMeshes[0]->mNumVertices = points.size();
    sceneData.mMeshes[0]->mNumFaces = points.size(); //indices.size();
    sceneData.mMeshes[0]->mFaces = new aiFace[points.size()];
    sceneData.mMeshes[0]->mVertices = new aiVector3D[points.size()];
    sceneData.mMeshes[0]->mNormals = new aiVector3D[points.size()];
    sceneData.mMeshes[0]->mTangents = new aiVector3D[points.size()];
    sceneData.mMeshes[0]->mBitangents = new aiVector3D[points.size()];
    sceneData.mMeshes[0]->mColors[0] = new aiColor4D[points.size()];
    sceneData.mRootNode = new aiNode;
    sceneData.mRootNode->mNumMeshes = 1;
    sceneData.mRootNode->mMeshes = new unsigned int[1] {0};

    for (int p = 0; p < points.size(); p++) {
        sceneData.mMeshes[0]->mVertices[p].Set(points[p].position.X, points[p].position.Y, points[p].position.Z);
        sceneData.mMeshes[0]->mNormals[p].Set(points[p].normal.X, points[p].normal.Y, points[p].normal.Z);
        sceneData.mMeshes[0]->mColors[0][p].r = points[p].color.R;
        sceneData.mMeshes[0]->mColors[0][p].b = points[p].color.B;
        sceneData.mMeshes[0]->mColors[0][p].g = points[p].color.G;
        sceneData.mMeshes[0]->mColors[0][p].a = 1;
    }
    for (unsigned int i = 0; i < points.size(); i++) {
        sceneData.mMeshes[0]->mFaces[i].mNumIndices = 1;
        sceneData.mMeshes[0]->mFaces[i].mIndices = new unsigned int [1] {i};
    }
    exporter.Export(&sceneData, "fbx", path);

    // Cleanup happens within aiScene
}

void PointMesh::CreateDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorSetLayoutCreateInfo layoutInformation, size_t* sizes)
{
    descriptor.Create(layout, layoutInformation, sizes);
}

PointMesh::~PointMesh()
{
    descriptor.CleanupDescriptor();
    if (isDataOnGPU) {
        VulkanUtility::DestroyBuffer(pointBuffer);
        VulkanUtility::FreeGPUMemoryBlock(pointBufferMemory);
        //VulkanUtility::DestroyBuffer(indexBuffer);
        //VulkanUtility::FreeGPUMemoryBlock(indexBufferMemory);
    }
}
