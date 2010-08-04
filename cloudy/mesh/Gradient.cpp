#include <cloudy/mesh/Gradient.hpp>
#include <iostream>
#include <algorithm>
#include <sstream>

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
      {
	from.getline(line, 256);
	std::stringstream Nfrom (line);
	Nfrom >> N;
      }
	
      // read gradients
      for (size_t i = 0; i < N; ++i)
      {
	 Gradient_color c;	       
	 double middle, blend, type, sa, ea;

	 from.getline(line, 256);
	 std::stringstream lfrom (line);
	 
	 lfrom >> c._start >> middle >> c._end;
	 lfrom >> c._start_color._r
	       >> c._start_color._g
	       >> c._start_color._b
	       >> sa;
	 lfrom >> c._end_color._r
	       >> c._end_color._g
	       >> c._end_color._b
	       >> ea;
	 lfrom >> blend >> type;

	 
	 std::cerr << "Gradient Begin " << 
	   c._start << " End " <<  c._end  
		   << " SC " <<  c._start_color._r << " " <<
	   c._start_color._g << " " << c._start_color._b
		   <<  " EC " <<  c._end_color._r << " " <<
	   c._end_color._g << " " <<  c._end_color._b
		   << "\n";
	 
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
      //      std::cerr << " t = " << t << " P = " << P << "\n";
      
      const Gradient_color & c = _gradient_colors[P];
      t = std::max(std::min(t, c._end), c._start);
      double l = (t - c._start)/(c._end - c._start);

      return (1.0 - l) * c._start_color + l * c._end_color;
   };

   const Color Red(1.0, 0.0, 0.0), Green(0.0, 1.0, 0.0),
     Blue(0.0, 0.0, 1.0), Black(0.0, 0.0, 0.0),
     Yellow(1.0, 1.0, 0.0), Purple(1.0, 0.0, 1.0);
}
