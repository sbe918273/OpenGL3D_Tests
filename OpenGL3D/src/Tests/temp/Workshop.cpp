#include "pch.h"
#include "math.h"
#include "glm.hpp"

static const float pi = 3.1415926f;

static void calcTangents(const glm::vec3* pos, const glm::vec2* uv, glm::vec3& outTangent, glm::vec3& outBitangent)
{
	const glm::vec3 edge1 = pos[1] - pos[0];
	const glm::vec3 edge2 = pos[2] - pos[0];
	const glm::vec2 deltaUV1 = uv[1] - uv[0];
	const glm::vec2 deltaUV2 = uv[2] - uv[0];

	const float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

	const glm::mat2 uvMatrix(
		deltaUV2.y, -deltaUV2.x,
		-deltaUV1.y, deltaUV1.x
	);

	const glm::mat3x2 edgeMatrix(
		edge1.x, edge2.x,
		edge1.y, edge2.y,
		edge1.z, edge2.z
	);

	const glm::mat3x2 tangentMatrix = f * uvMatrix * edgeMatrix;

	outTangent = glm::vec3(tangentMatrix[0][0], tangentMatrix[1][0], tangentMatrix[2][0]);
	outBitangent = glm::vec3(tangentMatrix[0][1], tangentMatrix[1][1], tangentMatrix[2][1]);
}

static void emplaceSpherePoints(unsigned int uSample, unsigned int vSample, float* outVertices)
{
	const unsigned int vertexCount = vSample * (uSample - 2) + 2;
	const unsigned int floatCount = 11 * vertexCount;

	outVertices[0] =  0.0f; // pos
	outVertices[1] =  1.0f;
	outVertices[2] =  0.0f;
	outVertices[3] =  0.0f; // texCoords
	outVertices[4] =  0.0f;
	outVertices[5] =  0.0f; // tangent
	outVertices[6] =  0.0f; 
	outVertices[7] =  1.0f;
	outVertices[8] = -1.0f; // bitangnet
	outVertices[9] =  0.0f;
	outVertices[10] = 0.0f;

	outVertices[floatCount - 11] =  0.0f; //pos
	outVertices[floatCount - 10] = -1.0f;
	outVertices[floatCount - 9] =   0.0f;
	outVertices[floatCount - 8] =   0.0f; //texCoords
	outVertices[floatCount - 7] =   1.0f;
	outVertices[floatCount - 6] =   0.0f; // tangent
	outVertices[floatCount - 5] =   0.0f; 
	outVertices[floatCount - 4] =  -1.0f;
	outVertices[floatCount - 3] =  -1.0f; // bitangent
	outVertices[floatCount - 2] =   0.0f;
	outVertices[floatCount - 1] =   0.0f;
	 
	const float uInc = pi / (uSample - 1);
	const float vInc = 2 * pi / vSample;

	float u = uInc;
	float v = 0.0f;

	unsigned int stride = 11;
	unsigned int idx = stride;

	for (unsigned int uIdx = 0; uIdx < uSample - 2; uIdx++)
	{
		for (unsigned int vIdx = 0; vIdx < vSample; vIdx++)
		{
			outVertices[idx++] = sin(u) * sin(v);									// 0
			outVertices[idx++] = cos(u);											// 1
			outVertices[idx++] = sin(u) * cos(v);									// 2
			outVertices[idx++] = (float)uIdx / (float)uSample;						// 3
			outVertices[idx++] = fmodf((float)vIdx / (float)vSample + 0.5f, 1.0f);	// 4
			
			glm::vec3 currentPos(outVertices[idx - 5], outVertices[idx - 4], outVertices[idx - 3]);
			glm::vec3 behindPos(outVertices[idx - stride - 5], outVertices[idx - stride - 4], outVertices[idx - stride - 3]);
			glm::vec3 lowerPos(outVertices[idx - vSample * stride - 5], outVertices[idx - vSample * stride - 4], outVertices[idx - vSample * stride - 3]);
			glm::vec3 positions[3] = { currentPos, behindPos, lowerPos };

			glm::vec2 currentTex(outVertices[idx - 2], outVertices[idx - 1]);
			glm::vec2 behindTex(outVertices[idx - stride - 2], outVertices[idx - stride - 1]);
			glm::vec2 lowerTex(outVertices[idx - vSample * stride - 2], outVertices[idx - vSample * stride - 1]);
			glm::vec2 texCoords[3] = { currentTex, behindTex, lowerTex };

			glm::vec3 tangent, bitangent;
			calcTangents(positions, texCoords, tangent, bitangent);

			outVertices[idx++] = tangent[0];
			outVertices[idx++] = tangent[1];
			outVertices[idx++] = tangent[2];
			outVertices[idx++] = bitangent[0];
			outVertices[idx++] = bitangent[1];
			outVertices[idx++] = bitangent[2];

			v += vInc;
		}
		u += uInc;
		v = 0.0f;
	}
}