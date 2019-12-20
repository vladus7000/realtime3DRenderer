#include "GenerateShadowMaps.hpp"
#include "Renderer.hpp"
#include "World.hpp"
#include "GBuffer.hpp"
#include "Resources.hpp"
#include "SettingsHolder.hpp"
#include "Settings/RenderSettings.hpp"

#include <limits>
#include <algorithm>

GenerateShadowMaps::~GenerateShadowMaps()
{
}

void GenerateShadowMaps::setup(Renderer& renderer, Resources& resources)
{
	const int depthSize = 1024;
	if (!m_depthTextureC1.m_DSV)
	{
		m_depthTextureC1 = resources.createDepthStencilTexture(1024, 1024);
		resources.registerResource(Resources::ResoucesID::ShadowMapC1, &m_depthTextureC1);

		m_depthTextureC2 = resources.createDepthStencilTexture(1024, 1024);
		resources.registerResource(Resources::ResoucesID::ShadowMapC2, &m_depthTextureC2);

		m_depthTextureC3 = resources.createDepthStencilTexture(1024, 1024);
		resources.registerResource(Resources::ResoucesID::ShadowMapC3, &m_depthTextureC3);
	}
}

void GenerateShadowMaps::release(Renderer& renderer, Resources& resources)
{
}

void GenerateShadowMaps::execute(Renderer& renderer)
{
	auto context = renderer.getContext();
	auto& world = renderer.getWorld();

	auto& lights = renderer.getWorld().getLights();

	auto settings = SettingsHolder::getInstance().getSetting<RenderSettings>(Settings::Type::Render);

	for (auto& l : lights)
	{
		if (l.m_type == Light::Type::Directional)
		{
			auto mainCam = renderer.getWorld().getCamera();
			auto mainCamInv = glm::inverse(mainCam.getView());
			auto lightCamera = l.m_camera.getView();

			const float ar = mainCam.getAR();
			const float tanHalfHFOV = tanf(glm::radians(mainCam.getFov() / 2.0f));
			const float tanHalfVFOV = tanf(glm::radians((mainCam.getFov() * ar) / 2.0f));
			
			for (int i = 0; i < settings->cascadesCount; i++)
			{
				settings->cascadesEndClip[i] = (mainCam.getProjection() * glm::vec4(0.0f, 0.0f, settings->cascadesEnd[i+1], 1.0f)).z;
			}

			for (int i = 0; i < settings->cascadesCount; i++) {
				float xn = settings->cascadesEnd[i] * tanHalfHFOV;
				float xf = settings->cascadesEnd[i + 1] * tanHalfHFOV;
				float yn = settings->cascadesEnd[i] * tanHalfVFOV;
				float yf = settings->cascadesEnd[i + 1] * tanHalfVFOV;

				glm::vec4 frustumCorners[] = {
					// near face
					glm::vec4(xn, yn, settings->cascadesEnd[i], 1.0),
					glm::vec4(-xn, yn, settings->cascadesEnd[i], 1.0),
					glm::vec4(xn, -yn, settings->cascadesEnd[i], 1.0),
					glm::vec4(-xn, -yn, settings->cascadesEnd[i], 1.0),

					// far face
					glm::vec4(xf, yf, settings->cascadesEnd[i + 1], 1.0),
					glm::vec4(-xf, yf, settings->cascadesEnd[i + 1], 1.0),
					glm::vec4(xf, -yf, settings->cascadesEnd[i + 1], 1.0),
					glm::vec4(-xf, -yf, settings->cascadesEnd[i + 1], 1.0)
				};

				glm::vec4 frustumCornersL[8];

				float minX = std::numeric_limits<float>::max();
				float maxX = std::numeric_limits<float>::min();
				float minY = std::numeric_limits<float>::max();
				float maxY = std::numeric_limits<float>::min();
				float minZ = std::numeric_limits<float>::max();
				float maxZ = std::numeric_limits<float>::min();

				for (int j = 0; j < 8; j++) {

					// Transform the frustum coordinate from view to world space
					glm::vec4 vW = mainCamInv * frustumCorners[j];

					// Transform the frustum coordinate from world to light space
					frustumCornersL[j] = lightCamera * vW;

					minX = std::min(minX, frustumCornersL[j].x);
					maxX = std::max(maxX, frustumCornersL[j].x);
					minY = std::min(minY, frustumCornersL[j].y);
					maxY = std::max(maxY, frustumCornersL[j].y);
					minZ = std::min(minZ, frustumCornersL[j].z);
					maxZ = std::max(maxZ, frustumCornersL[j].z);

					settings->cascadesProjectionMatrix[i] = glm::orthoLH_ZO(minX-10.0f, maxX + 10.0f, minY - 10.0f, maxY + 10.0f, minZ-300.0f, maxZ + 10.0f);
				}

			}

			Camera cam;
			cam.getView() = lightCamera;
			cam.getProjection() = settings->cascadesProjectionMatrix[0];
			renderer.depthPrepass(cam, m_depthTextureC1, 0.0f, 0.0f, m_depthTextureC1.m_w, m_depthTextureC1.m_h);

			cam.getProjection() = settings->cascadesProjectionMatrix[1];
			renderer.depthPrepass(cam, m_depthTextureC2, 0.0f, 0.0f, m_depthTextureC2.m_w, m_depthTextureC2.m_h);

			cam.getProjection() = settings->cascadesProjectionMatrix[2];
			renderer.depthPrepass(cam, m_depthTextureC3, 0.0f, 0.0f, m_depthTextureC3.m_w, m_depthTextureC3.m_h);
			break; // 1 dir light supported
		}
	}
}

