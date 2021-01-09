#include "engine-precompiled-header.h"

#include "DynamicTree.h"

namespace AAAAgames
{
    DynamicTreeNode::DynamicTreeNode()
        : m_parent(NULL_NODE),
        m_next(NULL_NODE),
        m_left(NULL_NODE),
        m_right(NULL_NODE),
        m_height(NULL_NODE),
        m_obj(nullptr)
    {

    }

    DynamicTreeNode::DynamicTreeNode(const DynamicTreeNode& node)
        : m_parent(node.m_parent),
        m_next(node.m_next),
        m_left(node.m_left),
        m_right(node.m_right),
        m_height(node.m_height),
        m_obj(node.m_obj)

    {

    }

    DynamicAABBTreeNode::DynamicAABBTreeNode()
        : DynamicTreeNode()
    {

    }

    DynamicAABBTreeNode::DynamicAABBTreeNode(const DynamicAABBTreeNode& node)
        : DynamicTreeNode(node)
    {
        m_aabb.SetMin(node.m_aabb.GetMin());
        m_aabb.SetMax(node.m_aabb.GetMax());
    }

    bool DynamicTreeNode::IsLeaf() const
    {
        return m_left == NULL_NODE;
    }

    
    DynamicTree::DynamicTree(unsigned int numObjects, double skinThickness) :
        m_root(NULL_NODE),
        m_nodeCount(0),
        m_nodeCapacity(numObjects),
        m_skinThickness(skinThickness)
    {
        
    }

    int DynamicAABBTree::allocateNode()
    {
        // if no free nodes, expand the node pool
        if (m_freeList == NULL_NODE)
        {
            //assert(m_nodeCount == m_nodeCapacity);

            m_nodeCapacity *= 2;
            m_nodes.resize(m_nodeCapacity);

            // create new list of nodes
            for (int i = m_nodeCount; i < m_nodeCapacity - 1; ++i)
            {
                m_nodes[i].m_next = i + 1;
                m_nodes[i].m_height = -1;
            }

            m_nodes[m_nodeCapacity - 1].m_next = NULL_NODE;
            m_nodes[m_nodeCapacity - 1].m_height = -1;

            // Assign the index of the first free node.
            m_freeList = m_nodeCount;
        }

        // Take a node from the free list
        int nodeIndex = m_freeList;

        m_freeList = m_nodes[nodeIndex].m_next;

        m_nodes[nodeIndex].m_parent = NULL_NODE;
        m_nodes[nodeIndex].m_left = NULL_NODE;
        m_nodes[nodeIndex].m_right = NULL_NODE;
        m_nodes[nodeIndex].m_height = 0;

        ++m_nodeCount;

        return nodeIndex;
    }

    void DynamicAABBTree::freeNode(int nodeIndex)
    {
        ENGINE_EXCEPT_IF(nodeIndex >= m_nodeCapacity || m_nodeCount <= 0, L"Node to free in Dynamic Tree doesn't exist!");

        m_nodes[nodeIndex].m_next = m_freeList;
        m_nodes[nodeIndex].m_height = -1;

        m_freeList = nodeIndex;

        --m_nodeCount;
    }

    DynamicAABBTree::DynamicAABBTree(unsigned int numObjects, double skinThickness)
        : DynamicTree(numObjects, skinThickness)
    {
        // Initialise the tree.
        m_nodes.resize(m_nodeCapacity);

        // Build a list of free nodes.
        for (int i = 0; i < m_nodeCapacity - 1; ++i)
        {
            m_nodes[i].m_next = i + 1;
            m_nodes[i].m_height = -1;
        }

        m_nodes[m_nodeCapacity - 1].m_next = NULL_NODE;
        m_nodes[m_nodeCapacity - 1].m_height = -1;

        // Assign index of the first free node.
        m_freeList = 0;
    }

    void DynamicAABBTree::InsertObject(const std::shared_ptr<RigidBody>& ptr)
    {
        // make sure object exists
        ENGINE_EXCEPT_IF(ptr == nullptr, L"Rigid Body to insert doesn't exist!");

        // make sure object has a shape
        ENGINE_EXCEPT_IF(ptr->GetShape().get() == nullptr, L"Rigid Body to insert doesn't have a shape!");

        // make sure object is new to the tree
        ENGINE_EXCEPT_IF(m_objectMap.find(ptr) != m_objectMap.end(), L"Object is already in the dynamic AABB tree!");

        Vec3f aabbHalfWidths = ptr->GetAABBWidths() * 0.5f;
        Vec3f lowerBound = ptr->GetWorldPosition() - aabbHalfWidths;
        Vec3f upperBound = ptr->GetWorldPosition() + aabbHalfWidths;

        // allocate node for the object
        int nodeIndex = allocateNode();

        // make sure bounds are valid
        for (unsigned int i = 0; i < 3; ++i)
            ENGINE_EXCEPT_IF(lowerBound[i] > upperBound[i], L"AABB upper bound is smaller than lower bound!");

        Vec3f aabbDimensions = upperBound - lowerBound;

        // if it's not a leaf node, "fatten" the AABB size w.r.t. its original size as well
        if (m_nodes[nodeIndex].IsLeaf())
        {
            m_nodes[nodeIndex].m_aabb.SetMin(lowerBound);
            m_nodes[nodeIndex].m_aabb.SetMax(upperBound);
        }

        else
        {
            Vec3f skinThickness = aabbDimensions;
            skinThickness *= m_skinThickness;

            m_nodes[nodeIndex].m_aabb.SetMin(lowerBound - skinThickness);
            m_nodes[nodeIndex].m_aabb.SetMax(upperBound + skinThickness);
        }

        // update surface area and center of AABB

        // height is 0 because it new objects are always in leaf nodes
        m_nodes[nodeIndex].m_height = 0;

        // insert leaf node
        insertLeaf(nodeIndex);

        m_objectMap.insert(std::unordered_map<std::shared_ptr<RigidBody>, int>::value_type(ptr, nodeIndex));

        m_nodes[nodeIndex].m_obj = ptr;
    }

    void DynamicAABBTree::RemoveObject(const std::shared_ptr<RigidBody>& ptr)
    {
        std::unordered_map <std::shared_ptr<RigidBody>, int>::iterator iter;

        iter = m_objectMap.find(ptr);

        ENGINE_EXCEPT_IF(iter == m_objectMap.end(), L"Attempted to remove non-existent object from dynamic tree!");

        int nodeIndex = iter->second;

        // remove object from the map
        m_objectMap.erase(iter);

        // remove the corresponding leaf node
        removeLeaf(nodeIndex);
        freeNode(nodeIndex);
    }

    int DynamicAABBTree::GetNumObjects()
    {
        return static_cast<int>(m_objectMap.size());
    }
    
    void DynamicAABBTree::RemoveAllObjects()
    {
        // Iterator pointing to the start of the particle map.
        std::unordered_map<std::shared_ptr<RigidBody>, int>::iterator iter = m_objectMap.begin();

        // Iterate over the map.
        while (iter != m_objectMap.end())
        {
            // Extract the node index.
            int nodeIndex = iter->second;

            ENGINE_EXCEPT_IF(nodeIndex >= m_nodeCapacity || !m_nodes[nodeIndex].IsLeaf(), L"Attempted to remove invalid leaf node!");

            removeLeaf(nodeIndex);
            freeNode(nodeIndex);

            ++iter;
        }

        // Clear the particle map.
        m_objectMap.clear();
    }

    bool DynamicAABBTree::UpdateObject(const std::shared_ptr<RigidBody>& ptr, bool alwaysReInsert)
    {
        // make sure object exists
        ENGINE_EXCEPT_IF(ptr == nullptr, L"Rigid Body to update doesn't exist!");

        // update the AABB of the corresponding object
        std::unordered_map<std::shared_ptr<RigidBody>, int>::iterator iter = m_objectMap.begin();

        iter = m_objectMap.find(ptr);

        ENGINE_EXCEPT_IF(iter == m_objectMap.end(), L"Attempted to update non-existent object in dynamic tree!");

        int nodeIndex = iter->second;

        ENGINE_EXCEPT_IF(nodeIndex >= m_nodeCapacity || !m_nodes[nodeIndex].IsLeaf(), L"Attempted to access invalid leaf node!");

        Vec3f newHalfWidths = ptr->GetAABBWidths() * 0.5f;
        Vec3f newMin = ptr->GetWorldPosition() - newHalfWidths;
        Vec3f newMax = ptr->GetWorldPosition() + newHalfWidths;

        // compute AABB limits and make sure bounds are valid
        // make sure bounds are valid
        for (unsigned int i = 0; i < 3; ++i)
            ENGINE_EXCEPT_IF(newMax[i] > newMin[i], L"AABB upper bound is smaller than lower bound!");

        Vec3f aabbDimensions = newMax - newMin;

        AABB newAABB(newMin, newMax);

        // remove current leaf node
        removeLeaf(nodeIndex);

        // if it's not a leaf node, "fatten" the AABB size w.r.t. its original size as well
        if (!m_nodes[nodeIndex].IsLeaf())
        {
            Vec3f skinThickness = aabbDimensions;
            skinThickness *= m_skinThickness;

            m_nodes[nodeIndex].m_aabb.SetMin(newMin - skinThickness);
            m_nodes[nodeIndex].m_aabb.SetMax(newMax + skinThickness);
        }

        // assign the new AABB
        m_nodes[nodeIndex].m_aabb.SetMin(newAABB.GetMin());
        m_nodes[nodeIndex].m_aabb.SetMax(newAABB.GetMax());

        // recompute surface area and center (if necessary)

        // insert new leaf node
        insertLeaf(nodeIndex);

        return true;
    }

    std::vector<std::shared_ptr<RigidBody>> DynamicAABBTree::Query(const std::shared_ptr<RigidBody>& ptr)
    {
        ENGINE_EXCEPT_IF(m_objectMap.count(ptr) == 0, L"Attempted to query but invalid ptr provided!");

        return Query(ptr, m_nodes[m_objectMap.find(ptr)->second].m_aabb);
    }

    std::vector<std::shared_ptr<RigidBody>> DynamicAABBTree::Query(const std::shared_ptr<RigidBody>& ptr, const AABB& aabb)
    {
        std::stack<size_t> nodeStack;
        nodeStack.push(m_root);

        std::vector<std::shared_ptr<RigidBody>> objList;

        while (nodeStack.size() > 0)
        {
            size_t nodeIndex = nodeStack.top();
            nodeStack.pop();

            // if null node, skip to next node
            if (nodeIndex == NULL_NODE)
            continue;
            
            // otherwise make a copy of the popped node's AABB
            AABB nodeAABB;
            
            nodeAABB.SetMin(m_nodes[nodeIndex].m_aabb.GetMin());
            nodeAABB.SetMax(m_nodes[nodeIndex].m_aabb.GetMax());
            
            // test for overlap with the input AABB
            if (aabb.Overlaps(nodeAABB))
            {
                // if there is overlap, check first if leaf node
                if (m_nodes[nodeIndex].IsLeaf())
                {
                    // add object to object list if the current node and queried node are not the same
                    if (m_nodes[nodeIndex].m_obj != ptr || ptr == nullptr)
                        objList.push_back(m_nodes[nodeIndex].m_obj);
                }
            
                // if not leaf node, add children nodes to stack
                else
                {
                    nodeStack.push(m_nodes[nodeIndex].m_left);
                    nodeStack.push(m_nodes[nodeIndex].m_right);
                }
            }
        }

        return objList;
    }

    std::vector<std::shared_ptr<RigidBody>> DynamicAABBTree::Query(const AABB& aabb)
    {
        // if tree is empty, return empty vector
        if (m_objectMap.size() == 0)
            return std::vector<std::shared_ptr<RigidBody>>();

        // otherwise test overlap with all objects and return results
        return Query(nullptr, aabb);
    }

    const AABB& DynamicAABBTree::GetAABB(const std::shared_ptr<RigidBody>& ptr)
    {
        // throw exception if AABB can't be obtained
        ENGINE_EXCEPT_IF(m_objectMap.count(ptr) == 0, L"Attempted to get AABB from tree but RB does not exist!");

        return m_nodes[m_objectMap[ptr]].m_aabb;
    }

    void DynamicAABBTree::insertLeaf(int nodeIndex)
    {
        // case where tree is empty
        if (m_root == NULL_NODE)
        {
            m_root = nodeIndex;
            m_nodes[m_root].m_parent = NULL_NODE;
            return;
        }

        AABB leafAABB;
        leafAABB.SetMin(m_nodes[nodeIndex].m_aabb.GetMin());
        leafAABB.SetMax(m_nodes[nodeIndex].m_aabb.GetMax());

        int index = m_root;

        // travel down the tree to find the best sibling node for the new leaf
        while (!m_nodes[index].IsLeaf())
        {
            int leftNodeIndex = m_nodes[index].m_left;
            int rightNodeIndex = m_nodes[index].m_right;

            f32 surfaceArea = m_nodes[index].m_aabb.ComputeSurfaceArea();

            AABB combinedAABB;
            combinedAABB.Merge(m_nodes[index].m_aabb, leafAABB);

            f32 combinedSurfaceArea = combinedAABB.ComputeSurfaceArea();

            f32 cost = combinedSurfaceArea * 2.0f;
            f32 inheritanceCost = (combinedSurfaceArea - surfaceArea) * 2.0f;

            f32 leftCost, rightCost;

            // compute cost of descending in the left subtree and right subtree
            if (m_nodes[leftNodeIndex].IsLeaf())
            {
                AABB tempAABB;
                tempAABB.Merge(leafAABB, m_nodes[leftNodeIndex].m_aabb);

                leftCost = tempAABB.ComputeSurfaceArea() + inheritanceCost;
            }

            else
            {
                AABB tempAABB;
                tempAABB.Merge(leafAABB, m_nodes[leftNodeIndex].m_aabb);

                float oldSurfaceArea = m_nodes[leftNodeIndex].m_aabb.ComputeSurfaceArea();
                float newSurfaceArea = tempAABB.ComputeSurfaceArea();

                leftCost = (newSurfaceArea - oldSurfaceArea) + inheritanceCost;
            }

            if (m_nodes[rightNodeIndex].IsLeaf())
            {
                AABB tempAABB;
                tempAABB.Merge(leafAABB, m_nodes[rightNodeIndex].m_aabb);

                rightCost = tempAABB.ComputeSurfaceArea() + inheritanceCost;
            }

            else
            {
                AABB tempAABB;
                tempAABB.Merge(leafAABB, m_nodes[rightNodeIndex].m_aabb);

                float oldSurfaceArea = m_nodes[rightNodeIndex].m_aabb.ComputeSurfaceArea();
                float newSurfaceArea = tempAABB.ComputeSurfaceArea();

                rightCost = (newSurfaceArea - oldSurfaceArea) + inheritanceCost;
            }

            // descend down the tree, going by the minimum cost
            if ((cost < leftCost) && (cost < rightCost))
                break;

            if (leftCost < rightCost)
                index = leftNodeIndex;

            else
                index = rightNodeIndex;
        }

        // now at best sibling node, create a new parent and attach both the sibling node and new leaf node
        int siblingIndex = index;

        int oldParentIndex = m_nodes[siblingIndex].m_parent;
        int newParentIndex = allocateNode();

        m_nodes[newParentIndex].m_parent = oldParentIndex;
        m_nodes[newParentIndex].m_aabb.Merge(leafAABB, m_nodes[siblingIndex].m_aabb);
        m_nodes[newParentIndex].m_height = m_nodes[siblingIndex].m_height + 1;

        // case where sibling was not the root node
        if (oldParentIndex != NULL_NODE)
        {
            if (m_nodes[oldParentIndex].m_left == siblingIndex)
                m_nodes[oldParentIndex].m_left = newParentIndex;

            else
                m_nodes[oldParentIndex].m_right = newParentIndex;

            m_nodes[newParentIndex].m_left = siblingIndex;
            m_nodes[newParentIndex].m_right = nodeIndex;
            m_nodes[siblingIndex].m_parent = newParentIndex;
            m_nodes[nodeIndex].m_parent = newParentIndex;
        }

        // case where sibling is the root node
        else
        {
            m_nodes[newParentIndex].m_left = siblingIndex;
            m_nodes[newParentIndex].m_right = nodeIndex;
            m_nodes[siblingIndex].m_parent = newParentIndex;
            m_nodes[nodeIndex].m_parent = newParentIndex;
            m_root = newParentIndex;
        }

        // walk back up the tree and fix heights and AABBs
        index = m_nodes[nodeIndex].m_parent;

        while (index != NULL_NODE)
        {
            index = balance(index);

            int left = m_nodes[index].m_left;
            int right = m_nodes[index].m_right;

            assert(left != NULL_NODE);
            assert(right != NULL_NODE);

            m_nodes[index].m_height = std::max(m_nodes[left].m_height, m_nodes[right].m_height) + 1;
            m_nodes[index].m_aabb.Merge(m_nodes[left].m_aabb, m_nodes[right].m_aabb);

            index = m_nodes[index].m_parent;
        }
    }

    void DynamicAABBTree::removeLeaf(int nodeIndex)
    {
        // case of root node, just set to null
        if (nodeIndex == m_root)
        {
            m_root = NULL_NODE;
            return;
        }

        int parentIndex = m_nodes[nodeIndex].m_parent;
        int grandParentIndex = m_nodes[parentIndex].m_parent;
        int siblingIndex;

        if (m_nodes[parentIndex].m_left == nodeIndex)
            siblingIndex = m_nodes[parentIndex].m_right;

        else
            siblingIndex = m_nodes[parentIndex].m_left;

        // remove parent and connect siblings to grandparent
        if (grandParentIndex != NULL_NODE)
        {
            if (m_nodes[grandParentIndex].m_left == parentIndex)
                m_nodes[grandParentIndex].m_left = siblingIndex;

            else
                m_nodes[grandParentIndex].m_right = siblingIndex;

            m_nodes[siblingIndex].m_parent = grandParentIndex;
            freeNode(parentIndex);

            // Adjust ancestor bounds.
            int index = grandParentIndex;

            while (index != NULL_NODE)
            {
                index = balance(index);

                int left = m_nodes[index].m_left;
                int right = m_nodes[index].m_right;

                m_nodes[index].m_aabb.Merge(m_nodes[left].m_aabb, m_nodes[right].m_aabb);
                m_nodes[index].m_height = 1 + std::max(m_nodes[left].m_height, m_nodes[right].m_height);

                index = m_nodes[index].m_parent;
            }
        }

        else
        {
            m_root = siblingIndex;
            m_nodes[siblingIndex].m_parent = NULL_NODE;
            freeNode(parentIndex);
        }
    }
    
    int DynamicAABBTree::rotateLeft(int nodeIndex)
    {
        int left = m_nodes[nodeIndex].m_left;
        int right = m_nodes[nodeIndex].m_right;

        int rightLeft = m_nodes[right].m_left;
        int rightRight = m_nodes[right].m_right;

        ENGINE_EXCEPT_IF(rightLeft >= m_nodeCapacity || rightRight >= m_nodeCapacity, L"Node index provided is out of bounds!");

        // Swap node and its right-hand child.
        m_nodes[right].m_left = nodeIndex;
        m_nodes[right].m_parent = m_nodes[nodeIndex].m_parent;
        m_nodes[nodeIndex].m_parent = right;

        // the node's old parent should now point to its right-hand child
        if (m_nodes[right].m_parent != NULL_NODE)
        {
            if (m_nodes[m_nodes[right].m_parent].m_left == nodeIndex)
                m_nodes[m_nodes[right].m_parent].m_left = right;

            else
            {
                //assert(m_nodes[m_nodes[right].m_parent].m_right == nodeIndex);
                ENGINE_EXCEPT_IF(m_nodes[m_nodes[right].m_parent].m_right != nodeIndex, L"Node index provided is out of bounds!");
                m_nodes[m_nodes[right].m_parent].m_right = right;
            }
        }

        else
            m_root = right;

        // rotate using nodeIndex node as pivot
        if (m_nodes[rightLeft].m_height > m_nodes[rightRight].m_height)
        {
            m_nodes[right].m_right = rightLeft;
            m_nodes[nodeIndex].m_right = rightRight;
            m_nodes[rightRight].m_parent = nodeIndex;

            m_nodes[nodeIndex].m_aabb.Merge(m_nodes[left].m_aabb, m_nodes[rightRight].m_aabb);
            m_nodes[right].m_aabb.Merge(m_nodes[nodeIndex].m_aabb, m_nodes[rightLeft].m_aabb);

            m_nodes[nodeIndex].m_height = 1 + std::max(m_nodes[left].m_height, m_nodes[rightRight].m_height);
            m_nodes[right].m_height = 1 + std::max(m_nodes[nodeIndex].m_height, m_nodes[rightLeft].m_height);
        }

        else
        {
            m_nodes[right].m_right = rightRight;
            m_nodes[nodeIndex].m_right = rightLeft;
            m_nodes[rightLeft].m_parent = nodeIndex;

            m_nodes[nodeIndex].m_aabb.Merge(m_nodes[left].m_aabb, m_nodes[rightLeft].m_aabb);
            m_nodes[right].m_aabb.Merge(m_nodes[nodeIndex].m_aabb, m_nodes[rightRight].m_aabb);

            m_nodes[nodeIndex].m_height = std::max(m_nodes[left].m_height, m_nodes[rightLeft].m_height) + 1;
            m_nodes[right].m_height = std::max(m_nodes[nodeIndex].m_height, m_nodes[rightRight].m_height) + 1;
        }

        return right;
    }

    int DynamicAABBTree::rotateRight(int nodeIndex)
    {
        int left = m_nodes[nodeIndex].m_left;
        int right = m_nodes[nodeIndex].m_right;

        int leftLeft = m_nodes[left].m_left;
        int leftRight = m_nodes[left].m_right;

        ENGINE_EXCEPT_IF(leftLeft >= m_nodeCapacity, L"Node index provided is out of bounds!");
        ENGINE_EXCEPT_IF(leftRight >= m_nodeCapacity, L"Node index provided is out of bounds!");

        // Swap node and its left-hand child.
        m_nodes[left].m_left = nodeIndex;
        m_nodes[left].m_parent = m_nodes[nodeIndex].m_parent;
        m_nodes[nodeIndex].m_parent = left;

        // the node's old parent should now point to its left-hand child
        if (m_nodes[left].m_parent != NULL_NODE)
        {
            if (m_nodes[m_nodes[left].m_parent].m_left == nodeIndex)
                m_nodes[m_nodes[left].m_parent].m_left = left;

            else
            {
                //assert(m_nodes[m_nodes[left].m_parent].m_right == nodeIndex);
                ENGINE_EXCEPT_IF(m_nodes[m_nodes[left].m_parent].m_right != nodeIndex, L"Node index provided is out of bounds!");
                m_nodes[m_nodes[left].m_parent].m_right = left;
            }
        }

        else m_root = left;

        // // rotate using nodeIndex node as pivot
        if (m_nodes[leftLeft].m_height > m_nodes[leftRight].m_height)
        {
            m_nodes[left].m_right = leftLeft;
            m_nodes[nodeIndex].m_left = leftRight;
            m_nodes[leftRight].m_parent = nodeIndex;

            m_nodes[nodeIndex].m_aabb.Merge(m_nodes[right].m_aabb, m_nodes[leftRight].m_aabb);
            m_nodes[left].m_aabb.Merge(m_nodes[nodeIndex].m_aabb, m_nodes[leftLeft].m_aabb);

            m_nodes[nodeIndex].m_height = std::max(m_nodes[right].m_height, m_nodes[leftRight].m_height) + 1;
            m_nodes[left].m_height = std::max(m_nodes[nodeIndex].m_height, m_nodes[leftLeft].m_height) + 1;
        }

        else
        {
            m_nodes[left].m_right = leftRight;
            m_nodes[nodeIndex].m_left = leftLeft;
            m_nodes[leftLeft].m_parent = nodeIndex;

            m_nodes[nodeIndex].m_aabb.Merge(m_nodes[right].m_aabb, m_nodes[leftLeft].m_aabb);
            m_nodes[left].m_aabb.Merge(m_nodes[nodeIndex].m_aabb, m_nodes[leftRight].m_aabb);

            m_nodes[nodeIndex].m_height = std::max(m_nodes[right].m_height, m_nodes[leftLeft].m_height) + 1;
            m_nodes[left].m_height = std::max(m_nodes[nodeIndex].m_height, m_nodes[leftRight].m_height) + 1;
        }

        return left;
    }

    int DynamicAABBTree::balance(int nodeIndex)
    {
        // make sure node index is valid
        ENGINE_EXCEPT_IF(nodeIndex == NULL_NODE, L"Attempted to balance node that doesn't exist!");

        // case where leaf node, or tree height is short enough that it's no balancing is needed
        if (m_nodes[nodeIndex].IsLeaf() || (m_nodes[nodeIndex].m_height < 2))
            return nodeIndex;

        int left = m_nodes[nodeIndex].m_left;
        int right = m_nodes[nodeIndex].m_right;

        ENGINE_EXCEPT_IF(left >= m_nodeCapacity || right >= m_nodeCapacity, L"Node index provided is out of bounds!");

        int currentBalance = m_nodes[right].m_height - m_nodes[left].m_height;

        // rotate right branch up
        if (currentBalance > 1)
            return rotateLeft(nodeIndex);

        // rotate left branch up
        if (currentBalance < -1)
            return rotateRight(nodeIndex);

        return nodeIndex;
    }

    unsigned int DynamicAABBTree::computeHeight() const
    {
        return computeHeight(m_root);
    }

    unsigned int DynamicAABBTree::computeHeight(int nodeIndex) const
    {
        ENGINE_EXCEPT_IF(nodeIndex >= m_nodeCapacity, L"Node index provided is out of bounds!");

        if (m_nodes[nodeIndex].IsLeaf())
            return 0;

        unsigned int height1 = computeHeight(m_nodes[nodeIndex].m_left);
        unsigned int height2 = computeHeight(m_nodes[nodeIndex].m_right);

        return std::max(height1, height2) + 1;
    }

    unsigned int DynamicAABBTree::GetHeight() const
    {
        if (m_root == NULL_NODE)
            return 0;

        return m_nodes[m_root].m_height;
    }

    unsigned int DynamicAABBTree::GetNodeCount() const
    {
        return m_nodeCount;
    }

    void DynamicAABBTree::validateStructure(int nodeIndex) const
    {
        if (nodeIndex == NULL_NODE)
            return;

        if (nodeIndex == m_root)
            ENGINE_EXCEPT_IF(m_nodes[nodeIndex].m_parent != NULL_NODE, L"Root node has a parent!");

        int left = m_nodes[nodeIndex].m_left;
        int right = m_nodes[nodeIndex].m_right;

        // if leaf, make sure height is 0 and there are no children nodes
        if (m_nodes[nodeIndex].IsLeaf())
        {
            ENGINE_EXCEPT_IF(left != NULL_NODE || right != NULL_NODE || m_nodes[nodeIndex].m_height != 0, L"Invalid Leaf Node found!");

            return;
        }

        // check the children and parent nodes
        ENGINE_EXCEPT_IF(left >= m_nodeCapacity || right >= m_nodeCapacity, L"Invalid child node index found!");
        ENGINE_EXCEPT_IF(m_nodes[left].m_parent != nodeIndex || m_nodes[right].m_parent != nodeIndex, L"Non-matching parent node index found!");

        // recursively validate down the tree
        validateStructure(left);
        validateStructure(right);

        //return true;
    }

    void DynamicAABBTree::validateMetrics(int nodeIndex) const
    {
        if (nodeIndex == NULL_NODE)
            return;

        int left = m_nodes[nodeIndex].m_left;
        int right = m_nodes[nodeIndex].m_right;

        // if leaf, make sure height is 0 and there are no children nodes
        if (m_nodes[nodeIndex].IsLeaf())
        {
            ENGINE_EXCEPT_IF(left != NULL_NODE || right != NULL_NODE || m_nodes[nodeIndex].m_height != 0, L"Invalid Leaf Node found!");

            return;
        }

        // check the children
        ENGINE_EXCEPT_IF(left >= m_nodeCapacity || right >= m_nodeCapacity, L"Invalid child node index found!");

        int height1 = m_nodes[left].m_height;
        int height2 = m_nodes[right].m_height;
        int height = std::max(height1, height2) + 1;

        ENGINE_EXCEPT_IF(m_nodes[nodeIndex].m_height != height, L"Incorrect tree height!");

        AABB aabb;
        aabb.Merge(m_nodes[left].m_aabb, m_nodes[right].m_aabb);

        Vec3f aabbMin = aabb.GetMin(), aabbMax = aabb.GetMax();
        Vec3f aabbMin2 = m_nodes[nodeIndex].m_aabb.GetMin(), aabbMax2 = m_nodes[nodeIndex].m_aabb.GetMax();

        for (unsigned int i = 0; i < 3; ++i)
        {
            ENGINE_EXCEPT_IF(aabbMin != aabbMin2, L"Incorrect lower bound!");
            ENGINE_EXCEPT_IF(aabbMax != aabbMax2, L"Incorrect upper bound!");
        }

        // recursively validate down the tree
        validateMetrics(left);
        validateMetrics(right);
    }

    void DynamicAABBTree::Validate() const
    {
        validateStructure(m_root);
        validateMetrics(m_root);

        int freeCount = 0;
        int freeIndex = m_freeList;

        while (freeIndex != NULL_NODE)
        {
            ENGINE_EXCEPT_IF(freeIndex >= m_nodeCapacity, L"Incorrect node index!");

            freeIndex = m_nodes[freeIndex].m_next;
            ++freeCount;
        }

        ENGINE_EXCEPT_IF(GetHeight() != computeHeight(), L"Incorrect tree height!");
        ENGINE_EXCEPT_IF(m_nodeCount + freeCount != m_nodeCapacity, L"Node count doesn't match ndoe capacity!");
    }

    void DynamicAABBTree::Rebuild()
    {
        std::vector<int> nodeIndices(m_nodeCount);
        int count = 0;

        for (int i = 0; i < m_nodeCapacity; ++i)
        {
            if (m_nodes[i].m_height < 0)
                continue;

            if (m_nodes[i].IsLeaf())
            {
                m_nodes[i].m_parent = NULL_NODE;
                nodeIndices[count] = i;

                ++count;
            }

            else
                freeNode(i);
        }

        while (count > 1)
        {
            f32 minCost = std::numeric_limits<float>::max();
            int minIndex1 = -1, minIndex2 = -1;

            for (int i = 0; i < count; i++)
            {
                AABB aabb1;
                aabb1.SetMin(m_nodes[nodeIndices[i]].m_aabb.GetMin());
                aabb1.SetMax(m_nodes[nodeIndices[i]].m_aabb.GetMax());

                for (int j = i + 1; j < count; j++)
                {
                    AABB aabb2;
                    aabb2.SetMin(m_nodes[nodeIndices[j]].m_aabb.GetMin());
                    aabb2.SetMax(m_nodes[nodeIndices[j]].m_aabb.GetMax());

                    AABB combinedAABB;

                    combinedAABB.Merge(aabb1, aabb2);
                    f32 cost = combinedAABB.ComputeSurfaceArea();

                    if (cost < minCost)
                    {
                        minIndex1 = i;
                        minIndex2 = j;
                        minCost = cost;
                    }
                }
            }

            int index1 = nodeIndices[minIndex1];
            int index2 = nodeIndices[minIndex2];

            int parent = allocateNode();

            m_nodes[parent].m_parent = NULL_NODE;
            m_nodes[parent].m_left = index1;
            m_nodes[parent].m_right = index2;
            m_nodes[parent].m_height = std::max(m_nodes[index1].m_height, m_nodes[index2].m_height) + 1;
            m_nodes[parent].m_aabb.Merge(m_nodes[index1].m_aabb, m_nodes[index2].m_aabb);

            m_nodes[index1].m_parent = parent;
            m_nodes[index2].m_parent = parent;

            nodeIndices[minIndex1] = parent;
            nodeIndices[minIndex2] = nodeIndices[--count];
        }

        m_root = nodeIndices[0];

        Validate();
    }

    void DynamicAABBTree::Render()
    {
        // render all the AABBs in the tree
        for (auto& elem : m_nodes)
        {
            Vec3f testMin = elem.m_aabb.GetMin();
            Vec3f testMax = elem.m_aabb.GetMax();

            std::cout << "(" << testMin.x << ", " << testMin.y << ", " << testMin.z << ") (" << testMax.x << ", " << testMax.y << ", " << testMax.z << ")" << std::endl;

            dynamic_cast<Shape*>(&elem.m_aabb)->RenderShape();
        }

        //std::cout << "one cycle" << std::endl;
    }
}