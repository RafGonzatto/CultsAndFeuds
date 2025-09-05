#include "Swarm/Core/Behaviors.h"
#include "Swarm/Core/World.h"
#include <cmath>
using namespace SwarmCore;
void Beh::Separation(World& W, float sep) {
	const float s2 = sep * sep; const int N = (int)W.Px.size(); std::vector<float> Ax(N, 0), Ay(N, 0), Az(N, 0); W.G.Cell = W.P.CellSize; W.G.build(W.Px, W.Py, W.Pz); for (int i = 0; i < N; ++i) { if (!W.Alive[i]) continue; float3 pi{ W.Px[i],W.Py[i],W.Pz[i] }; float pushx = 0, pushy = 0, pushz = 0; W.G.forNbr(pi, [&](int j) { if (j <= i || !W.Alive[j]) return; float dx = W.Px[j] - pi.x, dy = W.Py[j] - pi.y, dz = W.Pz[j] - pi.z; float d2 = dx * dx + dy * dy + dz * dz; if (d2 > 0 && d2 < s2) { float d = std::sqrt(d2), inv = 1.f / d, overlap = (sep - d) * 0.5f; pushx -= dx * inv * overlap; pushy -= dy * inv * overlap; pushz -= dz * inv * overlap; Ax[j] += dx * inv * overlap; Ay[j] += dy * inv * overlap; Az[j] += dz * inv * overlap; }}); Ax[i] += pushx; Ay[i] += pushy; Az[i] += pushz; W.Vx[i] += Ax[i]; W.Vy[i] += Ay[i]; W.Vz[i] += Az[i]; }
}