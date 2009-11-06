#ifndef _GRADIENT_H
#define _GRADIENT_H

#include <istream>
#include <vector>
#include <string>

namespace cloudy
{
   struct Color
   {
	 double _r, _g, _b;
	 Color (double r = 0.0,
		double g = 0.0,
		double b = 0.0) : _r(r), _g(g), _b(b) 
	 {}
   };

   inline Color operator *(double t, const Color &c)
   {
      return Color(t*c._r, t*c._g, t*c._b);
   }
   
   inline Color operator +(const Color &c, const Color &d)
   {
      return Color(c._r + d._r,
		   c._g + d._g,
		   c._b + d._b);
   }
   
   extern const Color Red, Green, Blue, Black, Yellow, Purple;

   class Gradient
   {
      public:
	 struct Gradient_color
	 {
	       double _start;
	       double _end;
	       Color _start_color;
	       Color _end_color;
	 };
	 

      private:
	 std::vector<Gradient_color> _gradient_colors;

      public:
	 bool read_ggr(std::istream &from);
	 Color operator ()(double t) const;
   };
}
#endif
