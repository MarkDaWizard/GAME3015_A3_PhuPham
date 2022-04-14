#include "Entity.h"

void AircraftMover::operator()(SceneNode& Node, float DeltaTime)
{
	Entity& entity = static_cast<Entity&>(Node);

	entity.AddVelocity(Velocity);
}

Entity::Entity()	:
	m_MoveSpeed(30.f)
{
	m_Render = true;
	m_velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
}

Entity::~Entity()
{
}

void Entity::Update(float DeltaTime, struct FrameResource* Frame)
{
	m_Pos.x += m_velocity.x * DeltaTime;
	m_Pos.y += m_velocity.y * DeltaTime;
	m_Pos.z += m_velocity.z * DeltaTime;

	// Rotation control
	if (m_velocity.x < 0.f)
	{
		m_Rot.z = 10.f;
	}

	else if (m_velocity.x > 0.f)
	{
		m_Rot.z = -10.f;
	}

	else
	{
		m_Rot.z = 0.f;
	}

	m_velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

	SceneNode::Update(DeltaTime, Frame);
}

void Entity::Draw(ID3D12GraphicsCommandList* CmdList, ID3D12DescriptorHeap* Desc,
	UINT DescSize, FrameResource* Frame,
	MeshGeometry* Mesh, float DeltaTime)
{
	SceneNode::Draw(CmdList, Desc, DescSize, Frame, Mesh, DeltaTime);
}
