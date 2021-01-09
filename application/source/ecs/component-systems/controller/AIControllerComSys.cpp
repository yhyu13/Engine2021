#include "application-precompiled-header.h"
#include "AIControllerComSys.h"

AAAAgames::AIControllerComSys::AIControllerComSys()
{
	m_systemSignature.AddComponent<AIControllerCom>();
	m_bb = MemoryManager::Make_shared<Blackboard>();
}

void AAAAgames::AIControllerComSys::PostRenderUpdate(double dt)
{
	EARLY_RETURN(dt);
	ForEach(
		[this, dt](EntityDecorator e)
	{
		auto aicom = e.GetComponent<AIControllerCom>();
		if (aicom->GetBlackboard() != m_bb.get())
		{
			aicom->SetBlackboard(m_bb.get());
		}
		aicom->Update(dt);
	}
	);
}

std::shared_ptr<BaseComponentSystem> AAAAgames::AIControllerComSys::Copy() const
{
	LOCK_GUARD_NC();
	auto ret = MemoryManager::Make_shared<AIControllerComSys>();
	ret->m_bufferedRegisteredEntities = m_bufferedRegisteredEntities; // Must copy
	ret->m_systemSignature = m_systemSignature; // Must copy
	ret->m_bb = MemoryManager::Make_shared<Blackboard>(*m_bb); // Custom variables
	return ret;
}
