#pragma once
#include <vec3.hpp>
#include "PointLight.h"

constexpr int COUNT_POINT_LIGHT = 4;

namespace LightData
{
	extern glm::vec3 lightColors[COUNT_POINT_LIGHT];
	extern PointLight pointLights[COUNT_POINT_LIGHT];
}