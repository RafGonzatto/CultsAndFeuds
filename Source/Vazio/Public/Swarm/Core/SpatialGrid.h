#pragma once
#include <unordered_map>
#include <vector>
#include <cmath>
#include "MathTypes.h"
namespace SwarmCore {
	struct Grid {
		float Cell = 200; std::unordered_map<long long, std::vector<int>> C; inline long long H(int x, int y, int z)const { return ((long long)(x & 0x1FFFFF) << 42) | ((long long)(y & 0x1FFFFF) << 21) | (long long)(z & 0x1FFFFF); } inline void clr() { C.clear(); }
		inline void ins(const float3& p, int i) { int x = (int)std::floor(p.x / Cell), y = (int)std::floor(p.y / Cell), z = (int)std::floor(p.z / Cell); C[H(x, y, z)].push_back(i); }
		template<class A> void forNbr(const float3& p, A&& a)const { int x = (int)std::floor(p.x / Cell), y = (int)std::floor(p.y / Cell), z = (int)std::floor(p.z / Cell); for (int dz = -1; dz <= 1; ++dz)for (int dy = -1; dy <= 1; ++dy)for (int dx = -1; dx <= 1; ++dx) { auto it = C.find(H(x + dx, y + dy, z + dz)); if (it == C.end()) continue; for (int j : it->second) a(j); } }
		void build(const std::vector<float>& Px, const std::vector<float>& Py, const std::vector<float>& Pz) { C.clear(); C.reserve(Px.size()); for (int i = 0; i < (int)Px.size(); ++i) { ins(float3{ Px[i],Py[i],Pz[i] }, i); } }
	};
}

