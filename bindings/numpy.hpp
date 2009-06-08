// Copyright Ravikiran Rajagopal 2003.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef NUMPY_RR2003_H
#define NUMPY_RR2003_H

// Meta-programming magic to interface ublas and numpy

#include <boost/iterator/iterator_facade.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/map.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/python/refcount.hpp>
#include <boost/type_traits/add_const.hpp>
#include <boost/type_traits/add_reference.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/utility/enable_if.hpp>

#include <algorithm>
#include <complex>
#include <exception>
#include <memory>

//#define NO_IMPORT_ARRAY
#define PY_ARRAY_UNIQUE_SYMBOL Wicket_Numpy
#include <numpy/arrayobject.h>

namespace numpy {

// Create a type map for numpy supported types
typedef boost::mpl::map<
  boost::mpl::pair< short,                       boost::mpl::int_<PyArray_SHORT> >
  , boost::mpl::pair< int,                       boost::mpl::int_<PyArray_INT> >
  , boost::mpl::pair< long,                      boost::mpl::int_<PyArray_LONG> >
  , boost::mpl::pair< long long,                 boost::mpl::int_<PyArray_LONGLONG> >
  , boost::mpl::pair< unsigned short,            boost::mpl::int_<PyArray_USHORT> >
  , boost::mpl::pair< unsigned int,              boost::mpl::int_<PyArray_UINT> >
  , boost::mpl::pair< unsigned long,             boost::mpl::int_<PyArray_ULONG> >
  , boost::mpl::pair< unsigned long long,        boost::mpl::int_<PyArray_ULONGLONG> >
  , boost::mpl::pair< float,                     boost::mpl::int_<PyArray_FLOAT> >
  , boost::mpl::pair< double,                    boost::mpl::int_<PyArray_DOUBLE> >
  , boost::mpl::pair< long double,               boost::mpl::int_<PyArray_LONGDOUBLE> >
  , boost::mpl::pair< std::complex<float>,       boost::mpl::int_<PyArray_CFLOAT> >
  , boost::mpl::pair< std::complex<double>,      boost::mpl::int_<PyArray_CDOUBLE> >
  , boost::mpl::pair< std::complex<long double>, boost::mpl::int_<PyArray_CLONGDOUBLE> >
  > numpy_typemap;

namespace detail {

// Storage shared with a numpy array
template<class T>
class numpy_storage_array:
  public boost::numeric::ublas::storage_array<numpy_storage_array<T> >
{
  typedef numpy_storage_array<T> self_type;

  template < class Value >
  class simple_iter
    : public boost::iterator_facade< simple_iter<Value>,
                                     Value,
                                     boost::random_access_traversal_tag,
                                     typename boost::add_reference<Value>::type,
                                     npy_intp >
  {
  private:
    struct enabler {};
    typedef typename boost::mpl::if_<typename boost::is_const<Value>::type,
                                     const numpy_storage_array<T>,
                                     numpy_storage_array<T>
                                     >::type array_type;

  public:
    explicit simple_iter( array_type *a, npy_intp indx = 0 )
      : array( a ), index( indx ) {}
    template <class OtherValue>
    simple_iter( simple_iter<OtherValue> const &other,
                 typename boost::enable_if<
                 boost::is_convertible< OtherValue*, Value* >, enabler
                 >::type = enabler() )
      : array( other.array ), index( other.index ) {}

  private:
    friend class boost::iterator_core_access;
    template <class> friend class simple_iter;

    void increment() { ++index; }
    void decrement() { --index; }
    template <class OtherValue>
    bool equal( simple_iter<OtherValue> const &other ) const
    {
      return index == other.index;
    }
    Value &dereference() const { return ( *array )[index]; }
    void advance( npy_intp n ) { index += n; }
    template <class OtherValue>
    npy_intp distance_to( simple_iter<OtherValue> const &other ) const
    {
      return other.index - index;
    }
  private:
    array_type *array;
    npy_intp index;
  };

public:
  typedef typename std::allocator<T>::size_type size_type;
  typedef typename std::allocator<T>::difference_type difference_type;
  typedef T value_type;
  typedef const T &const_reference;
  typedef T &reference;
  typedef const T *const_pointer;
  typedef T *pointer;
  typedef simple_iter<typename boost::add_const<T>::type> const_iterator;
  typedef simple_iter<T> iterator;

private:
  // No default constructor; must initialize object using a numpy array!
  explicit BOOST_UBLAS_INLINE numpy_storage_array();

public:
  // Construction and destruction
  explicit BOOST_UBLAS_INLINE numpy_storage_array( PyObject *arr )
    : boost::numeric::ublas::storage_array<numpy_storage_array<T> >()
  {
    if ( !PyArray_Check( arr )
         || ( PyArray_TYPE( arr ) != boost::mpl::at<numpy_typemap, T>::type::value )
         || ( PyArray_NDIM( arr ) != 1 )
         )
      throw std::bad_alloc();

    array_ = ( PyArrayObject * )arr;
    // Get a reference to the right sort of array
#ifdef NUMPY_STORAGE_ARRAY_MUST_BE_CONTIGUOUS
    array_ = PyArray_GETCONTIGUOUS( array_ );
#else
    Py_INCREF( array_ );
#endif
  }

  BOOST_UBLAS_INLINE numpy_storage_array(const numpy_storage_array &c)
    : boost::numeric::ublas::storage_array<numpy_storage_array<T> >(),
    array_( c.array_ )
  {
    Py_INCREF( array_ );
  }

  BOOST_UBLAS_INLINE ~numpy_storage_array()
  {
    Py_DECREF( array_ );
  }

  BOOST_UBLAS_INLINE void resize( size_type size_a )
  {
    if ( size_a != size() )
    {
      npy_intp newsize = size_a;
      PyArray_Dims newshape = { 1, &newsize };
      PyObject *dummy = PyArray_Resize( array_, &newshape, 1, NPY_ANYORDER );
      if ( dummy == 0 )
        throw std::bad_alloc();
      Py_DECREF( dummy );
      // Note that all iterators are invalid at this point, i.e., iterators of other
      // arrays which refer to this underlying storage could also be invalid!
    }
  }

  // Random Access Container
  BOOST_UBLAS_INLINE size_type size() const { return PyArray_SIZE( array_ ); }
  BOOST_UBLAS_INLINE size_type max_size() const { return size(); }
  BOOST_UBLAS_INLINE bool empty() const { return size() == 0; }

  // Element access
  BOOST_UBLAS_INLINE const_reference operator[](size_type i) const
  {
    BOOST_UBLAS_CHECK( i < size(), boost::numeric::ublas::bad_index() );
    return *( static_cast<T*>( PyArray_GETPTR1( array_, i ) ) );
  }

  BOOST_UBLAS_INLINE reference operator[](size_type i)
  {
    BOOST_UBLAS_CHECK( i < size(), boost::numeric::ublas::bad_index() );
    return *( static_cast<T*>( PyArray_GETPTR1( array_, i ) ) );
  }

  // Assignment
  BOOST_UBLAS_INLINE numpy_storage_array &operator=( const numpy_storage_array &a )
  {
    if ( this != &a )
    {
      resize( a.size() );
      std::copy( a.begin(), a.end(), begin() );
    }
    return *this;
  }
  BOOST_UBLAS_INLINE numpy_storage_array &assign_temporary( numpy_storage_array &a )
  {
    swap( a );
    return *this;
  }

  // Swapping
  BOOST_UBLAS_INLINE void swap( numpy_storage_array &a )
  {
    if ( this != &a )
    {
      std::swap( array_, a.array_ );
    }
  }

  BOOST_UBLAS_INLINE friend void swap( numpy_storage_array &a1, numpy_storage_array &a2 )
  {
    a1.swap( a2 );
  }

  BOOST_UBLAS_INLINE const_iterator begin() const
  {
    return const_iterator( this, 0 );
  }
  BOOST_UBLAS_INLINE const_iterator end() const
  {
    return const_iterator( this, size() );
  }

  BOOST_UBLAS_INLINE iterator begin()
  {
    return iterator( this, 0 );
  }
  BOOST_UBLAS_INLINE iterator end()
  {
    return iterator( this, size() );
  }

  // Reverse iterators
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;

  BOOST_UBLAS_INLINE const_reverse_iterator rbegin() const
  {
    return const_reverse_iterator( end() );
  }
  BOOST_UBLAS_INLINE const_reverse_iterator rend() const
  {
    return const_reverse_iterator( begin() );
  }
  BOOST_UBLAS_INLINE reverse_iterator rbegin()
  {
    return reverse_iterator( end() );
  }
  BOOST_UBLAS_INLINE reverse_iterator rend()
  {
    return reverse_iterator( begin() );
  }

  // Underlying array object
  PyArrayObject *get_array() const { return array_; }

#if 0
private:
  friend class boost::serialization::access;

  // Serialization
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    serialization::collection_size_type s(size_);
    ar & serialization::make_nvp("size",s);
    if ( Archive::is_loading::value ) {
      resize(s);
    }
    ar & serialization::make_array(data_, s);
  }
#endif

private:
  PyArrayObject *array_;
}; // numpy_storage_array

} // namespace detail

template <typename data_t>
struct array_from_py
{
  typedef typename
  boost::numeric::ublas::vector< data_t, detail::numpy_storage_array<data_t> > type;
};

template <typename data_t>
struct const_array_from_py
{
private:
  typedef typename array_from_py<data_t>::type o_type;
public:
  typedef typename boost::add_const<o_type>::type type;
};

} // namespace numpy

#endif
