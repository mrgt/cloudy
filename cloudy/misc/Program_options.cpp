#include <cloudy/misc/Program_options.hpp>
#include <string>
#include <iostream>
#include <sstream>
#include <boost/algorithm/string.hpp>

namespace cloudy
{
   namespace misc 
   {

      void get_options (const std::string &commandline,
                        std::map<std::string, std::string> &options)
      {
	 std::vector<std::string> argv;
	 boost::split(argv, commandline, boost::is_any_of(" \t\n") );      

	 for (size_t i = 1; i < argv.size(); ++i)
	 {
	    if (argv[i][0] == '+')
	    {
	       argv[i].erase(0,1);
	       options[argv[i]] = "true";
	    }
	    else if (argv[i][0] == '-')
	    {
	       argv[i].erase(0,1);
	       options[argv[i]] = std::string(argv[i+1]);
	       assert(i + 1 < argv.size());
	       i++;
	    }
	 }
      }
      
      void get_options(int argc, char **argv,
                       std::map<std::string, std::string> &options,
                       std::vector<std::string> &parameters)
      {
	 for (size_t i = 1; i < (size_t)argc; ++i)
	 {
	    if (argv[i][0] == '+')
	       options[std::string(argv[i]+1)] = "true";
	    else if (argv[i][0] == '-')
	    {
	       if ((size_t) argc <= i+1)
	       {
		  std::cerr << argv[0] << ": missing argument after option '" 
			    << argv[i] << std::endl;
		  continue;
	       }
	       
	    options[std::string(argv[i]+1)] = std::string(argv[i+1]);
	    i++;
	    }
	    else
	       parameters.push_back(argv[i]);
	 }
      }
      
      template <class T>
      T to_any (const std::string &str, const T &def)
      {
	 std::istringstream i(str);
	 T res;
	 if (i >> res)
	    return res;
	 else
	    return def;
      }
      
      double
      to_double (const std::string &str, double d)
      {
	 return to_any(str, d);
      }
      
      int
      to_int (const std::string &str, int d)
      {
	 return to_any(str, d);
      }
      
      unsigned
      to_unsigned (const std::string &str, unsigned d)
      {
	 return to_any(str, d);
      }
      
      std::string to_str(const std::string &str,
                         const std::string &def)
      {
	 if (str == "" || str.empty())
	    return def;
	 else return str;
      }
      
      std::string remove_extension(const std::string &str, char sep)
      {
	 std::string r = str;
	 size_t p = r.rfind(sep);
	 
	 if (p != std::string::npos)
	    r.erase(r.begin() + p, r.end());
	 return r;
      }
      
#if 0
      void
      change_extension(const std::string &str, const std::string &newext)
      {
	 size_t p = str.rfind(str, '.');
	 if (p == std::string::npos)
	    str.insert(str.end(), newext.begin(), newext.end());
	 else
	    str.replace(str, p, str.size(), newext.begin());
      }
#endif
   }
}
