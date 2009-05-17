#ifndef CLOUDY_PROGRESS_HPP
#define CLOUDY_PROGRESS_HPP

#include <iostream>

namespace cloudy
{
   namespace misc
   {
      class Progress_display
      {
	    unsigned long _expected, _current;
	    unsigned long _next;
	    size_t _tic;

	    std::ostream &_os;

	 public:
	    Progress_display(unsigned long expected,
	                     std::ostream &os = std::cerr):
	       _expected(expected),
	       _os(os),
	       _current(0),
	       _next(0),
	       _tic(0)
	    {
	       if (expected == 0)
		  _expected = 1;
	    }

	    void operator ++(int)
	    {
	       ++(*this);
	    }
	    
	    void operator ++()
	    {
	       _current++;

	       if (_current > _next)
	       {
		  _os << ((_tic%7 == 0) ? '|' : '=') << std::flush;
		  _tic++;
		  _next = (unsigned long) (double(_tic/70.0)*_expected);
	       }

	       if (_current == _expected)
		  _os << "|" << std::endl;
	    }
      };
   }
}

#endif
