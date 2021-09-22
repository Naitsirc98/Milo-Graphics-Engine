#include "milo/math/Math.h"
#include "milo/assets/meshes/Mesh.h"

// Foundations of Game Engine Development Volume 2: Rendering, pages 240-253
namespace milo {

	bool BoundingSphere::isVisible(const Matrix4& transform, const Plane* planes, uint32_t planeCount) const {
		// Assume uniform scale
		float r = this->radius;
		for(int32_t i = 0;i < planeCount;++i) {
			if(dot(planes[i].xyz, center) <= -r) return false;
		}
		return true;
	}

	bool AxisAlignedBoundingBox::isVisible(const Matrix4& transform, const Plane* planes, uint32_t planeCount) const {
		Vector3 size = this->size;
		for(int32_t i = 0;i < planeCount;++i) {
			const Plane& g = planes[i];
			float rg = fabs(g.x * size.x) + fabs(g.y * size.y) + fabs(g.z * size.z);
			if(dot(g.xyz, center) <= -rg) return false;
		}
		return true;
	}

	bool OrientedBoundingBox::isVisible(const Matrix4& transform, const Plane* planes, uint32_t planeCount) const {
		Vector3 size = this->size;
		for(int32_t i = 0;i < planeCount;++i) {
			const Plane& g = planes[i];
			float rg = fabs(dot(g.xyz, xAxis) * size.x) + fabs(dot(g.xyz, yAxis) * size.y) + fabs(dot(g.xyz, zAxis) * size.z);
			if(dot(g.xyz, center) <= -rg) return false;
		}
		return true;
	}

	static float calculateDiameter(const VertexList& vertices, int32_t& min, int32_t& max);

	BoundingSphere BoundingSphere::of(Mesh* mesh) {

		VertexList vertices(mesh);

		Vector3 center = {0, 0, 0};
		float radius = 0;

		// Determine initial center and radius
		int32_t min = 0, max = 0;
		float diameter = calculateDiameter(vertices, min, max);
		center = (vertices[min].position + vertices[max].position) * 0.5f;
		radius = sqrt(diameter) * 0.5f;

		// Make pass through vertices and adjust sphere as necessary
		for(uint32_t i = 0;i < vertices.size();++i) {

			const Vertex& vertex = vertices[i];

			Vector3 pv = vertex.position - center;
			float m2 = length2(pv);
			if(m2 > radius * radius) {
				Vector3 q = center - (pv * (radius / sqrt(m2)));
				center = (q + vertex.position) * 0.5f;
				radius = length(q - center);
			}
		}

		BoundingSphere sphere;
		sphere.center = center;
		sphere.radius = radius;

		return sphere;
	}

	AxisAlignedBoundingBox AxisAlignedBoundingBox::of(Mesh* mesh) {

		VertexList vertices(mesh);

		Vector3 center;
		Vector3 size;

		Vector3 vmin = vertices[0].position;
		Vector3 vmax = vertices[0].position;

		for(int32_t i = 1;i < vertices.size();++i) {
			vmin = milo::minv(vmin, vertices[i].position);
			vmax = milo::minv(vmax, vertices[i].position);
		}

		center = (vmin + vmax) * 0.5f;
		size = (vmax - vmin) * 0.5f;

		AABB aabb;
		aabb.center = center;
		aabb.size = size;

		return aabb;
	}

	static Vector3 makePerpendicularVector(const Vector3& v);
	static void calculateSecondaryDiameter(const VertexList& vertices, const Vector3& axis, int32_t& min, int32_t& max);
	static void findExtremalVertices(const VertexList& vertices, const Plane& plane, int32_t& e, int32_t& f);
	static void getPrimaryBoxDirections(const VertexList& vertices, int32_t& min, int32_t& max, Vector3 direction[9]);
	static void getSecondaryBoxDirections(const VertexList& vertices, const Vector3& axis, int32_t& min, int32_t& max, Vector3 direction[9]);

	OrientedBoundingBox OrientedBoundingBox::of(Mesh* mesh) {

		// Oriented Aligned Bounding Box

		VertexList vertices(mesh);

		Vector3 center;
		Vector3 size;
		Vector3 axis[3];

		int32_t a, b;
		Vector3 primaryDirections[9], secondaryDirections[9];

		calculateDiameter(vertices, a, b);

		getPrimaryBoxDirections(vertices, a, b, primaryDirections);

		float area = FLT_MAX;

		for(int32_t k = 0;k < 9;++k) {

			Vector3 s = normalize(primaryDirections[k]);
			calculateSecondaryDiameter(vertices, s, a, b);
			getSecondaryBoxDirections(vertices, s, a, b, secondaryDirections);

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

		OBB obb;
		obb.center = center;
		obb.size = size * 2.0f;
		obb.xAxis = axis[0];
		obb.yAxis = axis[1];
		obb.zAxis = axis[2];

		return obb;
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
		if(z < glm::min(x, y)) return Vector3(v.y, -v.x, 0.0f);
		if(y < x) return Vector3(-v.z, 0, v.x);
		return {0, v.z, -v.y};
	}

	static void calculateSecondaryDiameter(const VertexList& vertices, const Vector3& axis, int32_t& min, int32_t& max) {

		constexpr int32_t kDirectionCount = 4;
		static const Vector2 direction[kDirectionCount] = {
				{1, 0}, {0, 1}, {1, 1}, {1, -1}
		};

		float dmin[kDirectionCount];
		float dmax[kDirectionCount];
		int32_t imin[kDirectionCount];
		int32_t imax[kDirectionCount];

		// Create vectors x and y perpendicular to the primary axis
		Vector3 x = makePerpendicularVector(axis);
		Vector3 y = cross(axis, x);

		for(int32_t j = 0;j < kDirectionCount;++j) {

			Vector3 t = x * direction[j].x + y * direction[j].y;
			dmin[j] = dmax[j] = dot(t, vertices[0].position);
			imin[j] = imax[j] = 0;

			for(int32_t i = 1;i < vertices.size();++i) {

				float d = dot(t, vertices[i].position);
				if(d < dmin[j]) {
					dmin[j] = d;
					imin[j] = i;
				} else if(d > dmax[j]) {
					dmax[j] = d;
					imax[j] = i;
				}
			}
		}

		// Find diameter in plane perpendicular to primary axis
		Vector3 dv = vertices[imax[0]].position - vertices[imin[0]].position;
		float d2 = length2(dv - axis * dot(dv, axis));
		int32_t k = 0;

		for(int32_t j = 1;j < kDirectionCount;++j) {
			dv = vertices[imax[j]].position - vertices[imin[j]].position;
			float m2 = length2(dv - axis * dot(dv, axis));
			if(m2 > d2) {
				d2 = m2;
				k = j;
			}
		}

		min = imin[k];
		max = imax[k];
	}

	void findExtremalVertices(const VertexList& vertices, const Plane& plane, int32_t& e, int32_t& f) {

		e = 0;
		f = 0;

		float dmin = dot(plane.xyz, vertices[0].position);
		float dmax = dmin;

		for(int32_t i = 1;i < vertices.size();++i) {
			float m = dot(plane.xyz, vertices[i].position);
			if(m < dmin) {
				dmin = m;
				e = i;
			} else if(m > dmax) {
				dmax = m;
				f = i;
			}
		}
	}

	static void getPrimaryBoxDirections(const VertexList& vertices, int32_t& min, int32_t& max, Vector3 direction[9]) {

		int32_t c = 0;
		direction[0] = vertices[max].position - vertices[min].position;

		float dmax = distancePointLine(vertices[0].position, vertices[min].position, direction[0]);

		for(int32_t i = 1;i < vertices.size();++i) {
			float m = distancePointLine(vertices[i].position, vertices[min].position, direction[0]);
			if(m > dmax) {
				dmax = m;
				c = i;
			}
		}

		direction[1] = vertices[c].position - vertices[min].position;
		direction[2] = vertices[c].position - vertices[max].position;

		Vector3 normal = cross(direction[0], direction[1]);
		Plane plane(normal, -dot(normal, vertices[min].position));

		int32_t e, f;
		findExtremalVertices(vertices, plane, e, f);

		direction[3] = vertices[e].position - vertices[min].position;
		direction[4] = vertices[e].position - vertices[max].position;

		direction[5] = vertices[e].position - vertices[c].position;
		direction[6] = vertices[f].position - vertices[min].position;

		direction[7] = vertices[f].position - vertices[max].position;
		direction[8] = vertices[f].position - vertices[c].position;
	}

	static void getSecondaryBoxDirections(const VertexList& vertices, const Vector3& axis, int32_t& min, int32_t& max, Vector3 direction[9]) {

		direction[0] = vertices[max].position - vertices[min].position;
		Vector3 normal = cross(axis, direction[0]);
		Plane plane(normal, -dot(normal, vertices[min].position));

		int32_t e, f;
		findExtremalVertices(vertices, plane, e, f);

		direction[1] = vertices[e].position - vertices[min].position;
		direction[2] = vertices[e].position - vertices[max].position;

		direction[3] = vertices[f].position - vertices[min].position;
		direction[4] = vertices[f].position - vertices[max].position;

		for(int32_t j = 0;j < 5;++j) {
			direction[j] -= axis * dot(direction[j], axis);
		}
	}
}