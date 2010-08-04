#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>

#include <cloudy/misc/Program_options.hpp>
#include <cloudy/misc/Progress.hpp>
#include <cloudy/offset/Offset.hpp>
#include <cloudy/Cloud.hpp>

#include <boost/timer.hpp>
#include <fstream>
#include <vector>
#include <map>

///////////////////////////////////////////////////////////////

namespace cloudy { namespace offset {

template <class Vector, class OutputIterator>
bool segment_sphere_intersect(double radius,
			      const Vector &a, const Vector &b,
			      OutputIterator w)
{
  Vector v = b - a;
  double dAB = length(v);
  v = v/dAB;
  
  const double av = a * v;
  const double Delta = av * av + radius*radius - a.squared_length();
  bool result = false;
  
  const double pm[] = {+1.0, -1.0};
  for (size_t i = 0; i < 2; ++i)
    {
      Vector wi = a + (- av + pm[i]*sqrt(Delta)) * v;
      double ti = wi * v;
      
      if (ti >= 0 && ti <= dAB)
	{
	  *w++ = wi;
	  result = true;
	}
    }

  return result;
}

template <class RT, class Integrator>
void
aggregate_EX (const RT &rt,
	      typename RT::Vertex_handle v,
	      Integrator &ig, 
	      double radius) 
{
  typedef typename RT::Point Point;
  typedef typename RT::Geom_traits::Vector_3 Vector;
  typedef typename RT::Edge Edge;
  typedef typename RT::Cell_handle Cell_handle;
  typedef typename RT::Vertex_handle Vertex_handle;
  
  Point A = v->point();
  
  // get all vertices incident to v
  std::list<Vertex_handle> vertices;
  rt.incident_vertices(v,std::back_inserter(vertices));
  
  RT small_rt;
  Vertex_handle small_v = small_rt.insert(A);

  typename std::list<Vertex_handle>::iterator it;
  for(it = vertices.begin(); it != vertices.end(); it++)
    {
      // build edge from two vertices
      Cell_handle cell;
      int i1,i2;
      
      if(!rt.is_edge(v, *it, cell, i1, i2))
	continue;
      
      // tesselate the polygon around its first vertex
      typename RT::Cell_circulator c = rt.incident_cells(cell, i1, i2);
      typename RT::Cell_circulator done = c++;
      
      small_rt.insert((*it)->point());
      while (c != done)
	{
	  const Point u (rt.dual(c)); c++;
	  const Point v (rt.dual(c));
	  std::vector<Vector> w;
	  
	  if (segment_sphere_intersect (radius, u - A, v - A,
					std::back_inserter(w)))
	    {
	      for (size_t i = 0; i < w.size(); ++i)
		  small_rt.insert (A + w[i] + w[i]);
	    }
	}
    }

  No_subdivider sub(radius);
  aggregate(small_rt, small_v, sub, ig);
}

template <class Integrator, class RT>
typename Integrator::Result_type
integrate_EX (const RT &rt,
	      typename RT::Vertex_handle v, 
	      double R)
{
  typedef typename RT::Geom_traits K;
  
  Integrator ig(v->point());
  aggregate_EX(rt, v, ig, R);
  
  return ig.result();
}

  }}


template <class RT, class OutputIterator>
void Build_regular_triangulation(std::istream &is, RT &rt,
                                 OutputIterator vertex_handles)
{
   typedef typename RT::Point Point;
   typedef typename RT::Vertex_handle Vertex_handle;

   double mx = -1e6, my = -1e6, mz = -1e6, 
          Mx = +1e6, My = +1e6, Mz = +1e6; 

   cloudy::Data_cloud points;
   cloudy::load_cloud(is, points);

   std::cerr << "Building Delaunay triangulation... \n";
   cloudy::misc::Progress_display progress(points.size(), std::cerr);
   boost::timer t;

   Vertex_handle b[8] = {rt.insert(Point(mx, my, mz)),
                         rt.insert(Point(mx, my, Mz)),
			 rt.insert(Point(mx, My, mz)),
			 rt.insert(Point(mx, My, Mz)),
			 rt.insert(Point(Mx, my, mz)),
			 rt.insert(Point(Mx, my, Mz)),
			 rt.insert(Point(Mx, My, mz)),
			 rt.insert(Point(Mx, My, Mz))};

   for (size_t i = 0; i < points.size(); ++i)
   {
      cloudy::uvector v = points[i];
      *vertex_handles++ = rt.insert(Point(v[0], v[1], v[2]));
      progress++;
   }

   std::cerr << "done in " << t.elapsed() << "s\n";
}


std::ostream &
operator << (std::ostream &os, const cloudy::offset::Covariance_vector &cov)
{
   for (size_t i = 0; i < cov.size(); ++i)
      os << cov[i] << " ";
   return os;
}

template <class Integrator, class RT,
          class Iterator>
void
Batch_integrate(const RT &rt, Iterator begin, Iterator end,
                double R, int cell, std::ostream &os)
{
   if (cell >= 0)
   {
      begin += cell;
      typename Integrator::Result_type res =
	 cloudy::offset::integrate_EX<Integrator> (rt, *begin, R);
      os << res << "\n";
      return;
   }

   std::cerr << "Integrating... \n";
   cloudy::misc::Progress_display progress(end - begin, std::cerr);
   boost::timer t;
   size_t i = 0;
   
   for (; begin != end; ++begin)
   {
      typename Integrator::Result_type res =
	 cloudy::offset::integrate_EX<Integrator> (rt, *begin, R);
      os << res << "\n";
      ++progress;
      //std::cerr << i << "\n"; ++i;
   }

   std::cerr << "done in " << t.elapsed() << "s\n";
}

enum IntegrationType 
  {
    INTEGRATION_COVARIANCE, 
    INTEGRATION_VOLUME,
    INTEGRATION_MESH
  };

void Process_all(std::istream &is,  std::ostream &os, 
		 IntegrationType type,
                 double R, int cell)
{
   typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
   typedef CGAL::Delaunay_triangulation_3<K> RT;
   typedef RT::Vertex_handle Vertex_handle;


   using namespace cloudy::offset;

   std::vector<Vertex_handle> vertices;
   RT rt;
   
   Build_regular_triangulation(is, rt, std::back_inserter(vertices));

   if (type == INTEGRATION_COVARIANCE)
   {
      Batch_integrate< Covariance_integrator<K> >
	 (rt, vertices.begin(), vertices.end(), R, cell, os);
   }
   else if (type == INTEGRATION_MESH)
   {
#if 1
     Batch_integrate< Mesh_integrator<K> >
       (rt, vertices.begin(), vertices.end(), R, cell, os);
#else
     Batch_integrate< Mesh_integrator<K> >
       (rt, vertices.begin(), vertices.end(), R, cell, os);
#endif
   }
   else
   {
      Batch_integrate< Volume_integrator<K> >
	 (rt, vertices.begin(), vertices.end(), R, cell, os);
   }

}

int main(int argc, char **argv)
{
   std::map<std::string, std::string> options;
   std::vector<std::string> param;
   cloudy::misc::get_options (argc, argv, options, param);

   IntegrationType type = INTEGRATION_VOLUME;
   if (options["type"] == "covariance")
     type = INTEGRATION_COVARIANCE;
   else if (options["type"] == "mesh")
     type = INTEGRATION_MESH;

   double R = cloudy::misc::to_double(options["R"], 0.1);
   int cell = cloudy::misc::to_int(options["N"], -1);

   if (param.size() == 1)
   {
      std::ifstream is(param[0].c_str());
      Process_all(is, std::cout, type, R, cell);
   }
   else if (param.size() == 2)
   {
      std::ifstream is(param[0].c_str());
      std::ofstream os(param[1].c_str());
      Process_all(is, os, type, R, cell);
   }
   else
      Process_all(std::cin, std::cout, type, R, cell);
}
