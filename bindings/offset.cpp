#include "numpy.hpp"
#include "numpyregister.hpp"

#include <boost/python/def.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/module.hpp>
#include <boost/python/scope.hpp>
#include <boost/python/numeric.hpp>
#include <boost/python/return_value_policy.hpp>
#include <boost/python/return_by_value.hpp>
#include <boost/python/reference_existing_object.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/python.hpp>

////////////////////////////////////////////////////////////////

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Regular_triangulation_euclidean_traits_3.h> 
#include <CGAL/Regular_triangulation_3.h>

#include <cloudy/offset/Offset.hpp>
#include <vector>


typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Regular_triangulation_euclidean_traits_3<K> Traits;
typedef CGAL::Regular_triangulation_3<Traits> RT;

using namespace cloudy::offset;

bool
get_matrix_size(const boost::python::numeric::array &ar, 
                size_t &W, size_t &H)
{
   boost::python::object shape = ar.attr("shape");
   //FIXME
   //size_t len = boost::python::extract<size_t>(shape.attr("length"));
   
   //if (len != 2)
   //   return false;

   W = boost::python::extract<size_t>(shape[0]);
   H = boost::python::extract<size_t>(shape[1]);	 
   return true;
}

class _Regular_triangulation_3: public RT
{
   public:
      typedef RT::Vertex_handle Vertex_handle;
      typedef RT::Weighted_point Weighted_point;
      typedef RT::Bare_point Point;

   private:
      std::vector<Vertex_handle> _vertices;
      Vertex_handle _b[8];

      void __insert_breakers()
      {
	 double mx = -1e6, my = -1e6, mz = -1e6, 
	        Mx = +1e6, My = +1e6, Mz = +1e6; 
	 
	 _b[0] = RT::insert(Weighted_point(Point(mx, my, mz), 0.0));
	 _b[1] = RT::insert(Weighted_point(Point(mx, my, Mz), 0.0));
	 _b[2] = RT::insert(Weighted_point(Point(mx, My, mz), 0.0));
	 _b[3] = RT::insert(Weighted_point(Point(mx, My, Mz), 0.0));
	 _b[4] = RT::insert(Weighted_point(Point(Mx, my, mz), 0.0));
	 _b[5] = RT::insert(Weighted_point(Point(Mx, my, Mz), 0.0));
	 _b[6] = RT::insert(Weighted_point(Point(Mx, My, mz), 0.0));
	 _b[7] = RT::insert(Weighted_point(Point(Mx, My, Mz), 0.0));
      }

   public:
      _Regular_triangulation_3()
      {
	 __insert_breakers();
      }

      _Regular_triangulation_3(const boost::python::numeric::array &ar)
      {
	 size_t w, h;
	 if (!get_matrix_size(ar, w, h))
	    return;
	 std::cerr << w << " / " << h <<std::endl;

	 if (h != 3 && h != 4)
	    return;

	 __insert_breakers();
	 for (size_t i = 0; i < w; ++i)
	 {
	    std::vector<double> v(h);
	    for (size_t j = 0; j < h; ++j)
	       v[j] = boost::python::extract<double>
		             (ar[boost::python::make_tuple(i,j)]);
	    insert(v);
	 }
      }

      template<class Vector>
      size_t insert(Vector c)
      {
	 if (c.size() < 3)
	    return -1;

	 double weight = (c.size() > 3) ? c[3] : 0.0;
	 Weighted_point wp(Point(c[0], c[1], c[2]), weight);
	 
	 Vertex_handle vh = RT::insert(wp);
	 _vertices.push_back(vh);

	 return _vertices.size() - 1;
      }

      Vertex_handle vertex(size_t idx)
      {
	 return _vertices[idx];
      }

      size_t size()
      {
	 return _vertices.size();
      }
};


double
_volume (_Regular_triangulation_3 &rt, size_t idx, double R)
{
   _Regular_triangulation_3::Vertex_handle v = rt.vertex(idx);
   return volume<Clamp_subdivider>(rt, v, R);
}


Covariance_vector
_covariance (_Regular_triangulation_3 &rt, size_t idx, double R)
{
   _Regular_triangulation_3::Vertex_handle v = rt.vertex(idx);   
   return covariance<Clamp_subdivider>(rt, v, R);
}


BOOST_PYTHON_MODULE(cloudy)
{
  using namespace boost::python;

  import_array();
  import_smart_ptr_deleter_type();
  numeric::array::set_module_and_type("numpy", "ndarray"); 

  numpy::register_default_ublas_to_python( true );
  numpy::register_ublas_from_python_converters();
  typedef numpy::array_from_py<double>::type Numpy_array;

  class_<_Regular_triangulation_3>("Regular_triangulation",init<>())
     .def(init<numeric::array>())
     .def("insert", &_Regular_triangulation_3::insert<Numpy_array>)
     .def("size", &_Regular_triangulation_3::size);
  def("volume", &_volume);
  def("covariance", &_covariance);
}
