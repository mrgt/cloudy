#ifndef CLOUDY_KDTREE_HPP
#define CLOUDY_KDTREE_HPP

#include <ANN/ANN.h>
#include <cloudy/Cloud.hpp>
#include <iostream>

namespace cloudy
{
   class KD_tree
   {
         std::vector<ANNpoint> _points;
	 size_t _dim;
         ANNkd_tree* _tree;
	 
	 template <class V>
	 void
	 _insert (const V &v)
	 {
	    ANNpoint p =  new ANNcoord[_dim];

	    typename V::const_iterator it = v.begin();
	    for (size_t i = 0;
		 i < _dim && it != v.end(); ++i, ++it)
	       p[i] = *it;
	    _points.push_back(p);
	 }

      public:
	 uvector
	 operator [] (size_t idx) const
	 {
	    uvector v(_dim);
	    for (size_t i = 0; i < _dim; ++i)
	       v[i] = _points[idx][i];
	    return v;
	 }	 

	 size_t size() const
	 {
	    return _points.size();
	 }
	 
      public:
	 KD_tree (const Data_cloud &c) : _tree(NULL)
	 {
	    set_cloud(c);
	 }

	 KD_tree () : _dim(0), _tree(NULL)
	 {}
     
	 ~KD_tree ()
	 {
	    if (_tree)
	       delete _tree;
	 }

	 void set_cloud(const Data_cloud &c)
	 {
	    assert(c.size() != 0.0);
	    _dim = c[0].size();
	    _points.reserve(c.size());
	    
	    for (size_t i = 0; i < c.size(); ++i)
	       _insert(c[i]);

	    _tree = new ANNkd_tree(&_points.front(),
				   _points.size(), _dim);
	 }
     
	 template <class V>
	 void
	 find_knn(const V &p, size_t k,
		  std::vector<size_t> &indices, 
		  double eps = 0.0) const
	 {
	    assert (p.size() == _dim);
	    assert (_tree != NULL);
       
	    std::vector<double> query(_dim);
	    std::vector<int> iindices(k);
	    std::vector<double> squared_distances(k);
       
	    std::copy(p.begin(), p.end(), query.begin());
	    indices.resize(k);

	    _tree->annkSearch(&query.front(),
			      k, &iindices.front(),
			      &squared_distances.front(),
			      eps);
	    std::copy(iindices.begin(), iindices.end(), indices.begin());
	 }

	 size_t
	 find_nn(const uvector &p) const
	 {
	    std::vector<double> query(_dim);
	    std::vector<int> indices(1);
	    std::vector<double> squared_distances(1);
       
	    std::copy(p.begin(), p.end(), query.begin());
	    indices.resize(1);

	    _tree->annkSearch(&query.front(),
			      1, &indices.front(),
			      &squared_distances.front(), 0);
	    return indices[0];
	 }
     
	 void
	 find_points_in_ball(const uvector &p,
			     double r,
			     std::vector<size_t> &indices, 
			     double eps = 0.0) const
	 {
	    assert (p.size() == _dim);
	    assert (_tree != NULL);
       
	    std::vector<double> query(_dim);
	    std::copy(p.begin(), p.end(), query.begin());
	    size_t k = _tree->annkFRSearch
	       (&query.front(), r*r, 0, NULL, NULL, eps);   
       
	    std::vector<int> iindices(k);
	    indices.resize(k);
       
	    _tree->annkFRSearch(&query.front(), r*r,
				k, &iindices.front(),
				NULL, eps);
	    //	    std::cout << iindices.size() << std::endl;
	    std::copy(iindices.begin(), iindices.end(), indices.begin());
	 }
     
	 size_t
	 count_points_in_ball(const uvector &p,
			      double r,
			      double eps = 0.0) const
	 {	    
	    assert (p.size() == _dim);
	    assert (_tree != NULL);
       
	    std::vector<double> query(_dim);
	    std::copy(p.begin(), p.end(), query.begin());
	    return _tree->annkFRSearch
	       (&query.front(), r*r, 0, NULL, NULL, eps);   	    
	 }

	 size_t dim() const
	 {
	    return _dim;
	 }
   };
}

#endif
