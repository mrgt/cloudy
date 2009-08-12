#ifndef CLOUDY_CONVOLVE_HPP
#define CLOUDY_CONVOLVE_HPP

#include <cloudy/Cloud.hpp>
#include <cloudy/KD_tree.hpp>

namespace cloudy
{
   class Tent_function
   {
	 double _r;
      public:
	 Tent_function(double r = 0.0) : _r(r)
	 {}

	 inline 
	 double support_radius() const
	 {
	    return _r;
	 }

	 inline
	 double operator () (const uvector &v) const
	 {
	    return std::max(1.0 - ublas::norm_2(v)/_r, 0.0);
	 }
   };

   class Uniform_function
   {
	 double _r;
      public:
	 Uniform_function(double r) : _r(r)
	 {}

	 inline
	 double support_radius() const
	 {
	    return _r;
	 }

	 inline
	 double operator () (const uvector &v) const
	 {
	    if (ublas::norm_2(v) <= _r)
	       return 1.0;
	    else
	       return 0.0;
	 }
   };

  template <class Type, class Function>
  class Convolution_functor
  {
    const KD_tree &_kd;
    const std::vector<Type> &_field;
    const Function &_f;

  public:
    Convolution_functor(const KD_tree &kd,
			const std::vector<Type> &input,
			const Function &f):  _kd(kd),
					     _field(input),
					     _f(f)
    {}

    Type operator() (const uvector &position) const
    {
      std::vector<size_t> indices;
      
      _kd.find_points_in_ball(position, _f.support_radius(),
			      indices);
      
      Type res = _field[indices[0]];
      for (size_t j = 1; j < indices.size(); ++j)
	res += _f(_kd[indices[j]]) * _field[indices[j]];      
      return res;
    }
  };

  template<class Type>
  class Convolution_uniform_functor:
    public Convolution_functor<Type, Uniform_function>
  {
    Uniform_function _realf;
  public:
    Convolution_uniform_functor(const KD_tree &kd,
				const std::vector<Type> &input,
				double r):
      Convolution_functor<Type, Uniform_function>(kd, input, _realf),
      _realf(r)
    {
    }
  };

   template <class Type, class Function> 
   void convolve(const KD_tree &kd,
		 const std::vector<Type> &input,
                 std::vector<Type> &output,
		 const Function &f)
   {
      assert(input.size() == kd.size());
      output.resize(input.size());

      for (size_t i = 0; i < kd.size(); ++i)
      {
	 std::vector<size_t> indices;

	 kd.find_points_in_ball(kd[i],
				f.support_radius(),
				indices);

	 output[i] = input[indices[0]];
	 for (size_t j = 1; j < indices.size(); ++j)
	    output[i] += f(kd[indices[j]]) * input[indices[j]];
      }
   }

   template <class Type>    
   void convolve_uniform(const KD_tree &kd,
                         const std::vector<Type> &input,
                         std::vector<Type> &output,
                         double R)
   {
      convolve<Type>(kd, input, output, Uniform_function(R));
   }
}

#endif

