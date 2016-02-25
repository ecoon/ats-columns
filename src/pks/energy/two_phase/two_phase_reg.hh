/* -*-  mode: c++; c-default-style: "google"; indent-tabs-mode: nil -*- */

/* -------------------------------------------------------------------------
ATS

License: see $ATS_DIR/COPYRIGHT
Author: Ethan Coon

Process kernel for energy equation for Richard's flow.
------------------------------------------------------------------------- */

#include "two_phase.hh"

namespace Amanzi {
namespace Energy {

RegisteredPKFactory_ATS<TwoPhase> TwoPhase::reg_("two-phase energy");

} // namespace
} // namespace
