#include <cloudy/linear/Covariance.hpp>
#include <cloudy/misc/Progress.hpp>
#include "pctviewer.hpp"
#include <fstream>

using namespace cloudy::linear;

bool setup(cloudy::view::Viewer &w,
           const std::map<std::string, std::string> &options,
           const std::vector<std::string> &parameters)
{
   Data_cloud_ptr cloud (new Data_cloud());
   Data_cloud_ptr covariance (new Data_cloud());

   if (parameters.size() < 2)
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

   std::cerr << "diagonalizing covariance matrices..\n";
   Data_cloud_ptr normals (new Data_cloud());
   Data_cloud_ptr K1 (new Data_cloud());
   Data_cloud_ptr K2 (new Data_cloud());
   Scalar_field_ptr anisotropy (new Scalar_field());

   cloudy::misc::Progress_display progress(cloud->size(), std::cerr);

   for (Data_cloud::iterator it = covariance->begin();
	it != covariance->end(); ++it)
   {
      std::vector<double> V;
      std::vector<uvector> D;

      covariance_extract_eigen(matrix_from_covariance_3(*it), V, D);
      covariance_sort_eigen(V, D);
      normals->push_back(D[0]);
      K1->push_back(D[1]);
      K2->push_back(D[2]);
      anisotropy->push_back(V[1]/(V[0] + V[1] + V[2]));
      ++progress;
   }
   std::cerr << "done\n";
   
   Cloud_drawer *c = new Cloud_drawer("Cloud", cloud, anisotropy);
   w.add_drawer(Drawer_ptr(c));
   w.add_drawer(Drawer_ptr(new Direction_drawer("Normals", *c, normals)));
   std::cerr << "added Normals, ";
   w.add_drawer(Drawer_ptr(new Direction_drawer("K1", *c, K1)));
   std::cerr << "K1, ";
   w.add_drawer(Drawer_ptr(new Direction_drawer("K2", *c, K2)));
   std::cerr << "K2\n";


   if (parameters.size() == 3)
     {
       std::string meshname = parameters[2];
       cloudy::view::Mesh_ptr mesh (new cloudy::Mesh());
       std::ifstream iso(meshname.c_str());

       std::cerr << "loading " << meshname << ".. "<< std::flush;
       mesh->read_off(iso);
       std::cerr << "done\n";
      
       w.add_drawer(Drawer_ptr(new Mesh_drawer(meshname, mesh)));
     }

   std::cerr << "initialization: done\n";

   return true;
}
