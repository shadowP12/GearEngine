set(GEAR_ENGINE_APPLICATION_SRC
	Source/Application/Application.cpp
	Source/Application/Window.cpp
)

set(GEAR_ENGINE_APPLICATION_INC
	Source/Application/Application.h
	Source/Application/Window.h
)

set(GEAR_ENGINE_RHI_SRC
		Source/RHI/RHIDefine.cpp
		Source/RHI/RHIDevice.cpp
		Source/RHI/RHIBuffer.cpp
		Source/RHI/RHICommandBuffer.cpp
		Source/RHI/RHIFramebuffer.cpp
		Source/RHI/RHIPipeline.cpp
		Source/RHI/RHIProgram.cpp
		Source/RHI/RHIQueue.cpp
		Source/RHI/RHIRenderPass.cpp
		Source/RHI/RHISwapChain.cpp
		Source/RHI/RHITexture.cpp
		Source/RHI/RHISynchronization.cpp
		Source/RHI/RHIDescriptorSet.cpp
		Source/RHI/Managers/SpirvManager.cpp
)

set(GEAR_ENGINE_RHI_INC
		Source/RHI/RHIDefine.h
		Source/RHI/RHIRenderState.h
		Source/RHI/RHIDevice.h
		Source/RHI/RHIBuffer.h
		Source/RHI/RHICommandBuffer.h
		Source/RHI/RHIFramebuffer.h
		Source/RHI/RHIPipeline.h
		Source/RHI/RHIProgram.h
		Source/RHI/RHIProgramParam.h
		Source/RHI/RHIQueue.h
		Source/RHI/RHIRenderPass.h
		Source/RHI/RHISwapChain.h
		Source/RHI/RHITexture.h
		Source/RHI/RHISynchronization.h
		Source/RHI/RHIDescriptorSet.h
		Source/RHI/Managers/SpirvManager.h
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
	${GEAR_ENGINE_UTILITY_SRC}
)