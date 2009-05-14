#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Regular_triangulation_euclidean_traits_3.h> 
#include <CGAL/Regular_triangulation_3.h>

#include <cloudy/misc/Program_options.hpp>
#include <cloudy/misc/Progress.hpp>
#include <cloudy/offset/Offset.hpp>
#include <cloudy/Cloud.hpp>
#include <cloudy/Convolve.hpp>

#include <boost/timer.hpp>
#include <fstream>
#include <vector>
#include <map>

using namespace cloudy;

void Process_all(double r,
                 std::istream &isCloud,
                 std::istream &isField, 
                 std::ostream &os)
{
    cloudy::Data_cloud points;
    cloudy::load_cloud(isCloud, points);

    cloudy::Data_cloud field, convolved_field;
    cloudy::load_cloud(isField, field);

    assert(field.size() == points.size());
    if (points.size() == 0)
       return;

    size_t fieldsize = field[0].size();
    for (size_t i = 0; i < field.size(); ++i)
    {
       assert (field[i].size() == fieldsize);
       points[i].resize(3);
    }

    cloudy::KD_tree kd(points);
    cloudy::convolve_uniform<uvector>(kd, field, convolved_field, r);

    write_cloud(os, convolved_field);
}

int main(int argc, char **argv)
{
   std::map<std::string, std::string> options;
   std::vector<std::string> param;
   cloudy::misc::get_options (argc, argv, options, param);
   double r = cloudy::misc::to_double(options["r"], 0.05);


   if (param.size() <= 2)
   {
      std::cerr << "Usage: " << argv[0] << " file.cloud file.p [outfile.p]";
      return -1;
   }

   std::ifstream isCloud(param[0].c_str());
   std::ifstream isField(param[1].c_str());
   
   if (param.size() == 3)
   {
      std::ofstream os(param[2].c_str());
      Process_all(r, isCloud, isField, os);
   }
   else
      Process_all(r, isCloud, isField, std::cout);
}
