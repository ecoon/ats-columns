/* -*-  mode: c++; c-default-style: "google"; indent-tabs-mode: nil -*- */

/*
  The elevation evaluator gets the surface elevation, slope, and updates pres + elev.

  Authors: Ethan Coon (ecoon@lanl.gov)
*/

#include "pres_elev_evaluator.hh"

namespace Amanzi {
namespace Flow {
namespace FlowRelations {

PresElevEvaluator::PresElevEvaluator(Teuchos::ParameterList& plist) :
    SecondaryVariableFieldEvaluator(plist) {


  Key domain;
  if (plist.isParameter("potential key"))
    domain = getDomain(plist.get<std::string>("potential key"));
  else
    domain = "surface";

  my_key_ = plist_.get<std::string>("potential key",  getKey(domain,"pres_elev"));
  pres_key_ = plist_.get<std::string>("height key", getKey(domain,"ponded_depth"));
  dependencies_.insert(pres_key_);
  elev_key_ = plist_.get<std::string>("elevation key", getKey(domain,"elevation"));

  dependencies_.insert(elev_key_);
}


PresElevEvaluator::PresElevEvaluator(const PresElevEvaluator& other) :
    SecondaryVariableFieldEvaluator(other),
    elev_key_(other.elev_key_),
    pres_key_(other.pres_key_) {};

Teuchos::RCP<FieldEvaluator>
PresElevEvaluator::Clone() const {
  return Teuchos::rcp(new PresElevEvaluator(*this));
}


void PresElevEvaluator::EvaluateField_(const Teuchos::Ptr<State>& S,
        const Teuchos::Ptr<CompositeVector>& result) {
  // update pressure + elevation
  Teuchos::RCP<const CompositeVector> pres = S->GetFieldData(pres_key_);
  Teuchos::RCP<const CompositeVector> elev = S->GetFieldData(elev_key_);

  result->Update(1.0, *elev, 1.0, *pres, 0.0);
}


// This is hopefully never called?
void PresElevEvaluator::EvaluateFieldPartialDerivative_(const Teuchos::Ptr<State>& S,
        Key wrt_key, const Teuchos::Ptr<CompositeVector>& result) {

  result->PutScalar(1.0);
}

} //namespace
} //namespace
} //namespace
