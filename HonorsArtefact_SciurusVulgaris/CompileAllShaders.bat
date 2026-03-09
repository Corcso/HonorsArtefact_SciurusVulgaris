C:\VulkanSDK\1.4.335.0\Bin\glslc.exe PointVertex.vert -o ./COMPILEDSHADER_PointVertex.spv
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe InstancePointVertex.vert -o ./COMPILEDSHADER_InstancePointPointVertex.spv
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe PointFragment.frag -o ./COMPILEDSHADER_PointFragment.spv
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe MeshVertex.vert -o ./COMPILEDSHADER_MeshVertex.spv
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe InstanceMeshVertex.vert -o ./COMPILEDSHADER_InstanceMeshVertex.spv
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe MeshFragment.frag -o ./COMPILEDSHADER_MeshFragment.spv

:: Defered Stuff
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe DeferedQuad.vert -o ./COMPILEDSHADER_DeferedQuad.spv
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe DeferedTree.frag -o ./COMPILEDSHADER_DeferedTree.spv
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe DeferedPoint.frag -o ./COMPILEDSHADER_DeferedPoint.spv
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe DeferredMesh.frag -o ./COMPILEDSHADER_DeferredMesh.spv
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe DeferredMeshNoVelocity.frag -o ./COMPILEDSHADER_DeferredMeshNoVelocity.spv
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe FXAAFrag.frag -o ./COMPILEDSHADER_FXAAFrag.spv
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe TAAFrag.frag -o ./COMPILEDSHADER_TAAFrag.spv

:: Mesh Shader
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe InstancePointMesh.mesh -o ./COMPILEDSHADER_InstancePointMesh.spv --target-spv=spv1.4 --target-env=vulkan1.2
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe InstancePointTask.task -o ./COMPILEDSHADER_InstancePointTask.spv --target-spv=spv1.4 --target-env=vulkan1.2
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe InstancePointMeshFromTask.mesh -o ./COMPILEDSHADER_InstancePointMeshFromTask.spv --target-spv=spv1.4 --target-env=vulkan1.2

pause