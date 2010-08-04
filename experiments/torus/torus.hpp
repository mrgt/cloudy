#ifndef TORUS_HPP 
#define TORUS_HPP

#include <cloudy/random/Random.hpp>
#include <fstream>
#include <cloudy/Cloud.hpp>

using namespace cloudy;
inline uvector uv3 (double x, double y, double z)
{
  uvector v(3);
  v(0) = x;
  v(1) = y;
  v(2) = z;
  return v;
}

inline uvector rotate_around_z(const uvector &v, double theta)
{
  return uv3(cos(theta)*v(0) - sin(theta)*v(1),
	     sin(theta)*v(0) + cos(theta)*v(1),
	     v(2));
}

enum
  {
    // Angle in (x,y), corresponding to radius R: theta
    COORD_THETA,
    COORD_PHI,
    COORD_L
  };

#define DBG(x) std::cerr << "in " <<  __FUNCTION__ << ", "<< #x << " = " << x << "\n"

struct Torus
{
  double _r,  _R; 

  uvector xyz_to_torus (const uvector &uv) const
  {
    double theta = atan2(uv(1), uv(0));
    uvector uv_in_xz = rotate_around_z(uv, -theta);
    uv_in_xz(0) -= _R;
    // uv_in_xz lies in the plan (x,z).

    double phi = atan2(uv_in_xz(2), uv_in_xz(0));
    double l = ublas::norm_2(uv_in_xz);

    //DBG(uv); DBG(uv_in_xz);
    // DBG(theta);  DBG(phi);  DBG(l); 

    return uv3(theta, phi, l);
  }

  uvector
  torus_to_xyz (const uvector &tor) const
  {
    double theta = tor(COORD_THETA), phi = tor(COORD_PHI),
      L = tor(COORD_L);
    
    uvector uv_in_xz = uv3(L * cos(phi) + _R, 0, L * sin(phi));
    return rotate_around_z(uv_in_xz, theta);
  }

  uvector
  torus_to_normal (const uvector &tor) const
  {
    double theta = tor(COORD_THETA), phi = tor(COORD_PHI);
    
    uvector uv_in_xz = uv3(cos(phi), 0, sin(phi));
    return rotate_around_z(uv_in_xz, theta);
  }

public:
  Torus(double R, double r): _r(r), _R(R) {}

  bool is_inside(const uvector &uv) const
  {
    uvector tor = xyz_to_torus(uv);
    return (tor(COORD_L) <= _r);
  }

  uvector project(const uvector &uv) const
  {
    uvector tor = xyz_to_torus(uv);
    tor(COORD_L) = _r;
    return torus_to_xyz(tor);
  }

  uvector get_normal(const uvector &uv) const  
  {
    return torus_to_normal(xyz_to_torus(uv));
  }
};

inline void generate(const Torus &torus, size_t N,
		     Data_cloud &cloud)
{
  random::Random_vector_in_ball<uvector> randball (3, torus._R + torus._r);
  boost::mt19937 engine;

  while (cloud.size() != N)
    {
      uvector uv = randball(engine);
      if (torus.is_inside(uv))
	cloud.push_back(torus.project(uv));
    }
}

#endif
