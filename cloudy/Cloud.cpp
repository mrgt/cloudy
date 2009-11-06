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

  uvector mean (const Data_cloud &dc)
  {
    if (dc.size() == 0)
      return uvector();

    uvector m = dc[0];
    for (size_t i = 0; i < dc.size(); ++i)
      m += dc[i];

    return (1.0/double(dc.size())) * m;
  }

  double simple_radius(const Data_cloud &dc)
  {
    if (dc.size() == 0)
      return 0.0;
    
    double r = ublas::norm_2(dc[0]);
    
    for (size_t i = 0; i < dc.size(); ++i)
      r = std::max(r, ublas::norm_2(dc[i]));

    return r;
  }
  
  void translate(Data_cloud &dc, const uvector &u)
  {
    uvector m = mean(dc);
    for (size_t i = 0; i < dc.size(); ++i)
      dc[i] +=  u;
  }

  void scale(Data_cloud &dc, double r)
  {
    uvector m = mean(dc);
    for (size_t i = 0; i < dc.size(); ++i)
      dc[i] *= r;
  }

  void normalize(Data_cloud &c, double rad)
  {
    translate(c, - mean(c));
    scale(c, rad/simple_radius(c));
  }
}
