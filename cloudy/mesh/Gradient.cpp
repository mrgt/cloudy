#include <cloudy/mesh/Gradient.hpp>
#include <iostream>
#include <algorithm>

namespace cloudy
{

   static bool operator < (const Gradient::Gradient_color &c1,
			   const Gradient::Gradient_color &c2)
   {
      return (c1._start < c2._start);
   }


   bool
   Gradient::read_ggr(std::istream &from)
   {
      char line[256];
      from.getline(line, 256);
      if (std::string(line) != "GIMP Gradient")
      {
	 std::cerr << "gradient::read_ggr: Unknown file format '"
		   << line << "'\n";

	 return false;
      }
      
      // skip name
      from.getline(line, 256);
      size_t N;
      from >> N;
      
      // read gradients
      for (size_t i = 0; i < N; ++i)
      {
	 Gradient_color c;	       
	 double middle, blend, type, sa, ea;
	 
	 from >> c._start >> middle >> c._end;
	 from >> c._start_color._r
	      >> c._start_color._g
	      >> c._start_color._b
	      >> sa;
	 from >> c._end_color._r
	      >> c._end_color._g
	      >> c._end_color._b
	      >> ea;
	 from >> blend >> type;
	 
	 _gradient_colors.push_back(c);
      }
      
      std::sort(_gradient_colors.begin(), _gradient_colors.end());

      return true;
   }
   
   Color
   Gradient::operator ()(double t) const
   {
      if (_gradient_colors.size() == 0)
	 return Color(0.0, 0.0, 0.0);

      size_t P = _gradient_colors.size() - 1;
      for (size_t i = 0; i < _gradient_colors.size(); ++i)
      {
	 if (_gradient_colors[i]._end >= t)
	 {
	    P = i;
	    break;
	 }
      }

//       size_t left = 0, right = _gradient_colors.size()-1;
      
//       if (_gradient_colors[left]._start >= t)
// 	 right = left;
      
//       if (_gradient_colors[right]._end <= t)
// 	 left = right;
      
//       while (left != right)
//       {
// 	 size_t mid = (left+right)/2;
	 
// 	 if (_gradient_colors[mid]._end >= t)
// 	    right = mid;
// 	 if (_gradient_colors[mid]._start <= t)
// 	    left = mid;
// 	 std::cerr << left << " " << mid << " " << right << " LMR\n";
//       }
      
      const Gradient_color & c = _gradient_colors[P];
      t = std::max(std::min(t, c._end), c._start);
      double l = (t - c._start)/(c._end - c._start);
      //std::cerr << "t=" << t  << " i=" << P << "\n";

      return (1.0 - l) * c._start_color + l * c._end_color;
   };
}
