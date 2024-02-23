#pragma once

typedef struct
{
	VkRenderPass RenderPass;
	VkDescriptorPool DescriptorPool;
	VkDevice Device;
	VkPhysicalDevice PhysicalDevice;
	uint32_t ImageCount;
	VkSampleCountFlagBits MsaaSamples;
} ImGui_ImplVulkan_InitInfo;

typedef struct
{
	VkDeviceMemory VertexBufferMemory;
	VkDeviceMemory IndexBufferMemory;
	VkDeviceSize VertexBufferSize;
	VkDeviceSize IndexBufferSize;
	VkBuffer VertexBuffer;
	VkBuffer IndexBuffer;
} ImGui_ImplVulkan_Buffers;

struct
{
	VkRenderPass RenderPass;
	VkDescriptorPool DescriptorPool;
	VkDevice Device;
	VkPhysicalDevice PhysicalDevice;
	uint32_t ImageCount;
	VkSampleCountFlagBits MsaaSamples;

	VkPipeline Pipeline;
	VkPipeline OpaquePipeline;
	VkShaderModule VertexShader;
	VkShaderModule FragmentShader;

	VkSampler Sampler;

	VkDescriptorSetLayout DescriptorSetLayout;
	VkDescriptorSet DescriptorSet;

	VkPipelineLayout PipelineLayout;

	VkImage FontImage;
	VkDeviceMemory FontImageMemory;
	VkImageView FontImageView;
	VkDeviceMemory FontUploadBufferMemory;
	VkBuffer FontUploadBuffer;

	bool SetNULL;
	uint32_t Index;
	ImGui_ImplVulkan_Buffers* Buffers;

	bool LastPipeline;//0 = Opaque, 1 = Transparent
	bool LastDescriptorSet;//0 = Font, 1 = Some other
} ImGui_ImplVulkan_Renderer_Info;

uint32_t ImGui_ImplVulkan_MemoryType(VkMemoryPropertyFlags PropertyFlags, uint32_t TypeBits);
bool ImGui_ImplVulkanPrintError(const char* Msg);
bool ImGui_ImplVulkan_CreateFontsTexture(VkCommandBuffer CommandBuffer);
void ImGui_ImplVulkan_DestroyFontUploadObjects();
void ImGui_ImplVulkan_NewFrame();
bool ImGui_CreateOrResizeBuffer(VkBuffer* Buffer, VkDeviceMemory* BufferMemory, VkDeviceSize* BufferSize, size_t NewBufferSize, VkBufferUsageFlagBits Usage);
void ImGui_SetupRenderState(ImDrawData* DrawData, VkCommandBuffer CommandBuffer, int FramebufferWidth, int FramebufferHeight);
bool ImGui_ImplVulkan_RenderDrawData(ImDrawData* DrawData, VkCommandBuffer CommandBuffer, int NonAlphaTextureCount, ImTextureID* NonAlphaTextures);
bool ImGui_CreateSampler();
bool ImGui_CreateDescriptorSets();
bool ImGui_CreatePipelineLayout();
bool ImGui_CreateShaderModules();
bool ImGui_CreateGraphicsPipeline();
void ImGui_ImplVulkan_Init(ImGui_ImplVulkan_InitInfo* InitInfo);
void ImGui_ImplVulkan_Shutdown();
