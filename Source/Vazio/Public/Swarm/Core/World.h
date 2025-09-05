#pragma once
#include <vector>
#include "Agent.h"
#include "Projectile.h"
#include "SpatialGrid.h"
namespace SwarmCore {
	struct Params { float CellSize = 200, Separation = 100, MaxSpeed = 800, WorldExtent = 20000; bool LockZ = false; float GroundZ = 0; bool ChaseTarget = false; float ChaseAccel = 800; };
	struct World {
		std::vector<float> Px, Py, Pz, Vx, Vy, Vz, HP, Cooldown; std::vector<type_t> Type; std::vector<int> Target; std::vector<uint8_t> Alive; std::vector<int> Free; std::vector<EnemyType> TypeDefs; Params P; Grid G; ProjectileRing Proj;
		float TargetX = 0, TargetY = 0, TargetZ = 0;
		void Configure(const Params& p, const std::vector<EnemyType>& T, int projCap, float projSpd, float projR) { P = p; TypeDefs = T; G.Cell = P.CellSize; Proj.Configure(projCap, projSpd, projR); }
		int Spawn(type_t t, float x, float y, float z, float vx = 0, float vy = 0, float vz = 0) { int i; if (!Free.empty()) { i = Free.back(); Free.pop_back(); Alive[i] = 1; } else { i = (int)Px.size(); Px.push_back(0); Py.push_back(0); Pz.push_back(0); Vx.push_back(0); Vy.push_back(0); Vz.push_back(0); HP.push_back(0); Cooldown.push_back(0); Type.push_back(0); Target.push_back(-1); Alive.push_back(1); } Px[i] = x; Py[i] = y; Pz[i] = z; Vx[i] = vx; Vy[i] = vy; Vz[i] = vz; HP[i] = TypeDefs[t].HP; Type[i] = t; Cooldown[i] = 0; return i; }
		void Kill(int i) { if (i >= 0 && i < (int)Alive.size() && Alive[i]) { Alive[i] = 0; Free.push_back(i); } }
		int Num() const { return (int)Alive.size() - (int)Free.size(); }
		void Step(float dt);
	};
}