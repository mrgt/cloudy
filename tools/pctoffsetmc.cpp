#include <cloudy/misc/Program_options.hpp>
#include <cloudy/misc/Progress.hpp>
#include <cloudy/random/Random.hpp>
#include <cloudy/KD_tree.hpp>

#include <boost/timer.hpp>
#include <fstream>
#include <vector>
#include <map>

///////////////////////////////////////////////////////////////


std::ostream &
operator << (std::ostream &os, const cloudy::uvector &cov)
{
   for (size_t i = 0; i < cov.size(); ++i)
      os << cov[i] << " ";
   return os;
}

cloudy::uvector 
three_to_four(const cloudy::uvector &v)
{
  if (v.size() < 4)
    {
      cloudy::uvector w(4);
      w(0) = v(0); w(1) = v(1);
      w(2) = v(2); w(3) = 0.0;
      return w;
    }
  return v;
}

cloudy::uvector
four_to_three(const cloudy::uvector &v)
{
  if (v.size() != 3)
    {
      cloudy::uvector w(3);
      w(0) = v(0); w(1) = v(1);
      w(2) = v(2);
      return w;
    }
  return v;
}

void Load_data(std::istream &is, cloudy::Data_cloud &points)
{
  points.clear();
  cloudy::load_cloud(is, points);

  for (size_t i = 0; i < points.size(); ++i)
      points[i] = three_to_four(points[i]);
}

class MC_volume_integrator
{
  const cloudy::KD_tree &_kd;
  std::vector<double> _results;
  double _total_value;

public:
  MC_volume_integrator(const cloudy::KD_tree &kd): _kd(kd)
  {
    _results.resize(kd.size(), 0);
    _total_value = 0.0;
  }

  void operator () (const cloudy::uvector &pos, double value)
  {
    size_t nn = _kd.find_nn(three_to_four(pos));
    _results[nn] += value;
    _total_value += value;
  }

  double result(size_t i)
  {
    assert(_total_value > 0);
    return _results[i]/_total_value;
  }
};

template <class MC_integrator>
void
Batch_integrate(const cloudy::KD_tree &kd, double R, 
		size_t N, std::ostream &os)
{
  MC_integrator ig (kd);

  std::cerr << "Integrating... \n";
  cloudy::misc::Progress_display progress(kd.size(), std::cerr);
  boost::timer t;

  cloudy::random::Random_vector_in_ball<cloudy::uvector> randball (3, R);
  boost::mt19937 engine;

  for (size_t i = 0; i < kd.size(); ++i)
    {
      cloudy::uvector p_0 = four_to_three(kd[i]);

      for (size_t j = 0; j < N; ++j)
	{
	  cloudy::uvector p = three_to_four(p_0 + randball(engine));
	  size_t k = kd.count_points_in_ball(p, R);
	  if (k <= 0) continue;

	  ig(p, 1.0/double(k));
	}
      ++progress;
    }
  
  for (size_t i = 0; i < kd.size(); ++i)
    os << ig.result(i) << std::endl;

  std::cerr << "done in " << t.elapsed() << "s\n";
}


void Process_all(std::istream &is,  std::ostream &os, bool covariance,
                 double R, size_t N)
{
   cloudy::Data_cloud points;
   Load_data(is, points);   
   cloudy::KD_tree kd(points);

   if (covariance == false)
   {
      Batch_integrate<MC_volume_integrator>
	(kd, R, N, os);
   }
   else
   {
     std::cerr << "Covariance unimplemented\n";
   }
}

int main(int argc, char **argv)
{
   std::map<std::string, std::string> options;
   std::vector<std::string> param;
   cloudy::misc::get_options (argc, argv, options, param);

   bool covariance = (options["type"] == "covariance");
   double R = cloudy::misc::to_double(options["R"], 0.1);
   size_t N = cloudy::misc::to_unsigned(options["N"], 100);

   if (param.size() == 1)
   {
      std::ifstream is(param[0].c_str());
      Process_all(is, std::cout, covariance, R, N);
   }
   else if (param.size() == 2)
   {
      std::ifstream is(param[0].c_str());
      std::ofstream os(param[1].c_str());
      Process_all(is, os, covariance, R, N);
   }
   else
      Process_all(std::cin, std::cout, covariance, R, N);
}
