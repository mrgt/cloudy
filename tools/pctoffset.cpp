#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Regular_triangulation_euclidean_traits_3.h> 
#include <CGAL/Regular_triangulation_3.h>

#include <cloudy/misc/Program_options.hpp>
#include <cloudy/misc/Progress.hpp>
#include <cloudy/offset/Offset.hpp>
#include <cloudy/Cloud.hpp>

#include <boost/timer.hpp>
#include <fstream>
#include <vector>
#include <map>

///////////////////////////////////////////////////////////////

template <class RT, class OutputIterator>
void Build_regular_triangulation(std::istream &is, RT &rt,
                                 OutputIterator vertex_handles)
{
   typedef typename RT::Weighted_point Weighted_point;
   typedef typename RT::Bare_point Point;
   typedef typename RT::Vertex_handle Vertex_handle;

   double mx = -1e6, my = -1e6, mz = -1e6, 
          Mx = +1e6, My = +1e6, Mz = +1e6; 

   cloudy::Data_cloud points;
   cloudy::load_cloud(is, points);

   std::cerr << "Building Regular triangulation... \n";
   cloudy::misc::Progress_display progress(points.size(), std::cerr);
   boost::timer t;

   Vertex_handle b[8] = {rt.insert(Weighted_point(Point(mx, my, mz), 0.0)),
                         rt.insert(Weighted_point(Point(mx, my, Mz), 0.0)),
			 rt.insert(Weighted_point(Point(mx, My, mz), 0.0)),
			 rt.insert(Weighted_point(Point(mx, My, Mz), 0.0)),
			 rt.insert(Weighted_point(Point(Mx, my, mz), 0.0)),
			 rt.insert(Weighted_point(Point(Mx, my, Mz), 0.0)),
			 rt.insert(Weighted_point(Point(Mx, My, mz), 0.0)),
			 rt.insert(Weighted_point(Point(Mx, My, Mz), 0.0))};

   for (size_t i = 0; i < points.size(); ++i)
   {
      cloudy::uvector v = points[i];
      if (v.size() < 4)
      {
	 cloudy::uvector w(4);
	 w(0) = v(0); w(1) = v(1);
	 w(2) = v(2); w(3) = 0.0;
	 v = w;
      }
      //std::cerr << v(0) << " " << v(1) << " " << v(2) << " " << v(3)
      //<< "\n";

      Weighted_point wp(Point(v[0], v[1], v[2]), v[3]);
      *vertex_handles++ = rt.insert(wp);
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

template <class Subdivider, class Integrator, class RT,
          class Iterator>
void
Batch_integrate(const RT &rt, Iterator begin, Iterator end,
                double R, int cell, std::ostream &os)
{
   if (cell >= 0)
   {
      begin += cell;
      typename Integrator::Result_type res =
	 cloudy::offset::integrate<Subdivider, Integrator> (rt, *begin, R);
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
	 cloudy::offset::integrate<Subdivider, Integrator> (rt, *begin, R);
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
   typedef CGAL::Regular_triangulation_euclidean_traits_3<K> Traits;
   typedef CGAL::Regular_triangulation_3<Traits> RT;
   typedef RT::Vertex_handle Vertex_handle;


   using namespace cloudy::offset;

   std::vector<Vertex_handle> vertices;
   RT rt;
   
   Build_regular_triangulation(is, rt, std::back_inserter(vertices));

   if (type == INTEGRATION_COVARIANCE)
   {
      Batch_integrate< Clamp_subdivider, Covariance_integrator<K> >
	 (rt, vertices.begin(), vertices.end(), R, cell, os);
   }
   else if (type == INTEGRATION_MESH)
   {
#if 0
     Batch_integrate< Tesselate_subdivider, Mesh_integrator<K> >
       (rt, vertices.begin(), vertices.end(), R, cell, os);
#else
     Batch_integrate< Clamp_subdivider, Mesh_integrator<K> >
       (rt, vertices.begin(), vertices.end(), R, cell, os);
#endif
   }
   else
   {
      Batch_integrate< Clamp_subdivider, Volume_integrator<K> >
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
