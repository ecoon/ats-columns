/*
  Functions for calculating the snow-surface energy balance.

  Incoming Longwave radation is cacualted in this version, but if data is
  available we could incorporate it with the available met data.

  Atmospheric pressure is often used in snow models, If data is available we
  could incorperate it but for now Pa is held constant at 100 Pa.

  *** Equation for saturated vapor pressure over water is taken from Bolton,
      1980 'Monthly Weather Review'

  *** Equation for saturated vaport pressure over snow is taken from Buck,
      1996 'Buck Research Manual'

  *** See: http://cires.colorado.edu/~voemel/vp.html

*/

#include <iostream>
#include <cmath>

#include "SnowEnergyBalance.hh"

#ifdef ENABLE_DBC
#include "dbc.hh"
#endif

void SurfaceEnergyBalance::UpdateIncomingRadiation(LocalData& seb) {
  // Calculate incoming short-wave radiation
  seb.st_energy.fQswIn = (1 - seb.st_energy.albedo_value) * seb.st_energy.QswIn;

  // Calculate incoming long-wave radiation
  seb.st_energy.fQlwIn = 1.08 * (1 - std::exp(-0.01 * std::pow(std::pow(10, 11.4 - (2353/seb.vp_air.dewpoint_temp)),
          (seb.st_energy.temp_air/2016))))
      * seb.st_energy.stephB * std::pow(seb.st_energy.temp_air,4);

  // Calculate D_h, D_e
  seb.st_energy.Dhe = (std::pow(seb.st_energy.VKc,2) * seb.st_energy.Us
                       / std::pow(std::log(seb.st_energy.Zr / seb.st_energy.Zo), 2));
}


void SurfaceEnergyBalance::UpdateEFluxesSnow(LocalData& seb, double T) {
  double Sqig;
  if (seb.st_energy.Us == 0.) {
    Sqig = 0.;
  } else {
    double Ri  = seb.st_energy.gZr * (seb.st_energy.temp_air-T)
                  / (seb.st_energy.temp_air*std::pow(seb.st_energy.Us,2));
    Sqig = 1 / (1 + 10*Ri);
  }

  // Calculate outgoing long-wave radiation
  seb.st_energy.fQlwOut = -seb.st_energy.SEs*seb.st_energy.stephB*std::pow(T,4);

  // Calculate sensible heat flux
  seb.st_energy.fQh = seb.st_energy.rowaCp*seb.st_energy.Dhe*Sqig*(seb.st_energy.temp_air-T);

  // Update vapor pressure of snow
  seb.vp_snow.temp = T;
  UpdateVaporPressure(seb.vp_snow);

  // Calculate latent heat flux
  seb.st_energy.fQe = seb.st_energy.rowaLs*seb.st_energy.Dhe*Sqig*0.622
      * (seb.vp_air.actual_vaporpressure-seb.vp_snow.saturated_vaporpressure) / seb.st_energy.Apa;

  // Calculate heat conducted to ground
  double Ks = 2.9e-6 * std::pow(seb.st_energy.density_snow,2);
  seb.st_energy.fQc = Ks * (T-seb.st_energy.temp_ground) / seb.st_energy.ht_snow;
}


// Determine energy available for melting.
double SurfaceEnergyBalance::CalcMeltEnergy(LocalData& seb) {
  // Melt energy is the balance
  return seb.st_energy.fQswIn + seb.st_energy.fQlwIn + seb.st_energy.fQlwOut
      + seb.st_energy.fQh + seb.st_energy.fQe - seb.st_energy.fQc;
}


// Energy balance for no-snow case.
void SurfaceEnergyBalance::UpdateGroundEnergy(LocalData& seb) {
  seb.st_energy.fQlwOut = -seb.st_energy.SEtun * seb.st_energy.stephB * std::pow(seb.st_energy.temp_ground,4);

  double Sqig;
  if (seb.st_energy.Us == 0.) {
    Sqig = 0.;
  } else {
    double Ri = seb.st_energy.gZr * (seb.st_energy.temp_air-seb.st_energy.temp_ground)
        / (seb.st_energy.temp_air*std::pow(seb.st_energy.Us,2));
    if (Ri < 0) { // Unstable condition
      Sqig = (1-10*Ri);
    } else { // Stable Condition
      Sqig = (1/(1+10*Ri));
    }
  }

  seb.st_energy.fQh = seb.st_energy.rowaCp * seb.st_energy.Dhe * Sqig * (seb.st_energy.temp_air - seb.st_energy.temp_ground);
  //  seb.st_energy.fQh = 0.;

  if (seb.st_energy.water_depth > 0.0) {
    // Checking for standing water
    UpdateVaporPressure(seb.vp_ground);
    seb.st_energy.fQe = seb.st_energy.rowaLe * seb.st_energy.Dhe * Sqig * 0.622
        * (seb.vp_air.actual_vaporpressure-seb.vp_ground.saturated_vaporpressure) / seb.st_energy.Apa;
  } else {
    // no standing water
   UpdateVaporPressure(seb.vp_ground);
  //  seb.st_energy.fQe = seb.st_energy.porrowaLe * seb.st_energy.Dhe * Sqig * 0.622
  //      * (seb.vp_air.actual_vaporpressure-seb.vp_ground.actual_vaporpressure) / seb.st_energy.Apa;
    seb.st_energy.fQe = seb.st_energy.porrowaLe * seb.st_energy.Dhe * Sqig * 0.622
        * (seb.vp_air.actual_vaporpressure-seb.vp_ground.saturated_vaporpressure) / seb.st_energy.Apa;
  }

  // Heat flux to ground surface is the balance.
  seb.st_energy.fQc = seb.st_energy.fQswIn + seb.st_energy.fQlwIn + seb.st_energy.fQlwOut
      + seb.st_energy.fQh + seb.st_energy.fQe;
}


// Calculate saturated and actual vapor pressures
void SurfaceEnergyBalance::UpdateVaporPressure(VaporPressure& vp) {
  double temp;
  //Convert from Kelvin to Celsius
  temp = vp.temp-273.15;
  // Sat vap. press o/water Dingman D-7 (Bolton, 1980)
  vp.saturated_vaporpressure = 0.611*std::exp(17.67*temp / (temp+243.5));
  // (Bolton, 1980)
  vp.actual_vaporpressure = vp.saturated_vaporpressure * vp.relative_humidity;
  // Find dewpoint Temp Dingman D-11
  vp.dewpoint_temp = (std::log(vp.actual_vaporpressure) + 0.4926) / (0.0708-0.00421*std::log(vp.actual_vaporpressure));
  // Convert Tdp from Celsius to Kelvin
  vp.dewpoint_temp = vp.dewpoint_temp + 273.15;
}


// Take a weighted average to get the albedo.
double SurfaceEnergyBalance::CalcAlbedo(EnergyBalance& eb) {
  double perSnow = 0.0, perTundra=0.0, perWater=0.0;
  double AlTundra=0.15, AlWater=0.6;
  double AlSnow = 0.0;
  double TransitionVal = eb.AlbedoTrans;  // Set to 2 cm

  if (eb.density_snow <= 432.238) {
    AlSnow = 1.0 - 0.247 * std::pow(0.16 + 110*std::pow(eb.density_snow/1000, 4), 0.5);
  } else {
    AlSnow = 0.6 - eb.density_snow / 4600;
  }

  if (eb.ht_snow > TransitionVal) {
    // Snow is too deep for albedo weighted average, just use all snow
    perSnow = 1.;
  } else if (eb.water_depth <= 0.0) {  // dry ground
    // Transition to dry ground
    perSnow = std::pow(eb.ht_snow/TransitionVal, 2);
    perTundra = 1 - perSnow;
  } else {
    // Transitions to surface water
    perSnow = eb.ht_snow / TransitionVal;
    perWater = 1 - perSnow;
  }

#ifdef ENABLE_DBC
  ASSERT(std::abs((perSnow + perTundra + perWater) - 1.) < 1.e-16);
#endif

  // weighted average function for surface albedo
  return AlSnow*perSnow + AlTundra*perTundra + AlWater*perWater;
}


// Surface Energy Balance residual
double SurfaceEnergyBalance::EnergyBalanceResidual(LocalData& seb, double Xx) {
  UpdateEFluxesSnow(seb, Xx);

  // energy balance
  double res = seb.st_energy.ht_snow
      * (seb.st_energy.fQswIn + seb.st_energy.fQlwIn + seb.st_energy.fQlwOut
         + seb.st_energy.fQh + seb.st_energy.fQe - seb.st_energy.fQc);
  return res;
}


// Use a bisection method to calculate the temperature of the snow.
double SurfaceEnergyBalance::CalcSnowTemperature(LocalData& seb) {
  double tol = 1.e-6;
  double deltaX = 5;

  double Xx = seb.st_energy.temp_air;
  double FXx = EnergyBalanceResidual(seb, Xx);
  // NOTE: decreasing function
  // Bracket the root by (a,b)
  double a,b,Fa,Fb;
  if (FXx > 0) {
    b = Xx;
    Fb = FXx;
    a = Xx;
    Fa = FXx;
    while (Fa > 0) {
      b = a;
      Fb = Fa;
      a += deltaX;
      Fa = EnergyBalanceResidual(seb,a);
    }
  } else {
    a = Xx;
    Fa = FXx;
    b = Xx;
    Fb = FXx;
    while (Fb < 0) {
      a = b;
      Fa = Fb;
      b -= deltaX;
      Fb = EnergyBalanceResidual(seb,b);
    }
  }

#ifdef ENABLE_DBC
  ASSERT(Fa*Fb < 0);
#endif

  int maxIterations = 200;
  double res;
  int iter;
  // Bisection Iterations Loop: Solve for Ts using Energy balance equation
  for (int i=0; i<maxIterations; ++i) {
    Xx = (a+b)/2;
    res = EnergyBalanceResidual(seb, Xx);

    if (res>0) {
      b=Xx;
    } else {
      a=Xx;
    }
    if (std::abs(res)<tol) {
      break;
    }
    iter=i;
  }

#ifdef ENABLE_DBC
  ASSERT(std::abs(res) <= tol);
#endif
  return Xx;
}


// Alter mass flux due to melting.
void SurfaceEnergyBalance::UpdateMassMelt(EnergyBalance& eb) {
  // Melt rate given by energy rate available divided by heat of fusion.
  double melt = eb.Qm / (eb.density_w * eb.Hf);
  eb.Mr += melt;
  eb.MIr -= melt;
}


// Alter mass fluxes due to sublimation/condensation.
void SurfaceEnergyBalance::UpdateMassSublCond(EnergyBalance& eb) {
  double SublR = -eb.fQe / (eb.density_w * eb.Ls); // [m/s]

  if (SublR < 0 && eb.temp_snow == 273.15) {
    // Condensation, not sublimation.
    // Snow is melting, surface temp = 0 C and condensation is applied as
    // water and drains through snow.  Therefore add directly to melt.
    eb.Mr += -SublR;
  } else {
    //    if (SublR > 0) {
      eb.MIr += -SublR;
      //    }
  }
}


// Alter mass fluxes due to evaporation from soil (no snow).
void SurfaceEnergyBalance::UpdateMassEvap(EnergyBalance& eb) {
  eb.Mr += eb.fQe / (eb.density_w*eb.Le); // [m/s]
}


// Alter mass fluxes in the case of all snow disappearing.
void SurfaceEnergyBalance::WaterMassCorrection(EnergyBalance& eb) {
  if (eb.MIr < 0) {
    // convert ht_snow to SWE
    double swe = eb.ht_snow * eb.density_snow / eb.density_w;
    double swe_change = (eb.MIr + eb.Ps) * eb.dt;
    if (swe + swe_change < 0) {
      // No more snow!  Take the rest out of the ground.
      // -- AA re-visit: should we take some from sublimation?
      eb.Mr += (swe + swe_change) / eb.dt;
    }
  }
}


// Calculate snow change ~> settling of previously existing snow (Martinec, 1977)
void SurfaceEnergyBalance::UpdateSnow(EnergyBalance& eb) {
  if (eb.MIr < 0.) {
    // sublimation, remove snow now
    eb.ht_snow = eb.ht_snow + (eb.MIr * eb.dt * eb.density_w / eb.density_snow);
  }

  // settle the pre-existing snow
  eb.age_snow += eb.dt / 86400.;
  double ndensity = std::pow(eb.age_snow,0.3);
  double dens_settled = eb.density_freshsnow*ndensity;
  double ht_settled = eb.ht_snow * eb.density_snow / dens_settled;

  // Match Frost Age with Assinged density
     //Calculating which Day frost density matched snow Defermation fucntion from (Martinec, 1977) 
  double frost_age = pow((eb.density_frost /eb.density_freshsnow),(1/0.3))-1;
  frost_age = frost_age + eb.dt / 86400.; 

  // determine heights of the sources
  double ht_precip = eb.Ps * eb.density_w / eb.density_freshsnow;
  double ht_frost = eb.MIr > 0. ? eb.MIr * eb.dt * eb.density_w / eb.density_frost : 0.;

  eb.ht_snow = ht_precip + ht_frost + ht_settled;

  // Possibly settling resulted in negative snow pack, if the snow was disappearing?
  eb.ht_snow = std::max(eb.ht_snow, 0.);

  // Take the height-weighted average to determine new density
  if (eb.ht_snow > 0.) {
    eb.density_snow = (ht_precip * eb.density_freshsnow + ht_frost * eb.density_frost
                       + ht_settled * dens_settled) / eb.ht_snow;
  } else {
    eb.density_snow = eb.density_freshsnow;
  }

  // Take the mass-weighted average to determine new age
  if (eb.ht_snow > 0.) {
     eb.age_snow = (eb.age_snow * ht_settled * dens_settled
                     + frost_age * ht_frost * eb.density_frost + eb.dt / 86400. * ht_precip * eb.density_freshsnow)
      / (ht_settled * dens_settled + ht_frost * eb.density_frost + ht_precip * eb.density_freshsnow);    
   } else {
    eb.age_snow = 0;
  }
}


// Main snow energy balance function.
void SurfaceEnergyBalance::SnowEnergyBalance(LocalData& seb) {
  // Caculate Vapor pressure and dewpoint temperature from Air
  UpdateVaporPressure(seb.vp_air);

  // Find effective Albedo
  seb.st_energy.albedo_value = CalcAlbedo(seb.st_energy);

  // Update temperature-independent fluxes, the short- and long-wave incoming
  // radiation.
  UpdateIncomingRadiation(seb);

  // Convert snow precipitation length (m) to SWE length
  //  SWE(seb.st_energy);

  // Initialize mass change rates
  seb.st_energy.Mr = seb.st_energy.Pr / seb.st_energy.dt; // precipitation rain
  seb.st_energy.MIr = 0;

  if (seb.st_energy.ht_snow > 0) { // If snow
    // Step 1: Energy Balance
    // Calculate the temperature of the snow.
    seb.st_energy.temp_snow = CalcSnowTemperature(seb);

    // Calculate the energy available for melting, Qm
    if (seb.st_energy.temp_snow <= 273.15) { // Snow is not melting
      seb.st_energy.Qm = 0; //  no water leaving snowpack as melt water
    } else {
      seb.st_energy.temp_snow = 273.15; // Set snow temperature to zero
      UpdateEFluxesSnow(seb, seb.st_energy.temp_snow);
      seb.st_energy.Qm = CalcMeltEnergy(seb); // Recaculate energy balance with melting.
    }

    // Step 2: Mass Balance
    // -- melt
    UpdateMassMelt(seb.st_energy);

    // -- sublimation and condensation rates between ice, melt, and air
    UpdateMassSublCond(seb.st_energy);

    // Make sure proper mass of snowpack water gets delivered to AT
    WaterMassCorrection(seb.st_energy);

  } else { // no snow
    // Energy balance
    UpdateGroundEnergy(seb);

    // Mass balance
    UpdateMassEvap(seb.st_energy);
  }

  // Update snow pack, density
  UpdateSnow(seb.st_energy);

  // set water temp
  seb.st_energy.Trw = seb.st_energy.ht_snow > 0. ? 273.15 : seb.st_energy.temp_air;
}


// Main energy-only function.
void SurfaceEnergyBalance::UpdateEnergyBalance(LocalData& seb) {
  if (seb.st_energy.ht_snow > 0.) {
    // Caculate Vapor pressure and dewpoint temperature from Air
    UpdateVaporPressure(seb.vp_air);

    // Find effective Albedo
    seb.st_energy.albedo_value = CalcAlbedo(seb.st_energy);

    // Update temperature-independent fluxes, the short- and long-wave incoming
    // radiation.
    UpdateIncomingRadiation(seb);

    seb.st_energy.temp_snow = CalcSnowTemperature(seb);

    if (seb.st_energy.temp_snow <= 273.15) { // Snow is not melting
      seb.st_energy.Qm = 0; //  no water leaving snowpack as melt water
    } else {
      seb.st_energy.temp_snow = 273.15; // Set snow temperature to zero
      UpdateEFluxesSnow(seb, seb.st_energy.temp_snow);
    }

    //    double Ks = 2.9e-6 * std::pow(seb.st_energy.density_snow,2);
    //    seb.st_energy.fQc = Ks * (seb.st_energy.temp_snow - seb.st_energy.temp_ground) / seb.st_energy.ht_snow;
  } else {
    // Caculate Vapor pressure and dewpoint temperature from Air
    UpdateVaporPressure(seb.vp_air);

    // Find effective Albedo
    seb.st_energy.albedo_value = CalcAlbedo(seb.st_energy);

    // Update temperature-independent fluxes, the short- and long-wave incoming
    // radiation.
    UpdateIncomingRadiation(seb);

    // Energy balance
    UpdateGroundEnergy(seb);
  }
}