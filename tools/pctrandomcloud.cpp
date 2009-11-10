#include <cloudy/misc/Program_options.hpp>
#include <cloudy/random/Random.hpp>
#include <cloudy/Cloud.hpp>

int main(int argc, char **argv)
{
   std::map<std::string, std::string> options;
   std::vector<std::string> param;
   cloudy::misc::get_options (argc, argv, options, param);
   
   double R = cloudy::misc::to_double(options["R"], 0.1);
   size_t N = cloudy::misc::to_unsigned(options["N"], 100);

   cloudy::random::Random_vector_in_ball<cloudy::uvector> randball (3,R);
   boost::mt19937 engine;

   cloudy::Data_cloud dc;
   for (size_t i = 0; i < N; ++i)
     {
       cloudy::uvector v = randball(engine);
       dc.push_back(v);
     }     
   cloudy::write_cloud(std::cout, dc);
}
