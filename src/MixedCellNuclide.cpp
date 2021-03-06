/*! \file MixedCellNuclide.cpp
    \brief Implements the MixedCellNuclide class used by the Generic Repository 
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
#include "MixedCellNuclide.h"
#include "Material.h"
#include "SolLim.h"

using namespace std;
using boost::lexical_cast;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
MixedCellNuclide::MixedCellNuclide():
  deg_rate_(0),
  tot_deg_(0),
  last_degraded_(0),
  v_(0),
  porosity_(0),
  sol_limited_(true),
  kd_limited_(true)
{
  wastes_ = deque<mat_rsrc_ptr>();
  set_geom(GeometryPtr(new Geometry()));
  last_updated_=0;
  vec_hist_ = VecHist();
  conc_hist_ = ConcHist();
  update(0);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
MixedCellNuclide::MixedCellNuclide(QueryEngine* qe) : 
  deg_rate_(0),
  tot_deg_(0),
  last_degraded_(0),
  v_(0),
  porosity_(0),
  sol_limited_(true),
  kd_limited_(true)
{
  wastes_ = deque<mat_rsrc_ptr>();
  set_geom(GeometryPtr(new Geometry()));
  last_updated_=0;
  vec_hist_ = VecHist();
  conc_hist_ = ConcHist();
  update(0);
  initModuleMembers(qe);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
MixedCellNuclide::~MixedCellNuclide(){
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void MixedCellNuclide::initModuleMembers(QueryEngine* qe){
  set_v(lexical_cast<double>(qe->getElementContent("advective_velocity")));
  set_deg_rate(lexical_cast<double>(qe->getElementContent("degradation")));
  set_kd_limited(lexical_cast<bool>(qe->getElementContent("kd_limited")));
  set_porosity(lexical_cast<double>(qe->getElementContent("porosity")));
  set_sol_limited(lexical_cast<bool>(qe->getElementContent("sol_limited")));
  LOG(LEV_DEBUG2,"GRDRNuc") << "The MixedCellNuclide Class initModuleMembers(qe) function has been called";;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
NuclideModelPtr MixedCellNuclide::copy(const NuclideModel& src){
  const MixedCellNuclide* src_ptr = dynamic_cast<const MixedCellNuclide*>(&src);

  set_v(src_ptr->v());
  set_deg_rate(src_ptr->deg_rate());
  set_kd_limited(src_ptr->kd_limited());
  set_porosity(src_ptr->porosity());
  set_sol_limited(src_ptr->sol_limited());
  set_tot_deg(0);
  set_last_degraded(TI->time());

  // copy the geometry AND the centroid. It should be reset later.
  set_geom(GeometryPtr(new Geometry()));
  geom_->copy(src_ptr->geom(), src_ptr->geom()->centroid());

  wastes_ = deque<mat_rsrc_ptr>();
  vec_hist_ = VecHist();
  conc_hist_ = ConcHist();
  update(TI->time());

  return shared_from_this();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void MixedCellNuclide::update(int the_time) {
  update_vec_hist(the_time);
  update_conc_hist(the_time);
  set_last_updated(the_time);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void MixedCellNuclide::print(){
    LOG(LEV_DEBUG2,"GRDRNuc") << "MixedCellNuclide Model";;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void MixedCellNuclide::absorb(mat_rsrc_ptr matToAdd)
{
  // Get the given MixedCellNuclide's contaminant material.
  // add the material to it with the material absorb function.
  // each nuclide model should override this function
  LOG(LEV_DEBUG2,"GRDRNuc") << "MixedCellNuclide is absorbing material: ";
  matToAdd->print();
  wastes_.push_back(matToAdd);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
mat_rsrc_ptr MixedCellNuclide::extract(const CompMapPtr comp_to_rem, double kg_to_rem)
{
  // Get the given MixedCellNuclide's contaminant material.
  // add the material to it with the material extract function.
  // each nuclide model should override this function
  LOG(LEV_DEBUG2,"GRDRNuc") << "MixedCellNuclide" << "is extracting composition: ";
  comp_to_rem->print() ;
  mat_rsrc_ptr to_ret = mat_rsrc_ptr(MatTools::extract(comp_to_rem, kg_to_rem, wastes_));
  update(TI->time());
  return to_ret;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void MixedCellNuclide::transportNuclides(int the_time){
  // This should transport the nuclides through the component.
  // It will likely rely on the internal flux and will produce an external flux. 
  update_degradation(the_time, deg_rate());
  update(the_time);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void MixedCellNuclide::set_deg_rate(double cur_rate){
  if( cur_rate < 0 || cur_rate > 1 ) {
    stringstream msg_ss;
    msg_ss << "The MixedCellNuclide degradation rate range is 0 to 1, inclusive.";
    msg_ss << " The value provided was ";
    msg_ss << cur_rate;
    msg_ss <<  ".";
    LOG(LEV_ERROR,"GRDRNuc") << msg_ss.str();;
    throw CycRangeException(msg_ss.str());
  } else {
    deg_rate_ = cur_rate;
  }
  assert((cur_rate >=0) && (cur_rate <= 1));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void MixedCellNuclide::set_porosity(double porosity){
  if( porosity < 0 || porosity > 1 ) {
    stringstream msg_ss;
    msg_ss << "The MixedCellNuclide porosity range is 0 to 1, inclusive.";
    msg_ss << " The value provided was ";
    msg_ss << porosity;
    msg_ss <<  ".";
    LOG(LEV_ERROR,"GRDRNuc") << msg_ss.str();;
    throw CycRangeException(msg_ss.str());
  } else {
    porosity_ = porosity;
  }
  assert((porosity >=0) && (porosity <= 1));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
double MixedCellNuclide::contained_mass(){
  return shared_from_this()->contained_mass(last_degraded());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
pair<IsoVector, double> MixedCellNuclide::source_term_bc(){
  return make_pair(contained_vec(last_degraded()), 
      tot_deg()*shared_from_this()->contained_mass(last_degraded()));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
IsoConcMap MixedCellNuclide::dirichlet_bc(){
  IsoConcMap dirichlet, whole_vol;
  whole_vol = conc_hist(last_degraded());
  IsoConcMap::const_iterator it;
  for( it=whole_vol.begin(); it!=whole_vol.end(); ++it){
    dirichlet[(*it).first] = tot_deg()*(*it).second ;
  }
  return dirichlet;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
ConcGradMap MixedCellNuclide::neumann_bc(IsoConcMap c_ext, Radius r_ext){
  ConcGradMap to_ret;

  IsoConcMap c_int = conc_hist(last_degraded());
  Radius r_int = geom_->radial_midpoint();
  
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
IsoFluxMap MixedCellNuclide::cauchy_bc(IsoConcMap c_ext, Radius r_ext){
  // -D dC/dx + v_xC = v_x C
  IsoFluxMap to_ret;
  ConcGradMap neumann = neumann_bc(c_ext, r_ext);
  ConcGradMap::iterator it;
  Iso iso;
  Elem elem;
  for( it = neumann.begin(); it != neumann.end(); ++it){
    iso = (*it).first;
    elem = iso/1000;
    to_ret.insert(make_pair(iso, -mat_table_->D(elem)*(*it).second + v()*shared_from_this()->dirichlet_bc(iso)));
  }
  return to_ret;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
IsoConcMap MixedCellNuclide::update_conc_hist(int the_time){
  return update_conc_hist(the_time, wastes_);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
IsoConcMap MixedCellNuclide::update_conc_hist(int the_time, deque<mat_rsrc_ptr> mats){
  // @TODO mats is unused in this (and analogous) functions.
  assert(last_degraded() <= the_time);

  pair<IsoVector, double> sum_pair; 
  sum_pair = vec_hist_[the_time];

  IsoConcMap to_ret;
  int iso;
  double mass;
  double m_ff;
  double m_aff;
  double vol;
  if(sum_pair.second != 0 && V_ff()!=0 && geom_->volume() != numeric_limits<double>::infinity()) { 
    mass = sum_pair.second;
    CompMapPtr curr_comp = sum_pair.first.comp();
    CompMap::const_iterator it;
    it=(*curr_comp).begin();
    while(it != (*curr_comp).end() ) {
      iso = (*it).first;
      if(kd_limited()){
        m_ff = sorb(the_time, iso, (*it).second*mass);
      } else { 
        m_ff = (*it).second*mass;
      }
      if(sol_limited()){
        m_aff = precipitate(the_time, iso, m_ff);
      } else { 
        m_aff = m_ff;
      }
      to_ret.insert(make_pair(iso, m_aff/V_ff()));
      ++it;
    }
  } else {
    to_ret[ 92235 ] = 0; 
  }
  conc_hist_[the_time] = to_ret ;

  return conc_hist_[the_time];
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
double MixedCellNuclide::update_degradation(int the_time, double cur_rate){
  assert(last_degraded() <= the_time);
  if(cur_rate != deg_rate()){
    set_deg_rate(cur_rate);
  };
  double total = tot_deg() + deg_rate()*(the_time - last_degraded());
  set_tot_deg(min(1.0, total));
  set_last_degraded(the_time);

  return tot_deg_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void MixedCellNuclide::update_vec_hist(int the_time){
  vec_hist_[ the_time ] = MatTools::sum_mats(wastes_) ;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void MixedCellNuclide::update_inner_bc(int the_time, std::vector<NuclideModelPtr> daughters){
  std::map<NuclideModelPtr, std::pair<IsoVector,double> > to_ret;
  std::vector<NuclideModelPtr>::iterator daughter;
  std::pair<IsoVector, double> source_term;
  for( daughter = daughters.begin(); daughter!=daughters.end(); ++daughter){
    source_term = (*daughter)->source_term_bc();
    if( source_term.second > 0 ){
      absorb((*daughter)->extract(source_term.first.comp(), source_term.second));
    }
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
double MixedCellNuclide::sorb(int the_time, int iso, double mass){
  if(!kd_limited()){
    throw CycException("The sorb function was called, but kd_limited=false.");
  }
  return SolLim::m_ff(mass, mat_table_->K_d(iso), V_s(), V_f(), tot_deg());

}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
double MixedCellNuclide::precipitate(int the_time, int iso, double mass){
  if(!sol_limited()){
    throw CycException("The sorb function was called, but sol_limited=false.");
  }
  return SolLim::m_aff(mass, mat_table_->K_d(iso), V_s(), V_f(), tot_deg(), mat_table_->S(iso));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
double MixedCellNuclide::V_f(){
  return MatTools::V_f(V_T(),porosity());
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
double MixedCellNuclide::V_s(){
  return MatTools::V_s(V_T(),porosity());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
double MixedCellNuclide::V_ff(){
  return tot_deg()*(MatTools::V_f(V_T(),porosity()));
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
double MixedCellNuclide::V_T(){
  return geom_->volume();
}

