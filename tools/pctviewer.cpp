#include <cloudy/view/Widget.hpp>
#include <qapplication.h>
#include <qmainwindow.h>

class Viewer : public cloudy::view::GL_widget
{
   public:
      Viewer(QWidget *parent): cloudy::view::GL_widget(parent)
      {}
};

int main(int argc, char** argv)
{  
   QApplication app(argc, argv);
   QMainWindow main;

   Viewer w(&main);
   main.setCentralWidget(&w);
   main.resize(500, 500);
   main.show();

   return app.exec();
}
