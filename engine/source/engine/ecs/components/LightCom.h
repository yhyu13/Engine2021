#pragma once
#include "engine/math/Geommath.h"
#include "engine/ecs/BaseComponent.h"
#include "engine/renderer/Buffer.h"

namespace longmarch
{
	/*
		A universal component manages lights parameters and shadowing parameters
	*/
	struct CACHE_ALIGN LightCom final: BaseComponent<LightCom>
	{
	public:
		enum class LIGHT_TYPE : uint32_t // Light type order is critical
		{
			DIRECTIONAL = 0,
			POINT = 1,
			SPOT = 2,
		};
	public:
		LightCom() = default;
		//! Allocate shadow maps on GPU, avoids re-initialization internally
		void AllocateShadowBuffer();
		//! Release shadow maps on GPU
		void ReleaseShadowBuffer();
		//! Return true if drop off execeed maximum distance (which implies that shadow would rather be turn off)
		bool HandleShadowBufferDropOff(float distance);

		virtual void JsonSerialize(Json::Value& value) const override;
		virtual void JsonDeserialize(const Json::Value& value) override;
		virtual void ImGuiRender() override;
	public:
		struct
		{
			float distance{ 20.0f };
			float innerConeRad{ 45 * DEG2RAD };
			float outterConeRad{ 55 * DEG2RAD };
			float softEdgeRatio{ 0.1f };
			float aspectWByH{ 1.0f }; //<! TODO implement eliptical spot light in gpu shader
		}spotLight;
		struct
		{
			float lambdaCSM{ 0.5f };
			uint32_t numOfCSM{ 4u };
		}directionalLight;
		struct
		{
			float radius{ 20.0f };
			float softEdgeRatio{ 0.1f };
		}pointLight;
		struct
		{
			bool bInit{ false };
			bool bCastShadow{ false };
			bool bEnableGaussianBlur{ false };
			bool bEnableTransparentShadow{ false };
			uint32_t shadowAlgorithmMode{ 0u };
			float depthBiasHigherBound{ 20.0f };
			float depthBiasMultiplier{ 10.0f };
			float nrmBiasMultiplier{ -1.0f };

			float dropOffDistance{ 100.0f }; //<! cutoff distance of shadow map
			float farZ{ 100.0f }; //<! far plane of the shadow map
			float nearZ{ 1.0f }; //<! near plane of the shadow map
			uint32_t dimension{ 512u };
			uint32_t origin_dimension{ 512u };
			uint32_t gaussianKernal{ 5u };
			uint32_t backBufferDimension{ 256u }; //<! Back shadow buffers are used for gaussian blur if necessary
			uint32_t origin_backBufferDimension{ 256u };
			std::shared_ptr<ShadowBuffer> shadowBuffer{ nullptr };
			std::shared_ptr<ShadowBuffer> shadowBuffer2{ nullptr }; //<! Secondary shadow buffer used for gaussian blur
			std::shared_ptr<ShadowBuffer> shadowBuffer3{ nullptr }; //<! Third shadow buffer used for shadow of transparent objects
		}shadow;
		float collisionRadius{ 0.1f }; //<! Object samller than this distance should not be lit by this light
		Vec4f attenuation{1.0f, 0.7f, 0.01f, 1.0f }; //<! Light attenuation: w / ( x + 1/d * y + 1/d^2 * z)
		LIGHT_TYPE type{};
	public:
		inline static const char* s_shadowAlgorithmModes[]{ "Direct", "PCF", "Vogel", "Poisson", "MSM2", "MSM4" };
	};
}
