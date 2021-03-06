// SolLimTests.cpp
#include <deque>
#include <map>
#include <gtest/gtest.h>

#include "SolLim.h"
#include "NuclideModel.h"
#include "CycException.h"
#include "Material.h"

using namespace std;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
class SolLimTest : public ::testing::Test {
  protected:
    CompMapPtr test_comp_;
    mat_rsrc_ptr test_mat_;
    int one_mol_;
    int u235_, am241_;
    int u_;
    double test_size_;
    Radius r_four_, r_five_;
    Length len_five_;
    point_t origin_;
    int time_;
    double K_d_;

    virtual void SetUp(){
      // composition set up
      u235_=92235;
      u_=92;
      one_mol_=1.0;
      test_comp_= CompMapPtr(new CompMap(MASS));
      (*test_comp_)[u235_] = one_mol_;
      test_size_=10.0;
      // material creation
      test_mat_ = mat_rsrc_ptr(new Material(test_comp_));
      test_mat_->setQuantity(test_size_);
      // useful numbers
      K_d_ = 0.001;
    }
    virtual void TearDown() {
    }
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
TEST_F(SolLimTest, m_f){
  double V_f, V_s, m_T;
  for(int i=1; i<10; i++){
    V_f=0.1*i;
    V_s=0.1*i;
    m_T=10.*i;
    EXPECT_FLOAT_EQ(m_T/(1+K_d_*(V_s/V_f)), SolLim::m_f(m_T, K_d_, V_s, V_f));
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
TEST_F(SolLimTest, m_s){
  double V_f, V_s, m_T;
  for(int i=1; i<10; i++){
    V_f=0.1*i;
    V_s=0.1*i;
    m_T=0.1*i;
    EXPECT_FLOAT_EQ((K_d_*(V_s/V_f))*m_T/(1+K_d_*(V_s/V_f)), SolLim::m_s(m_T, K_d_, V_s, V_f));
    EXPECT_FLOAT_EQ((K_d_*(V_s/V_f))*SolLim::m_f(m_T, K_d_,V_s, V_f), SolLim::m_s(m_T, K_d_, V_s, V_f));
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
TEST_F(SolLimTest, m_ds){
  double V_f, V_s, m_T, d;
  for(int i=1; i<10; i++){
    V_f=0.1*i;
    V_s=0.1*i;
    m_T=0.1*i;
    d=0.1*i;
    EXPECT_FLOAT_EQ(d*(K_d_*(V_s/V_f))*m_T/(1+K_d_*(V_s/V_f)), SolLim::m_ds(m_T, K_d_, V_s, V_f, d));
    EXPECT_FLOAT_EQ(d*(K_d_*(V_s/V_f))*SolLim::m_f(m_T, K_d_,V_s, V_f), SolLim::m_ds(m_T, K_d_, V_s, V_f, d));
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
TEST_F(SolLimTest, m_ms){
  double V_f, V_s, m_T, d;
  for(int i=1; i<10; i++){
    V_f=0.1*i;
    V_s=0.1*i;
    m_T=0.1*i;
    d=0.1*i;
    EXPECT_FLOAT_EQ((1-d)*(K_d_*(V_s/V_f))*m_T/(1+K_d_*(V_s/V_f)), SolLim::m_ms(m_T, K_d_, V_s, V_f, d));
    EXPECT_FLOAT_EQ((1-d)*(K_d_*(V_s/V_f))*SolLim::m_f(m_T, K_d_,V_s, V_f), SolLim::m_ms(m_T, K_d_, V_s, V_f, d));
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
TEST_F(SolLimTest, m_ff){
  double V_f, V_s, m_T, d;
  for(int i=1; i<10; i++){
    V_f=0.1*i;
    V_s=0.1*i;
    m_T=0.1*i;
    d=0.1*i;
    EXPECT_FLOAT_EQ(d*m_T/(1+K_d_*(V_s/V_f)), SolLim::m_ff(m_T, K_d_, V_s, V_f, d));
    EXPECT_FLOAT_EQ(d*SolLim::m_f(m_T, K_d_,V_s, V_f), SolLim::m_ff(m_T, K_d_, V_s, V_f, d));
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
TEST_F(SolLimTest, m_mf){
  double V_f, V_s, m_T, d;
  for(int i=1; i<10; i++){
    V_f=0.1*i;
    V_s=0.1*i;
    m_T=0.1*i;
    d=0.1*i;
    EXPECT_FLOAT_EQ((1-d)*m_T/(1+K_d_*(V_s/V_f)), SolLim::m_mf(m_T, K_d_, V_s, V_f, d));
    EXPECT_FLOAT_EQ((1-d)*SolLim::m_f(m_T, K_d_,V_s, V_f), SolLim::m_mf(m_T, K_d_, V_s, V_f, d));
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
TEST_F(SolLimTest, m_aff){
  double V_f, V_s, m_T, d;
  double C_sol = 0.001;
  double expected;
  for(int i=1; i<10; i++){
    V_f=0.1*i;
    V_s=0.1*i;
    m_T=0.1*i;
    d=0.1*i;
    EXPECT_FLOAT_EQ(0, SolLim::m_aff(m_T, K_d_, V_s, V_f, d, 0));
    expected = min(C_sol*V_f,(1-d)*m_T/(1+K_d_*(V_s/V_f)));
    EXPECT_FLOAT_EQ(expected, SolLim::m_aff(m_T, K_d_, V_s, V_f, d, C_sol));
    EXPECT_FLOAT_EQ(expected, SolLim::m_aff(m_T, K_d_, V_s, V_f, d, C_sol));
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
TEST_F(SolLimTest, m_ps){
  double V_f, V_s, m_T, d;
  double C_sol = 0.001;
  double expected;
  double m_ff;
  for(int i=1; i<10; i++){
    V_f=0.1*i;
    V_s=0.1*i;
    m_T=0.1*i;
    d=0.1*i;
    EXPECT_FLOAT_EQ(0, SolLim::m_ps(m_T, K_d_, V_s, V_f, d, 10000));
    m_ff = (d)*m_T/(1+K_d_*(V_s/V_f)); 
    expected = m_ff - min(C_sol*V_f, m_ff);
    EXPECT_FLOAT_EQ(expected, SolLim::m_ps(m_T, K_d_, V_s, V_f, d, C_sol));
    EXPECT_FLOAT_EQ(expected, SolLim::m_ps(m_T, K_d_, V_s, V_f, d, C_sol));
  }
}



