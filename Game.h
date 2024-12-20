/*
* DirectX 12 데모를 위한 추상 베이스 클래스
*/
#pragma once

#include <Events.h>

#include <memory> // std::enable_shared_from_this
#include <string> // std::wstring

class Window;

class Game : public std::enable_shared_from_this<Game>
{
public:
	Game(const std::wstring& name, int width, int height, bool vSync);
	virtual ~Game();

	int GetClientWidth() const
	{
		return m_Width;
	}

	int GetClientHeight() const
	{
		return m_Height;
	}

	virtual bool Initialize();

	virtual bool LoadContent() = 0;

	virtual void UnloadContent() = 0;

	virtual void Destroy();

protected:
	friend class Window;

	virtual void OnUpdate(UpdateEventArgs& e);

	virtual void OnRender(RenderEventArgs& e);

	virtual void OnKeyPressed(KeyEventArgs& e);

	virtual void OnKeyReleased(KeyEventArgs& e);

	virtual void OnMouseMoved(MouseMotionEventArgs& e);

	virtual void OnMouseButtonPressed(MouseButtonEventArgs& e);

	virtual void OnMouseButtonReleased(MouseButtonEventArgs& e);

	virtual void OnMouseWheel(MouseWheelEventArgs& e);

	virtual void OnResize(ResizeEventArgs& e);

	virtual void OnWindowDestroy();

	std::shared_ptr<Window> m_pWindow;

private:
	std::wstring m_Name;
	int m_Width;
	int m_Height;
	bool m_vSync;
};