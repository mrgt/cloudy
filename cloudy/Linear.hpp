#ifndef CLOUDY_LINEAR_HPP
#define CLOUDY_LINEAR_HPP

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>

namespace cloudy
{
   namespace ublas = boost::numeric::ublas;

   typedef boost::numeric::ublas::vector<double> uvector;
   typedef ublas::matrix<double, ublas::row_major> umatrix;	
   
   namespace linear{

      // Lapack bindings
      bool sp_diagonalize(umatrix& A, uvector &eigenvalues);
      uvector sp_solve (const umatrix& A, const uvector &b);
      bool svd (const umatrix& A, umatrix &U, uvector &S, umatrix &Vt);
   }
}

#endif
