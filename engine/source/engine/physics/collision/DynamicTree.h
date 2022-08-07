#pragma once

#include <unordered_map>
#include <vector>

#include <FastBVH.h>

#include "engine/math/Geommath.h"

#include "engine/physics/AABB.h"
#include "engine/physics/dynamics/RigidBody.h"

namespace longmarch
{
    template<typename Float = float>
    class FastBVH_RigidBodyConverter final {
    public:
        //! Converts a sphere to a bounding box.
        //! \param sphere The sphere to convert to a bounding box.
        //! \return A bounding box that encapsulates the sphere.
        FastBVH::BBox<Float> operator()(const RigidBody* rb) const noexcept {
            auto center = rb->GetShape()->GetCenter();
            auto radius = rb->GetShape()->GetRadius();
            auto min = center - radius;
            auto max = center + radius;
            return FastBVH::BBox<float>(FastBVH::Vector3<Float>{min.x, min.y, min.z}, FastBVH::Vector3<Float>{max.x, max.y, max.z});
        }
    };

    const int NULL_NODE = -1;

    struct DynamicTreeNode
    {
        DynamicTreeNode();
        DynamicTreeNode(const DynamicTreeNode& node);

        virtual bool IsLeaf() const;

        // parent node, next node, left and right childs respectively
        int m_parent;
        int m_next;
        int m_left;
        int m_right;

        // height is 0 for leaf nodes and -1 for free nodes
        int m_height;

        // index of object contained in the node (only for leaf nodes)
        std::shared_ptr<RigidBody> m_obj;
    };

    struct DynamicAABBTreeNode : DynamicTreeNode
    {
        DynamicAABBTreeNode();
        DynamicAABBTreeNode(const DynamicAABBTreeNode& node);

        // AABB of the node (not the same as the AABB of the object contained in the node (if it's a leaf node)
        AABB m_aabb;
    };

    class DynamicTree
    {
    public:
        DynamicTree(unsigned int numObjects = 16, double skinThickness = 0.05);

        //virtual void InsertObject(const std::shared_ptr<RigidBody>& ptr) = 0;

        //void InsertObject(std::shared_ptr<RigidBody>, Vec3f& pos, double radius);

        //void InsertObject(size_t objectID, Vec3f& lowerBound, Vec3f& upperBound);

        virtual int GetNumObjects() = 0;

        virtual void RemoveObject(const std::shared_ptr<RigidBody>& ptr) = 0;

        virtual void RemoveAllObjects() = 0;

        // Update object if it moves outside its enclosing shape
        //virtual bool UpdateObject(const std::shared_ptr<RigidBody>& ptr) = 0;

        //bool UpdateObject(size_t id, Vec3f& pos, double radius, bool alwaysReinsert = false);

        //bool UpdateObject(size_t id, Vec3f& lowerBound, Vec3f& upperBound, bool alwaysReinsert = false);

        // Query the tree to find intersecting objects
        virtual std::vector<std::shared_ptr<RigidBody>> Query(const std::shared_ptr<RigidBody>& ptr) = 0;

        //virtual unsigned int GetHeight() const;
        //virtual unsigned int GetNodeCount() const;

        

        // get the tree of nodes (for printing purposes)
        //virtual const std::vector<DynamicTreeNode>& GetNodes() const { return m_nodes; }

    protected:
        // internal index of the root node
        int m_root;

        // current number of nodes in the tree.
        int m_nodeCount;

        // current node capacity.
        int m_nodeCapacity;

        // index of node at the top of the free list.
        int m_freeList;

        // skin thickness of the BVs proportional to the size of the BV
        double m_skinThickness;

        // A map between object and node indices.
        std::unordered_map<std::shared_ptr<RigidBody>, int> m_objectMap;

        virtual int allocateNode() = 0;

        virtual void freeNode(int nodeIndex) = 0;

        virtual void insertLeaf(int nodeIndex) = 0;

        virtual void removeLeaf(int nodeIndex) = 0;

        // balances the tree.
        //virtual int balance(int nodeIndex);

        // compute the height of the tree.
        virtual unsigned int computeHeight() const = 0;

        // compute the height of a sub-tree.
        virtual unsigned int computeHeight(int nodeIndex) const = 0;

        
    };

    // specialization for spheres
    class DynamicAABBTree : DynamicTree
    {
    public:
        //! Constructor
        DynamicAABBTree(unsigned int numObjects = 16, double skinThickness = 0.05);

        void InsertObject(const std::shared_ptr<RigidBody>& ptr);

        int GetNumObjects();

        void RemoveObject(const std::shared_ptr<RigidBody>& ptr);

        void RemoveAllObjects();

        // Update object if it moves outside its fattened AABB
        //bool UpdateObject(size_t id, Vec3f& pos, double radius, bool alwaysReinsert = false);

        bool UpdateObject(const std::shared_ptr<RigidBody>& ptr, bool alwaysReInsert = false);

        // Query the tree to find candidate interactions for an object.
        std::vector<std::shared_ptr<RigidBody>> Query(const std::shared_ptr<RigidBody>& ptr);

        // Query the tree to find candidate interactions for a AABB.
        std::vector<std::shared_ptr<RigidBody>> Query(const std::shared_ptr<RigidBody>& ptr, const AABB& aabb);

        // Query the tree to find candidate interactions for an AABB.
        std::vector<std::shared_ptr<RigidBody>> Query(const AABB& aabb);

        // Get AABB of an object given its rigid body ptr
        const AABB& GetAABB(const std::shared_ptr<RigidBody>& ptr);

        // Get the height of the tree.
        unsigned int GetHeight() const;

        // Get the number of nodes in the tree.
        unsigned int GetNodeCount() const;

        virtual void Validate() const;

        virtual void Rebuild();

        virtual void Render();

    protected:

        std::vector<DynamicAABBTreeNode> m_nodes;

        virtual int allocateNode();

        virtual void freeNode(int nodeIndex);

        virtual void insertLeaf(int nodeIndex);

        virtual void removeLeaf(int  nodeIndex);

        virtual int rotateLeft(int nodeIndex);

        virtual int rotateRight(int nodeIndex);

        virtual int balance(int nodeIndex);

        // Compute the height of the tree.
        virtual unsigned int computeHeight() const;

        // Compute the height of a sub-tree.
        virtual unsigned int computeHeight(int nodeIndex) const;

        // Check if structure is valid from specified index onwards
        void validateStructure(int nodeIndex) const;

        // Check if metrics are valid from specified index onwards
        void validateMetrics(int nodeIndex) const;

        // The radius of the system
        double m_radius;
    };
}
