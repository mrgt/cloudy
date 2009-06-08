#ifndef CLOUDY_CLOUD_HPP
#define CLOUDY_CLOUD_HPP

#include <cloudy/linear/Linear.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <string>
#include <iostream>

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

#endif
