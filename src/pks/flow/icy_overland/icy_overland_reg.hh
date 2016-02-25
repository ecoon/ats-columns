/* -*-  mode: c++; c-default-style: "google"; indent-tabs-mode: nil -*- */

/* -------------------------------------------------------------------------
This is the flow component of the Amanzi code. 
License: BSD
Authors: Ethan Coon (ecoon@lanl.gov)
------------------------------------------------------------------------- */

#include "icy_overland.hh"

namespace Amanzi {
namespace Flow {

RegisteredPKFactory_ATS<IcyOverlandFlow> IcyOverlandFlow::reg_("overland flow with ice");

} // namespace
} // namespace
