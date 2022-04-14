#pragma once

#include "GameInfo.h"
#include "Category.h"
#include "Command.h"

class SceneNode
{
	friend class World;
	friend class Game;

public:
	SceneNode();
	virtual ~SceneNode();

protected:
	SceneNode* m_Parent;
	std::vector<SceneNode*>	m_ChildList;
	XMFLOAT3		m_Scale;
	XMFLOAT3		m_Rot;
	XMFLOAT3		m_Pos;
	XMFLOAT4X4		m_matWorld;
	int			m_CBIndex;
	int			m_MBIndex;
	int			m_TexIndex;
	bool		m_Render;
	Category	m_Category;

public:
	static int	m_GlobalCBIndex;

public:
	Category GetCategory()	const
	{
		return m_Category;
	}

	void SetCategory(Category Type)
	{
		m_Category = Type;
	}

	int GetTexIndex()	const
	{
		return m_TexIndex;
	}

	void SetTexIndex(int Index)
	{
		m_TexIndex = Index;
	}

	void SetMBIndex(int Index)
	{
		m_MBIndex = Index;
	}

public:
	const XMFLOAT3& GetScale()	const
	{
		return m_Scale;
	}

	const XMFLOAT3& GetRotation()	const
	{
		return m_Rot;
	}

	const XMFLOAT3& GetPos()	const
	{
		return m_Pos;
	}

public:
	void SetScale(const XMFLOAT3& Scale)
	{
		m_Scale = Scale;
	}

	void SetScale(float x, float y, float z)
	{
		m_Scale = XMFLOAT3(x, y, z);
	}

	void SetRot(const XMFLOAT3& Rot)
	{
		m_Rot = Rot;
	}

	void SetRot(float x, float y, float z)
	{
		m_Rot = XMFLOAT3(x, y, z);
	}

	void SetPos(const XMFLOAT3& Pos)
	{
		m_Pos = Pos;
	}

	void SetPos(float x, float y, float z)
	{
		m_Pos = XMFLOAT3(x, y, z);
	}

	void Move(const XMFLOAT3& Move)
	{
		m_Pos.x += Move.x;
		m_Pos.y += Move.y;
		m_Pos.z += Move.z;
	}

	void Move(float x, float y, float z)
	{
		m_Pos.x += x;
		m_Pos.y += y;
		m_Pos.z += z;
	}

	void Move(const XMFLOAT3& Dir, float Speed, float DeltaTime)
	{
		m_Pos.x += Dir.x * Speed * DeltaTime;
		m_Pos.y += Dir.y * Speed * DeltaTime;
		m_Pos.z += Dir.z * Speed * DeltaTime;
	}

	void Move(const XMFLOAT3& Velocity, float DeltaTime)
	{
		m_Pos.x += Velocity.x * DeltaTime;
		m_Pos.y += Velocity.y * DeltaTime;
		m_Pos.z += Velocity.z * DeltaTime;
	}

public:
	void AddChild(SceneNode* Child);

public:
	void OnCommand(const Command& Cmd, float DeltaTime);
	virtual void Update(float DeltaTime, struct FrameResource* Frame);
	virtual void Draw(ID3D12GraphicsCommandList* CmdList, ID3D12DescriptorHeap* Desc,
		UINT DescSize, struct FrameResource* Frame, 
		MeshGeometry* Mesh, float DeltaTime);

private:
	void UpdateChild(float DeltaTime, struct FrameResource* Frame);
};

