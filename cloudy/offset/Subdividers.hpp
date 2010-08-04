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

      class No_subdivider
      {
	 double _radius;
	    
	 public:
	    No_subdivider(double radius = 0.0):
	                     _radius(radius)
	    {}

	    template <class Integrator>
	    void aggregate(Integrator &ig,
	                   const typename Integrator::Vector &a,
			   const typename Integrator::Vector &b,
	                   const typename Integrator::Vector &c)
	    {
	       return ig.aggregate(a,b,c);
	    }
      };

      
     template<class Vector>
     double length(const Vector &v)
     {
       return sqrt(v.squared_length());
     }

      class Tesselate_subdivider
      {
	double _radius;
	double _eps;
	
      private:
	enum binary
	  {
	    b000,
	    b001,
	    b010,
	    b011,
	    b100,
	    b101,
	    b110,
	    b111,		  
	  };
	
	static inline size_t to_binary(bool a, bool b, bool c)
	{
	  return ((a ? 1 : 0) << 2)
	    | ((b ? 1 : 0) << 1)
	    | ((c ? 1 : 0) << 0);
	}
	
	template <class Integrator>
	void tesselate_spherical_triangle 
	   (Integrator &ig,
	    const typename Integrator::Vector &a,
	    const typename Integrator::Vector &b,
	    const typename Integrator::Vector &c)
	{
	  ig.aggregate(a, b, c);
	}
	
	template <class Vector>
	Vector segment_sphere_intersection(const Vector &a, const Vector &b)
	{
	  Vector v = b - a;
	  v = v/length(v);
	    
	  const double av = a * v;
	  const double Delta = av * av + _radius*_radius - a.squared_length();
	    	  
	  return a + (- av + sqrt(Delta)) * v;
	 }

	template <class Vector, class OutputIterator>
	inline
	void 
	split_segment (const Vector &b,
		       const Vector &c, 
		       size_t N,
		       OutputIterator it)
	{
	  Vector v = c - b;
	  double L = length(v);
	  v = v/L;

	  double increment = L / (N - 1);
	  double current = 0.0;

	  for (size_t i = 0; i < N; ++ i)
	    {
	      *it ++ =  b + current * v;
	      current += increment;
	    }
	}

#define DISPLAY(v) std::cerr << #v << " = " << v << "\n";

	template <class Vector, class OutputIterator>
	inline
	void 
	split_circular_arc (const Vector &center,
			    double radius,
			    const Vector &normal,
			    const Vector &b,
			    const Vector &c,
			    double eps,
			    OutputIterator iter)
	{
#if 0
	  *iter++ = b;
	  Vector v = (b + c)/2.0;
	  //- center;
	  //v = center + v * (radius/length(v));
	  *iter++ = v;
	  *iter++ = c;
#else
	  Vector xaxis = (b - center);
	  xaxis = xaxis / length(xaxis);
	  Vector yaxis = CGAL::cross_product(xaxis, normal);
	  Vector diffc = c - center;
	  double theta = atan2(diffc * yaxis, diffc * xaxis);
	  
	  size_t N = std::floor((fabs(theta) * radius)/eps)+1;
	  double theta_increment = theta/N;
	  DISPLAY(theta);
	  DISPLAY(N);	    
	  
	  *iter ++ = b;
	  theta = 0.0;
	  for (size_t i = 1; i < N; ++i)
	    {
	      *iter ++ = 
		(center + cos(theta) * xaxis + sin(theta) * yaxis);
	      theta += theta_increment;
	    }
	  *iter++ = c;
#endif
	}


	template <class Integrator>
	void aggregate_simple_triangle (Integrator &ig,
					bool inside,
					const typename Integrator::Vector &a,
					const typename Integrator::Vector &b,
					const typename Integrator::Vector &c)
	{
	  if (inside)
	    ig.aggregate(a,b,c);
	  else
	    ig.aggregate(a * (_radius/length(a)),
			 b * (_radius/length(b)),
			 c * (_radius/length(c)));
	}

	template <class Integrator>
	void split_intersecting_triangle
	   (Integrator &ig,
	    bool a_inside,
	    const typename Integrator::Vector &a,
	    const typename Integrator::Vector &b,
	    const typename Integrator::Vector &c)
	{
	  typedef typename Integrator::Vector Vector;

#if 0
 	  if (a_inside)
 	    {
 	      ig.aggregate(a,
			   b*(_radius/length(b)),
			   c*(_radius/length(c)));
 	    }
	  else
 	    {
 	      ig.aggregate(a*(_radius/length(a)), b, c);
 	    }

 	  return;
#endif

	  Vector triangle_normal
	    = CGAL::cross_product(b - a, c - a);
	  triangle_normal = triangle_normal/length(triangle_normal);
	  
	  double circle_distance = a * triangle_normal;
	  DISPLAY(circle_distance);
	  double circle_radius = sqrt(pow(_radius, 2.0) -
				      pow(circle_distance, 2.0));
	  DISPLAY(circle_radius);
	  Vector circle_center = circle_distance * triangle_normal;
	  DISPLAY(circle_center);
	  Vector b_circle = a_inside
	    ? segment_sphere_intersection(a, b) 
	    : segment_sphere_intersection(b, a);
	  Vector c_circle = a_inside
	    ? segment_sphere_intersection(a, c) 
	    : segment_sphere_intersection(c, a);

	  std::vector<Vector> intermediary_points, side_points;
	  split_circular_arc(circle_center, circle_radius,
			     triangle_normal, b_circle, c_circle, _eps,
			     std::back_inserter(intermediary_points));
	  size_t N = intermediary_points.size();
	  split_segment (b, c,  N, std::back_inserter(side_points));
	  DISPLAY(N);

	  assert(N >= 2);

	  // construct triangles, now.
	  // =====
	  // start with the easy ones: those with 'a' as base point
	  for (size_t i = 0; i < N - 1; ++i)
	    {
	      aggregate_simple_triangle(ig, a_inside,
					a, intermediary_points[i],
					intermediary_points[i+1]);
	    }

	  // now tesselate the other part
	  for (size_t i = 0; i < N - 1; ++i)
	    {
	      aggregate_simple_triangle(ig, !a_inside,
					intermediary_points[i],
					side_points[i], side_points[i+1]);
	      aggregate_simple_triangle(ig, !a_inside,
					intermediary_points[i],
					side_points[i+1],
					intermediary_points[i+1]);
	    }
	}
	
     public:
	Tesselate_subdivider(double radius,
			     double eps = 0.05):
	  _radius(radius),
	  _eps(eps * _radius)
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

	       switch(to_binary(sqa <= sqrad,
				sqb <= sqrad,
				sqc <= sqrad))
	       {
		  case b000:
		    // FIXME: case where the three points are outside,
		    // but there is an intersection.
		    tesselate_spherical_triangle(ig,
						 _radius*a/sqrt(sqa),
						 _radius*b/sqrt(sqb), 
						 _radius*c/sqrt(sqc));
		     break;

	          case b001:
		    split_intersecting_triangle(ig, true, c, a, b);
		    break;

	          case b010:
		    split_intersecting_triangle(ig, true, b, c, a);
		    break;

	          case b011:
		    split_intersecting_triangle(ig, false, a, b, c);
		    break;

	          case b100:
		    split_intersecting_triangle(ig, true, a, b, c);
		    break;

	          case b101:
		    split_intersecting_triangle(ig, false, b, c, a);
		    break;

	          case b110:
		    split_intersecting_triangle(ig, false, c, a, b);
		    break;

		  case b111:
		     ig.aggregate(a,b,c);
		     break;
	       }	       
	}
     };
   }
}

#endif 
