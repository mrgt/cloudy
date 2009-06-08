// Copyright (c) 2006 INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// Author: Camille Wormser
// Version: 1.0

#include <algorithm>
#include <cloudy/view/Director.hpp>

namespace cloudy {
   namespace view {

Director::Director() {
  vCenter[0] = vCenter[1] = vCenter[2] = 0.;
  vRadius = 1.;
  vRotation[0] = 1.;
  vRotation[1] = vRotation[2] = vRotation[3] = 0.;
  tRotation[0] = 1.;
  tRotation[1] = tRotation[2] = tRotation[3] = 0.;
  vCut[0] = 1;
  vCut[1] = vCut[2] = 0.;
  vCut[3] = 0.1;
}

Director::~Director() {
}

Director::Director(const Director& b)
{
  std::copy(b.vCenter, b.vCenter+3, vCenter);

  vRadius = b.vRadius;

  std::copy(b.vRotation, b.vRotation+4, vRotation);

  std::copy(b.vCut, b.vCut+4, vCut);

  std::copy(b.vMat, b.vMat+16, vMat);
}

// calcule le quaternion de la rotation définie par le glissement de souris
// (x,y) -> (xx, yy) 
void Director::qRotation(double x, double y, double xx, double yy, double* q) {
  x = x;
  y = y;
  xx = xx;
  yy = yy;
  double norm = sqrt(x*x + y*y);
  if(norm > .9999) {
    x = .9999*x/norm;
    y = .9999*y/norm;
  }
  norm = sqrt(xx*xx + yy*yy);
  if(norm > .9999) {
    xx = .9999*xx/norm;
    yy = .9999*yy/norm;
  }
  // std::cout << 1. - x*x - y*y << " xx\n";
  double z = sqrt(1. - x*x - y*y);
  double zz = sqrt(1. - xx*xx - yy*yy);
  // calcule le vecteur de rotation
  double rx = y*zz - z*yy;
  double ry = z*xx - x*zz;
  double rz = x*yy - y*xx;
  double n = sqrt(rx*rx + ry*ry + rz*rz);
  rx = rx/2.;
  ry = ry/2.;
  rz = rz/2.;
  // en pratique, angle petit :
  // et n = sin t = t
  double cos = sqrt(1. - n*n/4.);
  //  std::cout << "cos t/2 = " << cos << "  et  sin t/2 v = " << rx << ";"<< ry << ";"<< rz << "\n";
  // calcule le quaternion
  q[0] = cos;
  q[1] = rx;
  q[2] = ry;
  q[3] = rz;
}

// effectue r = q*r
void Director::qMulti(double* q, double* r) {
  double rx = q[0]*r[0] - q[1]*r[1] - q[2]*r[2] - q[3]*r[3];
  double ry = q[0]*r[1] + q[1]*r[0] + q[2]*r[3] - q[3]*r[2];
  double rz = q[0]*r[2] + q[2]*r[0] + q[3]*r[1] - q[1]*r[3];
  double rt = q[0]*r[3] + q[3]*r[0] + q[1]*r[2] - q[2]*r[1];
  r[0] = rx;
  r[1] = ry;
  r[2] = rz;
  r[3] = rt;
}

// effectue r = bar(r)
void Director::qBar(double* r) {
  r[1] = -r[1];
  r[2] = -r[2];
  r[3] = -r[3];
}

// effectue v = Rot(q)*v où v est un vecteur
// en calculant q*v*bar(q)
// attention, si q n'est pas unitaire, il faudra
// renormaliser d'un facteur |q|^2
void Director::qMultiVect(double* q, double* v) {
  double w[] = {0., v[0], v[1], v[2]};
  qBar(w);
  qMulti(q,w);
  qBar(w);
  qMulti(q,w);
  v[0] = w[1];
  v[1] = w[2];
  v[2] = w[3];
}

void Director::matTransfo(double* Rot, bool avecTransZoom) {
  double xx = Rot[1]*Rot[1];
  double xy = Rot[1]*Rot[2];
  double xz = Rot[1]*Rot[3];
  double xw = Rot[1]*Rot[0];
  double yy = Rot[2]*Rot[2];
  double yz = Rot[2]*Rot[3];
  double yw = Rot[2]*Rot[0];
  double zz = Rot[3]*Rot[3];
  double zw = Rot[3]*Rot[0];
  if(avecTransZoom) {
  //  std::cout << "fin précalc\n";
  vMat[0]  = vRadius*(1 - 2*(yy + zz));
  vMat[1]  = vRadius*     2*(xy + zw);
  vMat[2]  = vRadius*     2*(xz - yw);
  vMat[4]  = vRadius*     2*(xy - zw);
  vMat[5]  = vRadius*(1 - 2*(xx + zz));
  vMat[6]  = vRadius*     2*(yz + xw);
  vMat[8]  = vRadius*     2*(xz + yw);
  vMat[9]  = vRadius*     2*(yz - xw);
  vMat[10] = vRadius*(1 - 2*(xx + yy));
  vMat[3]  = vMat[7] = vMat[11] = 0;

  vMat[12] = vRadius*vCenter[0];
  vMat[13] = vRadius*vCenter[1];
  vMat[14] = vRadius*vCenter[2];

  // si on veut zoomer par rapport au centre de l'objet
  //  vMat[12] = vCenter[0];
  //  vMat[13] = vCenter[1];
  //  vMat[14] = vCenter[2];
  vMat[15] = 1;
  }
  else {
  vMat[0]  = (1 - 2*(yy + zz));
  vMat[1]  =      2*(xy + zw);
  vMat[2]  =      2*(xz - yw);
  vMat[4]  =      2*(xy - zw);
  vMat[5]  = (1 - 2*(xx + zz));
  vMat[6]  =      2*(yz + xw);
  vMat[8]  =      2*(xz + yw);
  vMat[9]  =      2*(yz - xw);
  vMat[10] = (1 - 2*(xx + yy));
  vMat[3]  = vMat[7] = vMat[11] = 0;
  vMat[12] = 0.;
  vMat[13] = 0.;
  vMat[14] = 0.;
  vMat[15] = 1;
  }
}

void Director::rotZtoCut(double* v, double& angle) {
  double n = 1 - vCut[2]*vCut[2];
  if(n <= 0.0001) {
    v[0] = 1.;
    v[1] = v[2] = 0.;
    if(vCut[2] > 0.) 
      angle = 0.;
    else 
      angle = 3.1416;
  }
  else {
    n = sqrt(n);
    v[0] = -vCut[1]/n;
    v[1] = vCut[0]/n;
    v[2] = 0.;
    if(vCut[2] > 0.)
      angle = asin(n);
    else 
      angle = 3.1416 - asin(n);
  }
}

const double* Director::center() {
  return vCenter;
}

void Director::preImageSansTransZoom(double* point) {
  qBar(vRotation);
  qMultiVect(vRotation, point);
  qBar(vRotation);
}

void Director::image(double* point) {
  qMultiVect(vRotation, point);
  point[0] += vCenter[0];
  point[1] += vCenter[1];
  point[2] += vCenter[2];
  point[0] *= vRadius;
  point[1] *= vRadius;
  point[2] *= vRadius;
}

double& Director::radius() {
  return vRadius;
}

const double* Director::mat() {
  matTransfo(vRotation, true);
  return vMat;
}

const double* Director::matSansTransZoom() {
  matTransfo(vRotation, false);
  return vMat;
}

const double* Director::tempMat() {
  matTransfo(tRotation, false);
  return vMat;
}

void Director::tempReset() {
  tRotation[0] = 1.;
  tRotation[1] = tRotation[2] = tRotation[3] = 0.;
}

const double* Director::cut() {
  return vCut;
}

double& Director::hCut() {
  return vCut[3];
}

void Director::rotation(double x, double y, double xx, double yy) {
  double q[] = {0., 0., 0., 0.};
  qRotation(x, y, xx, yy, q);
  qMulti(q, vRotation);
  qMultiVect(q, vCenter);
  qMulti(q, tRotation);
}

void Director::rotationObj(double x, double y, double xx, double yy) {
  double q[] = {0., 0., 0., 0.};
  qRotation(x, y, xx, yy, q);
  qMulti(q, vRotation);
}

void Director::rotationCut(double x, double y, double xx, double yy) {
  double q[] = {0., 0., 0., 0.};
  qRotation(x, y, xx, yy, q);
  qBar(q);
  qBar(vRotation);
  qMulti(vRotation, q);
  qBar(q);
  qMulti(vRotation, q);
  qBar(vRotation);
  qMultiVect(q, vCut);
}

void Director::recenter(double x, double y, double z) {
  vCenter[0] = -x;
  vCenter[1] = -y;
  vCenter[2] = -z;

  qMultiVect(vRotation, vCenter);
  vCenter[0] *= vRadius;
  vCenter[1] *= vRadius;
  vCenter[2] *= vRadius;
  
}

void Director::translation(double dx, double dy, double dz) {
  vCenter[0] += dx/vRadius;
  vCenter[1] += dy/vRadius;
  vCenter[2] += dz/vRadius;

  // si on veut zoomer par rapport au centre de l'objet
  //  vCenter[0] += dx;
  //  vCenter[1] += dy;
  //  vCenter[2] += dz;
}

void Director::select(double x, double y, 
		      double& X, double& Y, double& Z,
		      double& XX, double& YY, double& ZZ) {
  double norm = sqrt(x*x + y*y);
  if(norm > .9999) {
    x = .9999*x/norm;
    y = .9999*y/norm;
  }
  double w[] = {x, y, sqrt(1. - x*x - y*y)};
  w[0] -= vCenter[0];
  w[1] -= vCenter[1];
  w[2] -= vCenter[2];
  w[0] /= vRadius;
  w[1] /= vRadius;
  w[2] /= vRadius;
  qBar(vRotation);
  qMultiVect(vRotation, w);
  qBar(vRotation);
  X = w[0];
  Y = w[1];
  Z = w[2];
  double h = (w[0]*vCut[0] + w[1]*vCut[1] + w[2]*vCut[2] + vCut[3])/
             (vCut[0]*vCut[0] + vCut[1]*vCut[1] + vCut[2]*vCut[2]);
  XX = w[0] - h*vCut[0];
  YY = w[1] - h*vCut[1];
  ZZ = w[2] - h*vCut[2];
}

double Director::height(double x, double y) {
  double norm = sqrt(x*x + y*y);
  if(norm > .9999) {
    x = .9999*x/norm;
    y = .9999*y/norm;
  }
    double w[] = {x, y, -sqrt(1. - x*x - y*y)};
  w[0] -= vCenter[0];
  w[1] -= vCenter[1];
  w[2] -= vCenter[2];
  w[0] /= vRadius;
  w[1] /= vRadius;
  w[2] /= vRadius;
  qBar(vRotation);
  qMultiVect(vRotation, w);
  qBar(vRotation);

  return w[0]*vCut[0] + w[1]*vCut[1] + w[2]*vCut[2];
}

   }
}
