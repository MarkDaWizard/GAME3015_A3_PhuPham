#pragma once
#include "Entity.h"
class Image :
	public Entity
{
public:
	Image();
	~Image();

public:
	virtual void Update(float DeltaTime, struct FrameResource* Frame);
	virtual void Draw(ID3D12GraphicsCommandList* CmdList, ID3D12DescriptorHeap* Desc,
		UINT DescSize, struct FrameResource* Frame,
		MeshGeometry* Mesh, float DeltaTime);
};

