#pragma once

#include "State.h"

class StateStack
{
public:
	enum Action
	{
		Push,
		Pop,
		Clear
	};

public:
	explicit			StateStack();
	~StateStack();

	template <typename T>
	void				registerState(State_ID stateID,
		ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList,
		MeshGeometry* Mesh, int CbvSize, Game* game);

	void				update(float dt, struct FrameResource* Frame);
	void				draw(ID3D12GraphicsCommandList* CmdList, ID3D12DescriptorHeap* Desc,
		UINT DescSize, struct FrameResource* Frame,
		float DeltaTime);
	void				handleEvent();

	void				pushState(State_ID stateID);
	void				popState();
	void				clearStates();

	bool				isEmpty() const;


public:
	State*			FindState(State_ID stateID);
	State* GetCurrentState()
	{
		return m_Stack.back();
	}
	void				applyPendingChanges();


private:
	struct PendingChange
	{
		explicit			PendingChange(Action action, State_ID stateID = State_ID::None);

		Action				action;
		State_ID			stateID;
	};


private:
	std::vector<State*>	m_Stack;
	std::vector<PendingChange>							m_PendingList;
	std::map<State_ID, State*>	m_Factories;
};

template <typename T>
void StateStack::registerState(State_ID stateID,
	ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList,
	MeshGeometry* Mesh, int CbvSize, Game* game)
{
	auto	iter = m_Factories.find(stateID);

	if (iter != m_Factories.end())
	{
		delete iter->second;
		m_Factories.erase(iter);
	}

	T* Obj = new T(this);

	Obj->SetMesh(Mesh);
	Obj->SetCbvSrvSize(CbvSize);
	Obj->SetGame(game);

	Obj->Init(Device, CmdList);

	m_Factories.insert(std::make_pair(stateID, (State*)Obj));
}

