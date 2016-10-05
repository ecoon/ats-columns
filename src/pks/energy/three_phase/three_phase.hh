/* -*-  mode: c++; c-default-style: "google"; indent-tabs-mode: nil -*- */

/* -------------------------------------------------------------------------
ATS

License: see $ATS_DIR/COPYRIGHT
Author: Ethan Coon

Fully three-phase (air, water, ice) permafrost energy equation, with only
mobile water.

Inherits TwoPhase instead of EnergyBase to pick up the enthalpy from TwoPhase.
------------------------------------------------------------------------- */

#ifndef PKS_ENERGY_THREE_PHASE_HH_
#define PKS_ENERGY_THREE_PHASE_HH_

#include "pk_factory.hh"
#include "two_phase.hh"

namespace Amanzi {
namespace Energy {

class ThreePhase : public TwoPhase {

public:

  ThreePhase(Teuchos::Ptr<State> S, const Teuchos::RCP<Teuchos::ParameterList>& plist,
             Teuchos::ParameterList& FElist,
             const Teuchos::RCP<TreeVector>& solution) :
    PKDefaultBase(S, plist, FElist, solution),
    TwoPhase(S, plist, FElist, solution) {}

  // Virtual destructor
  virtual ~ThreePhase() {}

  virtual void initialize(const Teuchos::Ptr<State>& S);
  
protected:
  virtual void SetupPhysicalEvaluators_(const Teuchos::Ptr<State>& S);

private:
  static RegisteredPKFactory<ThreePhase> reg_;

  friend  class MPCCoupledFlowEnergy;
};

} // namespace Energy
} // namespace Amanzi

#endif
