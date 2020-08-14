#include "RHIDefine.h"

VkBufferUsageFlags toVkBufferUsage(DescriptorType usage)
{
    VkBufferUsageFlags result = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (usage & DESCRIPTOR_TYPE_UNIFORM_BUFFER)
    {
        result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if (usage & DESCRIPTOR_TYPE_RW_BUFFER)

    {
        result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }
    if (usage & DESCRIPTOR_TYPE_BUFFER)
    {
        result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }
    if (usage & DESCRIPTOR_TYPE_INDEX_BUFFER)
    {
        result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if (usage & DESCRIPTOR_TYPE_VERTEX_BUFFER)
    {
        result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if (usage & DESCRIPTOR_TYPE_INDIRECT_BUFFER)
    {
        result |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    }
    return result;
}

VkImageUsageFlags toVkImageUsage(DescriptorType usage)
{
    VkImageUsageFlags result = 0;
    if (DESCRIPTOR_TYPE_TEXTURE == (usage & DESCRIPTOR_TYPE_TEXTURE))
        result |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (DESCRIPTOR_TYPE_RW_TEXTURE == (usage & DESCRIPTOR_TYPE_RW_TEXTURE))
        result |= VK_IMAGE_USAGE_STORAGE_BIT;
    return result;
}

VkAccessFlags toVkAccessFlags(ResourceState state)
{
    VkAccessFlags ret = 0;
    if (state & RESOURCE_STATE_COPY_SOURCE)
    {
        ret |= VK_ACCESS_TRANSFER_READ_BIT;
    }
    if (state & RESOURCE_STATE_COPY_DEST)
    {
        ret |= VK_ACCESS_TRANSFER_WRITE_BIT;
    }
    if (state & RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
    {
        ret |= VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    }
    if (state & RESOURCE_STATE_INDEX_BUFFER)
    {
        ret |= VK_ACCESS_INDEX_READ_BIT;
    }
    if (state & RESOURCE_STATE_UNORDERED_ACCESS)
    {
        ret |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    }
    if (state & RESOURCE_STATE_INDIRECT_ARGUMENT)
    {
        ret |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
    }
    if (state & RESOURCE_STATE_RENDER_TARGET)
    {
        ret |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }
    if (state & RESOURCE_STATE_DEPTH_WRITE)
    {
        ret |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }
    if ((state & RESOURCE_STATE_SHADER_RESOURCE) || (state & RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE))
    {
        ret |= VK_ACCESS_SHADER_READ_BIT;
    }
    if (state & RESOURCE_STATE_PRESENT)
    {
        ret |= VK_ACCESS_MEMORY_READ_BIT;
    }

    return ret;
}

VkImageLayout toVkImageLayout(ResourceState usage)
{
    if (usage & RESOURCE_STATE_COPY_SOURCE)
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    if (usage & RESOURCE_STATE_COPY_DEST)
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    if (usage & RESOURCE_STATE_RENDER_TARGET)
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    if (usage & RESOURCE_STATE_DEPTH_WRITE)
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    if (usage & RESOURCE_STATE_UNORDERED_ACCESS)
        return VK_IMAGE_LAYOUT_GENERAL;

    if ((usage & RESOURCE_STATE_SHADER_RESOURCE) || (usage & RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE))
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    if (usage & RESOURCE_STATE_PRESENT)
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    if (usage == RESOURCE_STATE_COMMON)
        return VK_IMAGE_LAYOUT_GENERAL;

    return VK_IMAGE_LAYOUT_UNDEFINED;
}

VkImageAspectFlags toVkAspectMask(VkFormat format)
{
    VkImageAspectFlags result = 0;
    switch (format)
    {
        // Depth
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D32_SFLOAT:
            result = VK_IMAGE_ASPECT_DEPTH_BIT;
            break;
        // Stencil
        case VK_FORMAT_S8_UINT:
            result = VK_IMAGE_ASPECT_STENCIL_BIT;
            break;
        // Depth/stencil
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            result = VK_IMAGE_ASPECT_DEPTH_BIT;
            result |= VK_IMAGE_ASPECT_STENCIL_BIT;
            break;
        // Color
        default: result = VK_IMAGE_ASPECT_COLOR_BIT; break;
    }
    return result;
}

VkPipelineStageFlags toPipelineStageFlags(VkAccessFlags accessFlags, QueueType queueType)
{
    VkPipelineStageFlags flags = 0;

    switch (queueType)
    {
        case QUEUE_TYPE_GRAPHICS:
        {
            if ((accessFlags & (VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)) != 0)
                flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;

            if ((accessFlags & (VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)) != 0)
            {
                flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                flags |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
                flags |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
                flags |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
                flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            }

            if ((accessFlags & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT) != 0)
                flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

            if ((accessFlags & (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)) != 0)
                flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

            if ((accessFlags & (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)) != 0)
                flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            break;
        }
        case QUEUE_TYPE_COMPUTE:
        {
            if ((accessFlags & (VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)) != 0 ||
                (accessFlags & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT) != 0 ||
                (accessFlags & (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)) != 0 ||
                (accessFlags & (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)) != 0)
                return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

            if ((accessFlags & (VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)) != 0)
                flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

            break;
        }
        case QUEUE_TYPE_TRANSFER:
            return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        default:
            break;
    }

    if ((accessFlags & VK_ACCESS_INDIRECT_COMMAND_READ_BIT) != 0)
        flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;

    if ((accessFlags & (VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT)) != 0)
        flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;

    if ((accessFlags & (VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT)) != 0)
        flags |= VK_PIPELINE_STAGE_HOST_BIT;

    if (flags == 0)
        flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    return flags;
}