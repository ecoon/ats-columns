/* -*-  mode: c++; c-default-style: "google"; indent-tabs-mode: nil -*- */

/* -------------------------------------------------------------------------
ATS

License: see $ATS_DIR/COPYRIGHT
Author: Ethan Coon
------------------------------------------------------------------------- */

#include "advection_diffusion.hh"

namespace Amanzi {
namespace Energy {

RegisteredPKFactory_ATS<AdvectionDiffusion> AdvectionDiffusion::reg_("advection-diffusion energy");

} // namespace Energy
} // namespace Amanzi
