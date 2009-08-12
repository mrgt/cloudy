#ifndef CLOUDY_RANDOM_RANDOM_HPP

#include <cloudy/linear/Linear.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/variate_generator.hpp>

namespace cloudy 
{
  namespace random
  {
    template <class Vector>
    class Gaussian_vector
    {
    protected:
	 boost::normal_distribution<> _nd;
	 size_t _dim;
	 
      public:
	 Gaussian_vector(size_t dim, double sigma) : 
	    _nd(0.0, sigma),
	    _dim(dim)
	 {}
	 
	 
	 template <class Engine>
	 Vector operator () (Engine &eng)
	 {
	    boost::variate_generator<Engine&,
	       boost::normal_distribution<double> > vg (eng, _nd);

	    Vector res(_dim);
	    for (size_t i = 0; i < _dim; ++i)
	       res[i] = vg();
	    return res;
	 }
   };
   
   template <class Vector>
   class Random_vector_on_sphere : public Gaussian_vector<Vector>
   {
	 double _radius;

      public:
	 Random_vector_on_sphere (size_t dim, double radius) : 
	    Gaussian_vector<Vector>(dim, 1.0),
	    _radius (radius)
	 {}

	 template <class Engine>
	 Vector operator () (Engine &eng)
	 {
	    Vector dir;
	    
	    do dir = Gaussian_vector<Vector>::operator() (eng);
	    while (inner_prod(dir,dir) == 0.0);
	    
	    return (_radius/sqrt(ublas::inner_prod(dir,dir)))*dir;
	 }
   };

   template <class Vector>
   class Random_vector_in_ball : public Gaussian_vector<Vector>
   {
      public:
	 double _radius;
	 boost::uniform_real<> _u01;
      public:
         Random_vector_in_ball (size_t dim, double radius) : 
	    Gaussian_vector<Vector>(dim, 1.0),
	    _radius (radius)
	 {}

	 template <class Engine>
	 uvector operator () (Engine &eng)
	 {
	    Vector dir;
	    
	    do dir = Gaussian_vector<Vector>::operator() (eng);
	    while (inner_prod(dir,dir) == 0.0);
	    
	    double r = _radius*pow(_u01(eng), 1.0/dir.size());
	    return (r/ublas::norm_2(dir))*dir;
	 }
   };

   template <class Point>
   class Random_point_on_triangle
   {
      public:
	 Point _a, _b, _c;
	 boost::uniform_real<> _u01;

      public:
	 Random_point_on_triangle (const Point &a,
				   const Point &b,
				   const Point &c) : 
	    _a(a), _b(b), _c(c)
   	 {}

	 template <class Engine>
	 uvector operator () (Engine &eng)
	 {
	    double ta(_u01(eng)), tb(_u01(eng));
	    
	    if (ta+tb > 1.0)
	    {
	       ta = 1.0 - ta;
	       tb = 1.0 - tb;
	    }
	    double tc = 1.0 - ta - tb;

	    return ta * _a + tb * _b + tc * _c;
	 }
   };
  }
}

#endif //CLOUDY_RANDOM_RANDOM_HPP
