#ifndef CLOUDY_INTEGRATORS_HPP
#define CLOUDY_INTEGRATORS_HPP

#include <cloudy/mesh/Mesh.hpp>
#include <cloudy/linear/Linear.hpp>
#include <math.h>

namespace cloudy {
   namespace offset {

      // The concept of Integrator should implement two functions:
      // - aggregate(a,b,c) adds a tetrahedron whose vertices are 0, a, b, c;
      // - result() which returns the result of the computation.

      template <class K>
      class Volume_integrator
      {
	 public:
	    typedef typename K::Point_3 Point;
	    typedef typename K::Vector_3 Vector;
	    typedef typename K::FT Result_type;
	    
	 private:
	    Result_type _result;
	    Point _center;
	    
	 public:
	    Volume_integrator(const Point &center) :
	       _result(0),
	       _center(center)
	    {}
	    
	    void aggregate(const Vector &a, 
	                   const Vector &b,
	                   const Vector &c)
	    {
	       const double  vol =  1.0/6.0*fabs(a * CGAL::cross_product(b,c));
#if 0
	       std::cerr << "a = " << a << "\n"
			 << "b = " << b << "\n"
			 << "c = " << c << "\n"
			 << "vol = " << vol << "\n";
#endif
	       _result += vol;
	    }
	    
	    const Result_type &result() const
	    {
	       return _result;
	    }
      };
      
      
      //////////////////////////////////////////////////////////////////////

      // The coordinates of v are M11 M12 M13 M21 M23 M33
      typedef cloudy::uvector Covariance_vector;
      
      template <class K>
      class Covariance_integrator
      {
	 public:
	    typedef typename K::Point_3 Point;
	    typedef typename K::Vector_3 Vector;
	    typedef typename K::FT FT;
	    typedef Covariance_vector Result_type;
	    
	 private:
	    Result_type _result;
	    Point _center;
	    
	 public:
	    Covariance_integrator(const Point &center):
	       _result(6),
	       _center(center)
	    {
	       std::fill(_result.begin(), _result.end(), 0);
	    }
	    
	    void aggregate(const Vector &a, 
	                   const Vector &b,
	                   const Vector &c)
	    {
	       const double &m11 = a.x(), &m12 = b.x(), &m13 = c.x();
	       const double &m21 = a.y(), &m22 = b.y(), &m23 = c.y();
	       const double &m31 = a.z(), &m32 = b.z(), &m33 = c.z();
	       
	       const double det60 =
		  fabs(m11*m33*m22 - m11*m32*m23 - m21*m12*m33 +
		       m21*m13*m32 + m31*m12*m23 - m31*m13*m22)/6.0;
	       
	       const double
		  R11 = (m11*m11 + m11*m12 + m11*m13 +
		         m12*m12 + m12*m13 + m13*m13) * det60,
		  R12 = (m11*m21 + m11*m22/2.0 + m11*m23/2.0 +
		         m12*m21/2.0 + m12*m22 + m12*m23/2.0 +
		         m13*m21/2.0 + m13*m22/2.0 + m13*m23) * det60,
		  R13 = (m11*m31 + m11*m32/2.0 + m11*m33/2.0 + 
		         m12*m31/2.0 + m12*m32 + m12*m33/2.0 + 
		         m13*m31/2.0 + m13*m32/2.0 + m13*m33) * det60,
		  R22 = (m21*m21 + m21*m22 + m21*m23 + 
		         m22*m22 + m22*m23 + m23*m23) * det60,
		  R23 = (m31*m21 + m31*m22/2.0 + m31*m23/2.0 +
		         m32*m21/2.0 + m32*m22 + m32*m23/2.0 +
		         m33*m21/2.0 + m33*m22/2.0 + m33*m23) * det60,
		  R33 = (m31*m31 + m31*m32 + m31*m33 +
		         m32*m32 + m32*m33 + m33*m33) * det60;
	       _result(0) += R11;
	       _result(1) += R12;
	       _result(2) += R13;

	       _result(3) += R22;
	       _result(4) += R23;

	       _result(5) += R33;
	    }
	    
	    const Result_type &result() const
	    {
	       return _result;
	    }
	    
      };

      //////////////////////////////////////////////////////////////////////
     template <class K>
     class Mesh_integrator
     {
     public:
	    typedef typename K::Point_3 Point;
	    typedef typename K::Vector_3 Vector;	
       	    typedef Mesh Result_type;

     private:
	    Point _center;
            Mesh _result;

            uvector uv(const Point &point)
            {
	      uvector v(3);
	      v(0) = point.x();
	      v(1) = point.y();
	      v(2) = point.z();
	      return v;
            }

	 public:
	    Mesh_integrator(const Point &center):
	       _center(center)
	    {}

	    inline
	    void aggregate(const Vector &a, 
	                   const Vector &b,
	                   const Vector &c)
	    {
	      _result.append_triangle(uv(_center+a),
				      uv(_center+b),
				      uv(_center+c));
	    }


       	    const Result_type &result() const
	    {
	       return _result;
	    }
	    
      };

   } 
}

#endif
