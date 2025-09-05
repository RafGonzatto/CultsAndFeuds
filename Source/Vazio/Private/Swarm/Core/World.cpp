#include "Swarm/Core/World.h"
#include "Swarm/Core/Behaviors.h"
#include <cmath>
using namespace SwarmCore;
void World::Step(float dt) {
	// Separation to keep spacing
	Beh::Separation(*this, P.Separation);
	const float ms2 = P.MaxSpeed * P.MaxSpeed, ext = P.WorldExtent;
	const int N = (int)Px.size();
	for (int i = 0; i < N; ++i) {
		if (!Alive[i]) continue;
		// Optional chase towards Target
		if (P.ChaseTarget) {
			float dx = TargetX - Px[i];
			float dy = TargetY - Py[i];
			float dz = (P.LockZ ? 0.f : (TargetZ - Pz[i]));
			float d2 = dx*dx + dy*dy + dz*dz;
			if (d2 > 1.f) {
				float d = std::sqrt(d2);
				float inv = 1.f / d;
				Vx[i] += dx * inv * P.ChaseAccel * dt;
				Vy[i] += dy * inv * P.ChaseAccel * dt;
				if (!P.LockZ) Vz[i] += dz * inv * P.ChaseAccel * dt;
			}
		}
		// Clamp speed
		float s2 = Vx[i] * Vx[i] + Vy[i] * Vy[i] + Vz[i] * Vz[i];
		if (s2 > ms2) { float s = std::sqrt(s2), k = P.MaxSpeed / s; Vx[i] *= k; Vy[i] *= k; Vz[i] *= k; }
		// Integrate
		Px[i] += Vx[i] * dt; Py[i] += Vy[i] * dt; Pz[i] += Vz[i] * dt;
		// Optional Z lock to ground plane
		if (P.LockZ) { Pz[i] = P.GroundZ; Vz[i] = 0; }
		// Bounce at world extent
		if (Px[i]<-ext || Px[i]>ext) Vx[i] *= -1; if (Py[i]<-ext || Py[i]>ext) Vy[i] *= -1; if (!P.LockZ && (Pz[i]<-ext || Pz[i]>ext)) Vz[i] *= -1;
	}
	Proj.Step(dt);
}