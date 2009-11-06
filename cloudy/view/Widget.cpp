#include <cloudy/view/Widget.hpp>

namespace cloudy
{
   namespace view {
   GL_widget::GL_widget(QWidget* parent)
      : QGLWidget(parent)
   {
      setFocusPolicy(Qt::ClickFocus);
      setFocus();
      shifting = false;
   }

   GL_widget::~GL_widget()
   {
   }

   void GL_widget::prepaintGL()
   {     
     glMatrixMode(GL_MODELVIEW);
     glLoadMatrixd(b.mat());
     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   }

   void GL_widget::postpaintGL()
   {
     //cout << __LINE__ << endl;
   }

   void GL_widget::initializeGL()
   {
      glClearColor(1.,1.,1.,1.);
      glColor3f(1.0,1.0,1.0);
      glEnable(GL_DEPTH_TEST);
   }

   void GL_widget::resizeGL(int x, int y)
   {
     //     std::cout << "wheeling" << std::endl;
      double r = (double)y/(double)x;
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
  
      glViewport(0, 0, x, y);
      if (x<y)
      {
	 side = x/2;
	 glOrtho(-1., 1., -r, r, -40., 40.);
      }
      else
      {
	 glOrtho(-1./r, 1./r, -1., 1., -40., 40.);
	 side = y/2;
      }
      xside = x/2;
      yside = y/2;

      glMatrixMode(GL_MODELVIEW);
   }

   void GL_widget::mouseMoveEvent(QMouseEvent* e)
   {
      int x = e->pos().x(), y = e->pos().y();

      // on normalise les positions courantes et précédentes de la souris
      double Xold = ((double) xold - xside)/side;
      double Yold = ((double) yold - yside)/side;
      double X = ((double) x - xside)/side;
      double Y = ((double) y - yside)/side;

      bool need_redisplay = false;
  
      if (e->buttons() & Qt::LeftButton) { // rotation de la scène
	 b.rotation(Xold, -Yold, X, -Y);
	 need_redisplay = true;
      }
  
      if (e->buttons() & Qt::MidButton)
      { // translation de l'objet
	 double vx = X - Xold;
	 double vy = Y - Yold;
	 b.translation(vx, -vy, 0.);
	 rayon = b.radius();
	 need_redisplay = true;
      }



      // mise-à-jour de la position souris
      xold=x; 
      yold=y;

      if(need_redisplay)
	{
	  fast_draw = true;
	  updateGL();
	  fast_draw = false;
	}
   }

   void GL_widget::mousePressEvent(QMouseEvent* e)
   {
     //std::cout << "Old Mouse Press" << std::endl;
      xold = e->pos().x();
      yold = e->pos().y();
   }

     void GL_widget:: mouseReleaseEvent(QMouseEvent* e)
     {
       	 updateGL();
     }

   void GL_widget::wheelEvent (QWheelEvent* e)
   {
     //     std::cout << "wheeling" << std::endl;
      double  position[3];
      bool z = true;
      int d = e->delta();
   
      if( d < 0 )
      {
	 z = false; //unzoom
	 d = -d;
      }
   
      d /= 120;
   
      for(int i = 0; i < d; ++i)
      {
	 if (z)
	 {
	    if(b.radius() < 1000)
	       b.radius() *= 1.2;
	    rayon = b.radius();
	  
	    position[0] = -1.5*rayon;
	    position[1] = 1.0*rayon;
	    position[2] = 4.0*rayon;
	    b.preImageSansTransZoom(position);
	 }
	 else
	 {
	    if(b.radius() > .0001)
	       b.radius() /= 1.2;
	    rayon = b.radius();
	 
	    position[0] = -1.5*rayon;
	    position[1] = 1.0*rayon;
	    position[2] = 4.0*rayon;
	    b.preImageSansTransZoom(position);
	 }	 
      }
   
      updateGL();
      e->accept();
   }

   void GL_widget::keyPressEvent (QKeyEvent* e)
   {
      double position[3];
      if (e->key() == Qt::Key_X)
      {
	 std::cout << "Zooming in\n";
	 if(b.radius() < 1000)
	    b.radius() *= 1.2;
	 rayon = b.radius();
	 
	 position[0] = -1.5*rayon;
	 position[1] = 1.0*rayon;
	 position[2] = 4.0*rayon;
	 b.preImageSansTransZoom(position);
      }
      else if (e->key() == Qt::Key_C)
      {
	 if(b.radius() > .0001)
	    b.radius() /= 1.2;
	 rayon = b.radius();
	 
	 position[0] = -1.5*rayon;
	 position[1] = 1.0*rayon;
	 position[2] = 4.0*rayon;
	 b.preImageSansTransZoom(position);
      }
      updateGL();
      e->accept();

//   if(observer->keyboard(e->ascii(), 0, 0))
//   {
//     updateGL();
//     e->accept();
//  }
   }

#define idx(i,j) (4*(j)+(i))
   uvector
   GL_widget::getCameraZ ()
   {
      const double *M = b.mat();
      return uv3(M[idx(2,0)], M[idx(2,1)], M[idx(2,2)]);
   }
}
}
