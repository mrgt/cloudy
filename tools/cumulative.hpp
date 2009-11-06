#ifndef CUMULATIVE_HPP
#define CUMULATIVE_HPP

#include <vector>
#include <algorithm>
#include <iostream>

template < typename Sequence >
class ForwardCompare
{
  Sequence const &_sequence;

public:
  ForwardCompare (Sequence const & ref) : _sequence (ref)
  {}

  bool operator() (size_t a, size_t b) const
  {
    return (_sequence[a] < _sequence[b]);
  }
};

template <typename Sequence>
ForwardCompare <Sequence> make_forward (Sequence const &seq)
{
  return (ForwardCompare<Sequence>(seq));
}

void
cumulative_function(const std::vector<double> &radii, 
		    const std::vector<double> &weights,
		    std::vector<double> &ordradii,
		    std::vector<double> &ordvalues)
{
  ordradii.clear();
  ordvalues.clear();
  
  if (radii.size() == 0)
    return;

  // sort both radii and weights according to radii
  std::vector<int> index;
  for (int i = 0; i < radii.size(); ++i)
    index.push_back(i);

  std::sort(index.begin(), index.end(), make_forward(radii));

  // compute cumulative function 
  double curr = 0.0;
  double curv = 0.0;
  int cur = -1;

  for (size_t i = 0; i < index.size(); ++i)
  {
    size_t idx = index[i];
    
    curv += weights[idx];
    if (cur >= 0 && (curr == radii[idx]))
	ordvalues[cur] = curv;
    else
      {
	cur++;
	curr = radii[idx];
	ordradii.push_back(curr);
	ordvalues.push_back(curv);
      }
  }
}

#include <gsl/gsl_multifit.h>

void
linear_fit_quadratic(const std::vector<double> &Xin,
		     const std::vector<double> &Yin,
		     double &c_0, double &c_1, double &c_2)
{
  assert (Xin.size() == Yin.size());

  int i, n;
  double xi, yi, ei, chisq;
  gsl_matrix *X, *cov;
  gsl_vector *y, *c;
  
  n = Xin.size();     
  X = gsl_matrix_alloc (n, 3);
  y = gsl_vector_alloc (n);
  c = gsl_vector_alloc (3);
  cov = gsl_matrix_alloc (3, 3);
  
  for (i = 0; i < n; i++)
    {
      double xi = Xin[i], yi = Yin[i];
      gsl_matrix_set (X, i, 0, 1.0);
      gsl_matrix_set (X, i, 1, xi);
      gsl_matrix_set (X, i, 2, xi*xi);           
      gsl_vector_set (y, i, yi);
    }
  
  // do the actual fitting
  gsl_multifit_linear_workspace * work 
    = gsl_multifit_linear_alloc (n, 3);
  gsl_multifit_linear (X,  y, c, cov,
		       &chisq, work);
  gsl_multifit_linear_free (work);
  
  c_0 = gsl_vector_get(c,0);
  c_1 = gsl_vector_get(c,1);
  c_2 = gsl_vector_get(c,2);
  
  gsl_matrix_free (X);
  gsl_vector_free (y);
  gsl_vector_free (c);
  gsl_matrix_free (cov);
}

#endif
