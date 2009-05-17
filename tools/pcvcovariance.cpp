#include <cloudy/linear/Covariance.hpp>
#include "pctviewer.hpp"
#include <fstream>

using namespace cloudy::linear;

bool setup(cloudy::view::Viewer &w,
           const std::map<std::string, std::string> &options,
           const std::vector<std::string> &parameters)
{
   Data_cloud_ptr cloud (new Data_cloud());
   Data_cloud_ptr covariance (new Data_cloud());

   if (parameters.size() != 2)
   {
      std::cerr << "usage: pcvcovariance file.cloud file.p" << std::endl;
      return false;
   }

   std::ifstream is(parameters[0].c_str());
   std::ifstream isP(parameters[1].c_str());

   std::cerr << "loading " << parameters[0] << ".. ";
   cloudy::load_cloud(is, *cloud);
   std::cerr << "done\n";

   std::cerr << "loading " << parameters[1] << ".. ";
   cloudy::load_cloud(isP, *covariance);

   assert(cloud->size() == covariance->size());

   if (covariance->size() == 0)
      return false;
   
   if ((*covariance)[0].size() < 6)
   {
      std::cerr << "pcvcovariance: the .p file should correspond to 3D "
		<< "covariance matrices " << std::endl;
      return false;
   }

   // Handle cloudy-old .p files   
   if ((*covariance)[0].size() == 7)
   {
      for (size_t i = 0; i < covariance->size(); ++i)
      {
	 uvector v(6);
	 std::copy((*covariance)[i].begin()+1, (*covariance)[i].end(),
	           v.begin());
	 (*covariance)[i] = v;
      }
   }
   std::cerr << "done\n";

   std::cerr << "diagonalizing covariance matrices.. ";
   Data_cloud_ptr normals (new Data_cloud());
   for (Data_cloud::iterator it = covariance->begin();
	it != covariance->end(); ++it)
   {
      std::vector<double> V;
      std::vector<uvector> D;

      covariance_extract_eigen(matrix_from_covariance_3(*it), V, D);
      covariance_sort_eigen(V, D);
      normals->push_back(D[0]);
   }
   std::cerr << " done\n";
   
   w.add_drawer(Drawer_ptr(new Cloud_drawer("Cloud", cloud)));
   w.add_drawer(Drawer_ptr(new Direction_drawer("Normals", cloud, normals)));
   return true;
}