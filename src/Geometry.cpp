/** \file Geometry.cpp
 * \brief Implements the Geometry class that defines the cylindrical extent of a 
 * component
 * \author Kathryn D. Huff
 */

#include <iostream>
#include <boost/math/constants/constants.hpp>

#include "Geometry.h"
#include "CycException.h"

using namespace std;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
Geometry::Geometry() {
  inner_radius_ = 0; // 0 indicates a solid
  outer_radius_ = numeric_limits<double>::infinity(); // inf indicates an infinite object.
  point_t origin = {0,0,0}; 
  centroid_ = origin; // by default, the origin is the centroid
  length_ = 0;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
Geometry::Geometry(Radius inner_radius, Radius outer_radius,
    point_t centroid, Length length) {
  set_radius(INNER, inner_radius); 
  set_radius(OUTER, outer_radius); 
  set_centroid(centroid); 
  set_length(length);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
GeometryPtr Geometry::copy(GeometryPtr src, point_t centroid){
  // need a fresh central position for each geometry,
  // no two objects may have exactly the same properties.
  // http://plato.stanford.edu/entries/identity-indiscernible/
  GeometryPtr to_ret = GeometryPtr(new Geometry(src->inner_radius(),
      src->outer_radius(),
      centroid, src->length()));
  return to_ret;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void Geometry::set_radius(BoundaryType boundary, Radius radius) { 
  switch(boundary){
    case INNER:
      inner_radius_ = radius;
      break;
    case OUTER:
      outer_radius_ = radius;
      break;
    default:
      throw CycException("Only INNER or OUTER radii may be set.");
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void Geometry::set_centroid(point_t centroid) { 
  centroid_=centroid;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void Geometry::set_length(Length length) { 
  length_=length;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
const Radius Geometry::inner_radius(){return inner_radius_;}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
const Radius Geometry::outer_radius(){return outer_radius_;}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
const point_t Geometry::centroid(){return centroid_;}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
const Length Geometry::length(){return length_;}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
const double Geometry::x(){return (centroid()).x_;}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
const double Geometry::y(){return (centroid()).y_;}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
const double Geometry::z(){return (centroid()).z_;}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
const Volume Geometry::volume(){
  // infinite until proven finite
  double infty = numeric_limits<double>::infinity(); 
  Volume to_ret = infty; 
  if( outer_radius() != infty ) { 
    to_ret = solid_volume( outer_radius(), length() )
           - solid_volume( inner_radius(), length() );
  }
  return to_ret;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
const Radius Geometry::radial_midpoint(){
  Radius to_ret;
  if(outer_radius_ == numeric_limits<double>::infinity()) { 
    to_ret = numeric_limits<double>::infinity();
  } else { 
    to_ret = outer_radius_ - (outer_radius_ - inner_radius_)/2;
  }
  return to_ret;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
Volume Geometry::solid_volume(Radius radius, Length length){
  const Volume pi = boost::math::constants::pi<double>();
  return pi*radius*radius*length;
}

