/**
* Wrapper class for a ID3D12CommandQueue
*/

#pragma once

#include <d3d12.h>  // For ID3D12CommandQueue, ID3D12Device2, and ID3D12Fence
#include <wrl.h> // For Microsoft::WRL::ComPtr

#include <cstdint> // For uint64_t
#include <queue>    // For std::queue

class CommandQueue
{
public:
	CommandQueue(Microsoft::WRL::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);
	virtual ~CommandQueue();

	// Get an available command list from the command queue.
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> GetCommandList();

	uint64_t ExecuteCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);

	uint64_t Signal();
	bool IsFenceComplete(uint64_t fenceValue);
	void WaitForFenceValue(uint64_t fenceValue);
	void Flush();

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetD3D12CommandQueue() const;

protected:
	// 사용 가능한 커맨드리스트 및 얼로케이터가 없을 때 생성한다.
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CreateCommandAllocator();
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CreateCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator);
private:
	// 'in-flight' 커맨드 얼로케이터를 추적하기 위한 구조체
	struct CommandAllocatorEntry
	{
		uint64_t fenceValue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	};

	// gpu queue에서 'in-flight'한 커맨드 얼로케이터를 큐잉하기 위해 사용한다.
	// fence value가 완료되면 얼로케이터를 재사용할 수 있다.
	using CommandAllocatorQueue = std::queue<CommandAllocatorEntry>;
	// 얼로케이터와 마찬가지로 재사용 가능한 커맨드 리스트를 큐잉하기 위해 사용한다.
	// 커맨드리스트는 얼로케이터와 달리 커맨드 큐에서 execute된 즉시 재사용 가능하므로 fence value를 추적할 필요가 없다.
	using CommandListQueue = std::queue<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>>;

	D3D12_COMMAND_LIST_TYPE						 m_CommandListType;
	Microsoft::WRL::ComPtr<ID3D12Device2>		 m_d3d12Device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue>	 m_d3d12CommandQueue;
	Microsoft::WRL::ComPtr<ID3D12Fence>			 m_d3d12Fence;
	HANDLE										 m_FenceEvent;
	uint64_t									 m_FenceValue; // command allocator마다 fence value있는데 이건 왜 있을까?

	CommandAllocatorQueue						 m_CommandAllocatorQueue;
	CommandListQueue 							 m_CommandListQueue;
};