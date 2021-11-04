#include "pch.h"
#include "LightData.h"

glm::vec3 LightData::lightColors[] = {
	{1.0f, 1.0f, 1.0f},
	{1.0f, 0.0f, 0.0f},
	{0.0f, 1.0f, 0.0f},
	{0.0f, 0.0f, 1.0f},
};

PointLight LightData::pointLights[] = {
	{glm::vec3(-0.7f,  -0.2f,  2.0f), 1.0f, 0.09f, 0.032f, 0.2f * LightData::lightColors[0], 0.5f * LightData::lightColors[0], 1.0f * LightData::lightColors[0]},
	{glm::vec3(-1.9f,  -0.8f,  1.0f), 1.0f, 0.09f, 0.032f, 0.2f * LightData::lightColors[1], 0.5f * LightData::lightColors[1], 1.0f * LightData::lightColors[1]},
	{glm::vec3(-2.6f,  -1.3f,  1.2f), 1.0f, 0.09f, 0.032f, 0.2f * LightData::lightColors[2], 0.5f * LightData::lightColors[2], 1.0f * LightData::lightColors[2]},
	{glm::vec3(-0.1f,  -0.5f,  1.1f), 1.0f, 0.09f, 0.032f, 0.2f * LightData::lightColors[3], 0.5f * LightData::lightColors[3], 1.0f * LightData::lightColors[3]},
};