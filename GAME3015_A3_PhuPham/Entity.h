#pragma once
#include "SceneNode.h"
#include "FrameResource.h"

/// <summary>
/// Function Object that will move an aircraft/player
/// </summary>
struct AircraftMover
{
	AircraftMover(float x, float y, float z) :
		Velocity(x, y, z)
	{
	}

	void operator() (class SceneNode& Node, float DeltaTime);

	XMFLOAT3	Velocity;
};

/// <summary>
/// Game elements in the world.
/// </summary>
class Entity	:
	public SceneNode
{
	friend class World;

public:
	Entity();
	~Entity();
		
protected:
	float	m_MoveSpeed;
	/// <summary>
	/// Vector3 for the velocity of the enitiy
	/// </summary>
	XMFLOAT3 m_velocity;

public:
	virtual void Update(float DeltaTime, struct FrameResource* Frame);
	virtual void Draw(ID3D12GraphicsCommandList* CmdList, ID3D12DescriptorHeap* Desc,
		UINT DescSize, struct FrameResource* Frame,
		MeshGeometry* Mesh, float DeltaTime);

	void SetVelocity(const XMFLOAT3& Velocity)
	{
		m_velocity = Velocity;
	}

	void AddVelocity(const XMFLOAT3& Velocity)
	{
		m_velocity.x += Velocity.x;
		m_velocity.y += Velocity.y;
		m_velocity.z += Velocity.z;
	}

	//void Move()
};

