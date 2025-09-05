#pragma once
namespace SwarmCore {
	struct float3 { float x = 0, y = 0, z = 0; };
	inline float3 operator+(float3 a, float3 b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
	inline float3 operator*(float3 a, float s) { return { a.x * s, a.y * s, a.z * s }; }
	inline float len2(float3 a) { return a.x * a.x + a.y * a.y + a.z * a.z; }
}
