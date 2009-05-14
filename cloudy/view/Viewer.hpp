#include <cloudy/view/Widget.hpp>
#include <cloudy/Cloud.hpp>
#include <boost/shared_ptr.hpp>


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
	 bool _enabled;

      public:
	 Drawer(bool enab = true): _enabled(enab) {}

	 virtual ~Drawer(){};
	 virtual void draw(size_t stride) = 0;

	 void enable()  {_enabled = true;} 
	 void disable() {_enabled = false;} 
	 bool enabled() {return _enabled;}
   };

      typedef boost::shared_ptr<Drawer> Drawer_ptr;

   class Direction_field_drawer: public Drawer
   {
	 Data_cloud_ptr _cloud;
	 Data_cloud_ptr _directions;
	 Data_indices_ptr _indices;

      public:
	 Direction_field_drawer(const Data_cloud_ptr cloud,
	                        const Data_indices_ptr indices,
	                        const Data_cloud_ptr directions):
	    _cloud(cloud), _indices(indices), _directions(directions)
	 {}

	 virtual void draw(size_t stride)
	 {
	 }
   };

   inline void
   gl_uvertex(const uvector &v)
   {
      	       glVertex3f(v(0), v(1), v(2));
   }

   class Line_drawer: public Drawer
   {
	 Data_cloud_ptr _cloud;
	 Data_lines_ptr _lines;

      public:
	 Line_drawer(const Data_cloud_ptr cloud,
	             const Data_lines_ptr lines): 
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
	 
      public:
	 Cloud_drawer(const Data_cloud_ptr cloud):
	    _cloud(cloud)
	 {}

	 virtual void draw(size_t stride)
	 {
	    glBegin(GL_POINTS);
	    for (Data_cloud::iterator it = _cloud->begin();
		 it != _cloud->end(); ++it)
	       gl_uvertex(*it);
	    glEnd();
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
   };

}
}
