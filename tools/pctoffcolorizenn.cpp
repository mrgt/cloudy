#include <cloudy/mesh/Mesh.hpp>
#include <cloudy/misc/Program_options.hpp>
#include <cloudy/misc/Progress.hpp>
#include <cloudy/Cloud.hpp>
#include <cloudy/Convolve.hpp>

#include <boost/timer.hpp>
#include <fstream>
#include <vector>
#include <map>

using namespace cloudy;

void Process_all(std::istream &isCloud,
                 std::istream &isField, 		
		 std::istream &isGradient, 
                 std::istream &isOff, 
                 std::ostream &os,
		 size_t comp = 0,
		 size_t clamp = 3)
{
  std::cerr << "loading cloud\n";
  cloudy::Data_cloud points;
  cloudy::load_cloud(isCloud, points);
  for (size_t i = 0; i < points.size(); ++i)
    {
      points[i].resize(clamp);
    }

  std::cerr << "loading field\n";
  cloudy::Data_cloud ifield;
  cloudy::load_cloud(isField, ifield);
  
  std::vector<double> field;
  for (size_t i = 0; i < ifield.size(); ++i)
    field.push_back(ifield[i][comp]);

  std::cerr << "loading OFF\n";
  cloudy::Mesh mesh;
  mesh.read_off(isOff);
  
  std::cerr << "loading gradient\n";
  cloudy::Gradient ggr;
  ggr.read_ggr(isGradient);

  if(field.size() != points.size())
    {
      std::cerr << "The two files should be of the same size\n";
      return;
    }
  
  if (points.size() == 0)
    return;
  
  cloudy::KD_tree kd(points);
  cloudy::Nearest_neighbor_functor<double> f(kd, field);
  
  mesh.simple_colorize(f, ggr, true);
  mesh.write_off(os);
}

int main(int argc, char **argv)
{
   std::map<std::string, std::string> options;
   std::vector<std::string> param;
   cloudy::misc::get_options (argc, argv, options, param);

   size_t clamp = cloudy::misc::to_double(options["tmax"], 3);
   size_t comp = cloudy::misc::to_int(options["comp"], 0);
   

   if (param.size() < 2)
   {
      std::cerr << "Usage: " << argv[0] << " in.cloud in.p file.ggr in.off [out.off] [-comp function component in .p] [-clamp number of position coordinates in .cloud]]"
		<< std::endl;
      return -1;
   }

   std::ifstream isCloud(param[0].c_str());
   std::ifstream isField(param[1].c_str());
   std::ifstream isGradient(param[2].c_str());
   std::ifstream isOff(param[3].c_str());
   
   if (param.size() == 5)
   {
     std::cerr << "outputing in " << param[4] << "\n";
      std::ofstream os(param[4].c_str());
      Process_all(isCloud, isField, isGradient, isOff, os, comp, clamp);
   }
   else
     Process_all(isCloud, isField, isGradient, isOff, std::cout, comp, clamp);
}
