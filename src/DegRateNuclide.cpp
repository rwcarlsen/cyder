/*! \file DegRateNuclide.cpp
    \brief Implements the DegRateNuclide class used by the Generic Repository 
    \author Kathryn D. Huff
 */
#include <iostream>
#include <fstream>
#include <deque>
#include <time.h>
#include <assert.h>
#include <boost/lexical_cast.hpp>

#include "CycException.h"
#include "Logger.h"
#include "Timer.h"
#include "DegRateNuclide.h"
#include "Material.h"

using namespace std;
using boost::lexical_cast;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DegRateNuclide::DegRateNuclide(){
  set_deg_rate(0);
  tot_deg_ = 0;
  last_degraded_ = 0;
  wastes_ = deque<mat_rsrc_ptr>();

  set_geom(GeometryPtr(new Geometry()));

  vec_hist_ = VecHist();
  conc_hist_ = ConcHist();
  update_degradation(0, deg_rate_);
  update_vec_hist(0);
  //update_conc_hist(0);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DegRateNuclide::~DegRateNuclide(){
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void DegRateNuclide::initModuleMembers(QueryEngine* qe){
  double deg_rate = lexical_cast<double>(qe->getElementContent("degradation"));
  init(deg_rate);
  LOG(LEV_DEBUG2,"GRDRNuc") << "The DegRateNuclide Class initModuleMembers(qe) function has been called";;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void DegRateNuclide::init(double deg_rate) {
  set_deg_rate(deg_rate);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
NuclideModel* DegRateNuclide::copy(NuclideModel* src){
  DegRateNuclide* src_ptr = dynamic_cast<DegRateNuclide*>(src);

  set_deg_rate(src_ptr->deg_rate_);
  tot_deg_ = 0;
  last_degraded_ = TI->time();
  wastes_ = deque<mat_rsrc_ptr>();

  // copy the geometry AND the centroid. It should be reset later.
  set_geom(GeometryPtr(new Geometry()));
  geom_->copy(src_ptr->geom(), src_ptr->geom()->centroid());

  vec_hist_ = VecHist();
  conc_hist_ = ConcHist();
  update_degradation(TI->time(), deg_rate_);
  update_vec_hist(TI->time());
  update_conc_hist(TI->time());

  return (NuclideModel*)this;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void DegRateNuclide::print(){
    LOG(LEV_DEBUG2,"GRDRNuc") << "DegRateNuclide Model";;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void DegRateNuclide::absorb(mat_rsrc_ptr matToAdd)
{
  // Get the given DegRateNuclide's contaminant material.
  // add the material to it with the material absorb function.
  // each nuclide model should override this function
  LOG(LEV_DEBUG2,"GRDRNuc") << "DegRateNuclide is absorbing material: ";
  matToAdd->print();
  wastes_.push_back(matToAdd);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void DegRateNuclide::extract(const CompMapPtr comp_to_rem, double kg_to_rem)
{
  // Get the given DegRateNuclide's contaminant material.
  // add the material to it with the material extract function.
  // each nuclide model should override this function
  LOG(LEV_DEBUG2,"GRDRNuc") << "DegRateNuclide" << "is extracting composition: ";
  comp_to_rem->print() ;
  MatTools::extract(comp_to_rem, kg_to_rem, wastes_);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void DegRateNuclide::transportNuclides(int the_time){
  // This should transport the nuclides through the component.
  // It will likely rely on the internal flux and will produce an external flux. 
  update_degradation(the_time, deg_rate_);
  update_vec_hist(the_time);
  update_conc_hist(the_time);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void DegRateNuclide::set_deg_rate(double deg_rate){
  if( deg_rate < 0 || deg_rate > 1 ) {
    stringstream msg_ss;
    msg_ss << "The DegRateNuclide degradation rate range is 0 to 1, inclusive.";
    msg_ss << " The value provided was ";
    msg_ss << deg_rate;
    msg_ss <<  ".";
    LOG(LEV_ERROR,"GRDRNuc") << msg_ss.str();;
    throw CycRangeException(msg_ss.str());
  } else {
    deg_rate_ = deg_rate;
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
double DegRateNuclide::contained_mass(int the_time){ 
  return vec_hist(the_time).second;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
double DegRateNuclide::contained_mass(){
  return contained_mass(last_degraded_);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
pair<IsoVector, double> DegRateNuclide::source_term_bc(){
  return make_pair(contained_vec(last_degraded_), 
      tot_deg()*contained_mass(last_degraded_));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
IsoConcMap DegRateNuclide::dirichlet_bc(){
  IsoConcMap dirichlet, whole_vol;
  whole_vol = conc_hist(last_degraded_);
  IsoConcMap::const_iterator it;
  for( it=whole_vol.begin(); it!=whole_vol.end(); ++it){
    dirichlet[(*it).first] = tot_deg()*(*it).second ;
  }
  return dirichlet;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
ConcGradMap DegRateNuclide::neumann_bc(IsoConcMap c_ext, Radius r_ext){
  ConcGradMap to_ret;

  IsoConcMap c_int = conc_hist(last_degraded_);
  Radius r_int = this->radial_midpoint();
  
  int iso;
  IsoConcMap::iterator it;
  for( it=c_int.begin(); it != c_int.end(); ++it){
    iso = (*it).first;
    if( c_ext.count(iso) != 0) {  
      // in both
      to_ret[iso] = calc_conc_grad(c_ext[iso], c_int[iso]*tot_deg(), r_ext, r_int);
    } else {  
      // in c_int_only
      to_ret[iso] = calc_conc_grad(0, c_int[iso]*tot_deg(), r_ext, r_int);
    }
  }
  for( it=c_ext.begin(); it != c_ext.end(); ++it){
    iso = (*it).first;
    if( c_int.count(iso) == 0) { 
      // in c_ext only
      to_ret[iso] = calc_conc_grad(c_ext[iso], 0, r_ext, r_int);
    }
  }

  return to_ret;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
ConcGrad DegRateNuclide::calc_conc_grad(Concentration c_ext, Concentration 
    c_int, Radius r_ext, Radius r_int){

  ConcGrad to_ret;
  if(r_ext <= r_int){ 
    stringstream msg_ss;
    msg_ss << "The outer radius must be greater than the inner radius.";
    LOG(LEV_ERROR, "GRDRNuc") << msg_ss.str();;
    throw CycRangeException(msg_ss.str());
  } else if( r_ext == numeric_limits<double>::infinity() ){
    stringstream msg_ss;
    msg_ss << "The outer radius cannot be infinite.";
    LOG(LEV_ERROR, "GRDRNuc") << msg_ss.str();;
    throw CycRangeException(msg_ss.str());
  } else {
    to_ret = (c_ext - c_int) / (r_ext - r_int) ;
  }
  return to_ret;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
Radius DegRateNuclide::radial_midpoint(){
  Radius to_ret;
  if(geom_->outer_radius() == numeric_limits<double>::infinity()) { 
    to_ret = numeric_limits<double>::infinity();
  } else { 
    to_ret = geom_->outer_radius() - (geom_->outer_radius() - geom_->inner_radius())/2;
  }
  return to_ret;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
IsoConcMap DegRateNuclide::cauchy_bc(){
  /// @TODO This is just a placeholder
  return conc_hist(last_degraded_); 
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
IsoConcMap DegRateNuclide::update_conc_hist(int the_time){
  return update_conc_hist(the_time, wastes_);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
IsoConcMap DegRateNuclide::update_conc_hist(int the_time, deque<mat_rsrc_ptr> mats){
  assert(last_degraded_ <= the_time);

  IsoConcMap to_ret;

  pair<IsoVector, double> sum_pair; 
  sum_pair = vec_hist(the_time);

  if(sum_pair.second != 0 && geom_->volume() != numeric_limits<double>::infinity()) { 
    double scale = sum_pair.second/geom_->volume();
    CompMapPtr curr_comp = sum_pair.first.comp();
    CompMap::const_iterator it;
    for(it = (*curr_comp).begin(); it != (*curr_comp).end(); ++it) {
      to_ret[ (*it).first ] = ((*it).second)*scale ;
    }
  } else {
    to_ret[ 92235 ] = 0; 
  }
  conc_hist_[the_time] = to_ret ;
  return to_ret;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
double DegRateNuclide::update_degradation(int the_time, double deg_rate){
  assert(last_degraded_ <= the_time);
  assert(deg_rate<=1.0 && deg_rate >= 0.0);
  double total = 0;
  total = tot_deg() + deg_rate*(the_time - last_degraded_);
  tot_deg_ = min(1.0, total);
  last_degraded_ = the_time;

  return tot_deg_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
double DegRateNuclide::tot_deg(){
  return tot_deg_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void DegRateNuclide::update_vec_hist(int the_time){
  // CompMapPtr one_comp = CompMapPtr(new CompMap(MASS));
  // (*one_comp)[92235] = 1;
  // vec_hist_[ the_time ] = make_pair(IsoVector(one_comp), 10*(1-tot_deg()));
  vec_hist_[ the_time ] = MatTools::sum_mats(wastes_) ;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
IsoVector DegRateNuclide::contained_vec(int the_time){
  IsoVector to_ret = vec_hist(the_time).first;
  return to_ret;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
pair<IsoVector, double> DegRateNuclide::vec_hist(int the_time){
  pair<IsoVector, double> to_ret;
  VecHist::const_iterator it;
  if( !vec_hist_.empty() ) {
    it = vec_hist_.find(the_time);
    if( it != vec_hist_.end() ){
      to_ret = (*it).second;
      assert(to_ret.second < 1000 );
    } 
  } else { 
    CompMapPtr zero_comp = CompMapPtr(new CompMap(MASS));
    (*zero_comp)[92235] = 0;
    to_ret = make_pair(IsoVector(zero_comp),0);
  }
  return to_ret;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
IsoConcMap DegRateNuclide::conc_hist(int the_time){
  IsoConcMap to_ret;
  ConcHist::iterator it;
  it = conc_hist_.find(the_time);
  if( the_time == last_degraded_ && it != conc_hist_.end() ){
    to_ret = (*it).second;
  } else {
    to_ret[92235] = 0 ; // zero
  }
  return to_ret;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
Concentration DegRateNuclide::conc_hist(int the_time, Iso tope){
  Concentration to_ret;
  IsoConcMap conc_map = conc_hist(the_time);
  IsoConcMap::iterator it;
  it = conc_map.find(tope);
  if(it != conc_map.end()){
    to_ret = (*it).second;
  }else{
    to_ret = 0;
  }
  return to_ret;
}


