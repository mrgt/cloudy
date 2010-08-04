#ifndef CLOUDY_LINEAR_HPP
#define CLOUDY_LINEAR_HPP

#include <vector>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>
#define  CLOUDY_USE_LAPACK

namespace cloudy
{
   namespace ublas = boost::numeric::ublas;
   
   typedef ublas::vector<double> uvector;
   typedef ublas::matrix<double, ublas::row_major> umatrix;
   typedef ublas::compressed_vector<double> compressed_vector;
   typedef ublas::vector< ublas::coordinate_vector<double> > coordinate_vector;
   typedef ublas::compressed_matrix<double> compressed_matrix;

   typedef ublas::generalized_vector_of_vector <double,
						ublas::row_major,
						coordinate_vector >
               gvov_matrix;
  
  
   extern uvector cross_prod(uvector X, uvector Y);
   
   namespace linear{

#ifdef CLOUDY_USE_LAPACK
      // Lapack bindings
      bool sp_diagonalize(umatrix& A, uvector &eigenvalues);
      uvector sp_solve (const umatrix& A, const uvector &b);
      bool svd (const umatrix& A, umatrix &U, uvector &S, umatrix &Vt);
#endif
   }
}

#endif
