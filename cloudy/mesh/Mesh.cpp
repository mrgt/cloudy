#include <cloudy/mesh/Mesh.hpp>
#include <cloudy/random/Random.hpp>
#include <string>
#include <list>
#include <sstream>
#include <time.h>

namespace cloudy
{
   std::string off_read_line(std::istream &is)
   {
      std::string line = "";
      do 
      {
	 char s[1000];
	 is.getline(s, 1000);
	 
	 if (s[0] != '#')
	    line = s;
      } while(line == "");
      return line;
   }

  class pov_vector
  {
  public:
    const uvector &_v;
  public:
    pov_vector(const uvector &v): _v(v)
    {
    }
  };

  std::ostream &
  operator << (std::ostream &os, const pov_vector &v)
  {
    return os << "<" << v._v(0) << ", " << v._v(1) << ", " << v._v(2) << ">";
  }

  

  void
  Mesh::export_pov(std::ostream &os)
  {
    os << "mesh\n{\n";
      for (size_t i = 0; i < _triangles.size(); ++i)
      {
	if (_flags & MESH_NORMAL)
	  {
	    os << "  smooth_triangle\n"
	       << "  {\n"
	       << pov_vector(_points[_triangles[i].a]) << ",\n"
	       << pov_vector(_normals[_triangles[i].a]) << ",\n"
	       << pov_vector(_points[_triangles[i].b]) << ",\n"
	       << pov_vector(_normals[_triangles[i].b]) << ",\n"
	       << pov_vector(_points[_triangles[i].c]) << ",\n"
	       << pov_vector(_normals[_triangles[i].c]) << "\n"
	       << "  }\n";
	  }
	else
	  {
	    os << "  triangle\n"
	       << "  {\n"
	       << pov_vector(_points[_triangles[i].a]) << ",\n"
	       << pov_vector(_points[_triangles[i].b]) << ",\n"
	       << pov_vector(_points[_triangles[i].c]) << "\n"
	       << "  }\n";
	  }
      }
      os << "}\n\n";
  }

   void Mesh::read_off(std::istream &is)
   {
     if (!is) 
       {
	 std::cerr << "Mesh::read_off: Unable to read file\n";
	 return;
       }

      clear();

      std::string type = off_read_line(is);
      if (type == "CNOFF")
	 _flags = MESH_NORMAL | MESH_COLOR;
      else if (type == "NOFF")
	 _flags = MESH_NORMAL;
      else if (type == "COFF")
	 _flags = MESH_COLOR;
      else if (type == "OFF")
	 _flags = 0;
      else
      {
	 std::cerr << "Mesh::read_off: Unsupported file format: " << type 
		   << std::endl;
	 return;
      }

      size_t Nvertices, Nfaces, Nedges;

      std::istringstream ss(off_read_line(is));
      ss >> Nvertices >> Nfaces >> Nedges; 

#if 0
      std::cerr << "Nvertices: " << Nvertices << std::endl;
      std::cerr << "Nfaces: " << Nfaces << std::endl;
      std::cerr << "Nedges: " << Nedges << std::endl;
#endif

      for (size_t i = 0; i < Nvertices; ++i)
      {
	 std::istringstream ss(off_read_line(is));

	 uvector p(3), n(3); Color c;
	 ss >> p(0) >> p(1) >> p(2);
	 if (_flags & MESH_NORMAL)
	    ss >> n(0) >> n(1) >> n(2);
	 if (_flags & MESH_COLOR)
	    ss >> c._r >> c._g >> c._b;

	 insert_point(p,n,c);
      }

      for (size_t i = 0; i < Nfaces; ++i)
      {
	 std::istringstream ss(off_read_line(is));

	 size_t n;
	 ss >> n;
	 std::vector<size_t> v(n);
	 for (size_t j = 0; j < n; ++j)
	    ss >> v[j];
	 
	 for (size_t j = 0; j < n-2; ++j)
	 {
	   Mesh_triangle t (v[0], v[j+1], v[j+2]);
	    _triangles.push_back(t);
	 }
      }
   }

   void Mesh::write_off (std::ostream &os) const
   {
      if (_flags & MESH_COLOR)
	 os << "C";
      if (_flags & MESH_NORMAL)
	 os << "N";
      os << "OFF\n";
      os << _points.size() << " " << _triangles.size() << " 0\n";

      for (size_t i = 0; i < _points.size(); ++i)
      {
	 os << _points[i](0) << " " 
	    << _points[i](1) << " " 
	    << _points[i](2);

	 if (_flags & MESH_NORMAL)
	 {
	    os << " " << _normals[i](0)
	       << " " << _normals[i](1)
	       << " " << _normals[i](2);

	 }
	 if (_flags & MESH_COLOR)
	 {
	    os << " " << _colors[i]._r
	       << " " << _colors[i]._g
	       << " " << _colors[i]._b
	       << " " << 1.0;
	 }
	 os << std::endl;
      }

      for (size_t i = 0; i < _triangles.size(); ++i)
      {
	 os << "3 "
	    << _triangles[i].a << " "
	    << _triangles[i].b << " "
	    << _triangles[i].c << std::endl;
      }
   }

   double area(const uvector &a, const uvector &b, const uvector &c)
   {
      uvector C = cross_prod(b - a, c - a);
      return ublas::norm_2(C)/2.0;
   }

   double
   Mesh::area() const
   {
      double T = 0;
      for (size_t i = 0; i < _triangles.size(); ++i)
      {
	 T += cloudy::area(_points[_triangles[i].a],
			   _points[_triangles[i].b],
			   _points[_triangles[i].c]);
      }
      return T;
   }


   double
   Mesh::area(size_t i) const
   {
     assert(i < num_triangles());
     return cloudy::area(_points[_triangles[i].a],
			 _points[_triangles[i].b],
			 _points[_triangles[i].c]);    
   }


   void
   Mesh::uniform_sample(Data_cloud &cl, size_t N)
   {
     boost::mt19937 rng (static_cast<std::size_t>(time(0)));
      
      cl.clear();

      double density = N / area();

      std::vector<double> cumareas; double totarea = 0;
      for (size_t i = 0; i < _triangles.size(); ++i)
      {
	totarea += cloudy::area(_points[_triangles[i].a],
				_points[_triangles[i].b],
				_points[_triangles[i].c]);
	cumareas.push_back(totarea);
      }

      boost::uniform_real<> u01;
      
      for (size_t i = 0; i < N; ++i)
      {
	double f = u01(rng) * totarea;
	size_t t = 0;
	
	while (t < cumareas.size() && cumareas[t] < f)
	  ++t;

	cloudy::random::Random_point_on_triangle<uvector>
	  R(_points[_triangles[t].a],
	    _points[_triangles[t].b],
	    _points[_triangles[t].c]);

	cl.push_back(R(rng));
      }
   }


  static
  double max_length(const uvector &a, const uvector &b, 
		    const uvector &c)
  {
    return std::max(std::max(ublas::norm_2(b-a),
			     ublas::norm_2(c-a)),
		    ublas::norm_2(c-b));
  }

  size_t
  Mesh::insert_midpoint(size_t a, size_t b)
  {
    size_t r = _points.size();

    _points.push_back(0.5*(_points[a] + _points[b]));
    
    if (_flags & MESH_NORMAL)
      _normals.push_back(0.5*(_normals[a] + _normals[b]));

    if (_flags & MESH_COLOR)
      _colors.push_back(0.5*(_colors[a] + _colors[b]));

    return r;
  }


   void
   Mesh::simple_tesselate(double maxr)
   {
     std::list<Mesh_triangle> queue;
     for (size_t i = 0; i < _triangles.size(); ++i)
       queue.push_back(_triangles[i]);
     
     _triangles.clear();
     while(!queue.empty())
       {
	 Mesh_triangle t = queue.front(); queue.pop_front();
	 double l = max_length(_points[t.a], _points[t.b], _points[t.c]);

	 if (l < maxr)
	   {
	     _triangles.push_back(t);
	     continue;
	   }

	 size_t A = insert_midpoint(t.a, t.b);
	 size_t B = insert_midpoint(t.b, t.c);
	 size_t C = insert_midpoint(t.c, t.a);

	 queue.push_back(Mesh_triangle(t.a, A, C));
	 queue.push_back(Mesh_triangle(A, t.b, B));
	 queue.push_back(Mesh_triangle(C, B, t.c));
	 queue.push_back(Mesh_triangle(A, B, C));
       }
   }
}
