#include <cloudy/linear/Covariance.hpp>
#include <CGAL/linear_least_squares_fitting_3.h>


#ifdef CLOUDY_USE_LAPACK
#include <boost/numeric/ublas/matrix_proxy.hpp>
#endif

namespace cloudy
{
   namespace linear
   {
#ifdef CLOUDY_USE_LAPACK
      bool
      covariance_extract_eigen(const umatrix &m, 
                               std::vector<double> &eig,
                               Data_cloud &directions)
      {
	 umatrix M (m);
	 uvector eigvalues(m.size1());

	 if (!sp_diagonalize(M, eigvalues))
	    return false;
      
	 std::copy(eigvalues.begin(), eigvalues.end(),
	           std::back_inserter(eig));

	 directions.resize(M.size1());
	 for (size_t i = 0; i < M.size1(); ++i)
	    directions[i] = ublas::row(M,i);
	 return true;
      }

#else
      bool
      covariance_extract_eigen(const umatrix &m, 
			       std::vector<double> &eig,
			       Data_cloud &directions)
      {
	size_t dim = m.size1();
	size_t nentries = dim*(dim+1)/2;

	double *M = new double(nentries);
	size_t cur = 0;

	for (size_t i = 0; i < dim; ++i)
	  {
	    for (size_t j = i; j < dim; ++j)
	      {
		M[cur] = m(i,j);
		cur++;
	      }
	  }
	
	double *eigen_vectors = new double(dim*dim);
	double *eigen_values = new double(dim);

	CGAL::CGALi::eigen_symmetric<double>
	  (M, dim, eigen_vectors, eigen_values);

	for (size_t i = 0; i < dim; ++i)
	  {
	    uvector v(dim);
	    for (size_t j = 0; j < dim; ++j)
	      v[j] = eigen_vectors[dim*i+j];
	    directions.push_back(v);
	    eig.push_back(eigen_values[i]);
	  }
	delete M;
	delete eigen_vectors; 
	delete eigen_values;
      }
#endif


      umatrix matrix_from_covariance_3(const uvector v)
      {
	 umatrix m(3,3);
	 m(0,0) = v(0); m(0,1) = v(1); m(0,2) = v(2);
	 m(1,1) = v(3); m(1,2) = v(4);
	 m(2,2) = v(5);
	 
	 // fill symmetric entries
	 m(1, 0) = m(0,1); m(2, 0) = m(0,2); m(2,1) = m(1,2);
	 return m;
      }

     void
     covariance_add_vector(umatrix &m,
			   const uvector &v)
     {
       const size_t dim = v.size();

       for (size_t j = 0; j < dim; ++j)
	 for (size_t k = 0; k < dim; ++k)
	   m(j,k) += v[j] * v[k];        
     }


      struct eigen
      {
	    double value;
	    uvector direction;
	    
	 public:
	    eigen (double v = 0.0, uvector d = ublas::zero_vector<double>())
	       : value(v), direction(d) 
	    {}
      };

      class decreasing_eigenvalues
      {
	 public:
	    bool operator ()(const eigen &e1, const eigen &e2)
	    {
	       return e1.value > e2.value;
	    }
      };
      
      class increasing_eigenvalues
      {
	 public:
	    bool operator ()(const eigen &e1, const eigen &e2)
	    {
	       return e1.value < e2.value;
	    }
      };


      void 
      covariance_sort_eigen(std::vector<double> &eig,
                            Data_cloud &directions,
                            bool descending)
      {
	 std::vector<eigen> eigs;
	 
	 for (size_t i = 0; i < eig.size(); ++i)
	    eigs.push_back(eigen(eig[i], directions[i]));

	 if (descending)
	    sort(eigs.begin(), eigs.end(), decreasing_eigenvalues());
	 else
	    sort(eigs.begin(), eigs.end(), increasing_eigenvalues());

	 for (size_t i = 0; i < eig.size(); ++i)
	 {
	    eig[i] = eigs[i].value;
	    directions[i] = eigs[i].direction;
	 }
      }
   }
}
