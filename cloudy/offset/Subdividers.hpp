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
   } 
}

#endif 
