#ifndef ODE_INTERFACE_TRI_MESH_H
#define ODE_INTERFACE_TRI_MESH_H

#include "ODESurface.h"
#include <meshing/TriMesh.h>
#include <geometry/CollisionMesh.h>
#include <ode/common.h>
#include <ode/collision_trimesh.h>
using namespace Math3D;

class ODETriMesh
{
 public:
  ODETriMesh();
  ~ODETriMesh();

  void Create(const Meshing::TriMesh& mesh,dSpaceID space,Vector3 offset=Vector3(0.0));
  void Clear();
  void DrawGL();
  void SetLastTransform(const Matrix4& mat);
  void GetLastTransform(Matrix4& mat);
  void SetPadding(Real outerMargin);
  Real GetPadding();

  dGeomID geom() const { return geomID; }
  dTriMeshDataID triMeshData() const { return triMeshDataID; }
  ODESurfaceProperties& surf() { return surface; }

 private:
  dTriMeshDataID triMeshDataID;
  dGeomID geomID;
  
  dReal* verts;
  int* indices;
  dReal* normals;
  int numVerts;
  int numTris;
  int numVertComponents;

  dReal lastTransform[16];
  Geometry::CollisionMesh* collisionMesh;
  ODESurfaceProperties surface;
};

#endif
