#include "SceneNode.h"
#include "FrameResource.h"

int SceneNode::m_GlobalCBIndex = 0;

SceneNode::SceneNode()	:
	m_Parent(nullptr),
	m_Render(false),
	m_MBIndex(-1),
	m_CBIndex(-1),
	m_TexIndex(0),
	m_Category(Category::Scene)
{
}

SceneNode::~SceneNode()
{
	auto	iter = m_ChildList.begin();
	auto	iterEnd = m_ChildList.end();

	for (; iter != iterEnd; ++iter)
	{
		delete* iter;
	}
}

void SceneNode::AddChild(SceneNode* Child)
{
	Child->m_Parent = this;

	m_ChildList.push_back(Child);
}

void SceneNode::OnCommand(const Command& Cmd, float DeltaTime)
{
	if (Cmd.category == m_Category)
		Cmd.action(*this, DeltaTime);

	auto	iter = m_ChildList.begin();
	auto	iterEnd = m_ChildList.end();

	for (; iter != iterEnd; ++iter)
	{
		(*iter)->OnCommand(Cmd, DeltaTime);
	}
}

void SceneNode::Update(float DeltaTime, FrameResource* Frame)
{
	if (m_Render)
	{
		XMMATRIX	matScale, matRot, matPos;

		matScale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
		matRot = XMMatrixRotationX(m_Rot.x) * XMMatrixRotationY(m_Rot.y) * XMMatrixRotationZ(m_Rot.z);
		matPos = XMMatrixTranslation(m_Pos.x, m_Pos.y, m_Pos.z);

		XMMATRIX	matWorld = matScale * matRot * matPos;
		XMStoreFloat4x4(&m_matWorld, matWorld);

		m_CBIndex = m_GlobalCBIndex;
		++m_GlobalCBIndex;

		matWorld = XMMatrixTranspose(matWorld);

		auto currObjectCB = Frame->ObjectCB.get();

		ObjectConstants objConstants;
		XMStoreFloat4x4(&objConstants.World, matWorld);

		currObjectCB->CopyData(m_CBIndex, objConstants);
	}

	UpdateChild(DeltaTime, Frame);
}

void SceneNode::Draw(ID3D12GraphicsCommandList* CmdList, ID3D12DescriptorHeap* Desc,
	UINT DescSize, struct FrameResource* Frame,
	MeshGeometry* Mesh, float DeltaTime)
{
	auto	iter = m_ChildList.begin();
	auto	iterEnd = m_ChildList.end();

	for (; iter != iterEnd; ++iter)
	{
		(*iter)->Draw(CmdList, Desc, DescSize, Frame, Mesh, DeltaTime);
	}
}

void SceneNode::UpdateChild(float DeltaTime, FrameResource* Frame)
{
	auto	iter = m_ChildList.begin();
	auto	iterEnd = m_ChildList.end();

	for (; iter != iterEnd; ++iter)
	{
		(*iter)->Update(DeltaTime, Frame);
	}
}
