set(GEAR_ENGINE_APPLICATION_SRC
	Source/Application/Application.cpp
	Source/Application/Window.cpp
)

set(GEAR_ENGINE_APPLICATION_INC
	Source/Application/Application.h
	Source/Application/Window.h
)

set(GEAR_ENGINE_RHI_SRC
	Source/RHI/RHI.cpp
	Source/RHI/RHIDevice.cpp
	Source/RHI/RHIUtility.cpp
	Source/RHI/RHIBuffers.cpp
	Source/RHI/RHICommandBuffer.cpp
	Source/RHI/RHIFramebuffer.cpp
	Source/RHI/RHIPipelineState.cpp
	Source/RHI/RHIProgram.cpp
	Source/RHI/RHIProgramParam.cpp
	Source/RHI/RHIQueue.cpp
	Source/RHI/RHIRenderPass.cpp
	Source/RHI/RHISwapChain.cpp
	Source/RHI/RHITexture.cpp
	Source/RHI/RHITextureView.cpp
	Source/RHI/RHIContext.cpp
	Source/RHI/RHISynchronization.cpp
	Source/RHI/Managers/RHIPipelineStateManager.cpp
	Source/RHI/Managers/RHIProgramManager.cpp
	Source/RHI/Managers/RHIVertexInputManager.cpp
)

set(GEAR_ENGINE_RHI_INC
	Source/RHI/RHI.h
	Source/RHI/RHIDefine.h
	Source/RHI/RHIDevice.h
	Source/RHI/RHIUtility.h
	Source/RHI/RHIBuffers.h
	Source/RHI/RHICommandBuffer.h
	Source/RHI/RHIFramebuffer.h
	Source/RHI/RHIPipelineState.h
	Source/RHI/RHIProgram.h
	Source/RHI/RHIProgramParam.h
	Source/RHI/RHIQueue.h
	Source/RHI/RHIRenderPass.h
	Source/RHI/RHISwapChain.h
	Source/RHI/RHITexture.h
	Source/RHI/RHITextureView.h
	Source/RHI/RHIContext.h
	Source/RHI/RHISynchronization.h
	Source/RHI/Managers/RHIPipelineStateManager.h
	Source/RHI/Managers/RHIProgramManager.h
	Source/RHI/Managers/RHIVertexInputManager.h
)

set(GEAR_ENGINE_MESH_SRC
	Source/Mesh/VertexDescription.cpp
)

set(GEAR_ENGINE_MESH_INC
	Source/Mesh/VertexDescription.h
)

set(GEAR_ENGINE_TEXTURE_SRC
	Source/Texture/Texture2D.cpp
)

set(GEAR_ENGINE_TEXTURE_INC
	Source/Texture/Texture2D.h
)

set(GEAR_ENGINE_UTILITY_INC
	Source/Utility/FileSystem.h
	Source/Utility/Hash.h
	Source/Utility/Log.h
	Source/Utility/Module.h
)

set(GEAR_ENGINE_UTILITY_SRC
		Source/Utility/FileSystem.cpp
)

set(GEAR_ENGINE_MATH_INC
	Math/GMath.h
)

set(GEAR_ENGINE_SRC
	${GEAR_ENGINE_APPLICATION_SRC}
	${GEAR_ENGINE_RHI_SRC}
	${GEAR_ENGINE_MESH_SRC}
	${GEAR_ENGINE_TEXTURE_SRC}
	${GEAR_ENGINE_UTILITY_SRC}
)