/*! \file SolLim.cpp
    \brief Implements the SolLim class used by the Generic Repository 
    \author Kathryn D. Huff
 */
#include <iostream>
#include <fstream>
#include <deque>
#include <time.h>
#include <assert.h>

#include "CycException.h"
#include "SolLim.h"
#include "Material.h"

using namespace std;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
double SolLim::m_f(double m_T, double K_d, double V_s, double V_f){
  return m_T/(1+K_d*(V_s/V_f));
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
double SolLim::m_s(double m_T, double K_d, double V_s, double V_f){
  return K_d*m_f(m_T,K_d,V_s,V_f)*(V_s/V_f);
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
double SolLim::m_ds(double m_T, double K_d, double V_s, double V_f, double d){
  return d*m_s(m_T,K_d,V_s,V_f);
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
double SolLim::m_ms(double m_T, double K_d, double V_s, double V_f, double d){
  return (1-d)*m_s(m_T,K_d,V_s,V_f);
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
double SolLim::m_ff(double m_T, double K_d, double V_s, double V_f, double d){
  return d*m_f(m_T,K_d,V_s,V_f);
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
double SolLim::m_mf(double m_T, double K_d, double V_s, double V_f, double d){
  return (1-d)*m_f(m_T,K_d,V_s,V_f);
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
double SolLim::m_aff(double m_T, double K_d, double V_s, double V_f, double d, double C_sol){
  return min(C_sol*V_f, m_ff(m_T,K_d,V_s,V_f,d));
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
double SolLim::m_ps(double m_T, double K_d, double V_s, double V_f, double d, double C_sol){
  return m_ff(m_T,K_d,V_s,V_f,d) - m_aff(m_T,K_d,V_s,V_f,d,C_sol);
}
