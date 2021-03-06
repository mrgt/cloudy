#include "pctviewer.hpp"
#include <fstream>

bool setup(cloudy::view::Viewer &w,
           std::map<std::string, std::string> &options,
           std::vector<std::string> &parameters)
{
   Data_cloud_ptr cloud (new Data_cloud());

   if (parameters.size() < 1)
   {
      std::cerr << "usage: pcvcloud file.cloud" << std::endl;
      return false;
   }

   std::cerr << parameters[0] << "\n";
   std::ifstream is(parameters[0].c_str());
   cloudy::load_cloud(is, *cloud);

   Scalar_field_ptr weights;

   if (parameters.size() >= 2 && 
       parameters[1].find(".p") != std::string::npos)
     {
       std::ifstream fs(parameters[1].c_str());
       weights = Scalar_field_ptr(new std::vector<double>());
       cloudy::load_data<double>(fs, std::back_inserter(*weights));
     }
   
   w.add_drawer(Drawer_ptr(new Cloud_drawer("cloud", cloud, weights)));

   // temporary
   if (parameters.size() == 2 && 
       parameters[1].find(".off") != std::string::npos)
     {
       std::string meshname = parameters[1];
       cloudy::view::Mesh_ptr mesh (new cloudy::Mesh());
       std::ifstream iso(meshname.c_str());
       mesh->read_off(iso);
       
       w.add_drawer(Drawer_ptr(new Mesh_drawer(meshname, mesh)));
     }

   return true;
}
