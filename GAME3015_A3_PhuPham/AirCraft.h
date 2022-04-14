#pragma once

#include "Entity.h"

/// <summary>
/// A plane that will act as an enemy character
/// </summary>
class AirCraft	:
	public Entity
{
public:
	AirCraft();
	~AirCraft();

private:
	float	m_Dir;
	XMFLOAT3 startingPos;
	float offsetLimit;

public:
	virtual void Update(float DeltaTime, struct FrameResource* Frame);
	virtual void Draw(ID3D12GraphicsCommandList* CmdList, ID3D12DescriptorHeap* Desc,
		UINT DescSize, struct FrameResource* Frame,
		MeshGeometry* Mesh, float DeltaTime);

	void SetStartPos(const XMFLOAT3& Pos)
	{
		m_Pos = Pos;
		startingPos = Pos;
	}
};

