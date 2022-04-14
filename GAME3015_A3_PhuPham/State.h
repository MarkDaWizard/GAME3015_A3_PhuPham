#pragma once

#include "GameInfo.h"
#include "SceneNode.h"
#include "CommandQueue.h"

class StateStack;
class State
{
public:
	State(StateStack* stack);
	virtual ~State();

protected:
	StateStack* m_Stack;
	SceneNode* m_RootNode;

public:
	class World* m_World;
	class Game* m_Game;
	std::map<std::string, std::unique_ptr<Material>> m_Materials;
	std::map<std::string, std::unique_ptr<Texture>> m_Textures;
	MeshGeometry* m_Mesh;
	CommandQueue	m_CommandQueue;
	ID3D12Device* m_Device;
	ID3D12GraphicsCommandList* m_CmdList;
	ComPtr<ID3D12DescriptorHeap> m_SrvDescriptorHeap = nullptr;
	int			m_CbvSrvDescriptorSize;

public:
	void SetMesh(MeshGeometry* Mesh)
	{
		m_Mesh = Mesh;
	}

	void SetGame(Game* game)
	{
		m_Game = game;
	}

	void SetCbvSrvSize(int Size)
	{
		m_CbvSrvDescriptorSize = Size;
	}

	ID3D12DescriptorHeap* GetSrvDesc()	const
	{
		return m_SrvDescriptorHeap.Get();
	}

public:
	virtual void ProcessInput();
	virtual void Init(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList);
	virtual bool Update(float DeltaTime, struct FrameResource* Frame);
	virtual void Draw(ID3D12GraphicsCommandList* CmdList, ID3D12DescriptorHeap* Desc,
		UINT DescSize, struct FrameResource* Frame,
		float DeltaTime);

public:
	virtual void LoadTextures();
	void LoadTex(std::string name, std::wstring path);
	virtual void CreateMaterial();
};

