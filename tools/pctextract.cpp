#include <cloudy/misc/Program_options.hpp>
#include <cloudy/misc/Progress.hpp>
#include <cloudy/Cloud.hpp>
#include <cloudy/mesh/Mesh.hpp>

#include <boost/timer.hpp>
#include <fstream>
#include <vector>
#include <map>

using namespace cloudy;

void Process_all(size_t N,
                 std::istream &isOff, 
                 std::ostream &os)
{
  cloudy::Mesh mesh;
  mesh.read_off(isOff);
  
  cloudy::Data_cloud points;
  mesh.uniform_sample(points, N);

  write_cloud(os, points);
}

int main(int argc, char **argv)
{
   std::map<std::string, std::string> options;
   std::vector<std::string> param;

   cloudy::misc::get_options (argc, argv, options, param);
   size_t N = cloudy::misc::to_unsigned(options["N"], 50000);


   if (param.size() < 1)
   {
      std::cerr << "Usage: " << argv[0] << " file.off [outfile.cloud -N num]"
		<< std::endl;
      return -1;
   }

   std::ifstream isOff(param[0].c_str());
    
   if (param.size() == 2)
   {
      std::ofstream os(param[1].c_str());
      Process_all(N, isOff, os);
   }
   else
      Process_all(N, isOff, std::cout);
}
