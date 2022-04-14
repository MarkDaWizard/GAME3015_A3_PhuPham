#include "State.h"

State::State(StateStack* stack)	:
	m_Stack(stack)
{
}

State::~State()
{
	delete	m_RootNode;
}

void State::ProcessInput()
{
}

void State::Init(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList)
{
	m_Device = Device;
	m_CmdList = CmdList;

	m_RootNode = new SceneNode;


	LoadTextures();


	CreateMaterial();
}

bool State::Update(float DeltaTime, FrameResource* Frame)
{
	m_RootNode->Update(DeltaTime, Frame);
	SceneNode::m_GlobalCBIndex = 0;

	return true;
}

void State::Draw(ID3D12GraphicsCommandList* CmdList, ID3D12DescriptorHeap* Desc, UINT DescSize,
	FrameResource* Frame, float DeltaTime)
{
	m_RootNode->Draw(CmdList, m_SrvDescriptorHeap.Get(), DescSize, Frame,
		m_Mesh, DeltaTime);
}

void State::LoadTextures()
{
}

void State::LoadTex(std::string name, std::wstring path)
{
	std::unique_ptr<Texture> Tex = std::make_unique<Texture>();
	Tex->Name = name;
	Tex->Filename = path;
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(m_Device,
		m_CmdList, Tex->Filename.c_str(),
		Tex->Resource, Tex->UploadHeap));
	m_Textures[Tex->Name] = std::move(Tex);
}

void State::CreateMaterial()
{
}
