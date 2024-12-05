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
	// ��� ������ Ŀ�ǵ帮��Ʈ �� ��������Ͱ� ���� �� �����Ѵ�.
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CreateCommandAllocator();
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CreateCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator);
private:
	// 'in-flight' Ŀ�ǵ� ��������͸� �����ϱ� ���� ����ü
	struct CommandAllocatorEntry
	{
		uint64_t fenceValue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	};

	// gpu queue���� 'in-flight'�� Ŀ�ǵ� ��������͸� ť���ϱ� ���� ����Ѵ�.
	// fence value�� �Ϸ�Ǹ� ��������͸� ������ �� �ִ�.
	using CommandAllocatorQueue = std::queue<CommandAllocatorEntry>;
	// ��������Ϳ� ���������� ���� ������ Ŀ�ǵ� ����Ʈ�� ť���ϱ� ���� ����Ѵ�.
	// Ŀ�ǵ帮��Ʈ�� ��������Ϳ� �޸� Ŀ�ǵ� ť���� execute�� ��� ���� �����ϹǷ� fence value�� ������ �ʿ䰡 ����.
	using CommandListQueue = std::queue<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>>;

	D3D12_COMMAND_LIST_TYPE						 m_CommandListType;
	Microsoft::WRL::ComPtr<ID3D12Device2>		 m_d3d12Device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue>	 m_d3d12CommandQueue;
	Microsoft::WRL::ComPtr<ID3D12Fence>			 m_d3d12Fence;
	HANDLE										 m_FenceEvent;
	uint64_t									 m_FenceValue; // command allocator���� fence value�ִµ� �̰� �� ������?

	CommandAllocatorQueue						 m_CommandAllocatorQueue;
	CommandListQueue 							 m_CommandListQueue;
};