# Honours Artefact
## Cormac Somerville - 2200592
You should watch the demo video which goes over how to use the software as well as any important information. 

### System Requirements
- Windows machine  
- NVIDIA GPU which supports Mesh Shading (with minimum meshlet vertex/primitive counts of 128). Cards that or above an RTX2060 should suffice. 

### Libraries required to build the executable on your own machine. 
To Develop
1. Download Vulkan SDK 1.4.335.0 from LunarG https://vulkan.lunarg.com/sdk/home#windows
2. Install it to C:/VulkanSDK/1.4.335.0  
  
For metered builds, you must download NSight Perf yourself due to their licensing agreement. 
1. Download NVIDIA's NSight Perf SDK https://developer.nvidia.com/nsight-perf-sdk
2. After Unzipping the files, copy the NvPerf folder to C:/VulkanSDK/
3. Create a new folder in this folder called "utilities"
4. Copy the imports and include folder from Samples/NvPerfUtility to your utilities folder. 
5. Copy nvperf_grfx_host.dll into your working directory / exe folder. This DLL is in the NvPerf/bin/x64 folder.  

The above should allow you to build with NSight Perf SDK with this project. 


