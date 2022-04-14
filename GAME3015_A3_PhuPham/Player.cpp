#include "Player.h"

/// <summary>
/// On construction, create the key bindings as well.
/// </summary>
Player::Player()
{
	m_Category = Category::Scene;

	horizontalBounds = 28.f;
	verticalBoundFront = -20.f;
	verticalBoundBack = -30.f;

	m_Key[Action::MoveFront] = 'W';
	m_Key[Action::MoveBack] = 'S';
	m_Key[Action::MoveLeft] = 'A';
	m_Key[Action::MoveRight] = 'D';
}

Player::~Player()
{
}

void Player::Update(float DeltaTime, FrameResource* Frame)
{
	Entity::Update(DeltaTime, Frame);
}

void Player::Draw(ID3D12GraphicsCommandList* CmdList, ID3D12DescriptorHeap* Desc,
	UINT DescSize, struct FrameResource* Frame,
	MeshGeometry* Mesh, float DeltaTime)
{
	Entity::Draw(CmdList, Desc, DescSize, Frame, Mesh, DeltaTime);
}

void Player::HandleEvent(CommandQueue& Queue)
{
}

/// <summary>
/// Moves the player aircraft based on key inputs from player.
/// </summary>
/// <param name="Queue"></param>
void Player::HandleRealtimeInput(CommandQueue& Queue)
{
	if (GetAsyncKeyState(m_Key[Action::MoveFront]) & 0x8000)
	{
			Command	cmd;

			cmd.category = Category::Player;
			cmd.action = AircraftMover(0.f, 0.f, m_MoveSpeed);

			Queue.push(cmd);
	}

	if (GetAsyncKeyState(m_Key[Action::MoveBack]) & 0x8000)
	{
		if (m_Pos.z > verticalBoundBack)
		{
			Command	cmd;

			cmd.category = Category::Player;
			cmd.action = AircraftMover(0.f, 0.f, -m_MoveSpeed);

			Queue.push(cmd);
		}
	}

	if (GetAsyncKeyState(m_Key[Action::MoveRight]) & 0x8000)
	{
		if (m_Pos.x < horizontalBounds)
		{
			Command	cmd;

			cmd.category = Category::Player;
			cmd.action = AircraftMover(m_MoveSpeed, 0.f, 0.f);

			Queue.push(cmd);
		}
	}

	if (GetAsyncKeyState(m_Key[Action::MoveLeft]) & 0x8000)
	{
		if (m_Pos.x > -horizontalBounds)
		{
			Command	cmd;

			cmd.category = Category::Player;
			cmd.action = AircraftMover(-m_MoveSpeed, 0.f, 0.f);

			Queue.push(cmd);
		}
	}
}
