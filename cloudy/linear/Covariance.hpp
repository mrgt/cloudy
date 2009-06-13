#ifndef CLOUDY_COVARIANCE_HPP
#define CLOUDY_COVARIANCE_HPP

#include <cloudy/linear/Linear.hpp>
#include <cloudy/Cloud.hpp>

namespace cloudy
{
   namespace linear
   {
      umatrix matrix_from_covariance_3(const uvector v);
      bool
      covariance_extract_eigen(const umatrix &m, 
                               std::vector<double> &eig,
                               Data_cloud &directions);
      
      void 
      covariance_sort_eigen(std::vector<double> &eig,
                            Data_cloud &directions,
                            bool descending = true);
   }
}

#endif
