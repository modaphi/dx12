#include <Tutorial2.h>

#include <Application.h>
#include <CommandQueue.h>
#include <Helpers.h>
#include <Window.h>

#include <wrl.h>
using namespace Microsoft::WRL;

#include <d3dx12.h>
#include <d3dcompiler.h>

#include <algorithm> // For std::min and std::max.
#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

using namespace DirectX;

// vertex shader�� ������ ������ ����ü
struct VertexPosColor
{
	XMFLOAT3 Position;
	XMFLOAT3 Color;
};

// 3d ť�� �޽��� ���� ������
static VertexPosColor g_Vertices[8] = {
    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) }, // 0
    { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) }, // 1
    { XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) }, // 2
    { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) }, // 3
    { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) }, // 4
    { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(0.0f, 1.0f, 1.0f) }, // 5
    { XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) }, // 6
    { XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT3(1.0f, 0.0f, 1.0f) }  // 7
};

// ť�� ������ ���� ���Ǵ� triangle�� �����ϴ� �ε��� ���� ������
static WORD g_Indicies[36] =
{
    0, 1, 2, 0, 2, 3,
    4, 6, 5, 4, 7, 6,
    4, 5, 1, 4, 1, 0,
    3, 2, 6, 3, 6, 7,
    1, 5, 6, 1, 6, 2,
    4, 0, 3, 4, 3, 7
};

Tutorial2::Tutorial2(const std::wstring& name, int width, int height, bool vSync)
    : super(name, width, height, vSync)
    , m_ScissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX)) // scissor rectangle�� ȭ�� ��ü�� ������ ����
    , m_Viewport(CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)))
    , m_FoV(45.0)
    , m_ContentLoaded(false)
{
}

void Tutorial2::UnloadContent()
{
    m_ContentLoaded = false;
}

// Clamp a value between a min and max range.
template<typename T>
constexpr const T& clamp(const T& val, const T& min, const T& max)
{
	return val < min ? min : val > max ? max : val;
}

void Tutorial2::UpdateBufferResource(
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
    ID3D12Resource** pDestinationResource,
    ID3D12Resource** pIntermediateResource,
    size_t numElements, size_t elementSize, const void* bufferData,
    D3D12_RESOURCE_FLAGS flags)
{
    auto device = Application::Get().GetDevice();
    size_t bufferSize = numElements * elementSize;

    // Create a committed resource for the GPU resource in a default heap.
    // ���ҽ��� ������ ��ŭ ū GPU ���ҽ��� implicit heap�� �����Ѵ�.
    // ������ ���ҽ��� implicit heap�� �����ȴ�.

    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags),
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(pDestinationResource)));

    // CPU ���۵����Ϳ��� GPU �޸𸮷� ���� �����͸� �����ϴ� �� ���� �߰� ���ҽ��� �����Ѵ�.
    if (bufferData)
    {
        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ, // UPLOAD ���ҽ��� GENERIC_READ ���·� �����ž� �Ѵ�.
            nullptr,
			IID_PPV_ARGS(pIntermediateResource)));
    }

	// with the destination resource created, the CPU buffer data can be transferred to the GPU resources
	D3D12_SUBRESOURCE_DATA subresourceData = {}; // gpu resource�� ���ε� �� �����͸� �����ϴ� ����ü
	subresourceData.pData = bufferData;
	subresourceData.RowPitch = bufferSize;
	subresourceData.SlicePitch = subresourceData.RowPitch;

    UpdateSubresources(commandList.Get(), *pDestinationResource, *pIntermediateResource,
        0, 0, 1, &subresourceData);
}

/// <summary>
/// - vertex buffer������ ���ε�
/// - index buffer������ ���ε�
/// - dsv ���� descriptor heap ����
/// - vertex shader & pixel shader �ε�
/// - root signature ����
/// - PSO ����
/// - depath buffer ����
/// </summary>
/// <returns></returns>
/// 
bool Tutorial2::LoadContent()
{
	auto device = Application::Get().GetDevice();
	auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
    auto commandList = commandQueue->GetCommandList();

    // upload vertex buffer
	ComPtr<ID3D12Resource> intermediateVertexBuffer;
    UpdateBufferResource(commandList.Get(),
        &m_VertexBuffer,
        &intermediateVertexBuffer,
        _countof(g_Vertices), sizeof(VertexPosColor), g_Vertices);

    // create the vertex buffer view
	m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
    m_VertexBufferView.SizeInBytes = sizeof(g_Vertices);
	m_VertexBufferView.StrideInBytes = sizeof(VertexPosColor);

	// upload index buffer
    ComPtr<ID3D12Resource> intermediateIndexBuffer;
    UpdateBufferResource(commandList.Get(),
        &m_IndexBuffer, &intermediateIndexBuffer,
        _countof(g_Indicies), sizeof(WORD), g_Indicies);

	m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
	m_IndexBufferView.SizeInBytes = sizeof(g_Indicies);
	m_IndexBufferView.Format = DXGI_FORMAT_R16_UINT;

	// create the descriptor heap for the depth-stencil view
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DSVHeap)));

    // load the vertex shader
	ComPtr<ID3DBlob> vertexShaderBlob;
	ThrowIfFailed(D3DReadFileToBlob(L"VertexShader.cso", &vertexShaderBlob));

    // load the pixel shader
	ComPtr<ID3DBlob> pixelShaderBlob;
	ThrowIfFailed(D3DReadFileToBlob(L"PixelShader.cso", &pixelShaderBlob));

    // binary form�� compiled shader objects���� PSO(Pipeline State Object) ���� �ÿ� ���ȴ�.
	// PSO�� �����Ǳ� ���� vertex shader�� input layout�� ���ǵǾ�� �Ѵ�.
	// Input Layout�� Input Assembler �ܰ迡 �ٿ��Ǵ� ���ؽ� ������ ������ ������ �����Ѵ�.
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
    };

	// create the root signature
	// : programmable stage�� ���޵Ǵ� �Ķ���͵��� �����Ѵ�. PSO�� �����Ǳ� ���� root signature�� ���ǵǾ�� �Ѵ�.
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
	if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	// Allow input layout and deny unnecessary acess to certain pipeline stages.
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	CD3DX12_ROOT_PARAMETER1 rootParameters[1];
	rootParameters[0].InitAsConstants(sizeof(XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

    // ���� root signature�� �����ϴµ� ���� binary object�� root signature description�� �ø���������Ѵ�.
	ComPtr<ID3DBlob> rootSignatureBlob;
    ComPtr<ID3DBlob> errorBlob;
    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription, featureData.HighestVersion,
        &rootSignatureBlob, &errorBlob));

    // create the root signature
    ThrowIfFailed(device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
		rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)));

	// PSO configuration�� �ʿ��� stream token���� �����Ѵ�.
    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
    } pipelineStateStream;

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 1;
    rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	pipelineStateStream.pRootSignature = m_RootSignature.Get();
    pipelineStateStream.InputLayout = { inputLayout, _countof(inputLayout) };
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
    pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
    pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    pipelineStateStream.RTVFormats = rtvFormats;

    D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
    sizeof(PipelineStateStream), &pipelineStateStream
    };
    ThrowIfFailed(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&m_PipelineState)));
    OutputDebugStringA("PSO created\n");

    auto fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);

    m_ContentLoaded = true;

    // Resize/Create the depth buffer (descriptor heap�� �����Ǿ����� depth buffer�� �����ϰ� ������ �� �ִ�.)
    ResizeDepthBuffer(GetClientWidth(), GetClientHeight());

    return true;
}

/// <summary>
/// depth buffer�� ó�� �����ϰų� ȭ�� ũ�Ⱑ ����� �� ������Ѵ�.
/// </summary>
void Tutorial2::ResizeDepthBuffer(int width, int height)
{
    // depth-stencil descriptor heap�� ������ �Ŀ� resize�� �Ǿ�� �Ѵ�.
    if (m_ContentLoaded)
    {
		// Flush any GPU commands that might be referencing the depth buffer.
        Application::Get().Flush();

        width = std::max(1, width);
        height = std::max(1, height);

        auto device = Application::Get().GetDevice();

        D3D12_CLEAR_VALUE optimizedClearValue = {};
		optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        optimizedClearValue.DepthStencil = { 1.0f, 0 };

        // texture�� �����Ѵ�.
        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height,
                1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &optimizedClearValue,
			IID_PPV_ARGS(&m_DepthBuffer)));

		// update the depth-stencil view
        D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
		dsv.Format = DXGI_FORMAT_D32_FLOAT;
        dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsv.Texture2D.MipSlice = 0;
        dsv.Flags = D3D12_DSV_FLAG_NONE;
        
        device->CreateDepthStencilView(m_DepthBuffer.Get(), &dsv, 
			m_DSVHeap->GetCPUDescriptorHandleForHeapStart());
    }
}

void Tutorial2::OnResize(ResizeEventArgs& e)
{
    if (e.Width != GetClientWidth() || e.Height != GetClientHeight())
    {
        super::OnResize(e);

        m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f,
            static_cast<float>(e.Width), static_cast<float>(e.Height));

        ResizeDepthBuffer(e.Width, e.Height);
    }
}

// mvp ��Ʈ������ ������Ʈ �ϱ� ���� ���ȴ�. ��ũ���� �����Ǳ� ���� ȣ��ȴ�.
void Tutorial2::OnUpdate(UpdateEventArgs& e)
{
    static uint64_t frameCount = 0;
    static double totalTime = 0.0;

    super::OnUpdate(e);

    totalTime += e.ElapsedTime;
    frameCount++;

    if (totalTime > 1.0)
    {
        double fps = frameCount / totalTime;

        char buffer[512];
        sprintf_s(buffer, "FPS: %f\n", fps);
        OutputDebugStringA(buffer);

        frameCount = 0;
        totalTime = 0.0;
    }

    // Update the model matrix.
    float angle = static_cast<float>(e.TotalTime * 90.0);
    const XMVECTOR rotationAxis = XMVectorSet(0, 1, 1, 0);
    m_ModelMatrix = XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(angle));

    // Update the view matrix.
    const XMVECTOR eyePosition = XMVectorSet(0, 0, -10, 1);
    const XMVECTOR focusPoint = XMVectorSet(0, 0, 0, 1);
    const XMVECTOR upDirection = XMVectorSet(0, 1, 0, 0);
    m_ViewMatrix = XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);

    // Update the projection matrix.
    float aspectRatio = GetClientWidth() / static_cast<float>(GetClientHeight());
    m_ProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(m_FoV), aspectRatio, 0.1f, 100.0f);
}

// Transition a resource
void Tutorial2::TransitionResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
    Microsoft::WRL::ComPtr<ID3D12Resource> resource,
    D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        resource.Get(),
        beforeState, afterState);

    commandList->ResourceBarrier(1, &barrier);
}

// Clear a render target.
void Tutorial2::ClearRTV(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
    D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor)
{
    commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
}

void Tutorial2::ClearDepth(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
    D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth)
{
    commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
}

void Tutorial2::OnRender(RenderEventArgs& e)
{
    OutputDebugStringA("Starting render\n");

    super::OnRender(e);

	// draw Ŀ�ǵ� ������ ���� D3D12_COMMAND_LIST_TYPE_DIRECT Ÿ���� command queue�� �����´�.
    auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    auto commandList = commandQueue->GetCommandList();

	UINT currentBackBufferIndex = m_pWindow->GetCurrentBackBufferIndex();
	auto backBuffer = m_pWindow->GetCurrentBackBuffer();
	auto rtv = m_pWindow->GetCurrentRenderTargetView();
	auto dsv = m_DSVHeap->GetCPUDescriptorHandleForHeapStart();

    // Clear the render targets.
    {
        TransitionResource(commandList, backBuffer,
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

        FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };

        ClearRTV(commandList, rtv, clearColor);
        ClearDepth(commandList, dsv);
    }

	// pso�� ������ ���������ο� ���ε��Ѵ�. �� �Լ� ȣ�� �� ������ ���� ���α׷��Ӻ� ���̴� �ܰ���� ���ε��ȴ�.
    commandList->SetPipelineState(m_PipelineState.Get());
	// root signature�� pso�� ���������� ��������� command list���� �����ؾ� �Ѵ�.
    commandList->SetGraphicsRootSignature(m_RootSignature.Get());

    // setup input assembler
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // vertex�� index �����͸� ��� �ؼ��� �� �����Ѵ�.
	commandList->IASetVertexBuffers(0, 1, &m_VertexBufferView); // vertex buffer�� IA stage�� ���ε��Ѵ�.
	commandList->IASetIndexBuffer(&m_IndexBufferView); // index buffer�� IA stage�� ���ε��Ѵ�.

	// setup the rasterizer state : ����Ʈ�� ���� ��Ʈ�� RS(Rasterizer State)���� ���õȴ�.
	commandList->RSSetViewports(1, &m_Viewport);
	commandList->RSSetScissorRects(1, &m_ScissorRect);

    // bind the render target : Output Merger(OM) �ܰ迡 ���� Ÿ���� ���ε��ž� �Ѵ�.
	commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

	// update root parameters : root signature�� �ٲ𶧸��� ���������ο� ���ε��ƴ� �Ķ���͵��� ������Ʈ�ؾ� �Ѵ�.
    // ��ο� �� ���� ����� �Ķ���͵��� �ݵ�� ����ε� �ž� �Ѵ�.

	// update the MVP matrix : OnUpdate �Լ����� ������Ʈ �� mvp ��Ʈ������ ������Ʈ�ؼ� vertex shader�� constant buffer�� �����Ѵ�.
    XMMATRIX mvpMatrix = XMMatrixMultiply(m_ModelMatrix, m_ViewMatrix);
    mvpMatrix = XMMatrixMultiply(mvpMatrix, m_ProjectionMatrix);
    commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &mvpMatrix, 0); // matrix�� gpu�� ���ε��Ѵ�.

    // draw!
	commandList->DrawIndexedInstanced(_countof(g_Indicies), 1, 0, 0, 0);

    // present : ������ �̹����� ��ũ���� �׸���.
    {
        TransitionResource(commandList, backBuffer,
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

		m_FenceValues[currentBackBufferIndex] = commandQueue->ExecuteCommandList(commandList);
		currentBackBufferIndex = m_pWindow->Present(); // ���� �������� ���� back buffer �ε����� ��ȯ�Ѵ�.
		commandQueue->WaitForFenceValue(m_FenceValues[currentBackBufferIndex]); // fence value�� ������ ������ cpu �����带 �����.

        // ���� ���� �������� �����ϰ� ������ �� �ִ�.
    }
}

void Tutorial2::OnKeyPressed(KeyEventArgs& e)
{
    super::OnKeyPressed(e);

    switch (e.Key)
    {
    case KeyCode::Escape:
        Application::Get().Quit(0);
        break;
    case KeyCode::Enter:
        if (e.Alt)
        {
    case KeyCode::F11:
        m_pWindow->ToggleFullScreen();
        break;
        }
    case KeyCode::V:
        m_pWindow->ToggleVSync();
        break;
    }
}

void Tutorial2::OnMouseWheel(MouseWheelEventArgs& e)
{
    m_FoV -= e.WheelDelta; // wheel data�� ���콺 ���� normalized�� ȸ�� �����͸� �����Ѵ�.
    m_FoV = clamp(m_FoV, 12.0f, 90.0f);

    char buffer[256];
    sprintf_s(buffer, "FoV: %f\n", m_FoV);
    OutputDebugStringA(buffer);
}