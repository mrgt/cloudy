#include "pctviewer.hpp"
#include <fstream>

bool setup(cloudy::view::Viewer &w,
           const std::map<std::string, std::string> &options,
           const std::vector<std::string> &parameters)
{

  cloudy::view::Mesh_ptr mesh (new cloudy::Mesh());

   if (parameters.size() < 1)
   {
      std::cerr << "usage: pcvmesh file.off" << std::endl;
      return false;
   }

   std::ifstream is(parameters[0].c_str());
   mesh->read_off(is);

   w.add_drawer(Drawer_ptr(new Mesh_drawer(parameters[0], mesh)));
   return true;
}
