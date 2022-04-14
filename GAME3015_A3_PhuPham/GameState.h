#pragma once

#include "State.h"
#include "Image.h"
#include "Player.h"
#include "AirCraft.h"
#include "Entity.h"
#include "Terrain.h"

class StateStack;
class GameState	:
	public State
{
public:
	GameState(StateStack* stack);
	virtual ~GameState();

private:
	//Terrain
	Terrain* bg1 = new Terrain;
	Terrain* bg2 = new Terrain;
	//Enemies
	Aircraft* Enemy = new Aircraft;
	//Player
	Aircraft* PlayerAirCraft = new Aircraft;
	Player* player = new Player;
	//Wingman
	Aircraft* Wingman1 = new Aircraft;
	Aircraft* Wingman2 = new Aircraft;
	//Cmd Queue
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

