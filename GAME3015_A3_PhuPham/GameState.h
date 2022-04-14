#pragma once

#include "State.h"
#include "Image.h"
#include "Player.h"
#include "AirCraft.h"
#include "Entity.h"
#include "ScrollingGround.h"

class StateStack;
class GameState	:
	public State
{
public:
	GameState(StateStack* stack);
	virtual ~GameState();

private:
	ScrollingGround* bg1 = new ScrollingGround;
	ScrollingGround* bg2 = new ScrollingGround;
	AirCraft* Enemy = new AirCraft;
	AirCraft* PlayerAirCraft = new AirCraft;
	Player* player = new Player;
	CommandQueue	m_CommandQueue;

public:
	virtual void ProcessInput();
	virtual void Init(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList);
	virtual bool Update(float DeltaTime, struct FrameResource* Frame);
	virtual void Draw(ID3D12GraphicsCommandList* CmdList, ID3D12DescriptorHeap* Desc,
		UINT DescSize, struct FrameResource* Frame,
		float DeltaTime);

private:
	virtual void LoadTextures();
	virtual void CreateMaterial();
};

