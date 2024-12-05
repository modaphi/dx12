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
	// transition a resource (리소스 상태 전환) 
	void TransitionResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		Microsoft::WRL::ComPtr<ID3D12Resource> resource,
		D3D12_RESOURCE_STATES beforeStates, D3D12_RESOURCE_STATES afterState);

	// clear render target view
	void ClearRTV(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor);

	// clear the depth of a depth-stencil view(dsv)
	void ClearDepth(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth = 1.0f);

	// 버퍼 데이터를 보관할 큰 리소스를 생성하거나 CPU에서 GPU로 버퍼 데이터를 복사할 때 사용
	// destination source에 업로드가 끝날 때까지 이 함수에서 반환된 리소스는 파괴되면 안된다.
	void UpdateBufferResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource,
		size_t numElements, size_t elementSize, const void* bufferData,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

	// depth 버퍼를 리사이즈한다.
	void ResizeDepthBuffer(int width, int height);

	uint64_t m_FenceValues[Window::BufferCount] = {};

	// Vertex buffer for the cube
	Microsoft::WRL::ComPtr<ID3D12Resource> m_VertexBuffer;
	// buffer resource를 렌더링 파이프라인에 설명하기 위한 구조체. 버텍스 버퍼를 보관한다.
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_IndexBuffer;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView; // 인덱스 데이터를 보관한다.

	// depth buffer
	Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthBuffer;
	// descriptor heap for depth buffer(depth-stencil view는 DSV descriptor heap에 생성된다.)
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DSVHeap;

	// Root signature : 렌더링 파이프라인 단계들에 전달되는 파라미터들을 설명한다.
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;

	// Pipeline state object : 렌더링 파이프라인의 상태를 설명한다.
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;

	D3D12_VIEWPORT m_Viewport;
	D3D12_RECT m_ScissorRect;

	// viewport and scissor rectangle : rasterizer stage를 initialize하기 위해 사용된다.
	float m_FoV;

	DirectX::XMMATRIX m_ModelMatrix;
	DirectX::XMMATRIX m_ViewMatrix;
	DirectX::XMMATRIX m_ProjectionMatrix;

	bool m_ContentLoaded;
};