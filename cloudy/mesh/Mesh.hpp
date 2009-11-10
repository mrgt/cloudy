#ifndef MESH_HPP
#define MESH_HPP

#include <iostream>

#include <cloudy/misc/Progress.hpp>
#include <cloudy/Cloud.hpp>
#include <cloudy/mesh/Gradient.hpp>

namespace cloudy
{
   struct Mesh_triangle
   {
     size_t a,b,c;
   public:
     Mesh_triangle(size_t aa, size_t bb, size_t cc): a(aa), b(bb), c(cc) {}
   };

   enum
   {
      MESH_NORMAL = (1<<0),
      MESH_COLOR  = (1<<1),
   };

   double area(const uvector &a, const uvector &b, const uvector &c);

   class Mesh
   {
      public:
	 Data_cloud _points, _normals;
	 std::vector<Color> _colors;
	 std::vector<Mesh_triangle> _triangles;
	 size_t _flags;

         size_t insert_midpoint(size_t a, size_t b);
	 
      public:
	 void read_off (std::istream &is);
	 void write_off (std::ostream &os) const;
	 
	 void clear()
	 {
	    _points.clear();
	    _normals.clear();
	    _colors.clear();
	    _triangles.clear();
	    _flags = 0;
	 }

	 size_t insert_point(const uvector &p, 
			     const uvector &n = uvector(),
			     const Color &c = Color())
	 {
	    _points.push_back(p);
	    if (_flags & MESH_NORMAL)
	       _normals.push_back(n);
	    if (_flags & MESH_COLOR)
	       _colors.push_back(c);
	    return _points.size() - 1;
	 }
	 
	 void set_flags(size_t flags)
	 {
	    if (flags & MESH_NORMAL == 0)
	       _normals.resize(0);
	    else
	       _normals.resize(_points.size());

	    if (flags & MESH_COLOR == 0)
	       _colors.resize(0);
	    else
	       _colors.resize(_points.size());
	    
	    _flags = flags;
	 }

	 size_t get_flags() const
	 {
	    return _flags;
	 }

	 double area() const;

	 void uniform_sample(Data_cloud &cl, size_t N);

	 template <class Function>
	 void simple_colorize (const Function &f, 
			       const Gradient &g,
			       bool zero_average = false)
	 {
	    std::vector<double> values(_points.size());
	    double minval(0), maxval(0);
	    
	    std::cerr << "Colorizing ... \n";
	    cloudy::misc::Progress_display progress(_points.size(), std::cerr);

	    for (size_t i = 0; i < _points.size(); ++i)
	    { 
	       double v =  f(_points[i]);
	       values[i] = v;
	       if (i == 0)
	       {
		  minval = v;
		  maxval = v;
	       }
	       else if (v >= maxval)
		 maxval = v;
	       else if (v <= minval)
		 minval = v;	       
	    }

	    if (zero_average)
	      {
		double absmax = std::max(fabs(minval), fabs(maxval));
		minval = - absmax;
		maxval = + absmax;
	      }
	    
	    set_flags(get_flags()|MESH_COLOR);
	    for (size_t i = 0; i < _points.size(); ++i)
	    {
	       double t = (values[i] - minval) / (maxval - minval);
	       _colors[i] = g(t);
	       ++progress;
	    }
	 }

         void simple_tesselate(double maxr);

 	 void normalize (double radius)
 	 {
	    cloudy::normalize(_points, radius);
 	 }

         void append_triangle (const uvector &a, 
			       const uvector &b, 
			       const uvector &c)
         {
	   size_t ida = insert_point(a);
	   size_t idb = insert_point(b);
	   size_t idc = insert_point(c);
	   _triangles.push_back(Mesh_triangle(ida, idb, idc));
         }
   };

  std::ostream &
  operator << (std::ostream &os, const Mesh &mesh)
  {
    mesh.write_off(os);
    return os;
  }
}

#endif
