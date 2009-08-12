#ifndef CLOUDY_WIDGET_HPP
#define CLOUDY_WIDGET_HPP

#include <qgl.h>
#include <qevent.h>
#include <cloudy/view/Director.hpp>
#include <boost/numeric/ublas/vector.hpp>

namespace cloudy
{
   namespace view
   {
      typedef boost::numeric::ublas::vector<double> uvector;

      // shortcut to create a 3D vector
      inline uvector uv3(double x, double y, double z)
      {
	 uvector v(3);
	 v(0) = x; v(1) = y; v(2) = z;
	 return v;
      }


      class GL_widget : public QGLWidget
      {	 
	    Q_OBJECT
	 
	 public:
	    GL_widget(QWidget* parent);
	    ~GL_widget();
	    
	    uvector getCameraZ ();
	    
	 protected:
	    void initializeGL();
	    void prepaintGL();
	    void postpaintGL();
	    void resizeGL(int w, int h);
	    
	    void mouseMoveEvent (QMouseEvent* e);
	    void mousePressEvent (QMouseEvent* e);
	    void mouseReleaseEvent (QMouseEvent* e);
	    void wheelEvent (QWheelEvent* e);
	    
	    void keyPressEvent (QKeyEvent* e);
	    
	 private:
	    // xside = half of the width of the window
	    // yside = half of the height of the window
	    // side = min(xside, yside)
	    int side, xside, yside;
	    
	    // actual and previous positions of the mouse
	    int x, y, xold, yold;
	    
	    // tell wether the left, right or middle botton is pressed
	    int pushedL, pushedR, pushedM;
	    bool shifting;
	    double rayon;
	
	 public:
	    Director b;
	    bool fast_draw;
   };
}
}

#endif // WIDGET_H
