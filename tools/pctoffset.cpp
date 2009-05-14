#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Regular_triangulation_euclidean_traits_3.h> 
#include <CGAL/Regular_triangulation_3.h>

#include <cloudy/misc/Program_options.hpp>
#include <cloudy/misc/Progress.hpp>
#include <cloudy/offset/Offset.hpp>

#include <boost/timer.hpp>
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

   Vertex_handle b[8] = {rt.insert(Weighted_point(Point(mx, my, mz), 0.0)),
                         rt.insert(Weighted_point(Point(mx, my, Mz), 0.0)),
			 rt.insert(Weighted_point(Point(mx, My, mz), 0.0)),
			 rt.insert(Weighted_point(Point(mx, My, Mz), 0.0)),
			 rt.insert(Weighted_point(Point(Mx, my, mz), 0.0)),
			 rt.insert(Weighted_point(Point(Mx, my, Mz), 0.0)),
			 rt.insert(Weighted_point(Point(Mx, My, mz), 0.0)),
			 rt.insert(Weighted_point(Point(Mx, My, Mz), 0.0))};

   std::vector<Weighted_point> points;
   while (1)
   {
      Point p; double w = 0.0;

      is >> p; //is >> w;
      if (is.eof() || !(is.good()))
	 break;

      points.push_back(Weighted_point(p,w));
   }

   boost::timer t;
   std::cerr << "Building Regular triangulation... ";
   rt.insert(points.begin(), points.end());
   std::cerr << "done in " << t.elapsed() << "s\n";

   for (size_t i = 0 ; i < points.size(); ++i)
      *vertex_handles++ = rt.nearest_power_vertex(points[i]);

   // FIXME: this should be faster:
   // rt.insert(points.begin(), points.end());
}

template <class Subdivider, class Integrator, class RT,
          class Iterator>
void
Batch_integrate(RT rt, Iterator begin, Iterator end,
                double R, std::ostream &os)
{
   boost::timer t;

   std::cerr << "Integrating... \n";
   cloudy::misc::Progress_display progress(end - begin, std::cerr);
   for (; begin != end; ++begin)
   {
      typename Integrator::Result_type res =
	 cloudy::offset::integrate<Subdivider, Integrator> (rt, *begin, R);
      ++progress;
   }
}

std::ostream &
operator << (std::ostream &os, const cloudy::offset::Covariance_vector &cov)
{
   for (size_t i = 0; i < cov.size(); ++i)
      os << cov[i] << " ";
   return os;
}

int main(int argc, char **argv)
{
   //
   std::map<std::string, std::string> options;
   std::vector<std::string> param;
   cloudy::misc::get_options (argc, argv, options, param);

   //

   typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
   typedef CGAL::Regular_triangulation_euclidean_traits_3<K> Traits;
   typedef CGAL::Regular_triangulation_3<Traits> RT;
   typedef RT::Vertex_handle Vertex_handle;


   using namespace cloudy::offset;

   std::vector<Vertex_handle> vertices;
   RT rt;
   
   Build_regular_triangulation(std::cin, rt, std::back_inserter(vertices));

   cloudy::offset::Covariance_vector test;

   bool do_volume = false;
   if (do_volume)
   {
      Batch_integrate< Clamp_subdivider, Volume_integrator<K> >
	 (rt, vertices.begin(), vertices.end(), 0.1, std::cout);
   }
   else
   {
      Batch_integrate< Clamp_subdivider, Covariance_integrator<K> >
	 (rt, vertices.begin(), vertices.end(), 0.1, std::cout);
   }
}
