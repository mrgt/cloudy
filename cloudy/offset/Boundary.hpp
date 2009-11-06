#ifndef PCT_BOUNDARY_HPP
#define PCT_BOUNDARY_HPP

namespace cloudy { namespace offset {

      template <class RT, class Subdivider, class Integrator>
      void
      aggregate (const RT &rt,
                 typename RT::Vertex_handle v,
                 Subdivider &sub,
                 Integrator &ig) 
      {
	 typedef typename RT::Point Point;
	 typedef typename RT::Edge Edge;
	 typedef typename RT::Cell_handle Cell_handle;
	 typedef typename RT::Vertex_handle Vertex_handle;
	 
	 Point A = v->point();
	 
	 // get all vertices incident to v
	 std::list<Vertex_handle> vertices;
	 rt.incident_vertices(v,std::back_inserter(vertices));
   
	 typename std::list<Vertex_handle>::iterator it;
	 for(it = vertices.begin(); it != vertices.end(); it++)
	 {
	    // build edge from two vertices
	    Cell_handle cell;
	    int i1,i2;
	    
	    if(!rt.is_edge(v, *it, cell, i1, i2))
	       continue;
	    
	    // tesselate the polygon around its first vertex
	    typename RT::Cell_circulator c = rt.incident_cells(cell, i1, i2);
	    typename RT::Cell_circulator done = c;
	    const Point B (rt.dual(c)); c++;
	    
	    while (c != done)
	    {
	       const Point u (rt.dual(c)); c++;
	       const Point v (rt.dual(c));
	       sub.aggregate(ig, B-A, u-A, v-A);
	    }
	 }
      }
   }
}
#endif
