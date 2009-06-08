#ifndef CLOUDY_PROGRAMOPTIONS_HPP
#define CLOUDY_PROGRAMOPTIONS_HPP

#include <map>
#include <vector>
#include <string>
#include <stdio.h>

namespace cloudy
{
   namespace misc 
   {
      void get_options (int argc, char **argv,
                        std::map<std::string, std::string> &options,
                        std::vector<std::string> &parameters);
      void get_options (const std::string &commandline,
                        std::map<std::string, std::string> &options);
		     
      double to_double (const std::string &str, double def = 0.0);
      int to_int (const std::string &str, int def = 0);
      unsigned to_unsigned (const std::string &str, unsigned def = 0);
                            std::string to_str(const std::string &str,
		            const std::string &def = "");

//    void change_extension(const std::string &str, const std::string &newext);
//    std::string remove_extension(const std::string &str, char sep='.');

//    void print_pct(unsigned int i, unsigned int total);
   }
}

#endif
