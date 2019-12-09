#include "Core/Camera.hpp"
#include "Core/Window.hpp"
#include "Core/World.hpp"
#include "Core/Renderer.hpp"

Camera g_mainCamera;

bool mouseButtonPressed;
int lastX = 0;
int lastY = 0;

#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))

LRESULT UserFunc(HWND hwnd, UINT msg,
	WPARAM wParam, LPARAM lParam)
{
	const float dt = 1.0f / 60.0f;
	const float speed = dt * 50.0f;
	static bool first = false;
	switch (msg) {

	case WM_PAINT:
		break;
	case WM_KEYDOWN:
		if (wParam == 'W')
		{
			g_mainCamera.moveForward(speed);
			return 0L;
		}
		if (wParam == 'S')
		{
			g_mainCamera.moveBackward(speed);
			return 0L;
		}

		if (wParam == 'A')
		{
			g_mainCamera.moveLeft(speed);
			return 0L;
		}

		if (wParam == 'D')
		{
			g_mainCamera.moveRight(speed);
			return 0L;
		}
	break;
	case WM_LBUTTONDOWN:
		mouseButtonPressed = true;

		return 0;
		break;
	case WM_LBUTTONUP:
		mouseButtonPressed = false;
		first = false;
		return 0;
		break;

	case WM_MOUSEMOVE:
		if (mouseButtonPressed)
		{
			int X = GET_X_LPARAM(lParam);
			int Y = GET_Y_LPARAM(lParam);

			if (!first)
			{
				first = true;
				lastX = X;
				lastY = Y;
			}

			const float mouseSpeed = 5.0f * dt;
			float dx = float(X - lastX) * mouseSpeed;
			float dy = float(lastY - Y) * mouseSpeed;

			static float yaw = 0.0f;
			static float pitch = 0.0f;
			yaw += -dx;
			pitch += -dy;

			g_mainCamera.rotate(pitch, yaw);
			lastX = X;
			lastY = Y;

			//POINT pt;
			//pt.x = 400;
			//pt.y = 300;
			//ClientToScreen(hwnd, &pt);
			//SetCursorPos(pt.x, pt.y);
		}
		return 0;
		break;
	}

	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
	Window mainWindow(800, 600, hInstance, UserFunc);
	World mainWorld;
	Renderer mainRenderer(mainWindow, mainWorld);
	mainRenderer.initialize();

	auto newObjects = mainWorld.loadObjects("rungholt/house.obj", "rungholt/", mainRenderer);

	//while (it != mainWorld.getObjects().end())
	//{
	//	it->worldMatrix = glm::scale(glm::vec3{ 0.4f, 3.0f, 1.0f });
	//	++it;
	//}

	mainWorld.setAmbientLight({ 0.13f, 0.1f, 0.05f });

	Light l;
	l.m_direction = { 50.0f, 25.0f, 0.0f };
	l.m_intensity = { 10.0f * 252.0f / 255.0f, 10.0f * 212.0f / 255.0f, 10.0f * 64.0f / 255.0f };
	l.m_type = Light::Type::Directional;

	mainWorld.addLight(l);
	//l.m_intensity = { 0.0f, 1.0f, 0.0f };
	//mainWorld.addLight(l);
	//l.m_intensity = { 0.0f, 1.0f, 1.0f };
	//mainWorld.addLight(l);

	l.m_direction = { -50.0f, 25.0f, 0.0f };
	l.m_intensity = { 0.1f, 400.0f, 0.1f };
	//mainWorld.addLight(l);

	g_mainCamera.setProjection(60.0f, (float)mainWindow.getWidth() / (float)mainWindow.getHeight(), 0.1f, 1000.f);
	g_mainCamera.setView({ 120, 60, 4 }, { 0, 0, 0 });

	float angle = 0.0f;
	const float dt = 1.0f / 60.0f;
	while (!mainWindow.shouldClose())
	{
		g_mainCamera.updateView();
		mainWorld.setCamera(g_mainCamera);
		///float delta = (360.0f / 15.0f) * dt;

		mainWindow.peekMessages();

		mainRenderer.beginFrame();
		mainRenderer.drawFrame(dt);
		mainRenderer.endFrame();
	}

	mainWorld.deinitializeBuffers();
	mainRenderer.deinitialize();

	return 0;
}