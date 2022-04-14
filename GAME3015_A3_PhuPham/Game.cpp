
#include "Game.h"
#include "TitleState.h"
#include "MenuState.h"
#include "GameState.h"
#include "PauseState.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		Game theApp(hInstance);
		if (!theApp.Initialize())
			return 0;

		return theApp.Run();
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}
}

Game::Game(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
}

Game::~Game()
{
	delete	m_CurrFrameResource;
	if (md3dDevice != nullptr)
		FlushCommandQueue();
}

bool Game::Initialize()
{
	if (!D3DApp::Initialize())
		return false;

	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	// Get the increment size of a descriptor in this heap type.  This is hardware specific, 
	// so we have to query this information.
	m_CbvSrvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//LoadTextures();
	BuildRootSignature();
	//BuildDescriptorHeaps();
	CreateMesh();
	RegisterStates();
	BuildShadersAndInputLayout();
	//BuildMaterials();
	//BuildShapeRenderItems();
	BuildFrameResources();
	BuildPSOs();

	m_CurrFrameResource = new FrameResource(md3dDevice.Get(),
		1, (UINT)100, (UINT)100);

	// Execute the initialization commands.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	FlushCommandQueue();

	m_StateStack.pushState(State_ID::Title);

	m_CurrentState = m_StateStack.FindState(State_ID::Title);

	return true;
}

void Game::OnResize()
{
	D3DApp::OnResize();

	// The window resized, so update the aspect ratio and recompute the projection XMFLOAT4X4.
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_Proj, P);
}

void Game::Update(const GameTimer& gt)
{
	OnKeyboardInput(gt);
	UpdateCamera(gt);
	m_StateStack.update(gt.DeltaTime(), m_CurrFrameResource);
	//m_CurrentState->Update(gt.DeltaTime(), m_CurrFrameResource);
	//world->Update(gt.DeltaTime(), m_CurrFrameResource);

	// Cycle through the circular frame resource array.
	//m_CurrFrameResourceIndex = (m_CurrFrameResourceIndex + 1) % gNumFrameResources;
	//m_CurrFrameResource = m_FrameResources[m_CurrFrameResourceIndex].get();

	// Has the GPU finished processing the commands of the current frame resource?
	// If not, wait until the GPU has completed commands up to this fence point.
	if (m_CurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < m_CurrFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(mFence->SetEventOnCompletion(m_CurrFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	//UpdateObjectCBs(gt);
	UpdateMaterialCBs(gt);
	UpdateMainPassCB(gt);
}

void Game::Draw(const GameTimer& gt)
{
	auto cmdListAlloc = m_CurrFrameResource->CmdListAlloc;

	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(cmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), m_PSOs["opaque"].Get()));

	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Clear the back buffer and depth buffer.
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), (float*)&m_MainPassCB.FogColor, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_StateStack.GetCurrentState()->GetSrvDesc() };
	//ID3D12DescriptorHeap* descriptorHeaps[] = { m_SrvDescriptorHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mCommandList->SetGraphicsRootSignature(m_RootSignature.Get());

	auto passCB = m_CurrFrameResource->PassCB->Resource();
	mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	mCommandList->SetPipelineState(m_PSOs["alphaTested"].Get());


	/*m_CurrentState->Draw(mCommandList.Get(), nullptr, m_CbvSrvDescriptorSize,
		m_CurrFrameResource, gt.DeltaTime());*/
	m_StateStack.draw(mCommandList.Get(), nullptr, m_CbvSrvDescriptorSize,
			m_CurrFrameResource, gt.DeltaTime());
	//world->Draw(mCommandList.Get(), m_CurrentState->GetSrvDesc(), m_CbvSrvDescriptorSize,
	//	m_CurrFrameResource, gt.DeltaTime());



	/*DrawRenderItems(mCommandList.Get(), world->mRitemLayer[(int)RenderLayer::Opaque]);

	mCommandList->SetPipelineState(m_PSOs["alphaTested"].Get());
	DrawRenderItems(mCommandList.Get(), world->mRitemLayer[(int)RenderLayer::AlphaTested]);

	mCommandList->SetPipelineState(m_PSOs["transparent"].Get());
	DrawRenderItems(mCommandList.Get(), world->mRitemLayer[(int)RenderLayer::Transparent]);*/

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// Done recording commands.
	ThrowIfFailed(mCommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// Advance the fence value to mark commands up to this fence point.
	m_CurrFrameResource->Fence = ++mCurrentFence;


	// Add an instruction to the command queue to set a new fence point. 
	// Because we are on the GPU timeline, the new fence point won't be 
	// set until the GPU finishes processing all the commands prior to this Signal().
	mCommandQueue->Signal(mFence.Get(), mCurrentFence);
}

void Game::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_LastMousePos.x = x;
	m_LastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void Game::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void Game::OnMouseMove(WPARAM btnState, int x, int y)
{

}

void Game::ProcessInput()
{
	//world->ProcessInput();
	//m_CurrentState->ProcessInput();
	m_StateStack.handleEvent();
}

void Game::OnKeyboardInput(const GameTimer& gt)
{
	ProcessInput();
}

void Game::UpdateCamera(const GameTimer& gt)
{
	// Build the view XMFLOAT4X4.
	XMVECTOR pos = XMVectorSet(0, 60, -60.f, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR right = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, right);
	XMStoreFloat4x4(&m_View, view);
}

void Game::UpdateObjectCBs(const GameTimer& gt)
{
	//auto currObjectCB = m_CurrFrameResource->ObjectCB.get();
	//for (auto& e : world->mAllRitems)
	//{
	//	// Only update the cbuffer data if the constants have changed.  
	//	// This needs to be tracked per frame resource.
	//	if (e->NumFramesDirty > 0)
	//	{
	//		XMXMFLOAT4X4 world = XMLoadFloat4x4(&e->World);
	//		XMXMFLOAT4X4 texTransform = XMLoadFloat4x4(&e->TexTransform);

	//		ObjectConstants objConstants;
	//		XMStoreFloat4x4(&objConstants.World, XMXMFLOAT4X4Transpose(world));
	//		XMStoreFloat4x4(&objConstants.TexTransform, XMXMFLOAT4X4Transpose(texTransform));

	//		currObjectCB->CopyData(e->ObjCBIndex, objConstants);

	//		// Next FrameResource need to be updated too.
	//		e->NumFramesDirty--;
	//	}
	//}
}

void Game::UpdateMaterialCBs(const GameTimer& gt)
{
	auto currMaterialCB = m_CurrFrameResource->MaterialCB.get();
	for (auto& e : m_StateStack.GetCurrentState()->m_Materials)
	//for (auto& e : world->mMaterials)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		Material* mat = e.second.get();
		if (mat->NumFramesDirty > 0)
		{
			MaterialConstants matConstants;
			matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
			matConstants.FresnelR0 = mat->FresnelR0;
			matConstants.Roughness = mat->Roughness;

			currMaterialCB->CopyData(mat->MatCBIndex, matConstants);

			// Next FrameResource need to be updated too.
			mat->NumFramesDirty--;
		}
	}
}

void Game::UpdateMainPassCB(const GameTimer& gt)
{
	XMMATRIX view = XMLoadFloat4x4(&m_View);
	XMMATRIX proj = XMLoadFloat4x4(&m_Proj);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&m_MainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&m_MainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&m_MainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&m_MainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&m_MainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&m_MainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	m_MainPassCB.EyePosW = m_EyePos;
	m_MainPassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
	m_MainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
	m_MainPassCB.NearZ = 1.0f;
	m_MainPassCB.FarZ = 1000.0f;
	m_MainPassCB.TotalTime = gt.TotalTime();
	m_MainPassCB.DeltaTime = gt.DeltaTime();
	m_MainPassCB.AmbientLight = { 1.1f, 1.1f, 1.1f, 1.0f };

	//m_MainPassCB.Lights[0].Direction = { 0, -1.0f, 0 };
	//m_MainPassCB.Lights[0].Strength = { 0.5f, 0.5f, 0.1f };

	//m_MainPassCB.Lights[1].Position = { 0.0f, 10.0f, 0.0f };
	//m_MainPassCB.Lights[1].Strength = { 5.0f, 1.0f, 1.0f };

	//m_MainPassCB.Lights[2].Position = { 4.0f, 5.0f, 3.0f };
	//m_MainPassCB.Lights[2].Strength = { 2.0f, 0.0f, 2.0f };

	//m_MainPassCB.Lights[3].Position = { -5.0f, 7.0f, 3.0f };
	//m_MainPassCB.Lights[3].Strength = { 0.0f, 2.0f, 4.0f };

	auto currPassCB = m_CurrFrameResource->PassCB.get();
	currPassCB->CopyData(0, m_MainPassCB);
}

void Game::LoadTextures()
{
	// MAT1
	LoadTex("LandTex", L"Textures/Desert.dds");
	LoadTex("JetTex", L"Textures/Raptor.dds");
	LoadTex("Jet2Tex", L"Textures/Eagle.dds");
	LoadTex("GrassTex", L"Textures/grass.dds");
}

void Game::LoadTex(std::string name, std::wstring path)
{
	std::unique_ptr<Texture> Tex = std::make_unique<Texture>();
	Tex->Name = name;
	Tex->Filename = path;
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), Tex->Filename.c_str(),
		Tex->Resource, Tex->UploadHeap));
	m_Textures[Tex->Name] = std::move(Tex);
}

void Game::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[4];

	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsConstantBufferView(0);
	slotRootParameter[2].InitAsConstantBufferView(1);
	slotRootParameter[3].InitAsConstantBufferView(2);

	auto staticSamplers = GetStaticSamplers();

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(m_RootSignature.GetAddressOf())));
}

void Game::BuildDescriptorHeaps()
{
	// Create the SRV heap.
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = m_Textures.size() + 1;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_SrvDescriptorHeap)));

	// Fill out the heap with actual descriptors.
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(m_SrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());


	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	
	auto LandTex = m_Textures["LandTex"]->Resource;
	srvDesc.Format = LandTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(LandTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	auto JetTex = m_Textures["JetTex"]->Resource;
	hDescriptor.Offset(1, m_CbvSrvDescriptorSize);
	srvDesc.Format = JetTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(JetTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	auto Jet2Tex = m_Textures["Jet2Tex"]->Resource;
	hDescriptor.Offset(1, m_CbvSrvDescriptorSize);
	srvDesc.Format = Jet2Tex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(Jet2Tex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	auto GrassTex = m_Textures["GrassTex"]->Resource;
	hDescriptor.Offset(1, m_CbvSrvDescriptorSize);
	srvDesc.Format = GrassTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(GrassTex.Get(), &srvDesc, hDescriptor);

	// MAT2
	//int count = 0;
	//for (auto t = m_Textures.begin(); t != m_Textures.end(); ++t)
	//{
	//	auto Tex = t->second->Resource;
	//	if (count != 0) { hDescriptor.Offset(1, m_CbvSrvDescriptorSize); }
	//	srvDesc.Format = Tex->GetDesc().Format;
	//	md3dDevice->CreateShaderResourceView(Tex.Get(), &srvDesc, hDescriptor);
	//	count++;
	//}

}

void Game::BuildShadersAndInputLayout()
{
	const D3D_SHADER_MACRO defines[] =
	{
		"FOG", "1",
		NULL, NULL
	};

	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"FOG", "1",
		"ALPHA_TEST",
		NULL, NULL
	};

	m_Shaders["standardVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_0");
	m_Shaders["opaquePS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", defines, "PS", "ps_5_0");
	m_Shaders["alphaTestedPS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", alphaTestDefines, "PS", "ps_5_0");


	m_InputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

}


void Game::helpBuildSubMesh(GeometryGenerator::MeshData name, SubmeshGeometry& subName, UINT nameVertexOffset, UINT nameIndexOffset)
{
	subName.IndexCount = (UINT)name.Indices32.size();
	subName.StartIndexLocation = nameIndexOffset;
	subName.BaseVertexLocation = nameVertexOffset;
}

void Game::BuildPSOs()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

	//
	// PSO for opaque objects.
	//
	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { m_InputLayout.data(), (UINT)m_InputLayout.size() };
	opaquePsoDesc.pRootSignature = m_RootSignature.Get();
	opaquePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_Shaders["standardVS"]->GetBufferPointer()),
		m_Shaders["standardVS"]->GetBufferSize()
	};
	opaquePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_Shaders["opaquePS"]->GetBufferPointer()),
		m_Shaders["opaquePS"]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = mBackBufferFormat;
	opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = mDepthStencilFormat;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&m_PSOs["opaque"])));

	//
	// PSO for transparent objects
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentPsoDesc = opaquePsoDesc;

	D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
	transparencyBlendDesc.BlendEnable = true;
	transparencyBlendDesc.LogicOpEnable = false;

	//suppose that we want to blend the sourceand destination pixels based on the opacity of the source pixel :
	//source blend factor : D3D12_BLEND_SRC_ALPHA
	//destination blend factor : D3D12_BLEND_INV_SRC_ALPHA
	//blend operator : D3D12_BLEND_OP_ADD


	transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

	//you could  specify the blend factor or not..
	//F = (r, g, b) and F = a, where the color (r, g, b,a) is supplied to the  parameter of the ID3D12GraphicsCommandList::OMSetBlendFactor method.
	//transparencyBlendDesc.SrcBlend = D3D12_BLEND_BLEND_FACTOR;
	//transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_BLEND_FACTOR;

	//Hooman: try different blend operators to see the blending effect
	//D3D12_BLEND_OP_ADD,
	//D3D12_BLEND_OP_SUBTRACT,
	//D3D12_BLEND_OP_REV_SUBTRACT,
	//D3D12_BLEND_OP_MIN,
	//D3D12_BLEND_OP_MAX

	transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD,

		transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	//transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_BLUE;
	//Direct3D supports rendering to up to eight render targets simultaneously.
	transparentPsoDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&transparentPsoDesc, IID_PPV_ARGS(&m_PSOs["transparent"])));

	//
	// PSO for alpha tested objects
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC alphaTestedPsoDesc = opaquePsoDesc;
	alphaTestedPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_Shaders["alphaTestedPS"]->GetBufferPointer()),
		m_Shaders["alphaTestedPS"]->GetBufferSize()
	};
	alphaTestedPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&alphaTestedPsoDesc, IID_PPV_ARGS(&m_PSOs["alphaTested"])));
}

void Game::BuildFrameResources()
{
	/*for (int i = 0; i < gNumFrameResources; ++i)
	{
		m_FrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(),
			1, (UINT)world->mAllRitems.size(), (UINT)world->mMaterials.size()));
	}*/
}

void Game::BuildMaterials()
{
	// MAT3
	//int count = 0;
	//for (auto t = m_Textures.begin(); t != m_Textures.end(); ++t)
	//{
	//	auto Mat = std::make_unique<Material>();
	//	Mat->Name = t->first;
	//	Mat->MatCBIndex = count;
	//	Mat->DiffuseSrvHeapIndex = count;
	//	Mat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//	mMaterials[t->first] = std::move(Mat);
	//	count++;
	//}

	auto LandMat = std::make_unique<Material>();
	LandMat->Name = "LandMat";
	LandMat->MatCBIndex = 0;
	LandMat->DiffuseSrvHeapIndex = 0;
	LandMat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	world->mMaterials["LandMat"] = std::move(LandMat);

	auto JetMat = std::make_unique<Material>();
	JetMat->Name = "JetMat";
	JetMat->MatCBIndex = 1;
	JetMat->DiffuseSrvHeapIndex = 1;
	JetMat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	world->mMaterials["JetMat"] = std::move(JetMat);

	auto Jet2Mat = std::make_unique<Material>();
	Jet2Mat->Name = "Jet2Mat";
	Jet2Mat->MatCBIndex = 2;
	Jet2Mat->DiffuseSrvHeapIndex = 2;
	Jet2Mat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	world->mMaterials["Jet2Mat"] = std::move(Jet2Mat);

	auto GrassMat = std::make_unique<Material>();
	GrassMat->Name = "GrassMat";
	GrassMat->MatCBIndex = 3;
	GrassMat->DiffuseSrvHeapIndex = 3;
	GrassMat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	world->mMaterials["GrassMat"] = std::move(GrassMat);
}
//
//void Game::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
//{
//	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
//	UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));
//
//	auto objectCB = m_CurrFrameResource->ObjectCB->Resource();
//	auto matCB = m_CurrFrameResource->MaterialCB->Resource();
//
//	// For each render item...
//	for (size_t i = 0; i < ritems.size(); ++i)
//	{
//		auto ri = ritems[i];
//
//		cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
//		cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
//		cmdList->IASetPrimitiveTopology(ri->PrimitiveType);
//
//		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(
//			m_SrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart()
//			);
//
//		tex.Offset(ri->Mat->DiffuseSrvHeapIndex, m_CbvSrvDescriptorSize);
//
//		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
//		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;
//
//		cmdList->SetGraphicsRootDescriptorTable(0, tex);
//		cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
//		cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);
//
//		cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
//	}
//}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> Game::GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp };
}

float Game::GetHillsHeight(float x, float z)const
{
	return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}

XMFLOAT3 Game::GetHillsNormal(float x, float z)const
{
	// n = (-df/dx, 1, -df/dz)
	XMFLOAT3 n(
		-0.03f * z * cosf(0.1f * x) - 0.3f * cosf(0.1f * z),
		1.0f,
		-0.3f * sinf(0.1f * x) + 0.03f * x * sinf(0.1f * z));

	XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
	XMStoreFloat3(&n, unitNormal);

	return n;
}

void Game::RegisterStates()
{
	m_StateStack.registerState<TitleState>(State_ID::Title, md3dDevice.Get(), mCommandList.Get(), m_Mesh, m_CbvSrvDescriptorSize, this);
	m_StateStack.registerState<MenuState>(State_ID::Menu, md3dDevice.Get(), mCommandList.Get(), m_Mesh, m_CbvSrvDescriptorSize, this);
	m_StateStack.registerState<GameState>(State_ID::Game, md3dDevice.Get(), mCommandList.Get(), m_Mesh, m_CbvSrvDescriptorSize, this);
	m_StateStack.registerState<PauseState>(State_ID::Pause, md3dDevice.Get(), mCommandList.Get(), m_Mesh, m_CbvSrvDescriptorSize, this);
}

void Game::CreateMesh()
{
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

	m_Mesh->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, m_Mesh->VertexBufferUploader);

	m_Mesh->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, m_Mesh->IndexBufferUploader);


	m_Mesh->VertexByteStride = sizeof(Vertex);
	m_Mesh->VertexBufferByteSize = vbByteSize;
	m_Mesh->IndexFormat = DXGI_FORMAT_R16_UINT;
	m_Mesh->IndexBufferByteSize = ibByteSize;

	m_Mesh->DrawArgs["Quad"] = QuadSubmesh;
}

void Game::BuildShapeRenderItems()
{
	world->buildWorld(md3dDevice.Get(), mCommandList.Get());

}