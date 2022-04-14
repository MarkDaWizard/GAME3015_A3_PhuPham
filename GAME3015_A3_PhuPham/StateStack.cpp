#include "StateStack.h"
#include <cassert>


StateStack::StateStack()
	: m_Stack()
	, m_PendingList()
	, m_Factories()
{
}

StateStack::~StateStack()
{
	for (auto i = m_Factories.begin(); i != m_Factories.end(); ++i)
	{
		delete i->second;
	}
}

void StateStack::update(float dt, struct FrameResource* Frame)
{
	for (auto itr = m_Stack.rbegin(); itr != m_Stack.rend(); ++itr)
	{
		if ((*itr)->Update(dt, Frame))
			break;
	}

	applyPendingChanges();
}

void StateStack::draw(ID3D12GraphicsCommandList* CmdList, ID3D12DescriptorHeap* Desc,
	UINT DescSize, struct FrameResource* Frame,
	float DeltaTime)
{
	if (!m_Stack.empty())
		m_Stack.back()->Draw(CmdList, Desc, DescSize, Frame, DeltaTime);
}

void StateStack::handleEvent()
{
	// Iterate from top to bottom, stop as soon as handleEvent() returns false
	for (auto itr = m_Stack.rbegin(); itr != m_Stack.rend(); ++itr)
	{
		(*itr)->ProcessInput();
		//if (!(*itr)->ProcessInput())
		//	break;
	}

	applyPendingChanges();
}

void StateStack::pushState(State_ID stateID)
{
	m_PendingList.push_back(PendingChange(Push, stateID));
}

void StateStack::popState()
{
	m_PendingList.push_back(PendingChange(Pop));
}

void StateStack::clearStates()
{
	m_PendingList.push_back(PendingChange(Clear));
}

bool StateStack::isEmpty() const
{
	return m_Stack.empty();
}

State* StateStack::FindState(State_ID stateID)
{
	auto found = m_Factories.find(stateID);
	assert(found != m_Factories.end());

	return found->second;
}

void StateStack::applyPendingChanges()
{
	for (PendingChange change : m_PendingList)
	{
		switch (change.action)
		{
		case Push:
			m_Stack.push_back(FindState(change.stateID));
			break;

		case Pop:
			m_Stack.pop_back();
			break;

		case Clear:
			m_Stack.clear();
			break;
		}
	}

	m_PendingList.clear();
}

StateStack::PendingChange::PendingChange(Action action, State_ID stateID)
	: action(action)
	, stateID(stateID)
{
}


