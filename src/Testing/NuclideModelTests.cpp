// NuclideModelTests.cpp 
#include <gtest/gtest.h>

#include "NuclideModelTests.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_P(NuclideModelTests, transportNuclides){
  EXPECT_NO_THROW(nuclide_model_->transportNuclides(0));
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_P(NuclideModelTests, copy){
  //copy one model to another, shouldn't throw 
  NuclideModel* new_nuclide_model = (*GetParam())();
  EXPECT_NO_THROW(new_nuclide_model->copy(nuclide_model_));
  //check that the name matches. 
  EXPECT_EQ(nuclide_model_->name(), new_nuclide_model->name());
  delete new_nuclide_model;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_P(NuclideModelTests, absorb){
  //nuclide_model_->absorb() doesn't throw
  EXPECT_NO_THROW(nuclide_model_->absorb(test_mat_));
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_P(NuclideModelTests, extract){
  //nuclide_model_->extract() doesn't throw
  EXPECT_NO_THROW(nuclide_model_->absorb(test_mat_));
  EXPECT_NO_THROW(nuclide_model_->extract(test_comp_, test_size_));
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_P(NuclideModelTests, print){
  EXPECT_NO_THROW(nuclide_model_->print());
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_P(NuclideModelTests, name){
  EXPECT_NO_THROW(nuclide_model_->name());
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_P(NuclideModelTests, type){
  EXPECT_NO_THROW(nuclide_model_->type());
  EXPECT_GT(LAST_NUCLIDE, nuclide_model_->type());
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_P(NuclideModelTests, set_geom){
  // set it
  EXPECT_NO_THROW(nuclide_model_->set_geom(geom_));
  // check
  EXPECT_EQ(geom_->x(), nuclide_model_->geom()->x());
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_P(NuclideModelTests, crude_source_term){
  // check that the source term bc doesn't throw
  // before any contaminants, it had best be 0
  EXPECT_FLOAT_EQ(0, nuclide_model_->source_term_bc(u235_));
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_P(NuclideModelTests, crude_dirichlet){
  // check that the source term bc doesn't throw
  // before any contaminants, it had best be 0
  EXPECT_FLOAT_EQ(0, nuclide_model_->dirichlet_bc(u235_));
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_P(NuclideModelTests, crude_cauchy){
  // check that the source term bc doesn't throw
  // before any contaminants, it had best be 0
  EXPECT_FLOAT_EQ(0, nuclide_model_->cauchy_bc(u235_));
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_P(NuclideModelTests, crude_neumann){
  // check that the source term bc doesn't throw
  // before any contaminants, it had best be 0
  IsoConcMap zeromap;
  zeromap.insert(std::make_pair(92235,0));
  EXPECT_NO_THROW(nuclide_model_->set_geom(geom_));
  EXPECT_FLOAT_EQ(0, nuclide_model_->neumann_bc(zeromap,r_five_+10,u235_));
}