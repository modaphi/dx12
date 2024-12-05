/*
* 어플리케이션을 위한 윈도우를 생성하는 데 사용되는 클래스
*/

#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include <memory>
#include <string>

class Window;
class Game;
class CommandQueue;

class Application
{
public:
	// Create the application singleton with the application instance handle.
	static void Create(HINSTANCE hInstance);

	static void Destroy();

	static Application& Get();

	bool IsTearingSupported() const;

	std::shared_ptr<Window> CreateRenderWindow(const std::wstring& windowName, int clientWidth, int clientHeight, bool vSync = true);

	void DestroyWindow(const std::wstring& windowName);

	void DestroyWindow(std::shared_ptr<Window> window);
	std::shared_ptr<Window> GetWindowByName(const std::wstring& windowName);

	int Run(std::shared_ptr<Game> pGame);

	void Quit(int exitCode = 0);

	Microsoft::WRL::ComPtr<ID3D12Device2> GetDevice() const;

	std::shared_ptr<CommandQueue> GetCommandQueue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT) const;

	void Flush();

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type);
	UINT GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const;
	
protected:
	Application(HINSTANCE hInstance);
	~Application();

	Microsoft::WRL::ComPtr<IDXGIAdapter4> GetAdapter(bool bUseWarp);
	Microsoft::WRL::ComPtr<ID3D12Device2> CreateDevice(Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter);
	bool CheckTearingSupport();

private:
	Application(const Application& copy) = delete;
	Application& operator=(const Application& other) = delete;

	// The application instance handle that this application was created with.
	HINSTANCE m_hInstance;

	Microsoft::WRL::ComPtr<IDXGIAdapter4> m_dxgiAdapter;
	Microsoft::WRL::ComPtr<ID3D12Device2> m_d3d12Device;

	std::shared_ptr<CommandQueue> m_DirectCommandQueue;
	std::shared_ptr<CommandQueue> m_ComputeCommandQueue;
	std::shared_ptr<CommandQueue> m_CopyCommandQueue;

	bool m_TearingSupported;
};