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

static const double EPSILON = 1e-6;

double
k_distance(size_t k, double m, double D,
           const cloudy::KD_tree &kd, 
           const std::vector<double> &W,
           const cloudy::uvector &P,
           cloudy::uvector &bary)
{        
   double totalw = 0.0;
   double h = 0.0;
   
   while(totalw < m - EPSILON)
   {
      std::vector<size_t> knn;   
      kd.find_knn(P, k, knn);
      bary = W[knn[0]]*kd[knn[0]];
      
      size_t j = 0;
      h = 0; totalw = 0;
      for (; j < k; ++j)
      {
	 double w = W[knn[j]];
	 
	 if (totalw + w >= m - EPSILON)
	    w = m - totalw;
	 
	 if (j == 1)
	    bary = w*kd[knn[j]];
	 else
	    bary += w*kd[knn[j]];
	 h += w*squared_norm(kd[knn[j]] - P);
	 totalw += w;
	 //std::cerr << w << "\t" <<  squared_norm(kd[knn[j]] - P) 
	 //<< "\t" << h << "\n"; 
	 //std::cerr << ublas::norm_2(kd[knn[j]] - P) << "\n";
	 
	 if (totalw >= m - EPSILON)
	    break;
      }

      h /= totalw;
      bary /= totalw;

      // std::cerr << "k = " << j << ", w = " << totalw 
// 		<< ", D = " << sqrt(h) << "\n";

      if (sqrt(h) >= D)
	 return 10.0;

      k *= 2;
   }
//    std::cerr << "final: " << "w = " << totalw 
// 	     << ", h = " << h << "\n";
      
   //std::cerr << "total w = " << totalw << "\n";
   return h;
}

void Process_all(size_t k, double m, double D,
		 const std::string &weights,
                 std::istream &isCloud, 
                 std::ostream &os)
{
    cloudy::Data_cloud points;
    cloudy::load_cloud(isCloud, points);

    cloudy::KD_tree kd(points);
    cloudy::Data_cloud result;

    std::vector<double> w;
    if (weights != "")
    {
       std::ifstream ifw(weights.c_str());
       cloudy::load_data<double>(ifw, std::back_inserter(w));
       std::cerr << "Processing distance to " << weights << std::endl;
    }
    else
    {
       size_t N = points.size();
       w.resize (N);
       std::fill(w.begin(), w.end(), 1.0/double(N));
       m = 1.0/double(k);
       std::cerr << "Processing k-distance" << std::endl;       
    }
    
    
    cloudy::misc::Progress_display progress(points.size(), std::cerr);
    for (size_t i = 0; i < points.size(); ++i)
    {
       ++progress;

       const size_t dimension(points[i].size());
       cloudy::uvector bary;
       double h = k_distance(k, m, D, kd, w, points[i], bary);

       cloudy::uvector v(bary.size() + 1);
       std::copy(bary.begin(), bary.end(), v.begin());
       v[bary.size()] = h;
       result.push_back(v);
    }

    write_cloud(os, result);
}

int main(int argc, char **argv)
{
   std::map<std::string, std::string> options;
   std::vector<std::string> param;
   cloudy::misc::get_options (argc, argv, options, param);
   std::string weights = cloudy::misc::to_str(options["w"], "");
   size_t k = cloudy::misc::to_unsigned(options["k"], 50);
   double m = cloudy::misc::to_double(options["m"], 0.0);
   double D = cloudy::misc::to_double(options["D"], 0.0);

   if (param.size() == 2)
   {
      std::ifstream is(param[0].c_str());
      std::ofstream os(param[1].c_str());
      Process_all(k, m, D, weights, is, os);
   }
   else if (param.size() == 1)
   {
      std::ifstream is(param[0].c_str());
      Process_all(k, m, D, weights, is, std::cout);
   }
   else
     Process_all(k, m, D, weights, std::cin, std::cout);
}
