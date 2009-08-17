#ifndef CLOUDY_CLOUD_HPP
#define CLOUDY_CLOUD_HPP

#include <cloudy/linear/Linear.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <string>
#include <iostream>

namespace cloudy
{
   typedef std::vector<uvector> Data_cloud;

   std::istream &operator >> (std::istream &is, uvector &vec);
   std::ostream &operator << (std::ostream &os, uvector v);

   template <class T, class OutputIterator>
   void
   load_data(std::istream &is, OutputIterator output)
   {
      size_t i = 0;
      while(1)
      {
	 std::string line;
	 if (!getline(is, line) || line == "")
	    return;
	 if (line == "end")
	    return;
	 if (line[0] == '#')
	    continue;

	 std::stringstream linestream(line);
	 T result;

	 linestream >> result;
	 *output++ = result;
      }
   }

   template <class Iterator>
   void
   write_data(std::ostream &os, Iterator begin, Iterator end)
   {
      for (; begin != end; ++begin)
	 os << (*begin) << "\n";
   }

   void load_cloud(std::istream &is, Data_cloud &c);
   void write_cloud(std::ostream &os, Data_cloud &c);

  uvector mean (const Data_cloud &dc);
  double simple_radius(const Data_cloud &dc);
  void translate(Data_cloud &dc, const uvector &u);
  void scale(Data_cloud &dc, double r);
  void normalize(Data_cloud &c, double rad = 1.0);
  
}

#endif
