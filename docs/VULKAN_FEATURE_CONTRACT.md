# Vulkan feature contract

The engine targets Vulkan 1.4, but version numbers alone do not replace feature and limit queries.

## Hard baseline

Verify loader and selected device support for Vulkan 1.4, the active platform WSI extension, `VK_KHR_swapchain`, graphics and presentation queues, and required formats, modes, and limits.

## Required modern feature vocabulary

Query through `vkGetPhysicalDeviceFeatures2` and an explicit `pNext` chain. The engine contract includes dynamic rendering, synchronization2, timeline semaphores, scalar block layout, and the descriptor-indexing capabilities actually used by the bindless table: runtime arrays, sampled-image non-uniform indexing, partially bound descriptors, sampled-image update after bind, and any variable-count or update-unused behavior the implementation chooses.

Do not enable a feature because a field exists. Query it, record it, and reject or choose a documented fallback.

## Dynamic rendering

The normal path creates no legacy `VkRenderPass` or `VkFramebuffer`. Pipelines declare attachment formats through `VkPipelineRenderingCreateInfo`; command buffers begin and end rendering dynamically.

## Bindless baseline

Use descriptor indexing first:

- long-lived update-after-bind tables;
- fallback descriptors in reserved slots;
- stable CPU generational handles;
- shader-visible indices in materials and instances;
- deferred descriptor-slot reuse until GPU completion;
- capacities derived from queried limits.

Descriptor buffers and newer descriptor-heap mechanisms remain optional experiments until portability, tooling, and measured benefit justify migration.

## Validation

Development builds enable validation and debug utils. A lesson is not accepted with unexplained validation messages.
