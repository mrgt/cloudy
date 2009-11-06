#include <cloudy/misc/Program_options.hpp>
#include <cloudy/misc/Progress.hpp>
#include <cloudy/Cloud.hpp>
#include <cloudy/mesh/Mesh.hpp>

#include <boost/timer.hpp>
#include <fstream>
#include <vector>
#include <map>

using namespace cloudy;

void Process_all(double R,
                 std::istream &isOff, 
                 std::ostream &os)
{
  cloudy::Mesh mesh;
  mesh.read_off(isOff);
  mesh.normalize(R);
  mesh.write_off(os);
}

int main(int argc, char **argv)
{
   std::map<std::string, std::string> options;
   std::vector<std::string> param;

   cloudy::misc::get_options (argc, argv, options, param);
   size_t R = cloudy::misc::to_unsigned(options["R"], 1.0);


   if (param.size() < 1)
   {
      std::cerr << "Usage: " << argv[0] << " file.off [outfile.off -R radius]"
		<< std::endl;
      return -1;
   }

   std::ifstream isOff(param[0].c_str());
    
   if (param.size() == 2)
   {
      std::ofstream os(param[1].c_str());
      Process_all(R, isOff, os);
   }
   else
      Process_all(R, isOff, std::cout);
}
