#ifndef CLOUDY_OFFSET_HPP
#define CLOUDY_OFFSET_HPP

#include <cloudy/offset/Subdividers.hpp>
#include <cloudy/offset/Integrators.hpp>
#include <cloudy/offset/Boundary.hpp>

namespace cloudy { namespace offset {     


      template <class Subdivider, class Integrator, class RT>
      typename Integrator::Result_type
      integrate (const RT &rt,
                 typename RT::Vertex_handle v, 
                 double R)
      {
	 typedef typename RT::Geom_traits K;

	 Subdivider sub(R);
	 Integrator ig(v->point());
	 aggregate(rt, v, sub, ig);

	 return ig.result();
      }

      template <class Subdivider, class RT>
      double
      volume (const RT &rt,
              typename RT::Vertex_handle v, 
              double R)
      {
	 typedef typename RT::Geom_traits K;
	 return integrate< Subdivider, Volume_integrator<K> > (rt, v, R);
      }

      template <class Subdivider, class RT>
      Covariance_vector
      covariance (const RT &rt,
                  typename RT::Vertex_handle v, 
                  double R)
      {
	 typedef typename RT::Geom_traits K;
	 return integrate< Subdivider, Covariance_integrator<K> > (rt, v, R);
      }


      template <class K, class Functor>
      class Triangle_converter_functor 
      {
	    Functor &_functor;
	    typedef typename K::Point_3 Point;
	    typedef typename K::Vector_3 Vector;

	 public:
	    Triangle_converter_functor(Functor &functor):
	       _functor(functor)
	    {}
	    

	    inline
	    void operator() (const Point &center, const Point &a,
	                     const Point &b, const Point &c)
	    {
	       _functor(a,b,c);
	    }
      };
#if 0
      template <class Subdivider, class RT, class Functor>
      void
      boundary_triangles(const RT&rt, 
                         typename RT::Vertex_handle v,
                         double R,
                         Functor ft)
      {
	 typedef typename RT::Geom_traits K;
	 typedef Triangle_converter_functor<K, Functor> TetFunctor;
	 TetFunctor tet_ft(ft);

	 Subdivider sub(R);
	 Tetrahedra_integrator<K, TetFunctor> ig(v->point(), tet_ft);
	 aggregate(rt, v, sub, ig);
      }

      template <class Subdivider, class RT, class Functor>
      void
      tetrahedra(const RT&rt, 
                 typename RT::Vertex_handle v,
                 double R,
                 Functor ft)
      {
	 typedef typename RT::Geom_traits K;

	 Subdivider sub(R);
	 Tetrahedra_integrator<K, Functor> ig(v->point(), ft);
	 aggregate(rt, v, sub, ig);
      }
#endif

   }

}

#endif
