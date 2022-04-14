#include "World.h"
#include <iostream>

World::World()
{
}

World::~World()
{
	delete m_Mesh;
	delete	m_RootNode;
}

/// <summary>
/// Set the game world objects
/// </summary>
/// <param name="Device"></param>
/// <param name="CmdList"></param>
void World::buildWorld(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList)
{
	m_RootNode = new SceneNode;

	GeometryGenerator geoGen;

	GeometryGenerator::MeshData Quad = geoGen.CreateQuad(-0.5f, 0.5f, 1.f, 1.f, 0.f);
	UINT gridVertexOffset = 0;
	UINT gridIndexOffset = 0;
	SubmeshGeometry QuadSubmesh;
	QuadSubmesh.IndexCount = (UINT)Quad.Indices32.size();
	QuadSubmesh.StartIndexLocation = gridIndexOffset;
	QuadSubmesh.BaseVertexLocation = gridVertexOffset;

	auto totalVertexCount =
		Quad.Vertices.size();

	std::vector<Vertex> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < Quad.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = Quad.Vertices[i].Position;
		vertices[k].Normal = Quad.Vertices[i].Normal;
		vertices[k].TexC = Quad.Vertices[i].TexC;
	}

	std::vector<std::uint16_t> indices;

	indices.insert(indices.end(), std::begin(Quad.GetIndices16()), std::end(Quad.GetIndices16()));

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	m_Mesh = new MeshGeometry;
	m_Mesh->Name = "Quad";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &m_Mesh->VertexBufferCPU));
	CopyMemory(m_Mesh->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &m_Mesh->IndexBufferCPU));
	CopyMemory(m_Mesh->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	m_Mesh->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(Device,
		CmdList, vertices.data(), vbByteSize, m_Mesh->VertexBufferUploader);

	m_Mesh->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(Device,
		CmdList, indices.data(), ibByteSize, m_Mesh->IndexBufferUploader);


	m_Mesh->VertexByteStride = sizeof(Vertex);
	m_Mesh->VertexBufferByteSize = vbByteSize;
	m_Mesh->IndexFormat = DXGI_FORMAT_R16_UINT;
	m_Mesh->IndexBufferByteSize = ibByteSize;

	m_Mesh->DrawArgs["Quad"] = QuadSubmesh;

	// BACKGROUNDS (2 for scrolling effect)
	bg1->SetPos(XMFLOAT3(0.f, 0.f, 0.f));
	bg1->SetScale(XMFLOAT3(160.f, 145.f, 1.f));
	bg1->SetRot(XMFLOAT3(3.14159f / 2.f, 0.f, 0.f));
	bg1->SetMBIndex(0);
	bg1->m_TexIndex = 0;

	m_RootNode->AddChild(bg1);

	//bg2->SetPos(XMFLOAT3(0.f, 0.f, 145.f));
	//bg2->SetScale(XMFLOAT3(160.f, 145.f, 1.f));
	//bg2->SetRot(XMFLOAT3(3.14159f / 2.f, 0.f, 0.f));
	//bg2->SetMBIndex(0);
	//bg2->m_TexIndex = 0;

	//m_RootNode->AddChild(bg2);

	//// ENEMY Character
	//Enemy->SetStartPos(XMFLOAT3(0.f, 10.f, 15.f));
	//Enemy->SetScale(XMFLOAT3(10.f, 10.f, 1.f));
	//Enemy->SetRot(XMFLOAT3(3.14159f / 2.f, 0.f, 0.f));
	//Enemy->SetMBIndex(2);
	//Enemy->SetCategory(Category::Enemy);
	//Enemy->m_TexIndex = 2;

	//m_RootNode->AddChild(Enemy);

	//// PLAYER Character
	//PlayerAirCraft->SetStartPos(XMFLOAT3(0.f, 10.f, -25.f));
	//PlayerAirCraft->SetScale(XMFLOAT3(10.f, 10.f, 1.f));
	//PlayerAirCraft->SetRot(XMFLOAT3(3.14159f / 2.f, 0.f, 0.f));
	//PlayerAirCraft->SetMBIndex(1);
	//PlayerAirCraft->SetCategory(Category::Player);
	//PlayerAirCraft->m_TexIndex = 1;

	//m_RootNode->AddChild(PlayerAirCraft);

	//// PLAYER
	////m_RootNode->AddChild(player);

	//player->AssignKey(Action::MoveFront, VK_UP);
	//player->AssignKey(Action::MoveBack, VK_DOWN);
	//player->AssignKey(Action::MoveLeft, VK_LEFT);
	//player->AssignKey(Action::MoveRight, VK_RIGHT);
}

void World::ProcessInput()
{
	player->HandleEvent(m_CommandQueue);
	player->HandleRealtimeInput(m_CommandQueue);
}

void World::Update(float DeltaTime, struct FrameResource* Frame)
{
	while (!m_CommandQueue.isEmpty())
	{
		m_RootNode->OnCommand(m_CommandQueue.pop(), DeltaTime);
	}

	m_RootNode->m_GlobalCBIndex = 0;
	m_RootNode->Update(DeltaTime, Frame);
}

void World::Draw(ID3D12GraphicsCommandList* CmdList, ID3D12DescriptorHeap* Desc,
	UINT DescSize, struct FrameResource* Frame, float DeltaTime)
{
	m_RootNode->Draw(CmdList, Desc, DescSize, Frame, m_Mesh, DeltaTime);
}
