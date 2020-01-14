#include "Core/Camera.hpp"
#include "Core/Window.hpp"
#include "Core/World.hpp"
#include "Core/Renderer.hpp"
#include "Core/Resources.hpp"
#include "Core/SettingsHolder.hpp"
#include "Core/Settings/RenderSettings.hpp"
#include "Core/Settings/WorldSettings.hpp"
#include "Core/UI/UI.hpp"
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
	uiInput(hwnd, msg, wParam, lParam);
	//	return 0;

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
	case WM_KEYDOWN:
		if (wParam == 'P')
		{
			auto set = SettingsHolder::getInstance().getSetting<WorldSettings>(Settings::Type::World);
			set->pause = !set->pause;
			return 0L;
		}
		if (wParam == 'C')
		{
			auto set = SettingsHolder::getInstance().getSetting<RenderSettings>(Settings::Type::Render);
			set->useCSforLighting = !set->useCSforLighting;
			return 0L;
		}
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

	auto newObjects = mainWorld.loadObjects("sceneblend.obj", "", resources);
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

		newObjects->worldMatrix = glm::scale(glm::vec3{ 3.0f, 3.0f, 3.0f });
		++newObjects;
	}
	minBB -= 50.0f;
	maxBB += 50.0f;

	//auto groundObject = mainWorld.loadObjects("cube.obj", "", resources);
	//groundObject->worldMatrix = glm::scale(glm::vec3{ 800.0f, 0.1f, 800.0f });
	//groundObject->name = "ground";

	int zombieCount = 5;

	auto pistol = mainWorld.loadObjects("pistol/pistol.obj", "pistol/", resources);

	auto t = pistol;
	while (t != mainWorld.getObjects().end())
	{
		t->name = "pistol";
		t->worldMatrix = glm::translate(glm::vec3{ 0.0f, 5.0f, -40.0f }) * glm::scale(glm::vec3{ 15.0f, 15.0f, 15.0f });
		++t;
	}

	std::random_device rd;
	std::mt19937 e2(rd());
	std::uniform_real_distribution<> zombieR(-400.0f, 400.0f);
	std::uniform_real_distribution<> distX(minBB.x, maxBB.x);
	std::uniform_real_distribution<> distY(minBB.y, maxBB.y);
	std::uniform_real_distribution<> distZ(minBB.z, maxBB.z);
	std::uniform_real_distribution<> colorRGB(15.0f, 30.0f);
	std::uniform_real_distribution<> radiuses(0.1f, 15.0f);

	std::vector<glm::vec3> targetPositions;
	int lightNumber = 50;
	Light l;
	for (int i = 0; i < lightNumber; i++)
	{
		l.m_position = { distX(e2), distY(e2), distZ(e2) };
		targetPositions.push_back(l.m_position);
		l.m_intensity = { colorRGB(e2), colorRGB(e2), colorRGB(e2) };
		//float in = glm::dot({ 0.2126f, 0.7152f, 0.0722f }, l.m_intensity);
		l.m_type = Light::Type::Point;
		l.m_radius = radiuses(e2);
		mainWorld.addLight(l);
	}
	auto zombie = mainWorld.loadObjects("zombie/0.obj", "zombie/", resources); // 5 shapes each
	for (int i = 0; i < zombieCount - 1; i++)
		mainWorld.loadObjects("zombie/0.obj", "zombie/", resources); // 5 shapes each

	{
		auto t_ = zombie;
		float rx, ry;
		for (int i = 0; i < zombieCount; i++)
		{
			rx = zombieR(e2);
			ry = zombieR(e2);
			for (int j = 0; j < 5; j++)
			{
				t_->worldMatrix = glm::translate(glm::vec3{ rx, 0.0f, ry }) * glm::scale(glm::vec3{ 6.0f, 6.0f, 6.0f });
				++t_;
			}
		}
	}
	g_mainCamera.setProjection(60.0f, (float)mainWindow.getWidth() / (float)mainWindow.getHeight(), 0.01f, 1000.f);
	g_mainCamera.setView({ 120, 60, 4 }, { 0, 0, 0 });

	const float dt = 1.0f / 60.0f;
    const float speed = dt * 50.0f;
	float angle = 0.0f;
	uiInit(&resources);
	//struct nk_context ctx;
	//struct nk_font font;
	//nk_init_fixed(&ctx, calloc(1, 10 * 1024*1024), 10 * 1024 * 1024, &font);
	//
	//enum { EASY, HARD };
	//static int op = EASY;
	//static float value = 0.6f;
	//static int i = 20;
	//
	//if (nk_begin(&ctx, "Show", nk_rect(50, 50, 220, 220),
	//	NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE)) {
	//	/* fixed widget pixel width */
	//	nk_layout_row_static(&ctx, 30, 80, 1);
	//	if (nk_button_label(&ctx, "button")) {
	//		/* event handling */
	//	}
	//
	//	/* fixed widget window ratio width */
	//	nk_layout_row_dynamic(&ctx, 30, 2);
	//	if (nk_option_label(&ctx, "easy", op == EASY)) op = EASY;
	//	if (nk_option_label(&ctx, "hard", op == HARD)) op = HARD;
	//
	//	/* custom widget pixel width */
	//	nk_layout_row_begin(&ctx, NK_STATIC, 30, 2);
	//	{
	//		nk_layout_row_push(&ctx, 50);
	//		nk_label(&ctx, "Volume:", NK_TEXT_LEFT);
	//		nk_layout_row_push(&ctx, 110);
	//		nk_slider_float(&ctx, 0, &value, 1.0f, 0.1f);
	//	}
	//	nk_layout_row_end(&ctx);
	//}
	//nk_end(&ctx);

	while (!mainWindow.shouldClose())
	{
		angle += dt*10.0f;
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
		g_mainCamera.getPosition().y = 7.0f;
		g_mainCamera.updateView();
		mainWorld.setCamera(g_mainCamera);

		auto t = pistol;
		for (int i = 0; i < 4; i++)
		{
			t->worldMatrix = glm::inverse(g_mainCamera.getView()) *glm::translate(glm::vec3(0.4f, -0.5f, 1.03f))*glm::rotate(glm::radians(90.0f), glm::vec3{ 0.0f, 1.0f, 0.0f })* glm::scale(glm::vec3{ 0.6f, 0.6f, 0.6f });// glm::translate(glm::vec3{ 0.0f, 5.0f, -40.0f }) *glm::rotate(glm::radians(angle), glm::vec3{ 0.0f, 1.0f, 0.0f }) * glm::scale(glm::vec3{ 15.0f, 15.0f, 15.0f });
			++t;
		}

		{
			auto t_ = zombie;
			float rx, ry;
			for (int i = 0; i < zombieCount; i++)
			{
				rx = zombieR(e2);
				ry = zombieR(e2);
				for (int j = 0; j < 5; j++)
				{
					glm::vec4 pos=  t_->worldMatrix[3];
					
					if (glm::length(glm::vec3(pos.x, pos.y, pos.z) - g_mainCamera.getPosition()) > 0.5f)
					{
						glm::vec3 newPos= glm::normalize(g_mainCamera.getPosition() - glm::vec3(pos.x, pos.y, pos.z)) * delta;
						pos.x += newPos.x;
						pos.z += newPos.z;
					}

					t_->worldMatrix[3] = pos;
					++t_;
				}
			}
		}

		mainRenderer.beginFrame();
		mainRenderer.drawFrame(dt);
		uiDraw();
		mainRenderer.endFrame();
	}
	uiDeinit();
	return 0;
}