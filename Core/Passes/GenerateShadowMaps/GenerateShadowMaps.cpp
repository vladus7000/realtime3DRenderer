#include "GenerateShadowMaps.hpp"
#include "Renderer.hpp"
#include "World.hpp"
#include "GBuffer.hpp"
#include "Resources.hpp"

#include <limits>
#include <algorithm>

glm::mat4 proj[3];

GenerateShadowMaps::~GenerateShadowMaps()
{
}

void GenerateShadowMaps::setup(Renderer& renderer, Resources& resources)
{
	const int depthSize = 1024;
	if (!m_depthTextureC1.m_DSV)
	{
		m_depthTextureC1 = resources.createDepthStencilTexture(4096, 4096);
		resources.registerResource(Resources::ResoucesID::ShadowMapC1, &m_depthTextureC1);

		m_depthTextureC2 = resources.createDepthStencilTexture(1024, 1024);
		resources.registerResource(Resources::ResoucesID::ShadowMapC2, &m_depthTextureC2);

		m_depthTextureC3 = resources.createDepthStencilTexture(512, 512);
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

	float cascadeEnd[] = {0.1f, 50.0f, 200.0f, 1000.0f };

	for (auto& l : lights)
	{
		if (l.m_type == Light::Type::Directional)
		{
			auto mainCam = renderer.getWorld().getCamera();
			auto mainCamInv = glm::inverse(mainCam.getView());
			auto lightCamera = l.m_camera.getView();

			float ar = 800.0f / 600.0f;
			float tanHalfHFOV = tanf(glm::radians(60.0f / 2.0f));
			float tanHalfVFOV = tanf(glm::radians((60.0f* ar) / 2.0f));
			
			for (int i = 0; i < 3; i++) {
				float xn = cascadeEnd[i] * tanHalfHFOV;
				float xf = cascadeEnd[i + 1] * tanHalfHFOV;
				float yn = cascadeEnd[i] * tanHalfVFOV;
				float yf = cascadeEnd[i + 1] * tanHalfVFOV;

				glm::vec4 frustumCorners[] = {
					// near face
					glm::vec4(xn, yn, cascadeEnd[i], 1.0),
					glm::vec4(-xn, yn, cascadeEnd[i], 1.0),
					glm::vec4(xn, -yn, cascadeEnd[i], 1.0),
					glm::vec4(-xn, -yn, cascadeEnd[i], 1.0),

					// far face
					glm::vec4(xf, yf, cascadeEnd[i + 1], 1.0),
					glm::vec4(-xf, yf, cascadeEnd[i + 1], 1.0),
					glm::vec4(xf, -yf, cascadeEnd[i + 1], 1.0),
					glm::vec4(-xf, -yf, cascadeEnd[i + 1], 1.0)
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

					proj[i] = glm::orthoLH_ZO(minX-50, maxX+50, minY-50, maxY+50, minZ-50, maxZ+50);
				}

			}
			Camera cam;
			cam.getView() = lightCamera;
			cam.getProjection() = proj[0];
			renderer.depthPrepass(cam, m_depthTextureC1, 0.0f, 0.0f, m_depthTextureC1.m_w, m_depthTextureC1.m_h);

			cam.getProjection() = proj[1];
			renderer.depthPrepass(cam, m_depthTextureC2, 0.0f, 0.0f, m_depthTextureC2.m_w, m_depthTextureC2.m_h);

			cam.getProjection() = proj[2];
			renderer.depthPrepass(cam, m_depthTextureC3, 0.0f, 0.0f, m_depthTextureC3.m_w, m_depthTextureC3.m_h);
			break; // 1 dir light supported
		}
	}
}

