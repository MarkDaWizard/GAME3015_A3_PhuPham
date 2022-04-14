#pragma once

#include "World.h"
#include <functional>
#include <queue>
#include "StateStack.h"

class RenderItem
{
public:
	RenderItem() = default;
	XMFLOAT4X4 World = MathHelper::Identity4x4();
	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
	int NumFramesDirty = 3;
	UINT ObjCBIndex = -1;
	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;
};

enum class RenderLayer : int
{
	Opaque = 0,
	Transparent,
	AlphaTested,
	Count
};

class Game : public D3DApp
{
public:
	Game(HINSTANCE hInstance);
	Game(const Game& rhs) = delete;
	Game& operator=(const Game& rhs) = delete;
	~Game();

	virtual bool Initialize()override;

private:
	virtual void OnResize()override;
	virtual void Update(const GameTimer& gt)override;
	virtual void Draw(const GameTimer& gt)override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

	void ProcessInput();
	void OnKeyboardInput(const GameTimer& gt);
	void UpdateCamera(const GameTimer& gt);
	void UpdateObjectCBs(const GameTimer& gt);
	void UpdateMaterialCBs(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);

	void helpBuildSubMesh(GeometryGenerator::MeshData name, SubmeshGeometry& subName, UINT nameVertexOffset, UINT nameIndexOffset);

	/// <summary>
	/// LoadTextures and LoadText is where we get all the textures from file and give them a key
	/// </summary>
	void LoadTextures();
	void LoadTex(std::string name, std::wstring path);
	void BuildRootSignature();
	void BuildDescriptorHeaps();
	void BuildShadersAndInputLayout();

	/// <summary>
	/// Builds all the entities in the game at initialization
	/// </summary>
	void BuildShapeRenderItems();
	void BuildPSOs();
	void BuildFrameResources();
	void BuildMaterials();

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

	float GetHillsHeight(float x, float z)const;
	XMFLOAT3 GetHillsNormal(float x, float z)const;
	void RegisterStates();
	void CreateMesh();

private:

	std::vector<std::unique_ptr<FrameResource>> m_FrameResources;
	FrameResource* m_CurrFrameResource = nullptr;
	int m_CurrFrameResourceIndex = 0;

	UINT m_CbvSrvDescriptorSize = 0;

	ComPtr<ID3D12RootSignature> m_RootSignature = nullptr;

	ComPtr<ID3D12DescriptorHeap> m_SrvDescriptorHeap = nullptr;
	std::map<std::string, std::unique_ptr<Texture>> m_Textures;
	std::map<std::string, ComPtr<ID3DBlob>> m_Shaders;
	std::map<std::string, ComPtr<ID3D12PipelineState>> m_PSOs;

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputLayout;

	PassConstants m_MainPassCB;

	XMFLOAT3 m_EyePos = { 0.0f, 0.0f, 0.0f };
	XMFLOAT4X4 m_View = MathHelper::Identity4x4();
	XMFLOAT4X4 m_Proj = MathHelper::Identity4x4();

	POINT m_LastMousePos;

	World* world = new World();
	class State* m_CurrentState;
	MeshGeometry* m_Mesh;

	StateStack	m_StateStack;
};