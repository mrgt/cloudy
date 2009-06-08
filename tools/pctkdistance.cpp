#include <cloudy/misc/Program_options.hpp>
#include <cloudy/misc/Progress.hpp>
#include <cloudy/Cloud.hpp>
#include <cloudy/KD_tree.hpp>
#include <math.h>

#include <boost/timer.hpp>
#include <fstream>
#include <vector>
#include <map>

using namespace cloudy;

inline
double squared_norm(const uvector &u)
{
   return pow(ublas::norm_2(u), 2.0);
}

void Process_all(size_t k,
                 std::istream &isCloud, 
                 std::ostream &os)
{
    cloudy::Data_cloud points;
    cloudy::load_cloud(isCloud, points);

    cloudy::KD_tree kd(points);
    cloudy::Data_cloud result;

    for (size_t i = 0; i < points.size(); ++i)
    {
       std::vector<size_t> knn;
       const size_t dimension(points[i].size());

       kd.find_knn(points[i], k, knn);

       double h = 0.0;
       cloudy::uvector bary(dimension);
       std::fill(bary.begin(), bary.end(), 0);

       for (size_t j = 0; j < k; ++j)
       {
	  bary += points[knn[j]];
	  h += squared_norm(points[knn[j]]);
       }
       h /= k; bary /= k;

       cloudy::uvector v(dimension + 1);
       std::copy(bary.begin(), bary.end(), v.begin());
       v[dimension] = h - squared_norm(bary);
       result.push_back(v);
    }

    write_cloud(os, result);
}

int main(int argc, char **argv)
{
   std::map<std::string, std::string> options;
   std::vector<std::string> param;
   cloudy::misc::get_options (argc, argv, options, param);
   size_t k  = cloudy::misc::to_unsigned(options["k"], 50);

   if (param.size() == 2)
   {
      std::ifstream is(param[0].c_str());
      std::ofstream os(param[1].c_str());
      Process_all(k, is, os);
   }
   else if (param.size() == 1)
   {
      std::ifstream is(param[0].c_str());
      Process_all(k, is, std::cout);
   }
   else
      Process_all(k, std::cin, std::cout);
}
