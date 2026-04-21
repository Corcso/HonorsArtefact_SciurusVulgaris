:: Photogrammetric Generation Shaders
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe MeshVertexPhotogrammetric.vert -O -o ./COMPILEDSHADER_MeshVertexPhotogrammetric.spv
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe MeshFragmentPhotogrammetric.frag -O -o ./COMPILEDSHADER_MeshFragmentPhotogrammetric.spv

:: Other 
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe PointVertex.vert -O -o ./COMPILEDSHADER_PointVertex.spv
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe InstancePointVertex.vert -O -o ./COMPILEDSHADER_InstancePointPointVertex.spv
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe PointFragment.frag -O -o ./COMPILEDSHADER_PointFragment.spv
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe MeshVertex.vert -O -o ./COMPILEDSHADER_MeshVertex.spv
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe InstanceMeshVertex.vert -O -o ./COMPILEDSHADER_InstanceMeshVertex.spv
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe Skybox.frag -O -o ./COMPILEDSHADER_Skybox.spv

:: Defered Stuff
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe DeferedQuad.vert -O -o ./COMPILEDSHADER_DeferedQuad.spv
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe DeferedTree.frag -O -o ./COMPILEDSHADER_DeferedTree.spv
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe DeferedPoint.frag -O -o ./COMPILEDSHADER_DeferedPoint.spv
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe DeferredMesh.frag -O -o ./COMPILEDSHADER_DeferredMesh.spv
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe DeferredMeshNoVelocity.frag -O -o ./COMPILEDSHADER_DeferredMeshNoVelocity.spv
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe FXAAFrag.frag -O -o ./COMPILEDSHADER_FXAAFrag.spv
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe TAAFrag.frag -O -o ./COMPILEDSHADER_TAAFrag.spv

:: Mesh Shader
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe InstancePointMesh.mesh -O -o ./COMPILEDSHADER_InstancePointMesh.spv --target-spv=spv1.4 --target-env=vulkan1.2
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe InstancePointTask.task -O -o ./COMPILEDSHADER_InstancePointTask.spv --target-spv=spv1.4 --target-env=vulkan1.2
C:\VulkanSDK\1.4.335.0\Bin\glslc.exe InstancePointMeshFromTask.mesh -O -o ./COMPILEDSHADER_InstancePointMeshFromTask.spv --target-spv=spv1.4 --target-env=vulkan1.2

pause