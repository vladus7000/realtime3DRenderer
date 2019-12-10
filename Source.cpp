#include "Core/Camera.hpp"
#include "Core/Window.hpp"
#include "Core/World.hpp"
#include "Core/Renderer.hpp"
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

			const float mouseSpeed = 10.0f * dt;
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

	glm::vec3 minBB = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	glm::vec3 maxBB = glm::vec3(FLT_MIN, FLT_MIN, FLT_MIN);

	auto newObjects = mainWorld.loadObjects("rungholt/house.obj", "rungholt/", mainRenderer);
	while (newObjects != mainWorld.getObjects().end())
	{
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

	auto it = mainWorld.loadObjects("cube.obj", "", mainRenderer);
	//it->worldMatrix = glm::translate(10.0f * glm::vec3{ 50.0f, 25.0f, 0.0f }) * glm::scale(glm::vec3{ 10.0f, 10.0f, 10.0f });

	mainWorld.setAmbientLight({ 0.13f, 0.1f, 0.05f });

	Light l;
	l.m_direction = { 50.0f, 25.0f, 0.0f };
	l.m_intensity = { 252.0f / 255.0f, 212.0f / 255.0f, 64.0f / 255.0f }; // Sun
	l.m_intensity *= 5.0f;
	l.m_type = Light::Type::Directional;

	mainWorld.addLight(l);

	std::random_device rd;
	std::mt19937 e2(rd());
	std::uniform_real_distribution<> distX(minBB.x, maxBB.x);
	std::uniform_real_distribution<> distY(minBB.y, maxBB.y);
	std::uniform_real_distribution<> distZ(minBB.z, maxBB.z);
	std::uniform_real_distribution<> colorRGB(5.0f, 20.0f);

	std::vector<glm::vec3> targetPositions;
	int lightNumber = 1000;
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

	float angle = 0.0f;
	const float dt = 1.0f / 60.0f;
	while (!mainWindow.shouldClose())
	{
		g_mainCamera.updateView();
		mainWorld.setCamera(g_mainCamera);
		float delta = 9.0f * dt;
		auto& lights = mainWorld.getLights();
		for (int i = 0; i < lightNumber; i++)
		{
			if (glm::length(lights[i].m_position - targetPositions[i]) < 0.5f)
			{
				targetPositions[i] = { distX(e2), distY(e2), distZ(e2) };
			}
			lights[i].m_position += glm::normalize(targetPositions[i] - lights[i].m_position) * delta;
		}
		mainWindow.peekMessages();

		mainRenderer.beginFrame();
		mainRenderer.drawFrame(dt);
		mainRenderer.endFrame();
	}

	mainWorld.deinitializeBuffers();
	mainRenderer.deinitialize();

	return 0;
}