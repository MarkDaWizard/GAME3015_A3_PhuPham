#include "TitleState.h"
#include "StateStack.h"

TitleState::TitleState(StateStack* stack)	:
	State(stack)
{
}

TitleState::~TitleState()
{
}

void TitleState::ProcessInput()
{
	if (GetAsyncKeyState('E') & 0x8000)
	{
		m_Stack->popState();
		m_Stack->pushState(State_ID::Menu);

		//m_Stack->FindState(State_ID::Menu);
	}
}

void TitleState::Init(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList)
{
	State::Init(Device, CmdList);

	Back->SetPos(XMFLOAT3(0.f, 0.f, 0.f));
	Back->SetScale(XMFLOAT3(95.f, 70.f, 1.f));
	Back->SetRot(XMFLOAT3(45.f * 3.141592f / 180.f, 0.f, 0.f));
	Back->SetMBIndex(0);
	Back->SetTexIndex(0);

	m_RootNode->AddChild(Back);

	/*Text->SetPos(XMFLOAT3(0.f, 0.f, 0.f));
	Text->SetScale(XMFLOAT3(95.f, 20.f, 1.f));
	Text->SetRot(XMFLOAT3(45.f * 3.141592f / 180.f, 0.f, 0.f));
	Text->SetMBIndex(1);
	Text->SetTexIndex(1);

	m_RootNode->AddChild(Text);*/
}

bool TitleState::Update(float DeltaTime, FrameResource* Frame)
{
	State::Update(DeltaTime, Frame);

	return true;
}

void TitleState::Draw(ID3D12GraphicsCommandList* CmdList, ID3D12DescriptorHeap* Desc, 
	UINT DescSize, FrameResource* Frame, float DeltaTime)
{
	State::Draw(CmdList, Desc, DescSize, Frame, DeltaTime);
}

void TitleState::LoadTextures()
{
	// MAT1
	LoadTex("TitleBack", L"Textures/TitleScreen1.dds");
	//LoadTex("TitleText", L"Textures/PlayText.dds");

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


	auto BackTex = m_Textures["TitleBack"]->Resource;
	srvDesc.Format = BackTex->GetDesc().Format;
	m_Device->CreateShaderResourceView(BackTex.Get(), &srvDesc, hDescriptor);

	/*auto TextTex = m_Textures["TitleText"]->Resource;
	hDescriptor.Offset(1, m_CbvSrvDescriptorSize);
	srvDesc.Format = TextTex->GetDesc().Format;
	m_Device->CreateShaderResourceView(TextTex.Get(), &srvDesc, hDescriptor);*/
}

void TitleState::CreateMaterial()
{
	auto BackMat = std::make_unique<Material>();
	BackMat->Name = "TitleBack";
	BackMat->MatCBIndex = 0;
	BackMat->DiffuseSrvHeapIndex = 0;
	BackMat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_Materials["TitleBack"] = std::move(BackMat);

	/*auto TexMat = std::make_unique<Material>();
	TexMat->Name = "TitleText";
	TexMat->MatCBIndex = 1;
	TexMat->DiffuseSrvHeapIndex = 1;
	TexMat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_Materials["TitleText"] = std::move(TexMat);*/
}
