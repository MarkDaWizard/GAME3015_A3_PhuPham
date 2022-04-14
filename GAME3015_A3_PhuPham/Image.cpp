#include "Image.h"

Image::Image()
{
}

Image::~Image()
{

}

void Image::Update(float DeltaTime, FrameResource* Frame)
{
	Entity::Update(DeltaTime, Frame);
}

void Image::Draw(ID3D12GraphicsCommandList* CmdList, ID3D12DescriptorHeap* Desc,
	UINT DescSize, struct FrameResource* Frame,
	MeshGeometry* Mesh, float DeltaTime)
{
	if (m_CBIndex != -1 && m_MBIndex != -1)
	{
		UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
		UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

		auto objectCB = Frame->ObjectCB->Resource();
		auto matCB = Frame->MaterialCB->Resource();

		CmdList->IASetVertexBuffers(0, 1, &Mesh->VertexBufferView());
		CmdList->IASetIndexBuffer(&Mesh->IndexBufferView());
		CmdList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(
			Desc->GetGPUDescriptorHandleForHeapStart()
			);

		tex.Offset(m_TexIndex, DescSize);

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + m_CBIndex * objCBByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + m_MBIndex * matCBByteSize;

		CmdList->SetGraphicsRootDescriptorTable(0, tex);
		CmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
		CmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

		CmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);
	}

	Entity::Draw(CmdList, Desc, DescSize, Frame, Mesh, DeltaTime);
}