TODO List : 

*Major feature*:

<!-- (1) Refactor ECS Archtype memory management : instead of using entities and component manager, using entity->Archtype->component managers.

a, You need a archtype2component managers dictionary such that all allocations of components of certain archtype can be managed in per-archtype fashion. **This way, we can eliminate memory diffusion**

b, Change the way delete an entity works: 

1, on delete, Entity at index M is invalidate, its components at index M is also destructed 

2, when a new entity is pushed into this archtype, it is placed at location M. (This means you need maintain a free list of index that are available to be allocated) 

3, We need a GC cycle that periodically and smartly (through timing the cost of GC) **do memory defragmenting**.

c, Condsider refactor Entity holding method from class instances into reference counting pointers. This way, we can add validation and archetype bit mask into Entity class (as we no longer cares about sizeof(Entity) since we use pointer now)

(2) RenderDoc debug marker for OpenGL. [check here](https://stackoverflow.com/questions/54278607/how-to-create-debugging-markers-in-opengl) -->

<!-- (1) Using RefPtr to hold GameWorld life cycle -->

(0) Adding pause command in the spin lock

(1) Enable GLM SIMD instructions

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