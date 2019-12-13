#include "Core/Camera.hpp"
#include "Core/Window.hpp"
#include "Core/World.hpp"
#include "Core/Renderer.hpp"
#include "Core/Resources.hpp"
#include <algorithm>
#include <random>
#include <vector>

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
	switch (msg) {

	case WM_PAINT:
		break;
	case WM_LBUTTONDOWN:
	{
		mouseButtonPressed = true;
		POINT pt;
		pt.x = 400;
		pt.y = 300;
		ClientToScreen(hwnd, &pt);
		SetCursorPos(pt.x, pt.y);
		ShowCursor(false);
		return 0;
	}
		break;
	case WM_LBUTTONUP:
		mouseButtonPressed = false;
		ShowCursor(true);
		return 0;
		break;
	case WM_MOUSELEAVE:
		mouseButtonPressed = false;
		ShowCursor(true);
		break;
	case WM_MOUSEMOVE:
		if (mouseButtonPressed)
		{
			int X = GET_X_LPARAM(lParam);
			int Y = GET_Y_LPARAM(lParam);

			const float mouseSpeed = 10.0f * dt;
			float dx = float(X - 400) * mouseSpeed;
			float dy = float(300 - Y) * mouseSpeed;

			static float yaw = 0.0f;
			static float pitch = 0.0f;
			yaw += -dx;
			pitch += -dy;

			g_mainCamera.rotate(pitch, yaw);
			POINT pt;
			pt.x = 400;
			pt.y = 300;
			ClientToScreen(hwnd, &pt);
			SetCursorPos(pt.x, pt.y);
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
	Resources resources;
	Renderer mainRenderer(mainWindow, resources);
	mainRenderer.setWorld(&mainWorld);

	glm::vec3 minBB = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	glm::vec3 maxBB = glm::vec3(FLT_MIN, FLT_MIN, FLT_MIN);

	mainWorld.initSun(resources);

	auto newObjects = mainWorld.loadObjects("rungholt/house.obj", "rungholt/", resources);
	while (newObjects != mainWorld.getObjects().end())
	{
		//newObjects->worldMatrix = glm::scale(glm::vec3{ 0.1f, 0.1f, 0.1f });
#undef min
#undef max
		minBB.x = std::min(minBB.x, newObjects->minCoord.x);
		minBB.y = std::min(minBB.y, newObjects->minCoord.y);
		minBB.z = std::min(minBB.z, newObjects->minCoord.z);

		maxBB.x = std::max(maxBB.x, newObjects->maxCoord.x);
		maxBB.y = std::max(maxBB.y, newObjects->maxCoord.y);
		maxBB.z = std::max(maxBB.z, newObjects->maxCoord.z);

		++newObjects;
	}

	auto groundObject = mainWorld.loadObjects("cube.obj", "", resources);
	groundObject->worldMatrix =  glm::scale(glm::vec3{ 800.0f, 0.1f, 800.0f });
	groundObject->name = "ground";

	std::random_device rd;
	std::mt19937 e2(rd());
	std::uniform_real_distribution<> distX(minBB.x, maxBB.x);
	std::uniform_real_distribution<> distY(minBB.y, maxBB.y);
	std::uniform_real_distribution<> distZ(minBB.z, maxBB.z);
	std::uniform_real_distribution<> colorRGB(1.0f, 20.0f);

	std::vector<glm::vec3> targetPositions;
	int lightNumber = 1;
	Light l;
	for (int i = 0; i < lightNumber; i++)
	{
		l.m_position = { distX(e2), distY(e2), distZ(e2) };
		targetPositions.push_back(l.m_position);
		l.m_intensity = { colorRGB(e2), colorRGB(e2), colorRGB(e2) };
		//float in = glm::dot({ 0.2126f, 0.7152f, 0.0722f }, l.m_intensity);
		l.m_type = Light::Type::Point;
		mainWorld.addLight(l);
	}

	g_mainCamera.setProjection(60.0f, (float)mainWindow.getWidth() / (float)mainWindow.getHeight(), 0.1f, 1000.f);
	g_mainCamera.setView({ 120, 60, 4 }, { 0, 0, 0 });

	const float dt = 1.0f / 60.0f;
    const float speed = dt * 50.0f;

	while (!mainWindow.shouldClose())
	{
		float delta = 9.0f * dt;
		auto& lights = mainWorld.getLights();
		for (int i = 0; i < lightNumber; i++)
		{
			if (glm::length(lights[i+1].m_position - targetPositions[i]) < 0.5f)
			{
				targetPositions[i] = { distX(e2), distY(e2), distZ(e2) };
			}
			lights[i+1].m_position += glm::normalize(targetPositions[i] - lights[i+1].m_position) * delta;
		}

		mainWorld.updateSun(dt);
		mainWindow.peekMessages();
        bool wPressed = GetAsyncKeyState(0x57) & (1<<16); // w
        bool aPressed = GetAsyncKeyState(0x41) & (1 << 16); // a
        bool sPressed = GetAsyncKeyState(0x53) & (1 << 16); // s
        bool dPressed = GetAsyncKeyState(0x44) & (1 << 16); // d

        if (wPressed)
        {
            g_mainCamera.moveForward(speed);
        }
        if (sPressed)
        {
            g_mainCamera.moveBackward(speed);
        }

        if (aPressed)
        {
            g_mainCamera.moveLeft(speed);
        }

        if (dPressed)
        {
            g_mainCamera.moveRight(speed);
        }
		g_mainCamera.updateView();
		mainWorld.setCamera(g_mainCamera);

		mainRenderer.beginFrame();
		mainRenderer.drawFrame(dt);
		mainRenderer.endFrame();
	}

	return 0;
}