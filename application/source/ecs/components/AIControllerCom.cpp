#include "application-precompiled-header.h"
#include "AIControllerCom.h"

AAAAgames::AIControllerCom::AIControllerCom(Entity _this)
	:
	m_this(_this)
{
}

void AAAAgames::AIControllerCom::Update(double dt)
{
	BT.RunBT(EntityDecorator{ m_this, m_world }, dt);
}

BehaviorTree& AAAAgames::AIControllerCom::GetBT()
{
	return BT;
}

void AAAAgames::AIControllerCom::SetBT(const fs::path& filepath)
{
	BT.BTDeserialize(filepath);
}

Blackboard* AAAAgames::AIControllerCom::GetBlackboard()
{
	return bb;
}

void AAAAgames::AIControllerCom::SetBlackboard(Blackboard* blackboard)
{
	bb = blackboard;
}
