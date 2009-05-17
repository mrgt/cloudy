#include <cloudy/view/Widget.hpp>
#include <cloudy/view/Editor.hpp>
#include <cloudy/Cloud.hpp>
#include <boost/shared_ptr.hpp>
#include <QTabWidget>

namespace cloudy
{
   namespace view {

   typedef std::vector<size_t> Data_indices;
   typedef std::pair<size_t, size_t> Line;
   typedef std::vector<Line> Data_lines;
   

   typedef cloudy::uvector uvector;
   typedef cloudy::Data_cloud Data_cloud;

   typedef boost::shared_ptr<Data_cloud> Data_cloud_ptr;
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
	 virtual void draw(size_t stride) = 0;

	 virtual void fill_editor(Editor *editor) 
	 {
	    editor->add_bool("Enable:", _enabled);
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

	 virtual void draw(size_t stride)
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

	 virtual void draw(size_t stride)
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
	 double _percentage;
	 
      public:
	 Cloud_drawer(const std::string &name,
	              const Data_cloud_ptr cloud):
	    Drawer(name),
	    _cloud(cloud)
	 {}

	 virtual void draw(size_t stride)
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

	 virtual void fill_editor(Editor *editor) 
	 {
	    Drawer::fill_editor(editor);
	    editor->add_double("Percentage:", _percentage);
	 }
   };

   class Viewer : public  cloudy::view::GL_widget
   {
	 typedef cloudy::view::GL_widget GL_widget;
	 typedef std::list<Drawer_ptr> Drawer_list;

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
	       if ((*it)->enabled()) (*it)->draw(1);
	    }
	    
	    postpaintGL();
	 }

	 void add_drawer(Drawer_ptr dr)
	 {
	    _drawers.push_back(dr);
	 }

	 void fill_dialog(QTabWidget *tab)
	 {
	    for(Drawer_list::iterator it =_drawers.begin();
		it != _drawers.end(); ++it)
	    {
	       Editor *edit = new Editor();
	       (*it)->fill_editor(edit);
	       edit->finish();
	       tab->addTab(edit, (*it)->name().c_str());

	       connect(edit, SIGNAL(stateChanged()),
	               this, SLOT(updateGL()));
	    }
	 }
   };

}
}
