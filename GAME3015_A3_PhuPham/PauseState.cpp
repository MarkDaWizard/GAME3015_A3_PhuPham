#include "PauseState.h"
#include "StateStack.h"

PauseState::PauseState(StateStack* stack) :
	State(stack)
{
}

PauseState::~PauseState()
{
}

void PauseState::ProcessInput()
{
	static bool Pause = false;

	if (GetAsyncKeyState('R') & 0x8000)
	{
		Pause = true;
	}

	else if (Pause)
	{
		Pause = false;
		m_Stack->popState();
		m_Stack->pushState(State_ID::Game);
	}

	if (GetAsyncKeyState('Q') & 0x8000)
	{
		m_Stack->popState();
		m_Stack->pushState(State_ID::Menu);
	}
}

void PauseState::Init(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList)
{
	State::Init(Device, CmdList);

	Pause->SetPos(XMFLOAT3(0.f, 0.f, 0.f));
	Pause->SetScale(XMFLOAT3(95.f, 70.f, 1.f));
	Pause->SetRot(XMFLOAT3(45.f * 3.141592f / 180.f, 0.f, 0.f));
	Pause->SetMBIndex(0);
	Pause->SetTexIndex(0);

	m_RootNode->AddChild(Pause);
}

bool PauseState::Update(float DeltaTime, FrameResource* Frame)
{
	State::Update(DeltaTime, Frame);

	return true;
}

void PauseState::Draw(ID3D12GraphicsCommandList* CmdList, ID3D12DescriptorHeap* Desc,
	UINT DescSize, FrameResource* Frame, float DeltaTime)
{
	State::Draw(CmdList, Desc, DescSize, Frame, DeltaTime);
}

void PauseState::LoadTextures()
{
	// MAT1
	LoadTex("Pause", L"Textures/Pause.dds");

	// Create the SRV heap.
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = m_Textures.size() + 1;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(m_Device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_SrvDescriptorHeap)));

	// Fill out the heap with actual descriptors.
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(m_SrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());


	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;


	auto PauseTex = m_Textures["Pause"]->Resource;
	srvDesc.Format = PauseTex->GetDesc().Format;
	m_Device->CreateShaderResourceView(PauseTex.Get(), &srvDesc, hDescriptor);
}

void PauseState::CreateMaterial()
{
	auto PauseMat = std::make_unique<Material>();
	PauseMat->Name = "Pause";
	PauseMat->MatCBIndex = 0;
	PauseMat->DiffuseSrvHeapIndex = 0;
	PauseMat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_Materials["Pause"] = std::move(PauseMat);
}

