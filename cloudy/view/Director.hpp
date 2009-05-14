// Copyright (c) 2006 INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// Author: Camille Wormser
// Version: 1.0

#ifndef CLOUDY_DIRECTOR_HPP
#define CLOUDY_DIRECTOR_HPP

#include <math.h>
#include <stdio.h>
#include <iostream>

namespace cloudy
{

   namespace view
   {

      class Director
      {
	 private:
	    double vCenter[3];
	    double vRadius;
	    double vMat[16];
	    double tRotation[4];
	    double vRotation[4];
	    double vCut[4];
	    
	    static void qRotation(double x, double y,
	                          double xx, double yy, double* q);
	    static void qMulti(double* q, double* r);
	    static void qMultiVect(double* q, double* v);
	    static void qBar(double* r);
      
	    void matTransfo(double* mat, bool avecTransZoom);
	    
	 public:
	    Director();
	    ~Director();
	    
	    Director(const Director& b);
	    
	    // Dans toute la suite, les coordonnées sont données entre
	    // -1 et 1. L'orientation des axes est l'orientation
	    // mathématique usuelle : x vers le haut, y vers la droite.
	    
	    // position du centre avant rotation, intégré à la matrice de
	    // transformation
	    const double* center();
	    
	    void image(double* point);
	    
	    void preImageSansTransZoom(double* point);
	    
	    // rayon, facteur de zoom, est intégré à la matrice de
	    // transformation
	    double&       radius();
	    
	    // matrice de transformation, c'est le pointeur à donner
	    // à OpenGL par glLoadMatrixd avant d'afficher l'objet
	    const double* mat();
	    
	    const double* matSansTransZoom();
	    
	    const double* tempMat();
	    
	    void tempReset();
	    
	    // équation du plan de coupe, solidaire du repère de l'objet,
	    // à donner à OpenGL par glClipPlane avant glLoadMatrixd
	    const double* cut();
	    
	    // hauteur du plan de coupe
	    double&       hCut();
	    
	    // effectue la rotation autour de O = (0, 0, 0) qui correspond
	    // au déplacement de la souris de (x,y) à (xx, yy).
	    void rotation(double x, double y, double xx, double yy);
	    
	    void rotationObj(double x, double y, double xx, double yy);
	    
	    // effectue la rotation du plan de coupe autour de "centre" 
	    // qui correspond au déplacement de la souris de (x,y) à (xx, yy).
	    void rotationCut(double x, double y, double xx, double yy);
	    
	    void rotZtoCut(double* v, double& angle);
	    
	    // effectue la translation qui correspond au déplacement
	    // à l'écran de (dx, dy, dz). En général dz = 0.
	    void translation(double dx, double dy, double dz);
	    
	    // remet le centre à la position voulue.
	    void recenter(double x, double y, double z);
	    
	    // place dans (X,Y,Z) les coordonnées du point sélectionné à la
	    // souris en cliquant en (x,y)
	    void select(double x, double y, 
	                double& X, double& Y, double& Z,
                        double& XX, double& YY, double& ZZ);
	    
	    // renvoie la hauteur de la souris sur la boule dans la 
	    // direction du plan de coupe
	    double height(double x, double y);
	    
      };
   }
}


#endif
