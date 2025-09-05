#pragma once
#include <vector>
#include "MathTypes.h"
namespace SwarmCore {
	struct ProjectileRing { struct Item { float3 p, v; uint8_t alive = 0; }; std::vector<Item> Items; int Head = 0; float Speed = 2000, Radius = 20; void Configure(int cap, float spd, float r) { Items.assign(cap, {}); Head = 0; Speed = spd; Radius = r; } int Spawn(float3 p, float3 dir) { int i = Head; Head = (Head + 1) % ((int)Items.size()); Items[i].p = p; Items[i].v = dir * Speed; Items[i].alive = 1; return i; } void Step(float dt) { for (auto& it : Items) if (it.alive) it.p = it.p + it.v * dt; } };
}

