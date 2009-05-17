#include <cloudy/linear/Linear.hpp>

#include <boost/numeric/bindings/traits/ublas_matrix.hpp>
#include <boost/numeric/bindings/traits/ublas_vector.hpp>
#include <boost/numeric/bindings/traits/ublas_vector2.hpp>
#include <boost/numeric/bindings/lapack/syev.hpp>
#include <boost/numeric/bindings/lapack/sysv.hpp>
#include <boost/numeric/bindings/lapack/heev.hpp>
#include <boost/numeric/bindings/lapack/gesvd.hpp>
#include <boost/numeric/ublas/lu.hpp> 
#include <boost/numeric/ublas/io.hpp>


namespace cloudy
{
   namespace linear {
   namespace lapack = boost::numeric::bindings::lapack;

   bool
   sp_diagonalize(umatrix& A, uvector &eigenvalues)
   {
      const int ierr = lapack::syev('V', 'U', A, eigenvalues, 
				    lapack::optimal_workspace());
      
      if (ierr != 0)
      {
	 std::cerr << "lapack::syev gave error code " << ierr << ".\n";
	 return false;
      }
      
      return true;
   }
  

  // Solve Ax = b with A symmetric
  uvector
  sp_solve (const umatrix& A, const uvector &b)
  {
     umatrix Ap(A);
     uvector m(b);
     
     lapack::sysv('U', Ap, m);
     return m;
  }

   bool
   svd (const umatrix& A, 
	umatrix &U, uvector &S, umatrix &Vt)
   {
      umatrix Ap(A);
      return (lapack::gesvd(Ap, S, U, Vt) == 0);
   }

   }
}
