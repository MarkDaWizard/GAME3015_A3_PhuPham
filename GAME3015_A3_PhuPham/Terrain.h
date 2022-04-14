#pragma once

#include "Entity.h"

/// <summary>
/// The ground that will scroll downwards in the scene
/// </summary>
class Terrain : public Entity
{
public: 
	Terrain();
	~Terrain();

private:
	float scrollZBounds;
	float scrollZRestartPosition;
	float speed;

public:
public:
	virtual void Update(float DeltaTime, struct FrameResource* Frame);
	virtual void Draw(ID3D12GraphicsCommandList* CmdList, ID3D12DescriptorHeap* Desc,
		UINT DescSize, struct FrameResource* Frame,
		MeshGeometry* Mesh, float DeltaTime);
};

