/*
 * ViennaEnergyModel.hpp
 *
 * Header file for the energy model based on ViennaRNA. This class will be
 * included when USE_VIENNA_RNA is defined. It requires an installation of
 * Vienna RNA.
 */

#include "EnergyModel.hpp"

#pragma once

extern "C" {
    #include "params.h"
    #include "utils.h"
    #include "energy_const.h"
    #include "fold.h"
}

/* ViennaEnergyModel inherits from the EnergyModel base class
 */

class ViennaEnergyModel: public EnergyModel {

protected:
    /* pointer to energy parameters */ 
    std::unique_ptr<vrna_param_s> energyParams_;

public:
    /* constructor - initialize the energy parameters by calling a Vienna function*/
    ViennaEnergyModel() : energyParams_(scale_parameters()) {
        //std::cout << "Temperature: " << energyParams_->temperature << std::endl;
        //std::cout << "Temperature: " << energyParams_->model_details.temperature << std::endl;
        //for (int i = 0; i < 20; i++) {
        //    std::cout << energyParams_->param_file[i];
        //}
        //std::cout << std::endl;
        //for (int i = 0; i < 20; i++) {
        //    std::cout << energyParams_->Tetraloop_E[i] << std::endl;
        //}

        //energyParams_->temperature = 37.0;
        //energyParams_->model_details.temperature = 37.0;
        //update_fold_params();
        //
        //std::cout << "Temperature: " << energyParams_->temperature << std::endl;
        //std::cout << "Temperature: " << energyParams_->model_details.temperature << std::endl;
        //
        //for (int i = 0; i < 20; i++) {
        //    std::cout << energyParams_->Tetraloop_E[i] << std::endl;
        //}
        vrna_md_t md;
        md.temperature = 37.0;
        energyParams_ = std::unique_ptr<vrna_param_s>(vrna_params(&md));

    };

   
    /* print name of the class*/
    void repr() override {
        std::cout << "Vienna Energy Model" << std::endl;
    }
 
    /* update the energy fold parameters - not used */
    void updateEnergyFoldParams() override {
        update_fold_params();
    }
   
    /* energy functions */
    int TermAU(int const &type);
    int E_hairpin(int size, int type, int si1, int sj1, const char *string);
    int E_intloop(int n1, int n2, int type, int type_2, int si1, int sj1, int sp1, int sq1);

    /* accessor functions */
    int getTerminalAU() override { return energyParams_->TerminalAU;}
    int getMLbase() override { return energyParams_->MLbase;}
    int getMLintern(unsigned int i) override { return energyParams_->MLintern[i];}
    int getMLclosing() override { return energyParams_->MLclosing;}

};

