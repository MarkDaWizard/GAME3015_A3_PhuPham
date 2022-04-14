#pragma once

#include "Entity.h"
#include "CommandQueue.h"

/// <summary>
/// Player character that will be controllable by the user. Currently, inputs are in progress
/// </summary>
class Player	:
	public Entity
{
public:
	Player();
	~Player();

private:
	unsigned char	m_Key[Action::Max];
	float horizontalBounds;
	float verticalBoundFront;
	float verticalBoundBack;

public:
	void AssignKey(Action action, unsigned char Key)
	{
		m_Key[action] = Key;
	}

public:
	/// <summary>
	/// Function to be called every frame to update the player
	/// </summary>
	/// <param name="DeltaTime"></param>
	/// <param name="Frame"></param>
	virtual void Update(float DeltaTime, struct FrameResource* Frame);
	/// <summary>
	/// Function called after update to render the player into the game
	/// </summary>
	/// <param name="CmdList"></param>
	/// <param name="DeltaTime"></param>
	virtual void Draw(ID3D12GraphicsCommandList* CmdList, ID3D12DescriptorHeap* Desc,
		UINT DescSize, struct FrameResource* Frame,
		MeshGeometry* Mesh, float DeltaTime);

public:
	void HandleEvent(CommandQueue& Queue);
	void HandleRealtimeInput(CommandQueue& Queue);
};

