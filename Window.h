
#pragma once

#define WIN32_LEAN_AND_MEAN // 안쓰는 window header 파일들을 제외시켜	컴파일 속도를 높인다.
#include <Windows.h>

// windows runtime library. needed or microsoft:wrl::ComPtr<> template
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_5.h>
#include <string>
#include <memory>

#include <Events.h>
#include <HighResolutionClock.h>
class Game;

class Window
{
public:
	static const UINT BufferCount = 3;

	// 윈도우 객체의 핸들을 가져온다. 없으면 nullptr를 반환한다.
	HWND GetWindowHandle() const;

	void Destroy();

	const std::wstring& GetWindowName() const;

	int GetClientWidth() const;
	int GetClientHeight() const;

	bool IsVSync() const;
	void SetVSync(bool vSync);
	void ToggleVSync();

	bool IsFullScreen() const;
	void SetFullScreen(bool fullScreen);
	void ToggleFullScreen();

	// Show(Hide) this window
	void Show();
	void Hide();

	UINT GetCurrentBackBufferIndex() const;

	// 현재 스왑체인의 백버퍼를 스크린에 표시한다.
	// 이후 현재의 버퍼 인덱스를 반환한다.
	UINT Present();

	// 현재 백버퍼의 렌더타겟 뷰를 반환한다.
	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRenderTargetView() const;

	Microsoft::WRL::ComPtr<ID3D12Resource> GetCurrentBackBuffer() const;

protected:
	friend LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	// application class는 윈도우 객체를 생성할 수 있다.
	friend class Application;
	
	// DirectXTemplate 클래스. 윈도우에 자기 자신을 등록할 수 있어야 한다.
	friend class Game;
	
	Window() = delete;
	Window(HWND hWnd, const std::wstring& windowName, int cleintWidth, int clientHeight, bool vSync);
	virtual ~Window();

	// Game을 윈도우에 등록한다. 윈도우가 게임 클래스의 콜백 함수를 호출할 수 있게 한다.
	void RegisterCallBacks(std::shared_ptr<Game> pGame);

	virtual void OnUpdate(UpdateEventArgs& e);
	virtual void OnRender(RenderEventArgs& e);

	virtual void OnKeyPressed(KeyEventArgs& e);
	virtual void OnKeyReleased(KeyEventArgs& e);

	virtual void OnMouseMoved(MouseMotionEventArgs& e);
	virtual void OnMouseButtonPressed(MouseButtonEventArgs& e);
	virtual void OnMouseButtonReleased(MouseButtonEventArgs& e);
	virtual void OnMouseWheel(MouseWheelEventArgs& e);

	virtual void OnResize(ResizeEventArgs& e);

	Microsoft::WRL::ComPtr<IDXGISwapChain4> CreateSwapChain();
	
	void UpdateRenderTargetViews();

private:
	Window(const Window& copy) = delete;
	Window& operator=(const Window& other) = delete;

	HWND m_hWnd;

	std::wstring m_WindowName;

	int m_ClientWidth;
	int m_ClientHeight;
	bool m_VSync;
	bool m_Fullscreen;

	HighResolutionClock m_UpdateClock;
	HighResolutionClock m_RenderClock;
	uint64_t m_FrameCounter;

	std::weak_ptr<Game> m_pGame;

	Microsoft::WRL::ComPtr<IDXGISwapChain4> m_dxgiSwapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_d3d12RTVDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_d3d12BackBuffers[BufferCount];

	UINT m_RTVDescriptorSize;
	UINT m_CurrentBackBufferIndex;

	RECT m_WindowRect;
	bool m_IsTearingSupported;
};
