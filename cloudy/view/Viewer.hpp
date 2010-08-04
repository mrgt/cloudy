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
   

     //typedef cloudy::uvector uvector;
     //typedef cloudy::Data_cloud Data_cloud;
   typedef std::vector<double> Scalar_field;

   
   typedef boost::shared_ptr<cloudy::Mesh> Mesh_ptr;
   typedef boost::shared_ptr<Data_cloud> Data_cloud_ptr;
   typedef boost::shared_ptr<Scalar_field> Scalar_field_ptr;
   typedef boost::shared_ptr<Data_indices> Data_indices_ptr;
   typedef boost::shared_ptr<Data_lines> Data_lines_ptr;
   typedef boost::shared_ptr< std::vector<size_t> > Matching_ptr;

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


   class Drawer
   {
	 std::string _name;
	 bool _enabled;

      public:
	 Drawer(const std::string &name,
	        bool enab = true): 
	    _name(name), _enabled(enab) {}

	 virtual ~Drawer(){};
         virtual void draw(bool fast = false) = 0;
         virtual void draw_pov(std::ostream &os) {}

	 virtual void fill_editor(Editor *editor) 
	 {
	    editor->add_bool("Enable:", _enabled);
	 }

	 void enable()  {_enabled = true;} 
	 void disable() {_enabled = false;} 
	 bool enabled() {return _enabled;}

	 std::string name() {return _name;}
   };

   class Cloud_drawer: public Drawer
   {
         Data_cloud_ptr _cloud;
         Scalar_field_ptr _field;
	 double _percentage;
         double _radius;
         bool _spheres;
         int _sphere_tessel;
         double _low_threshold;
	 std::vector<bool> _points_enabled;
	 
      public:
	 Cloud_drawer(const std::string &name,
	              const Data_cloud_ptr &cloud,
		      const Scalar_field_ptr field = Scalar_field_ptr()):
	    Drawer(name),
	    _cloud(cloud),
	    _field(field),
	    _percentage(1.0),
	    _radius(1.0),
	    _spheres(false),
	    _sphere_tessel(8),
	    _low_threshold(0.0),
	    _points_enabled()
	 {}

	 inline
	 bool point_enabled(size_t i)
	 {
	    return _points_enabled[i];
	 }

	 inline
	 size_t num_points()
	 {
	    if (!_cloud)
	       return 0;
	    return _cloud->size();
	 }

	 inline
	 const uvector &point(size_t i)
	 {
	    return (*_cloud)[i];
	 }

         virtual void draw(bool fast)
	 {	   
	    if (_cloud->size() == 0)
	       return;

	    
// 	    double low_thresh = 0.0;
// 	    if (_field)
// 	    {
// 	       double minW = (*_field)[0], maxW = (*_field)[0];
// 	       for (size_t i = 0; i < _field->size(); ++i)
// 	       {
// 		  minW = std::min(minW, (*_field)[i]);
// 		  maxW = std::max(maxW, (*_field)[i]);
// 	       }
// 	       low_thresh = minW + _low_threshold * (maxW - minW);
// 	    }

	    const size_t strid = stride();
	    if (_points_enabled.size() != num_points())
	       _points_enabled.resize(num_points(), false);
	    std::fill(_points_enabled.begin(), _points_enabled.end(), false);
	    for (size_t i = 0; i < num_points(); i += strid)
	    {
	       if (_field && ((*_field)[i] < _low_threshold))
		  continue;
	       _points_enabled[i] = true;
	    }

	    if (_field == 0 || fast == true)
	    {
	       size_t i = 0;
	       glBegin(GL_POINTS);
	       for (size_t i = 0; i < num_points(); ++i)
	       {
		  if (point_enabled(i))
		     gl_uvertex(point(i));
	       }
	       glEnd();
	    }
	    else if (_spheres == true )
	    {
	      GLUquadric* q = gluNewQuadric();
	     
	      for (size_t i = 0; i < num_points(); ++i)
		{
		  double rr = (*_field)[i]*_radius/1000.0f;

		  if (!point_enabled(i))
		    continue;

		  size_t div = _sphere_tessel;
		  if (rr >= 0.1)
		    div *= 4;

		  const uvector & v = point(i);
		  glPushMatrix();
		  glTranslatef(v[0], v[1], v[2]);
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
		  if (!point_enabled(i))
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

	    
	    if ((!_field) || (_field->size() == 0))
	      return;

	    double max = (*_field)[0], min = (*_field)[0];
	    for (size_t i = 1; i < (*_field).size(); ++i)
	      {
		max = std::max(max, (*_field)[i]);
		min = std::min(min, (*_field)[i]);
	      }
	    _low_threshold = min;
	    editor->add_double("Low threshold:", _low_threshold, min, max);
	 }
	 
	 virtual size_t stride()
	 {
	    if (_percentage < 0.01)
	       return _cloud->size();

	    return (size_t) (1.0/_percentage);
	 }
   };

   class Cloud_subdrawer: public Drawer
   {
      protected:
	 Cloud_drawer &_parent;

      public:
	 Cloud_subdrawer(const std::string &name,
 	                 Cloud_drawer &parent,
	                 bool enab = true): Drawer(name, enab),
				_parent(parent)
	   {}
	 virtual ~Cloud_subdrawer(){};

	 inline
	 const 
	 uvector &point(size_t i)	    
	 {
	    return _parent.point(i);
	 }

	 inline
	 bool point_enabled(size_t i)
	 {
	    return _parent.point_enabled(i);
	 }

	 inline
	 size_t num_points()
	 {
	    return _parent.num_points();
	 }
   };

   typedef boost::shared_ptr<Drawer> Drawer_ptr;
   typedef std::list<Drawer_ptr> Drawer_list;

#if 0
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

         virtual void draw(bool fast = false)
	 {
	    glBegin(GL_LINES);
	    for (Data_lines::iterator it = _lines->begin();
		 it != _lines->end(); ++it)
	    {
	       Line &l = *it;

	       if ((l.first != 0) || (l.second != 0))
		  continue;

	       gl_uvertex((*_cloud) [l.first]);
	       gl_uvertex((*_cloud) [l.second]);
	    }
	    glEnd();
	 }
   };
#endif

     void _draw_mesh(const std::vector<Mesh_triangle> &triangles,
		     const Data_cloud &points,
		     size_t polygon_mode = GL_FILL)
     {
       glPolygonMode(GL_FRONT_AND_BACK, polygon_mode);

       glBegin(GL_TRIANGLES);
       for (size_t i = 0; i != triangles.size(); ++i)
	 {
	   const cloudy::Mesh_triangle &t = triangles[i];
	   
	   gl_uvertex(points[t.a]);
	   gl_uvertex(points[t.b]);
	   gl_uvertex(points[t.c]);
	 }
       glEnd();
     }
     

   class Direction_drawer: public Cloud_subdrawer
   {
	 Data_cloud_ptr _directions;
	 Data_indices_ptr _indices;
         Color _color;
	 double _length;

      public:
	 Direction_drawer(const std::string &name,
	                  Cloud_drawer &parent,
	                  const Data_cloud_ptr directions,
			  const Color &col = Red,
	                  const Data_indices_ptr indices = Data_indices_ptr()):
	    Cloud_subdrawer(name, parent),
	    _directions(directions), _indices(indices), 
	    _color(col),
	    _length(0.03)
	 {}

         virtual void fill_editor(Editor *editor) 
	 {
	    Drawer::fill_editor(editor);
	    editor->add_double("Length:", _length, 0.0, 0.1);
	    editor->add_color("Color:", _color);
	 }

         virtual void draw(bool fast = false)
	 {
	    if (fast)
	       return;

	    if (_indices)
	       return;

	    glColor3f(_color._r, _color._g, _color._b);
	    glBegin(GL_LINES);
	    size_t numen = 0;
	    for (size_t i = 0; i < num_points(); ++i)
	    {
	       if (!point_enabled(i))
		  continue;
	       
	       numen++;
	       uvector v = point(i);
	       gl_uvertex(v + (_length/2.0) * (*_directions)[i]);
	       gl_uvertex(v - (_length/2.0) * (*_directions)[i]);
	    }
	    glEnd();

	    std::cerr << "num enabled = " <<  numen << "\n";
	 }
   };

   class kDistance_balls_drawer: public Cloud_subdrawer
   {
         double _radius, _exageration;
         int _sphere_tessel;
	 
      public:
	 kDistance_balls_drawer(const std::string &name,
	                        Cloud_drawer &parent,
	                        const Data_cloud_ptr cloud,
	                        double radius = 0.0):
	    Cloud_subdrawer(name, parent),
	    _radius(radius),
	    _exageration(1.0),
	    _sphere_tessel(8)
	 {
	 }

         virtual void draw(bool fast)
	 {
	    if (fast == true)
	       return;

	    GLUquadric* q = gluNewQuadric();
	    
	    
	    for (size_t i = 0; i != num_points();  ++i)
	    {
	       double rr = _radius*_radius - point(i)[3];
	       
	       if (rr <= 0.0)
		  continue;
	       
	       rr = sqrt(rr) * _exageration;

	       size_t div = _sphere_tessel;
	       if (rr >= 0.1)
		  div *= 4;
	       
	       glPushMatrix();
	       uvector v = point(i);
	       glTranslatef(v[0], v[1], v[2]);
	       gluSphere(q, rr, div, div);
	       glPopMatrix();
	    }
	    
	    gluDeleteQuadric(q);	    
	 }

	 virtual void fill_editor(Editor *editor) 
	 {
	    Drawer::fill_editor(editor);
	    editor->add_double_spin("Radius:", _radius, 0.0, 0.1, 4);
	    editor->add_integer_spin("Tesselation", _sphere_tessel, 0, 16);
	    editor->add_double_spin("Exageration", _exageration, 0.0, 10.0);
	 }	 
   };


   class Mesh_drawer: public Drawer
   {
         Mesh_ptr _mesh;
         bool _lines;

      public:
	 Mesh_drawer(const std::string &name,
	              const Mesh_ptr mesh):
	    Drawer(name),
	    _mesh(mesh),
	    _lines(false)
	 {}

     virtual void draw(bool fast)
	 {
	   glEnable(GL_POLYGON_OFFSET_FILL);
	   glPolygonOffset(1, 1);
	   glColor3f(1.0, 0.8, 0.5);
	   _draw_mesh(_mesh->_triangles, _mesh->_points, GL_FILL);
	   glDisable(GL_POLYGON_OFFSET_FILL);

	   if(_lines)
	     {
	       glColor3f(0.1, 0.1, 0.1);
	       _draw_mesh(_mesh->_triangles, _mesh->_points, GL_LINE);
	     }
	 }

         virtual void
         draw_pov (std::ostream &os)
         {
	   _mesh->export_pov(os);
         }

	 virtual void fill_editor(Editor *editor) 
	 {
	    Drawer::fill_editor(editor);
	    editor->add_bool("Draw segments:", _lines);
	 }
   };

   class Viewer : public  cloudy::view::GL_widget
   {
	 typedef cloudy::view::GL_widget GL_widget;

	 Drawer_list _drawers;
	 
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

	    for (Drawer_list::iterator it =_drawers.begin();
		 it != _drawers.end(); ++it)
	    {
	      if ((*it)->enabled()) (*it)->draw(fast_draw);
	    }
	    
	    postpaintGL();
	 }

         void
         draw_pov (std::ostream &os)
         {
	    for (Drawer_list::iterator it =_drawers.begin();
		 it != _drawers.end(); ++it)
	    {
	      if ((*it)->enabled()) (*it)->draw_pov(os);
	    }	    
	 }

	 void add_drawer(Drawer_ptr dr)
	 {
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
