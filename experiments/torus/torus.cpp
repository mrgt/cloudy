#include <cloudy/linear/Covariance.hpp>
#include <cloudy/misc/Progress.hpp>

#include "torus.hpp"
#include "pctviewer.hpp"

using namespace cloudy::view;
using namespace cloudy;

void diagonalize(const Data_cloud_ptr &covariance,
		 Data_cloud_ptr &normals,
		 Data_cloud_ptr &K1,
		 Data_cloud_ptr &K2,
		 Scalar_field_ptr &anisotropy)
{
  cloudy::misc::Progress_display progress(covariance->size(), std::cerr);
   for (Data_cloud::iterator it = covariance->begin();
        it != covariance->end(); ++it)
   {
      std::vector<double> V;
      std::vector<uvector> D;

      linear::covariance_extract_eigen
	(linear::matrix_from_covariance_3(*it), V, D);
      linear::covariance_sort_eigen(V, D);
      normals->push_back(D[0]);
      K1->push_back(D[1]);
      K2->push_back(D[2]);
      anisotropy->push_back(V[1]/(V[0] + V[1] + V[2]));
      ++progress;
   }
   std::cerr << "done\n";
}


bool setup(cloudy::view::Viewer &w,
           const std::map<std::string, std::string> &options,
           const std::vector<std::string> &parameters)
{
  using namespace cloudy::view;

  Torus torus(1.0, 0.2);

  Data_cloud_ptr cloud (new Data_cloud());
  Data_cloud_ptr covariance (new Data_cloud());

  assert(parameters.size() == 2);

  std::ifstream is(parameters[0].c_str());
  std::ifstream isP(parameters[1].c_str());
  
  std::cerr << "loading " << parameters[0] << ".. ";
  cloudy::load_cloud(is, *cloud);
  std::cerr << "done\n";
  
  std::cerr << "loading " << parameters[1] << ".. ";
  cloudy::load_cloud(isP, *covariance);

  assert(cloud->size() == covariance->size());

  std::cerr << "diagonalizing covariance matrices..\n";
  Data_cloud_ptr normals (new Data_cloud());
  Data_cloud_ptr K1 (new Data_cloud());
  Data_cloud_ptr K2 (new Data_cloud());
  Scalar_field_ptr anisotropy (new Scalar_field());
  diagonalize(covariance, normals, K1, K2, anisotropy);

  Data_cloud_ptr rnormals (new Data_cloud());
  double total_alpha (0.0);
  for (size_t i = 0; i < cloud->size(); ++i)
    {
      rnormals->push_back(torus.get_normal((*cloud)[i]));
      double alpha = acos( fabs(ublas::inner_prod((*rnormals)[i], (*normals)[i])) );
      total_alpha += alpha;
      //      std::cerr << alpha << "\n";
    }  
  std::cerr << "Average error: " << total_alpha/(cloud->size()) << "\n";
  
  Cloud_drawer *c = new Cloud_drawer("Cloud", cloud);
  w.add_drawer(Drawer_ptr(c));
  w.add_drawer(Drawer_ptr(new Direction_drawer("Normals", *c, normals)));
  w.add_drawer(Drawer_ptr(new Direction_drawer("Real Normals", *c, rnormals)));
   
   return true;
}
