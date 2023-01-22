TODO List : 

*Major feature*:

(1) Remove most uses of Rival lock as we will setup deferred command for ECS sync points.

(1.1) Expose component arrray pointer as UE5 Mass ECS

(1.2) Allow iterating on a view of entities within a chunk

(2) Implement adding multiple components in the same time to avoid repeately moving components one at a time

(3) Implement linked chunk data structure with std::vector like interface to drop in replace std::vector

(4) Refacotr memory allocator with ABA lock-free free list or integrate with Intel Thread Building BLock (TBB) or mimalloc

a, Let Phmap container, std::string, etc use custom allocator

(5) Implement vulkan renderer, and integrate with VulkanSceneGraph(?it's quite big a 3rd party project, what excatly should we integrate?)
a, consider render thread, draw thread design that support wide range of frame limit

*Minor Feature*:

(1) Fix SMAAT2X

(2) Implement best quality TAA available

(3) Integrate XeGTAO

(4) Refactor Animation update from preupdate to update