#pragma once

#include <Game.h>
#include <Window.h>

#include <DirectXMath.h>

class Tutorial2 : public Game
{
public:
	using super = Game;

	Tutorial2(const std::wstring& name, int width, int height, bool vSync = false);

	virtual bool LoadContent() override;

	virtual void UnloadContent() override;
protected:
	virtual void OnUpdate(UpdateEventArgs& e) override;
	virtual void OnRender(RenderEventArgs& e) override;
	virtual void OnKeyPressed(KeyEventArgs& e) override;
	virtual void OnMouseWheel(MouseWheelEventArgs& e) override;
	virtual void OnResize(ResizeEventArgs& e) override;

private:
	// helper functions
	// transition a resource (���ҽ� ���� ��ȯ) 
	void TransitionResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		Microsoft::WRL::ComPtr<ID3D12Resource> resource,
		D3D12_RESOURCE_STATES beforeStates, D3D12_RESOURCE_STATES afterState);

	// clear render target view
	void ClearRTV(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor);

	// clear the depth of a depth-stencil view(dsv)
	void ClearDepth(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth = 1.0f);

	// ���� �����͸� ������ ū ���ҽ��� �����ϰų� CPU���� GPU�� ���� �����͸� ������ �� ���
	// destination source�� ���ε尡 ���� ������ �� �Լ����� ��ȯ�� ���ҽ��� �ı��Ǹ� �ȵȴ�.
	void UpdateBufferResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource,
		size_t numElements, size_t elementSize, const void* bufferData,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

	// depth ���۸� ���������Ѵ�.
	void ResizeDepthBuffer(int width, int height);

	uint64_t m_FenceValues[Window::BufferCount] = {};

	// Vertex buffer for the cube
	Microsoft::WRL::ComPtr<ID3D12Resource> m_VertexBuffer;
	// buffer resource�� ������ ���������ο� �����ϱ� ���� ����ü. ���ؽ� ���۸� �����Ѵ�.
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_IndexBuffer;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView; // �ε��� �����͸� �����Ѵ�.

	// depth buffer
	Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthBuffer;
	// descriptor heap for depth buffer(depth-stencil view�� DSV descriptor heap�� �����ȴ�.)
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DSVHeap;

	// Root signature : ������ ���������� �ܰ�鿡 ���޵Ǵ� �Ķ���͵��� �����Ѵ�.
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;

	// Pipeline state object : ������ ������������ ���¸� �����Ѵ�.
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;

	D3D12_VIEWPORT m_Viewport;
	D3D12_RECT m_ScissorRect;

	// viewport and scissor rectangle : rasterizer stage�� initialize�ϱ� ���� ���ȴ�.
	float m_FoV;

	DirectX::XMMATRIX m_ModelMatrix;
	DirectX::XMMATRIX m_ViewMatrix;
	DirectX::XMMATRIX m_ProjectionMatrix;

	bool m_ContentLoaded;
};