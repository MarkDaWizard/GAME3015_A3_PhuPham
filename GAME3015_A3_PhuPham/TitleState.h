#pragma once

#include "State.h"
#include "Image.h"

class StateStack;
class TitleState	:
	public State
{
public:
	TitleState(StateStack* stack);
	virtual ~TitleState();

private:
	Image* Back = new Image;
	//Image* Text = new Image;

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

