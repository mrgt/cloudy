#include <cloudy/view/Viewer.hpp>
#include <cloudy/misc/Program_options.hpp>

#include <qapplication.h>
#include <qmainwindow.h>

using namespace cloudy::view;

void setup(cloudy::view::Viewer &w,
           const std::map<std::string, std::string> &options,
           const std::vector<std::string> &parameters);

int main(int argc, char** argv)
{  
   std::map<std::string, std::string> options;
   std::vector<std::string> parameters;
   cloudy::misc::get_options(argc, argv, options, parameters);

   QApplication app(argc, argv);
   QMainWindow main;

   Viewer w(&main);
   main.setCentralWidget(&w);
   main.resize(500, 500);
   main.show();   
   setup(w, options, parameters);

   return app.exec();
}
