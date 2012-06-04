#include <cloudy/view/Viewer.hpp>
#include <cloudy/misc/Program_options.hpp>

#include <qapplication.h>
#include <qmainwindow.h>

#include <QDialog>
#include <QPushButton>
#include <QHBoxLayout>

using namespace cloudy::view;
using namespace cloudy;

bool setup(cloudy::view::Viewer &w,
           std::map<std::string, std::string> &options,
           std::vector<std::string> &parameters);

int main(int argc, char** argv)
{  
   std::map<std::string, std::string> options;
   std::vector<std::string> parameters;
   cloudy::misc::get_options(argc, argv, options, parameters);

   QApplication app(argc, argv);
   QMainWindow main;

   Viewer w(0);
   
   if (!setup(w, options, parameters))
      return -1;
   
   w.setMinimumWidth(800);
   w.show();


   QDialog *dialog = new QDialog;
   QHBoxLayout *layout = new QHBoxLayout;
   QToolBox *tab = new QToolBox();
   layout->addWidget(tab);
   w.fill_dialog(tab);
   dialog->setMaximumWidth(250);
   dialog->setLayout(layout);

   QWidget *centralWidget = new QWidget;
   QHBoxLayout *centralLayout = new QHBoxLayout;
   centralLayout->addWidget(&w);
   centralLayout->addWidget(dialog);
   centralWidget->setLayout(centralLayout);

   main.setCentralWidget(centralWidget);
   main.resize(1050, 600);
   main.show(); 

   return app.exec();
}
