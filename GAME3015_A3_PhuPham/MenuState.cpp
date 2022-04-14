#include "MenuState.h"
#include "StateStack.h"
#include "Game.h"

MenuState::MenuState(StateStack* stack) :
	State(stack),
	SelectIndex(0)
{
}

MenuState::~MenuState()
{
}

void MenuState::ProcessInput()
{
	if (GetAsyncKeyState(VK_UP) & 0x8000)
	{
		SelectIndex = 0;
		Play->SetMBIndex(1);
		Play->SetTexIndex(1);
		Exit->SetMBIndex(4);
		Exit->SetTexIndex(4);
	}


	if (GetAsyncKeyState(VK_DOWN) & 0x8000)
	{
		SelectIndex = 1;
		Play->SetMBIndex(2);
		Play->SetTexIndex(2);
		Exit->SetMBIndex(3);
		Exit->SetTexIndex(3);
	}


	if (GetAsyncKeyState(VK_RETURN) & 0x8000)
	{
		if (SelectIndex == 0)
		{
			m_Stack->popState();
			m_Stack->pushState(State_ID::Game);
		}

		else
		{
			DestroyWindow(m_Game->MainWnd());
		}
	}
}

void MenuState::Init(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList)
{
	State::Init(Device, CmdList);

	Back->SetPos(XMFLOAT3(0.f, 0.f, 0.f));
	Back->SetScale(XMFLOAT3(95.f, 70.f, 1.f));
	Back->SetRot(XMFLOAT3(45.f * 3.141592f / 180.f, 0.f, 0.f));
	Back->SetMBIndex(0);
	Back->SetTexIndex(0);

	m_RootNode->AddChild(Back);

	Play->SetPos(XMFLOAT3(0.f, 10.f, 0.f));
	Play->SetScale(XMFLOAT3(30.f, 10.f, 1.f));
	Play->SetRot(XMFLOAT3(45.f * 3.141592f / 180.f, 0.f, 0.f));
	Play->SetMBIndex(1);
	Play->SetTexIndex(1);

	m_RootNode->AddChild(Play);

	Exit->SetPos(XMFLOAT3(0.f, 10.f, -10.f));
	Exit->SetScale(XMFLOAT3(30.f, 10.f, 1.f));
	Exit->SetRot(XMFLOAT3(45.f * 3.141592f / 180.f, 0.f, 0.f));
	Exit->SetMBIndex(4);
	Exit->SetTexIndex(4);

	m_RootNode->AddChild(Exit);
}

bool MenuState::Update(float DeltaTime, FrameResource* Frame)
{
	State::Update(DeltaTime, Frame);

	return true;
}

void MenuState::Draw(ID3D12GraphicsCommandList* CmdList, ID3D12DescriptorHeap* Desc,
	UINT DescSize, FrameResource* Frame, float DeltaTime)
{
	State::Draw(CmdList, Desc, DescSize, Frame, DeltaTime);
}

void MenuState::LoadTextures()
{
	// MAT1
	LoadTex("Back", L"Textures/TitleScreen.dds");
	LoadTex("PlaySelect", L"Textures/PlaySelect.dds");
	LoadTex("Play", L"Textures/Play.dds");
	LoadTex("ExitSelect", L"Textures/ExitSelect.dds");
	LoadTex("Exit", L"Textures/Exit.dds");

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


	auto BackTex = m_Textures["Back"]->Resource;
	srvDesc.Format = BackTex->GetDesc().Format;
	m_Device->CreateShaderResourceView(BackTex.Get(), &srvDesc, hDescriptor);

	auto PlaySelect = m_Textures["PlaySelect"]->Resource;
	hDescriptor.Offset(1, m_CbvSrvDescriptorSize);
	srvDesc.Format = PlaySelect->GetDesc().Format;
	m_Device->CreateShaderResourceView(PlaySelect.Get(), &srvDesc, hDescriptor);

	auto Play = m_Textures["Play"]->Resource;
	hDescriptor.Offset(1, m_CbvSrvDescriptorSize);
	srvDesc.Format = Play->GetDesc().Format;
	m_Device->CreateShaderResourceView(Play.Get(), &srvDesc, hDescriptor);

	auto ExitSelect = m_Textures["ExitSelect"]->Resource;
	hDescriptor.Offset(1, m_CbvSrvDescriptorSize);
	srvDesc.Format = ExitSelect->GetDesc().Format;
	m_Device->CreateShaderResourceView(ExitSelect.Get(), &srvDesc, hDescriptor);

	auto Exit = m_Textures["Exit"]->Resource;
	hDescriptor.Offset(1, m_CbvSrvDescriptorSize);
	srvDesc.Format = Exit->GetDesc().Format;
	m_Device->CreateShaderResourceView(Exit.Get(), &srvDesc, hDescriptor);
}

void MenuState::CreateMaterial()
{
	auto BackMat = std::make_unique<Material>();
	BackMat->Name = "Back";
	BackMat->MatCBIndex = 0;
	BackMat->DiffuseSrvHeapIndex = 0;
	BackMat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_Materials["Back"] = std::move(BackMat);

	auto PlaySelectMat = std::make_unique<Material>();
	PlaySelectMat->Name = "PlaySelect";
	PlaySelectMat->MatCBIndex = 1;
	PlaySelectMat->DiffuseSrvHeapIndex = 1;
	PlaySelectMat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_Materials["PlaySelect"] = std::move(PlaySelectMat);

	auto PlayMat = std::make_unique<Material>();
	PlayMat->Name = "Play";
	PlayMat->MatCBIndex = 2;
	PlayMat->DiffuseSrvHeapIndex = 2;
	PlayMat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_Materials["Play"] = std::move(PlayMat);

	auto ExitSelectMat = std::make_unique<Material>();
	ExitSelectMat->Name = "ExitSelect";
	ExitSelectMat->MatCBIndex = 3;
	ExitSelectMat->DiffuseSrvHeapIndex = 3;
	ExitSelectMat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_Materials["ExitSelect"] = std::move(ExitSelectMat);

	auto ExitMat = std::make_unique<Material>();
	ExitMat->Name = "Exit";
	ExitMat->MatCBIndex = 4;
	ExitMat->DiffuseSrvHeapIndex = 4;
	ExitMat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_Materials["Exit"] = std::move(ExitMat);
}
