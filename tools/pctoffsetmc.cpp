#include <cloudy/misc/Program_options.hpp>
#include <cloudy/misc/Progress.hpp>
#include <cloudy/random/Random.hpp>
#include <cloudy/KD_tree.hpp>

#include <boost/timer.hpp>
#include <fstream>
#include <vector>
#include <map>

///////////////////////////////////////////////////////////////

enum Integration_type
  {
    VOLUME,
    CURVATURE,
    COVARIANCE
  };

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

#ifdef OFF_CURVATURE

#include "cumulative.hpp"

class MC_curvature_measures_integrator
{
  const cloudy::KD_tree &_kd;
  std::vector< std::vector<double> > _radii;
  std::vector< std::vector<double> > _weights;
  double _total_value;

public:
  MC_curvature_measures_integrator(const cloudy::KD_tree &kd): _kd(kd)
  {
    _radii.resize(kd.size());
    _weights.resize(kd.size());
    _total_value = 0.0;
  }
  void operator () (const cloudy::uvector &pos, double value)
  {
    size_t nn = _kd.find_nn(three_to_four(pos));
    double rad = cloudy::ublas::norm_2(_kd[nn] - pos);

    _radii[nn].push_back(rad);
    _weights[nn].push_back(value);
    _total_value += value;
  }

  cloudy::uvector result(size_t i)
  {
    assert(_total_value > 0);

    if (_radii[i].size() < 3)
	return cloudy::ublas::zero_vector<double>(3);
    
    std::vector<double> radii, values;
    cumulative_function(_radii[i], _weights[i], 
			radii, values);

    for (size_t s = 0; s < _weights[i].size(); ++s)
      values[s] /= _total_value;
    
    double c0, c1, c2;
    linear_fit_quadratic(radii, values, c0, c1, c2);
    cloudy::uvector res(3);
    res[0] = c0; res[1] = c1; res[2] = c2;

    return res;
  }
};

#endif

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
  
  std::cerr << "Computing and writing\n";
  cloudy::misc::Progress_display progress2(kd.size(), std::cerr);
  for (size_t i = 0; i < kd.size(); ++i)
    {
      os << ig.result(i) << std::endl;
      ++progress;
    }

  std::cerr << "done in " << t.elapsed() << "s\n";
}


void Process_all(std::istream &is,  std::ostream &os, 
		 Integration_type type,
                 double R, size_t N)
{
   cloudy::Data_cloud points;
   Load_data(is, points);   
   cloudy::KD_tree kd(points);

   std::cerr << "type = " << type << "\n";
   switch (type)
     {
     case VOLUME:
       std::cerr << "type = " << type << "\n";
       Batch_integrate<MC_volume_integrator> (kd, R, N, os);
       break;

#ifdef OFF_CURVATURE
     case CURVATURE:
       Batch_integrate<MC_curvature_measures_integrator> (kd, R, N, os);
       break;
#endif
     }
}

int main(int argc, char **argv)
{
   std::map<std::string, std::string> options;
   std::vector<std::string> param;
   cloudy::misc::get_options (argc, argv, options, param);

   Integration_type type = VOLUME;
   if (options["type"] == "covariance")
     type = COVARIANCE;
   else if (options["type"] == "curvature")
     type = CURVATURE;
     
   double R = cloudy::misc::to_double(options["R"], 0.1);
   size_t N = cloudy::misc::to_unsigned(options["N"], 100);

   if (param.size() == 1)
   {
      std::ifstream is(param[0].c_str());
      Process_all(is, std::cout, type, R, N);
   }
   else if (param.size() == 2)
   {
      std::ifstream is(param[0].c_str());
      std::ofstream os(param[1].c_str());
      Process_all(is, os, type, R, N);
   }
   else
      Process_all(std::cin, std::cout, type, R, N);
}
