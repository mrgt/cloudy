#include <cloudy/Cloud.hpp>
namespace cloudy
{
   typedef std::vector<uvector> Data_cloud;

   std::istream &
   operator >> (std::istream &is, uvector &vec)
   {
      std::vector<double> v;
      while(! is.eof())
      {
	 double coord;

	 if (!(is >> coord))
	    break;

	 v.push_back(coord);
      }
      vec.resize(v.size());
      std::copy(v.begin(), v.end(), vec.begin());
      return is;
   }

   std::ostream &
   operator << (std::ostream &os, uvector v)
   {
      for (size_t i = 0; i < v.size(); ++i)
	 os << v[i] << " ";
      return os;
   }

   void 
   load_cloud(std::istream &is, Data_cloud &c)
   {
      cloudy::load_data<cloudy::uvector> (is, std::back_inserter(c));
   }

   void 
   write_cloud(std::ostream &os, Data_cloud &c)
   {
      cloudy::write_data(os, c.begin(), c.end());
   }
}
