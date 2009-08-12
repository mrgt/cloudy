#include <cloudy/view/Widget.hpp>
#include <cloudy/view/Editor.hpp>
#include <cloudy/Cloud.hpp>
#include <cloudy/mesh/Mesh.hpp>
#include <boost/shared_ptr.hpp>
#include <QToolBox>

namespace cloudy
{
   namespace view {

   typedef std::vector<size_t> Data_indices;
   typedef std::pair<size_t, size_t> Line;
   typedef std::vector<Line> Data_lines;
   

   typedef cloudy::uvector uvector;
   typedef cloudy::Data_cloud Data_cloud;
   typedef std::vector<double> Scalar_field;

   
   typedef boost::shared_ptr<cloudy::Mesh> Mesh_ptr;
   typedef boost::shared_ptr<Data_cloud> Data_cloud_ptr;
   typedef boost::shared_ptr<Scalar_field> Scalar_field_ptr;
   typedef boost::shared_ptr<Data_indices> Data_indices_ptr;
   typedef boost::shared_ptr<Data_lines> Data_lines_ptr;

   class Drawer
   {
	 std::string _name;
	 bool _enabled;

      public:
	 Drawer(const std::string &name,
	        bool enab = true): 
	    _name(name), _enabled(enab) {}

	 virtual ~Drawer(){};
         virtual void draw(size_t stride, bool fast = false) = 0;

	 virtual void fill_editor(Editor *editor) 
	 {
	    editor->add_bool("Enable:", _enabled);
	 }

	 virtual size_t stride() 
	 {
	    return 1;
	 }

	 void enable()  {_enabled = true;} 
	 void disable() {_enabled = false;} 
	 bool enabled() {return _enabled;}

	 std::string name() {return _name;}
   };

      typedef boost::shared_ptr<Drawer> Drawer_ptr;

   inline void
   gl_uvertex(const uvector &v)
   {
      	       glVertex3f(v(0), v(1), v(2));
   }

   inline void
   gl_unormal(const uvector &v)
   {
     glNormal3f(v(0), v(1), v(2));
   }

   class Direction_drawer: public Drawer
   {
	 Data_cloud_ptr _cloud;
	 Data_cloud_ptr _directions;
	 Data_indices_ptr _indices;
	 double _length;

      public:
	 Direction_drawer(const std::string &name,
	                  const Data_cloud_ptr cloud,
	                  const Data_cloud_ptr directions,
	                  const Data_indices_ptr indices = Data_indices_ptr()):
	    Drawer(name),
	    _cloud(cloud), _indices(indices), _directions(directions),
	    _length(0.1)
	 {}

         virtual void draw(size_t stride, bool fast = false)
	 {
	    if (!_indices)
	    {
	       glBegin(GL_LINES);
	       for (size_t i = 0; i < _cloud->size(); ++i)
	       {
		  if (i % stride != 0)
		     continue;
		  
		  if ((*_cloud) [i].size() != (*_directions) [i].size())
		  {
		     std::cerr << "bad sizes: " << i 
			       << " " << (*_cloud) [i].size()
			       << "vs" << (*_directions) [i].size() <<"\n";
		     continue;
		  }

		  gl_uvertex((*_cloud) [i] + (_length/2.0) * (*_directions)[i]);
		  gl_uvertex((*_cloud) [i] - (_length/2.0) * (*_directions)[i]);
	       }
	       glEnd();
	    }
	    else
	    {
	    }

	 }
   };

   class Line_drawer: public Drawer
   {
	 Data_cloud_ptr _cloud;
	 Data_lines_ptr _lines;

      public:
	 Line_drawer(const std::string &name,
	             const Data_cloud_ptr cloud,
	             const Data_lines_ptr lines): 
	    Drawer(name),
	    _cloud(cloud), _lines(lines)
	 {}

         virtual void draw(size_t stride, bool fast = false)
	 {
	    glBegin(GL_LINES);
	    for (Data_lines::iterator it = _lines->begin();
		 it != _lines->end(); ++it)
	    {
	       Line &l = *it;

	       if ((l.first % stride != 0) || (l.second % stride != 0))
		  continue;

	       gl_uvertex((*_cloud) [l.first]);
	       gl_uvertex((*_cloud) [l.second]);
	    }
	    glEnd();
	 }
   };

   class Cloud_drawer: public Drawer
   {
         Data_cloud_ptr _cloud;
         Scalar_field_ptr _field;
	 double _percentage;
         double _radius;
         bool _spheres;
         int _sphere_tessel;
	 
      public:
	 Cloud_drawer(const std::string &name,
	              const Data_cloud_ptr cloud,
		      const Scalar_field_ptr field = Scalar_field_ptr()):
	    Drawer(name),
	    _cloud(cloud),
	    _field(field),
	    _percentage(1.0),
	    _radius(1.0),
	    _spheres(false),
	    _sphere_tessel(8)
	 {}

         virtual void draw(size_t stride, bool fast)
	 {
	    if (_field == 0 || fast == true)
	    {
	       size_t i = 0;
	       glBegin(GL_POINTS);
	       for (Data_cloud::iterator it = _cloud->begin();
		    it != _cloud->end(); ++it, ++i)
	       {
		  if (i % stride == 0)
		     gl_uvertex(*it);
	       }
	       glEnd();
	    }
	    else if (_spheres == true )
	    {
	      GLUquadric* q = gluNewQuadric();

	      size_t i = 0;
	      for (Data_cloud::iterator it = _cloud->begin();
		   it != _cloud->end(); ++it, ++i)
		{
		  double rr = (*_field)[i]*_radius/1000.0f;

		  if (rr <= 0.001)
		    continue;

		  size_t div = _sphere_tessel;
		  if (rr >= 0.1)
		    div = 16;

		  glPushMatrix();
		  glTranslatef((*it)[0], (*it)[1], (*it)[2]);
		  gluSphere(q, rr, div, div);
		  glPopMatrix();
		}
	      
	      gluDeleteQuadric(q);

	    }
	    else
	    {
	       glEnable(GL_POINT_SMOOTH);
	       size_t i = 0;
	       for (Data_cloud::iterator it = _cloud->begin();
		    it != _cloud->end(); ++it, ++i)
	       {
		  if (i % stride != 0)
		     continue;

		  glPointSize((*_field)[i]*_radius);
		  glBegin(GL_POINTS);
		  gl_uvertex(*it);
		  glEnd();
	       }
	       glDisable(GL_POINT_SMOOTH);
	    }
	 }

	 virtual void fill_editor(Editor *editor) 
	 {
	    Drawer::fill_editor(editor);
	    editor->add_double("Percentage:", _percentage);
	    editor->add_double_spin("Radius:", _radius, 0.0, 10.0);
	    editor->add_bool("Spheres?", _spheres);
	    editor->add_integer_spin("Tesselation", _sphere_tessel, 0, 16);
	 }
	 
	 virtual size_t stride()
	 {
	    if (_percentage < 0.01)
	       return _cloud->size();

	    return (size_t) (1.0/_percentage);
	 }
   };

   class Mesh_drawer: public Drawer
   {
         Mesh_ptr _mesh;
	 
      public:
	 Mesh_drawer(const std::string &name,
	              const Mesh_ptr mesh):
	    Drawer(name),
	    _mesh(mesh)
	 {}

     virtual void draw(size_t stride, bool fast)
	 {
	   size_t i = 0;
	   glColor3f(1.0, 0.8, 0.5);
	   glBegin(GL_TRIANGLES);
	   for (size_t i = 0; i != _mesh->_triangles.size(); ++i)
	     {
	       const cloudy::Mesh_triangle &t = _mesh->_triangles[i];
	       
	       gl_uvertex(_mesh->_points[t.a]);
	       gl_uvertex(_mesh->_points[t.b]);
	       gl_uvertex(_mesh->_points[t.c]);
	     }
	   glEnd();
	 }

	 virtual void fill_editor(Editor *editor) 
	 {
	    Drawer::fill_editor(editor);
	 }
   };

   class Viewer : public  cloudy::view::GL_widget
   {
	 typedef cloudy::view::GL_widget GL_widget;
	 typedef std::list<Drawer_ptr> Drawer_list;

	 Drawer_list _drawers;
	 Drawer_ptr _main;
	 
      public:
	 Viewer(QWidget *parent): GL_widget(parent)
	 {}
	 
	 virtual void prepaintGL()
	 {
	    GL_widget::prepaintGL();
	 }
	 
	 virtual void paintGL()
	 {
	    prepaintGL();
	    glColor3f(0.0, 0.0, 0.0);

	    size_t stride = _main->stride();
	    for (Drawer_list::iterator it =_drawers.begin();
		 it != _drawers.end(); ++it)
	    {
	      if ((*it)->enabled()) (*it)->draw(stride, fast_draw);
	    }
	    
	    postpaintGL();
	 }

	 void add_drawer(Drawer_ptr dr)
	 {
	    if (_drawers.size() == 0)
	       _main = dr;

	    _drawers.push_back(dr);
	 }

	 void fill_dialog(QToolBox *tb)
	 {
	    for(Drawer_list::iterator it =_drawers.begin();
		it != _drawers.end(); ++it)
	    {
	       Editor *edit = new Editor();
	       (*it)->fill_editor(edit);
	       edit->finish();
	       tb->addItem(edit, (*it)->name().c_str());

	       connect(edit, SIGNAL(stateChanged()),
	               this, SLOT(updateGL()));
	    }
	 }
   };

}
}
