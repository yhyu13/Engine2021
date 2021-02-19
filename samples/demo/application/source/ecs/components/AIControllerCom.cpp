#include "application-precompiled-header.h"
#include "AIControllerCom.h"

longmarch::AIControllerCom::AIControllerCom(const EntityDecorator& _this)
	:
	m_this(_this.GetEntity())
{
}

void longmarch::AIControllerCom::Update(double dt)
{
	BT.RunBT(EntityDecorator{ m_this, m_world }, dt);
}

BehaviorTree& longmarch::AIControllerCom::GetBT()
{
	return BT;
}

void longmarch::AIControllerCom::SetBT(const fs::path& filepath)
{
	BT.BTDeserialize(filepath);
}

Blackboard* longmarch::AIControllerCom::GetBlackboard()
{
	return bb;
}

void longmarch::AIControllerCom::SetBlackboard(Blackboard* blackboard)
{
	bb = blackboard;
}
