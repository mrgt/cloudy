// Copyright Ravikiran Rajagopal 2003.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef NUMPYREGISTER_RR2003_H
#define NUMPYREGISTER_RR2003_H

// Meta-programming magic to interface ublas and numpy

#include "numpy.hpp"

#include <boost/mpl/at.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/deref.hpp>
#include <boost/mpl/equal.hpp>
#include <boost/mpl/has_key.hpp>
#include <boost/mpl/next_prior.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/python/refcount.hpp>
#include <boost/python/to_python_converter.hpp>
#include <boost/python/to_python_value.hpp>
#include <boost/python/type_id.hpp>

#include <algorithm>
#include <complex>
#include <exception>
#include <memory>

#include <iostream>

extern "C" {
  typedef void ( *deleter_function_t )( void* );

  typedef struct
  {
    PyObject_HEAD
    void *smart_ptr;
    deleter_function_t deleter_function;
  } SmartPtrDeleter;

  inline void SmartPtrDeleter_dealloc( PyObject *self_ )
  {
    SmartPtrDeleter *self = ( SmartPtrDeleter * )self_;
    self->deleter_function( self->smart_ptr );
    self->ob_type->tp_free((PyObject *)self);
  }

  inline PyTypeObject *get_SmartPtrDeleterTypeObject()
  {
    static PyTypeObject SmartPtrDeleterTypeObject = {
      PyObject_HEAD_INIT(NULL)
      0,                           /*ob_size*/
      "smartptrdeallocator",       /*tp_name*/
      sizeof(SmartPtrDeleter),     /*tp_basicsize*/
      0,                           /*tp_itemsize*/
      SmartPtrDeleter_dealloc,     /*tp_dealloc*/
      0,                           /*tp_print*/
      0,                           /*tp_getattr*/
      0,                           /*tp_setattr*/
      0,                           /*tp_compare*/
      0,                           /*tp_repr*/
      0,                           /*tp_as_number*/
      0,                           /*tp_as_sequence*/
      0,                           /*tp_as_mapping*/
      0,                           /*tp_hash */
      0,                           /*tp_call*/
      0,                           /*tp_str*/
      0,                           /*tp_getattro*/
      0,                           /*tp_setattro*/
      0,                           /*tp_as_buffer*/
      Py_TPFLAGS_DEFAULT,          /*tp_flags*/
      "Internal smart pointer deallocator object", /* tp_doc */
    };
    return &SmartPtrDeleterTypeObject;
  }

  inline void import_smart_ptr_deleter_type()
  {
    static bool firstTime = true;
    if ( firstTime )
    {
      get_SmartPtrDeleterTypeObject()->tp_new = PyType_GenericNew;
      if ( PyType_Ready( get_SmartPtrDeleterTypeObject() ) < 0 )
        throw std::runtime_error( "Unable to initialize smart pointer deleter" );
      firstTime = false;
    }
  }

} // extern "C"

namespace numpy {

namespace detail {

template <bool copy, typename array_t>
struct pyarray_from_vector_impl;

// Create a numpy array which is a copy of an array of type array_t. The created numpy
// array has one dimension.
template <typename array_t>
struct pyarray_from_vector_impl< true, array_t>
{
  typedef typename array_t::value_type data_t;
  typedef typename boost::mpl::at<numpy_typemap, data_t>::type typenum_t;
  static PyObject* convert( const array_t &in )
  {
    // Create a new numpy array and copy the data into it.
    npy_intp dims[1]; dims[0] = in.size();
    PyObject *obj = PyArray_SimpleNew( 1, dims, typenum_t::value );
    data_t *data = static_cast<data_t*>( PyArray_DATA( obj ) );
    std::copy( in.begin(), in.end(), data ); // new arrays are contiguous
    return obj;
  }
};

// Create a numpy array which points to the data area of an array of type array_t. The
// created numpy array has one dimension. We assume that array_t types store data
// contiguously in memory starting from the location of the first data element.
template <typename array_t>
struct pyarray_from_vector_impl< false, array_t>
{
  typedef typename array_t::value_type data_t;
  typedef typename boost::mpl::at<numpy_typemap, data_t>::type typenum_t;
  static PyObject* convert( const array_t &in )
  {
    // Create a new numpy array from existing data.
    npy_intp dims[1]; dims[0] = in.size();
    data_t *data = const_cast<data_t*>( &in[0] ); // assume data is contiguous
    PyObject *obj = PyArray_SimpleNewFromData( 1, dims, typenum_t::value, data );
    return obj;
  }
};

// Register a to-python converter converting an array to a numpy array. The execute
// function registers the converter for the current data type and proceeds to the next
// one. This struct is intended to be used only from
// register_vector_to_python_converters().
//   from_type_impl<copy,array_t> should provide a static function called "convert"
//                                   which converts arrays of type array_t to python
//   array_traits<data_t> should provide a type called "type" that contains the array
//                        type for a data_t (data_t could be double, int, etc.)
template < bool copy, template<bool, class> class from_type_impl,
           template<class> class array_traits, typename map_iter >
struct register_pyarray_from_type_impl
{
  typedef typename boost::mpl::deref<map_iter>::type pair_t;
  typedef typename pair_t::first data_t;
  typedef typename array_traits<data_t>::type array_t;
  typedef from_type_impl<copy, array_t> impl;
  typedef typename boost::mpl::next<map_iter>::type next_iter;

  // Ensure that the type of elements held in the array is supported by numpy.
  typedef typename boost::mpl::has_key<numpy_typemap, data_t>::type is_supported;

  static void execute()
  {
    BOOST_MPL_ASSERT( is_supported );
    boost::python::to_python_converter< array_t, impl, true >();
    boost::python::to_python_converter< boost::shared_ptr<array_t>,
      impl, true >();
    register_pyarray_from_type_impl< copy, from_type_impl,
      array_traits, next_iter>::execute();
  }
};

// Stopping condition for the template recursion above.
template < bool copy, template<bool, class> class from_type_impl,
           template<class> class array_traits>
struct register_pyarray_from_type_impl< copy, from_type_impl,
                                          array_traits,
                                          boost::mpl::end<numpy_typemap>::type>
{
  static void execute() {}
};

template <typename T> struct orientation_t;
template <> struct orientation_t< boost::numeric::ublas::row_major_tag >
{
  BOOST_STATIC_CONSTANT( npy_intp, value = NPY_CARRAY );
};
template <> struct orientation_t< boost::numeric::ublas::column_major_tag >
{
  BOOST_STATIC_CONSTANT( npy_intp, value = NPY_FARRAY );
};

template <bool copy, typename matrix_t>
struct pyarray_from_matrix_impl;

template <typename matrix_t>
struct pyarray_from_matrix_impl<false, matrix_t>
{
  typedef typename matrix_t::value_type data_t;
  typedef typename boost::mpl::at<numpy_typemap, data_t>::type typenum_t;
  BOOST_STATIC_CONSTANT( npy_intp,
                         orient = orientation_t<typename matrix_t::
                           orientation_category>::value );

  static PyObject* convert( const matrix_t &in )
  {
    // Create a new numpy array from existing data.
    npy_intp dims[2]; dims[0] = in.size1(); dims[1] = in.size2();
    data_t *data = const_cast<data_t*>( &( in( 0, 0 ) ) ); // assume data is contiguous
    // Must use PyArray_New because PyArray_SimpleNew does not support column major
    // arrays.
    PyObject *obj = PyArray_New( &PyArray_Type, 2, dims, typenum_t::value, NULL,
                                 data, 0, orient, NULL );
    return obj;
  }
};

template <typename matrix_t>
struct pyarray_from_matrix_impl<true, matrix_t>
{
  typedef typename matrix_t::value_type data_t;
  typedef typename boost::mpl::at<numpy_typemap, data_t>::type typenum_t;
  BOOST_STATIC_CONSTANT( npy_intp,
                         orient = orientation_t<typename matrix_t::
                           orientation_category>::value );

  static PyObject* convert( const matrix_t &in )
  {
    // Create a new numpy array from existing data.
    npy_intp dims[2]; dims[0] = in.size1(); dims[1] = in.size2();
    // Must use PyArray_New because PyArray_SimpleNew does not support column major
    // arrays. However, because of a numpy bug, c-contiguous arrays do not work now.
    PyObject *obj = PyArray_New( &PyArray_Type, 2, dims, typenum_t::value, NULL,
                                 NULL, 0, orient, NULL );
    std::copy( in.data().begin(), in.data().end(),
               static_cast<data_t*>( PyArray_DATA( obj ) ) );
    return obj;
  }
};

template <typename T>
struct ublas_vector_from_numpy
{
  typedef numpy_storage_array<T> array_storage_t;
  typedef boost::numeric::ublas::vector<T, array_storage_t > array_t;
  typedef boost::python::converter::rvalue_from_python_storage<array_t> storage_t;
  typedef typename boost::mpl::at<numpy_typemap, T>::type typenum_t;

  static void* convertible( PyObject *obj )
  {
    // Check for numpy array and correct data type
    if ( !PyArray_Check( obj )
         || ( PyArray_TYPE( obj ) != typenum_t::value )
         || ( PyArray_NDIM( obj ) != 1 )
      )
      return 0;
    return obj;
  }

  static void construct( PyObject *obj,
                         boost::python::converter::rvalue_from_python_stage1_data* data )
  {
    storage_t *the_storage = reinterpret_cast<storage_t*>( data );
    void *memory_chunk = the_storage->storage.bytes;
    array_storage_t dd( obj );
    array_t *v = new ( memory_chunk ) array_t( dd.size(), dd );
    data->convertible = memory_chunk;
  }

  static void register_from_python_converter()
  {
    boost::python::converter::registry::push_back(
      &ublas_vector_from_numpy<T>::convertible,
      &ublas_vector_from_numpy<T>::construct,
      boost::python::type_id<array_t>()
      );
  }
};

template <typename map_iter>
struct register_ublas_from_python_converter_impl
{
  typedef typename boost::mpl::deref<map_iter>::type pair_t;
  typedef typename pair_t::first data_t;
  typedef ublas_vector_from_numpy<data_t> converter_t;
  typedef typename boost::mpl::next<map_iter>::type next_iter;

  static void execute()
  {
    converter_t::register_from_python_converter();
    register_ublas_from_python_converter_impl<next_iter>::execute();
  }
};

template <>
struct register_ublas_from_python_converter_impl<boost::mpl::end<numpy_typemap>::type>
{
  static void execute() {}
};

// Needed to handle C++ type erasure in C
template <typename T, template <class> class smart_ptr>
void delete_smart_ptr_from_chunk( void *ptr )
{
  smart_ptr<T> *p = static_cast<smart_ptr<T> *>( ptr );
  // std::cout << "Deleting array at: " << p->get() << std::endl;
  delete p;
}

template <typename T, template <class> class smart_ptr>
void hookup_smart_ptr_deleter( PyArrayObject *array, smart_ptr<T> const &in_ptr )
{
  // Fill up the deleter object
  SmartPtrDeleter *deleter = PyObject_New( SmartPtrDeleter, get_SmartPtrDeleterTypeObject() );
  smart_ptr<T> *p = new smart_ptr<T>( in_ptr );
  deleter->smart_ptr = p;
  deleter->deleter_function =
    &detail::delete_smart_ptr_from_chunk<T, smart_ptr>;

  // Now hook the deleter into the array object
  // std::cout << "Converting array at: " << p->get() << std::endl;
  PyArray_BASE( array ) = ( PyObject * )deleter;
}

} // namespace detail

// Class that converts contiguous vectors to numpy arrays using
// pyarray_from_vector_impl.
template< bool copy, typename vector>
struct contiguous_vector_to_py
{
  static PyObject *convert( const vector &v )
  {
    PyObject *array = detail::pyarray_from_vector_impl<copy, vector>::convert( v );
    // return boost::python::incref( array );
    return array;
  }

  static PyObject *convert( boost::shared_ptr<vector> const &in_ptr )
  {
    PyObject *arrayObj = convert( *in_ptr );
    detail::hookup_smart_ptr_deleter<vector, boost::shared_ptr>( ( PyArrayObject * )arrayObj,
                                                                 in_ptr );
    return arrayObj;
  }

  static PyTypeObject *get_pytype() { return &PyArray_Type; }
};

// Class that converts contiguous matrices to numpy arrays using
// pyarray_from_vector_impl.
template< bool copy, typename matrix>
struct contiguous_matrix_to_py
{
  static PyObject *convert( const matrix &v )
  {
    PyObject *array = detail::pyarray_from_matrix_impl<copy, matrix>::convert( v );
    // return boost::python::incref( array );
    return array;
  }

  static PyObject *convert( boost::shared_ptr<matrix> const &in_ptr )
  {
    PyObject *arrayObj = convert( *in_ptr );
    detail::hookup_smart_ptr_deleter<matrix, boost::shared_ptr>( ( PyArrayObject * )arrayObj,
                                                                 in_ptr );
    return arrayObj;
  }

  static PyTypeObject *get_pytype() { return &PyArray_Type; }
};

// Registers vector to-python converters; see the documentation for
// detail::register_pyarray_from_vector_impl for more information.
template < bool copy, template<bool, typename T> class from_type_impl,
           template<typename T> class array_traits>
void register_type_to_python_converters()
{
  detail::register_pyarray_from_type_impl< copy, from_type_impl,
    array_traits, boost::mpl::begin<numpy_typemap>::type>::execute();
}

// Traits class for the default ublas vector type.
template <typename data_t>
struct default_ublas_vector_traits
{
  typedef typename boost::numeric::ublas::vector<data_t> type;
};

// Traits class for the default ublas matrix type.
template <typename data_t>
struct default_ublas_matrix_traits
{
  typedef typename boost::numeric::ublas::matrix<data_t> type;
};
template <typename data_t>
struct default_ublas_matrix_traits_col_major
{
  typedef typename boost::numeric::ublas::matrix<
    data_t, boost::numeric::ublas::column_major> type;
};

// Convenience function to register default ublas array to-python converters.
inline void register_default_ublas_to_python( bool reg_col_major=false )
{
  register_type_to_python_converters< false, contiguous_vector_to_py,
    default_ublas_vector_traits>();
  register_type_to_python_converters< false, contiguous_matrix_to_py,
    default_ublas_matrix_traits>();
  if ( reg_col_major )
    register_type_to_python_converters< false, contiguous_matrix_to_py,
      default_ublas_matrix_traits_col_major>();
}

// Convenience function to register default ublas array from-python converters.
inline void register_ublas_from_python_converters()
{
  detail::register_ublas_from_python_converter_impl
    <boost::mpl::begin<numpy_typemap>::type>::execute();
}

} // namespace numpy

namespace boost {
namespace python {
namespace detail {

// Hack to get return by value working properly; definitely not the right way, but it
// works.

template <typename T, typename L, typename A>
struct registry_to_python_value< boost::numeric::ublas::matrix<T, L, A> const &>
{
  typedef typename boost::numeric::ublas::matrix<T, L, A> matrix_t;
  typedef typename value_arg< matrix_t const&>::type argument_type;

  PyObject* operator()(argument_type in) const
  {
    // Do we need the extra incref in contiguous_matrix_to_py or should we go directly
    // to the implementation in pyarray_from_matrix_impl to avoid the incref?
    return numpy::contiguous_matrix_to_py<true, matrix_t>::convert( in );
  }

#ifndef BOOST_PYTHON_NO_PY_SIGNATURES
  PyTypeObject const* get_pytype() const {
    return converter::registered<matrix_t const&>::converters.to_python_target_type();
  }
#endif
  BOOST_STATIC_CONSTANT(bool, uses_registry = true);
};

template <typename T, typename A>
struct registry_to_python_value< boost::numeric::ublas::vector<T, A> const &>
{
  typedef typename boost::numeric::ublas::vector<T, A> vector_t;
  typedef typename value_arg< vector_t const&>::type argument_type;

  PyObject* operator()(argument_type in) const
  {
    // Do we need the extra incref in contiguous_vector_to_py or should we go directly
    // to the implementation in pyarray_from_vector_impl to avoid the incref?
    return numpy::contiguous_vector_to_py<true, vector_t>::convert( in );
  }

#ifndef BOOST_PYTHON_NO_PY_SIGNATURES
  PyTypeObject const* get_pytype() const {
    return converter::registered<vector_t const&>::converters.to_python_target_type();
  }
#endif
  BOOST_STATIC_CONSTANT(bool, uses_registry = true);
};

}}} // namespace boost::python::detail


#endif
