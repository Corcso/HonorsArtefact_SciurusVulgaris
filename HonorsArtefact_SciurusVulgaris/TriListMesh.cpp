#include "PCH.h"
#include "TriListMesh.h"
#include "VulkanUtility.h"
#include "Graphics.h"

void TriListMesh::CopyPointsToVRAM()
{
    // VERTEX BUFFER

    // Staging Vertex buffer
    VkBuffer stagingVertexBuffer;
    //VkDeviceMemory stagingVertexBufferMemory;
    VulkanMemoryAllocator::VulkanMemoryBlock stagingVertexBufferMemory;

    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    VulkanUtility::CreateBufferAndAssignMemory(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingVertexBuffer, &stagingVertexBufferMemory, VulkanMemoryAllocator::VulkanMemoryMapUsage::INSTANT);

    // Map GPU memory to CPU memory
    VulkanUtility::MapCopyBlockToGPU(stagingVertexBufferMemory, vertices.data(), bufferSize);

    VulkanUtility::CreateBufferAndAssignMemory(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &vertexBuffer, &vertexBufferMemory);

    // See below
    VulkanUtility::CopyBufferData(stagingVertexBuffer, vertexBuffer, bufferSize);

    // Dont need the stager anymore
    VulkanUtility::DestroyBuffer(stagingVertexBuffer);
    VulkanUtility::FreeGPUMemoryBlock(stagingVertexBufferMemory);

    // INDEX BUFFER
    bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingIndexBuffer;
    VulkanMemoryAllocator::VulkanMemoryBlock stagingIndexBufferMemory;
    //VkDeviceMemory stagingIndexBufferMemory;
    VulkanUtility::CreateBufferAndAssignMemory(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingIndexBuffer, &stagingIndexBufferMemory, VulkanMemoryAllocator::VulkanMemoryMapUsage::INSTANT);

    VulkanUtility::MapCopyBlockToGPU(stagingIndexBufferMemory, indices.data(), bufferSize);

    VulkanUtility::CreateBufferAndAssignMemory(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &indexBuffer, &indexBufferMemory);

    VulkanUtility::CopyBufferData(stagingIndexBuffer, indexBuffer, bufferSize);

    VulkanUtility::DestroyBuffer(stagingIndexBuffer);
    VulkanUtility::FreeGPUMemoryBlock(stagingIndexBufferMemory);
}

void TriListMesh::LoadFile(std::string path, int meshIndex)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiPostProcessSteps::aiProcess_Triangulate);

    for(int mesh = 0; mesh < scene->mNumMeshes; mesh++){
        if (mesh != meshIndex) continue;
        if (scene->mMeshes[mesh]->mNumVertices > 0) {
            for (int v = 0; v < scene->mMeshes[mesh]->mNumVertices; v++) {
                vertices.push_back(
                    {
                        HMM_V3(scene->mMeshes[mesh]->mVertices[v].x, scene->mMeshes[mesh]->mVertices[v].y, scene->mMeshes[mesh]->mVertices[v].z),
                        HMM_V3(scene->mMeshes[mesh]->mNormals[v].x, scene->mMeshes[mesh]->mNormals[v].y, scene->mMeshes[mesh]->mNormals[v].z),
                        HMM_V2(scene->mMeshes[mesh]->mTextureCoords[0][v].x, scene->mMeshes[mesh]->mTextureCoords[0][v].y)
                    });
            }
        }
        if (scene->mMeshes[mesh]->mNumFaces > 0) {
            for (int face = 0; face < scene->mMeshes[mesh]->mNumFaces; face++) {
                for (int index = 0; index < scene->mMeshes[mesh]->mFaces[face].mNumIndices; index++) {
                    indices.push_back(scene->mMeshes[mesh]->mFaces[face].mIndices[index]);
                }
            }
        }
    }

    // Scene deleted from heap when importer leaves scope
}

void TriListMesh::LoadFile(const aiScene* scene, int meshIndex)
{
    for (int mesh = 0; mesh < scene->mNumMeshes; mesh++) {
        if (mesh != meshIndex) continue;
        if (scene->mMeshes[mesh]->mNumVertices > 0) {
            for (int v = 0; v < scene->mMeshes[mesh]->mNumVertices; v++) {
                vertices.push_back(
                    {
                        HMM_V3(scene->mMeshes[mesh]->mVertices[v].x, scene->mMeshes[mesh]->mVertices[v].y, scene->mMeshes[mesh]->mVertices[v].z),
                        HMM_V3(scene->mMeshes[mesh]->mNormals[v].x, scene->mMeshes[mesh]->mNormals[v].y, scene->mMeshes[mesh]->mNormals[v].z),
                        HMM_V2(scene->mMeshes[mesh]->mTextureCoords[0][v].x, scene->mMeshes[mesh]->mTextureCoords[0][v].y)
                    });
            }
        }
        if (scene->mMeshes[mesh]->mNumFaces > 0) {
            for (int face = 0; face < scene->mMeshes[mesh]->mNumFaces; face++) {
                for (int index = 0; index < scene->mMeshes[mesh]->mFaces[face].mNumIndices; index++) {
                    indices.push_back(scene->mMeshes[mesh]->mFaces[face].mIndices[index]);
                }
            }
        }
    }
}

std::vector<TriListMesh> TriListMesh::LoadMultiMeshFile(std::string path, bool copyAllToVRAM)
{
    std::vector<TriListMesh> output;

    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(path, aiPostProcessSteps::aiProcess_Triangulate);
    output.reserve(scene->mNumMeshes);
    for (int mesh = 0; mesh < scene->mNumMeshes; mesh++) {
        output.emplace_back();
        output.back().LoadFile(scene, mesh);
        if (copyAllToVRAM) output.back().CopyPointsToVRAM();
    }

    return output;
}

void TriListMesh::CreateDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorSetLayoutCreateInfo layoutInformation, size_t* sizes)
{
    descriptor.Create(layout, layoutInformation, sizes);
}

TriListMesh::~TriListMesh()
{
    descriptor.CleanupDescriptor();
    VulkanUtility::DestroyBuffer(vertexBuffer);
    VulkanUtility::FreeGPUMemoryBlock(vertexBufferMemory);
    VulkanUtility::DestroyBuffer(indexBuffer);
    VulkanUtility::FreeGPUMemoryBlock(indexBufferMemory);
}
