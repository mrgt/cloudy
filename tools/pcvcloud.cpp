#include "pctviewer.hpp"
#include <fstream>

bool setup(cloudy::view::Viewer &w,
           const std::map<std::string, std::string> &options,
           const std::vector<std::string> &parameters)
{

   Data_cloud_ptr cloud (new Data_cloud());

   if (parameters.size() != 1)
   {
      std::cerr << "usage: pcvcloud file.cloud" << std::endl;
      return false;
   }

   std::cerr << parameters[0] << "\n";
   std::ifstream is(parameters[0].c_str());
   cloudy::load_cloud(is, *cloud);

//    for (size_t i = 0; i < cloud->size(); ++i)
//    {
//       std::cerr << (*cloud)[i].size() << std::endl;
//    }
   
   w.add_drawer(Drawer_ptr(new Cloud_drawer("cloud", cloud)));
   return true;
}
