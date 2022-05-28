TODO List

(1) Refactor ECS Archtype memory management : instead of using entities and component manager, using entity->Archtype->component managers.

a, You need to bit mask EntityType to direct to the index of m_maskEntityVecMap so that a entity can know which archtype it belongs to.

b, You need a archtype2component managers dictionary such that all allocations of components of certain archtype can be managed in per-archtype fashion. **This way, we can eliminate memory diffusion**


c, Change the way delete an entity works: 

1, on delete, Entity at index M is invalidate, its components at index M is also destructed 

2, when a new entity is pushed into this archtype, it is placed at location M. (This means you need maintain a free list of index that are available to be allocated) 

3, We need a GC cycle that periodically and smartly (through timing the cost of GC) **do memory defragmenting**.

(2) RenderDoc debug marker for OpenGL. [check here](https://stackoverflow.com/questions/54278607/how-to-create-debugging-markers-in-opengl)

(3) Refacotr memory allocator with ABA lock-free free list or integrate with Intel Thread Building BLock (TBB)

(4) Vulkan renderer, integrate with VulkanSceneGraph(?it's quite big a 3rd party project, what excatly should we integrate?)
a, consider render thread, draw thread design that support wide range of frame limit

