#ifndef SUBDIVIDER_HPP

#include <CGAL/Origin.h>
#include <math.h>

namespace cloudy {
   namespace offset {

      class Clamp_subdivider
      {
	 double _radius;
	    
	 public:
	    Clamp_subdivider(double radius = 0.0):
	                     _radius(radius)
	    {}

	    template <class Integrator>
	    void aggregate(Integrator &ig,
	                   const typename Integrator::Vector &a,
			   const typename Integrator::Vector &b,
	                   const typename Integrator::Vector &c)
	    {
	       typedef typename Integrator::Vector Vector;
	       typedef typename Integrator::Point Point;	       
	       
	       double aL = sqrt(a.squared_length()),
		  bL = sqrt(b.squared_length()),
		  cL = sqrt(c.squared_length());
	       Vector na(a), nb(b), nc(c);
	       
	       if (aL >= _radius) na = na * (_radius/aL);
	       if (bL >= _radius) nb = nb * (_radius/bL);
	       if (cL >= _radius) nc = nc * (_radius/cL);
	       
	       return ig.aggregate(na,nb,nc);
	    }
      };

      
#define binary(a1, a2, a3)  (((a1) << 0) | ((a2) << 1) | ((a3) << 2))

     class Tesselate_subdivider
     {
	double _radius;
	double _eps;

     private:
	template <class Integrator>
	void tesselate_spherical_triangle(Integrator &ig,
					  const typename Integrator::Vector &a,
					  const typename Integrator::Vector &b,
					  const typename Integrator::Vector &c)
	{
	}
	
	enum binary
	{
	   b000, b001, b010, b011,
	   b100, b101, b110, b111,		  
	};
	
	static inline size_t to_binary(bool a, bool b, bool c)
	{
	   return ((a ? 1 : 0) << 0)
	      | ((b ? 1 : 0) << 1)
	      | ((c ? 1 : 0) << 2);
	}

	
     public:
	Tesselate_subdivider(double radius,
			     double eps = 0.05):
	   _radius(radius),
	   _eps(eps)
	{}

	template <class Integrator>
	void aggregate (Integrator &ig,
			const typename Integrator::Vector &a,
			const typename Integrator::Vector &b,
			const typename Integrator::Vector &c)
	{
	       typedef typename Integrator::Vector Vector;
	       typedef typename Integrator::Point Point;	       
	       
	       const double sqrad = _radius*_radius;

	       const double sqa = a.squared_length(),
		  sqb = b.squared_length(),
		  sqc = c.squared_length();	      

	       switch(to_binary(sqa <= sqrad, sqb <= sqrad, sqc <= sqrad))
	       {
		  case b111:
		     ig.aggregate(a,b,c);
		     break;

		  case b000:
		     tesselate_spherical_triangle(a/sqrt(sqa), b/sqrt(sqb), 
						  c/sqrt(sqc));
		     break;
	       }	       
	}
     };
   }
}

#endif 
