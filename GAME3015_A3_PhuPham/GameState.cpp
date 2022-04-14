#include "GameState.h"
#include "StateStack.h"

GameState::GameState(StateStack* stack)	:
	State(stack)
{
}

GameState::~GameState()
{
}

void GameState::ProcessInput()
{
	player->HandleEvent(m_CommandQueue);
	player->HandleRealtimeInput(m_CommandQueue);

	static bool Pause = false;

	if (GetAsyncKeyState('P') & 0x8000)
	{
		Pause = true;
	}

	else if (Pause)
	{
		Pause = false;
		m_Stack->popState();
		m_Stack->pushState(State_ID::Pause);
	}
}

void GameState::Init(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList)
{
	State::Init(Device, CmdList);

	// BACKGROUNDS (2 for scrolling effect)
	bg1->SetPos(XMFLOAT3(0.f, 0.f, 0.f));
	bg1->SetScale(XMFLOAT3(160.f, 145.f, 1.f));
	bg1->SetRot(XMFLOAT3(3.14159f / 2.f, 0.f, 0.f));
	bg1->SetMBIndex(0);
	bg1->SetTexIndex(0);

	m_RootNode->AddChild(bg1);

	bg2->SetPos(XMFLOAT3(0.f, 0.f, 145.f));
	bg2->SetScale(XMFLOAT3(160.f, 145.f, 1.f));
	bg2->SetRot(XMFLOAT3(3.14159f / 2.f, 0.f, 0.f));
	bg2->SetMBIndex(0);
	bg2->SetTexIndex(0);

	m_RootNode->AddChild(bg2);

	// ENEMY Character
	Enemy->SetStartPos(XMFLOAT3(0.f, 10.f, 15.f));
	Enemy->SetScale(XMFLOAT3(10.f, 10.f, 1.f));
	Enemy->SetRot(XMFLOAT3(3.14159f / 2.f, 0.f, 0.f));
	Enemy->SetMBIndex(2);
	Enemy->SetCategory(Category::Enemy);
	Enemy->SetTexIndex(2);

	m_RootNode->AddChild(Enemy);

	// PLAYER Character
	PlayerAirCraft->SetStartPos(XMFLOAT3(0.f, 10.f, -25.f));
	PlayerAirCraft->SetScale(XMFLOAT3(10.f, 10.f, 1.f));
	PlayerAirCraft->SetRot(XMFLOAT3(3.14159f / 2.f, 0.f, 0.f));
	PlayerAirCraft->SetMBIndex(1);
	PlayerAirCraft->SetCategory(Category::Player);
	PlayerAirCraft->SetTexIndex(1);

	m_RootNode->AddChild(PlayerAirCraft);

	// PLAYER
	m_RootNode->AddChild(player);

	//player->AssignKey(Action::MoveFront, VK_UP);
	//player->AssignKey(Action::MoveBack, VK_DOWN);
	//player->AssignKey(Action::MoveLeft, VK_LEFT);
	//player->AssignKey(Action::MoveRight, VK_RIGHT);
}

bool GameState::Update(float DeltaTime, FrameResource* Frame)
{
	while (!m_CommandQueue.isEmpty())
	{
		m_RootNode->OnCommand(m_CommandQueue.pop(), DeltaTime);
	}
	State::Update(DeltaTime, Frame);

	return true;
}

void GameState::Draw(ID3D12GraphicsCommandList* CmdList, ID3D12DescriptorHeap* Desc,
	UINT DescSize, FrameResource* Frame, float DeltaTime)
{
	State::Draw(CmdList, Desc, DescSize, Frame, DeltaTime);
}

void GameState::LoadTextures()
{
	// MAT1
	LoadTex("LandTex", L"Textures/Desert.dds");
	LoadTex("JetTex", L"Textures/Raptor.dds");
	LoadTex("Jet2Tex", L"Textures/Eagle.dds");
	LoadTex("GrassTex", L"Textures/grass.dds");

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

	auto LandTex = m_Textures["LandTex"]->Resource;
	srvDesc.Format = LandTex->GetDesc().Format;
	m_Device->CreateShaderResourceView(LandTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	auto JetTex = m_Textures["JetTex"]->Resource;
	hDescriptor.Offset(1, m_CbvSrvDescriptorSize);
	srvDesc.Format = JetTex->GetDesc().Format;
	m_Device->CreateShaderResourceView(JetTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	auto Jet2Tex = m_Textures["Jet2Tex"]->Resource;
	hDescriptor.Offset(1, m_CbvSrvDescriptorSize);
	srvDesc.Format = Jet2Tex->GetDesc().Format;
	m_Device->CreateShaderResourceView(Jet2Tex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	auto GrassTex = m_Textures["GrassTex"]->Resource;
	hDescriptor.Offset(1, m_CbvSrvDescriptorSize);
	srvDesc.Format = GrassTex->GetDesc().Format;
	m_Device->CreateShaderResourceView(GrassTex.Get(), &srvDesc, hDescriptor);
}

void GameState::CreateMaterial()
{
	auto LandMat = std::make_unique<Material>();
	LandMat->Name = "LandMat";
	LandMat->MatCBIndex = 0;
	LandMat->DiffuseSrvHeapIndex = 0;
	LandMat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_Materials["LandMat"] = std::move(LandMat);

	auto JetMat = std::make_unique<Material>();
	JetMat->Name = "JetMat";
	JetMat->MatCBIndex = 1;
	JetMat->DiffuseSrvHeapIndex = 1;
	JetMat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_Materials["JetMat"] = std::move(JetMat);

	auto Jet2Mat = std::make_unique<Material>();
	Jet2Mat->Name = "Jet2Mat";
	Jet2Mat->MatCBIndex = 2;
	Jet2Mat->DiffuseSrvHeapIndex = 2;
	Jet2Mat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_Materials["Jet2Mat"] = std::move(Jet2Mat);

	auto GrassMat = std::make_unique<Material>();
	GrassMat->Name = "GrassMat";
	GrassMat->MatCBIndex = 3;
	GrassMat->DiffuseSrvHeapIndex = 3;
	GrassMat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_Materials["GrassMat"] = std::move(GrassMat);
}

