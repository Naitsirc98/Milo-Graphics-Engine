#include "milo/scenes/components/Collider.h"

// Foundations of Game Engine Development Volume 2: Rendering, pages 240-253

namespace milo {

	static float calculateDiameter(const VertexList& vertices, int32_t& min, int32_t& max);

	SphereCollider SphereCollider::of(Mesh* mesh) {

		VertexList vertices(mesh);

		Vector3 center = {0, 0, 0};
		float radius = 0;

		// Determine initial center and radius
		int32_t min = 0, max = 0;
		float diameter = calculateDiameter(vertices, min, max);
		center = (vertices[min].position + vertices[max].position) * 0.5f;
		radius = sqrt(diameter) * 0.5f;

		// Make pass through vertices and adjust sphere as necessary
		for(Vertex vertex : vertices) {

			Vector3 pv = vertex.position - center;
			float m2 = length2(pv);
			if(m2 > radius * radius) {
				Vector3 q = center - (pv * (radius / sqrt(m2)));
				center = (q + vertex.position) * 0.5f;
				radius = length(q - center);
			}
		}

		return {center, radius};
	}

	static Vector3 makePerpendicularVector(const Vector3& v);
	static void getPrimaryBoxDirections(const VertexList& vertices, int32_t& min, int32_t& max, Vector3 primaryDirections[9]);
	static float calculateSecondaryDiameter(const VertexList& vertices, const Vector3& axis, int32_t& min, int32_t& max);
	static void getSecondaryBoxDirections(const VertexList& vertices, const Vector3& axis, int32_t& min, int32_t& max, Vector3 secondaryDirections[9]);

	BoxCollider BoxCollider::of(Mesh* mesh) {

		// Oriented Aligned Bounding Box

		VertexList vertices(mesh);

		Vector3 center;
		Vector3 size;
		Vector3 axis[3];

		int32_t min, max;
		Vector3 primaryDirections[9], secondaryDirections[9];

		calculateDiameter(vertices, min, max);

		getPrimaryBoxDirections(vertices, min, max, primaryDirections);

		float area = FLT_MAX;

		for(int32_t k = 0;k < 9;++k) {

			Vector3 s = normalize(primaryDirections[k]);
			calculateSecondaryDiameter(vertices, s, min, max);
			getSecondaryBoxDirections(vertices, s, min, max, secondaryDirections);

			// Loop over all candidates for secondary axis
			for(int32_t j = 0;j < 5;++j) {

				Vector3 t = normalize(secondaryDirections[j]);
				Vector3 u = cross(s, t);

				float smin = dot(s, vertices[0].position);
				float smax = smin;

				float tmin = dot(t, vertices[0].position);
				float tmax = tmin;

				float umin = dot(u, vertices[0].position);
				float umax = umin;

				for(int32_t i = 1;i < vertices.size();++i) {

					float ds = dot(s, vertices[i].position);
					float dt = dot(t, vertices[i].position);
					float du = dot(u, vertices[i].position);

					smin = std::min(smin, ds);
					smax = std::max(smax, ds);

					tmin = std::min(tmin, dt);
					tmax = std::max(tmax, dt);

					umin = std::min(umin, du);
					umax = std::max(umax, du);
				}

				float hx = (smax - smin) * 0.5f;
				float hy = (tmax - tmin) * 0.5f;
				float hz = (umax - umin) * 0.5f;

				// Calculate one-eighth surface area and see if it's better
				float m = hx * hy + hy * hz + hz * hx;
				if(m < area) {
					center = (s * (smin + smax) + t * (tmin + tmax) + u * (umin + umax)) * 0.5f;
					size = {hx, hy, hz};
					axis[0] = s;
					axis[1] = t;
					axis[2] = u;
					area = m;
				}
			}
		}

		return {center, size, axis[0], axis[1], axis[2]};
	}

	static float calculateDiameter(const VertexList& vertices, int32_t& min, int32_t& max) {
		constexpr uint32_t kDirectionCount = 13;

		static const Vector3 direction[kDirectionCount] = {
				{1, 0, 0}, {0, 1, 0}, {0, 0, 1},
				{1, 1, 0}, {1, 0, 1}, {0, 1, 1},
				{1, -1, 0}, {1, 0, -1}, {0, 1, -1},
				{1, 1, 1}, {1, -1, 1}, {1, 1, -1}, {1, -1, -1}
		};

		float dmin[kDirectionCount];
		float dmax[kDirectionCount];
		int32_t imin[kDirectionCount];
		int32_t imax[kDirectionCount];

		// Find min and max dot products for each direction and record vertex indices
		for(int32_t j = 0;j < kDirectionCount;++j) {

			const Vector3& u = direction[j];

			dmin[j] = dmax[j] = dot(u, vertices[0].position);
			imin[j] = imax[j] = 0;

			for(int32_t i = 1;i < vertices.size();++i) {

				float d = dot(u, vertices[i].position);

				if(d < dmin[j]) {
					dmin[j] = d;
					imin[j] = i;
				} else if(d > dmax[j]) {
					dmax[j] = d;
					imax[j] = i;
				}
			}
		}

		// Find direction for which vertices at min and max extents are furthest apart
		float d2 = length2(vertices[imax[0]].position - vertices[imin[0]].position);
		int32_t k = 0;
		for(int32_t j = 1;j < kDirectionCount;++j) {

			float m2 = length2(vertices[imax[j]].position - vertices[imin[j]].position);
			if(m2 > d2) {
				d2 = m2;
				k = j;
			}
		}

		min = imin[k];
		max = imax[k];

		return d2;
	}

	static Vector3 makePerpendicularVector(const Vector3& v) {
		float x = fabs(v.x);
		float y = fabs(v.y);
		float z = fabs(v.z);
		if(z < std::min(x, y)) return Vector3(v.y, -v.x, 0.0f);
		if(y < x) return Vector3(-v.z, 0, v.x);
		return Vector3(0, v.z, -v.y);
	}

	static void getPrimaryBoxDirections(const VertexList& vertices, int32_t& min, int32_t& max, Vector3 primaryDirections[9]) {

	}

	static float calculateSecondaryDiameter(const VertexList& vertices, const Vector3& axis, int32_t& min, int32_t& max) {

	}

	static void getSecondaryBoxDirections(const VertexList& vertices, const Vector3& axis, int32_t& min, int32_t& max, Vector3 secondaryDirections[9]) {

	}

}
