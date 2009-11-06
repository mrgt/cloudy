#include "pctviewer.hpp"
#include <fstream>

void setup(cloudy::view::Viewer &w,
           const std::map<std::string, std::string> &options,
           const std::vector<std::string> &parameters)
{

   Data_cloud_ptr cloud (new Data_cloud());

   if (parameters.size() == 1)
      cloudy::load_cloud(std::cin, *cloud);
   else
   {
      std::ifstream is(parameters[0].c_str());
      cloudy::load_cloud(is, *cloud);
   }
   
   w.add_drawer(Drawer_ptr(new Cloud_drawer(cloud)));
}
