#include "bsdf.h"

#include <iostream>
#include <algorithm>
#include <utility>

using std::min;
using std::max;
using std::swap;

namespace CGL {

void make_coord_space(Matrix3x3& o2w, const Vector3D& n) {
  Vector3D z = Vector3D(n.x, n.y, n.z);
  Vector3D h = z;
  if (fabs(h.x) <= fabs(h.y) && fabs(h.x) <= fabs(h.z)) h.x = 1.0;
  else if (fabs(h.y) <= fabs(h.x) && fabs(h.y) <= fabs(h.z)) h.y = 1.0;
  else h.z = 1.0;

  z.normalize();
  Vector3D y = cross(h, z);
  y.normalize();
  Vector3D x = cross(z, y);
  x.normalize();

  o2w[0] = x;
  o2w[1] = y;
  o2w[2] = z;
}

// Mirror BSDF //

Spectrum MirrorBSDF::f(const Vector3D& wo, const Vector3D& wi) {
  return {};
}

Spectrum MirrorBSDF::sample_f(const Vector3D& wo, Vector3D* wi, float* pdf) {
  // 1.2
  // Using BSDF::reflect(), implement sample_f for a mirror surface
  reflect(wo, wi);
  *pdf = 1;
  return reflectance / abs_cos_theta(*wi);
}

// Microfacet BSDF //

double MicrofacetBSDF::G(const Vector3D& wo, const Vector3D& wi) {
  return 1.0 / (1.0 + Lambda(wi) + Lambda(wo));
}

double MicrofacetBSDF::D(const Vector3D& h) {
  // 2.2
  // Compute Beckmann normal distribution function (NDF) here.
  // You will need the roughness alpha.

  double tan_t = tan(acos(h.z));
  return exp(-tan_t * tan_t / alpha / alpha) / (PI * alpha * alpha * pow(h.z, 4));
}

Spectrum MicrofacetBSDF::F(const Vector3D& wi) {
  // 2.3
  // Compute Fresnel term for reflection on dielectric-conductor interface.
  // You will need both eta and etaK, both of which are Spectrum.

  Spectrum eta_2_k_2 = eta * eta + k * k;
  Spectrum two_eta_cos_t = 2 * eta * wi.z;
  double cos_t_2 = wi.z * wi.z;

  Spectrum r_s = (eta_2_k_2 - two_eta_cos_t + cos_t_2) / (eta_2_k_2 + two_eta_cos_t + cos_t_2);
  Spectrum r_p = (eta_2_k_2 * cos_t_2 - two_eta_cos_t + 1) / (eta_2_k_2 * cos_t_2 + two_eta_cos_t + 1);

  return (r_s + r_p) / 2;
}

Spectrum MicrofacetBSDF::f(const Vector3D& wo, const Vector3D& wi) {
  // 2.1
  // Implement microfacet model here

  if (wo.z > 0 && wi.z > 0) {
    Vector3D h = wo + wi;
    h.normalize();
    return F(wi) * G(wo, wi) * D(h) / (4 * wo.z * wi.z);
  } else
    return {};
}

Spectrum MicrofacetBSDF::sample_f(const Vector3D& wo, Vector3D* wi, float* pdf) {
  // 2.4
  // *Importance* sample Beckmann normal distribution function (NDF) here.
  // Note: You should fill in the sampled direction *wi and the corresponding *pdf,
  //       and return the sampled BRDF value.

  // *wi = cosineHemisphereSampler.get_sample(pdf); //placeholder
  // return MicrofacetBSDF::f(wo, *wi);

  double theta_h = atan(sqrt(-alpha * alpha * log(1 - random_uniform())));
  double phi_h = 2 * PI * random_uniform();
  Vector3D h = {sin(theta_h) * cos(phi_h), sin(theta_h) * sin(phi_h), cos(theta_h)};
  *wi = 2 * (dot(h, wo)) * h - wo;

  if (wi->z > 0) {
    double p_theta = 2 * sin(theta_h) / (alpha * alpha * pow(cos(theta_h), 3))
                     * exp(-tan(theta_h) * tan(theta_h) / alpha / alpha);
    double p_phi = 0.5 / PI;
    *pdf = (float) (p_theta * p_phi / (4 * sin(theta_h) * dot(*wi, h)));

    return f(wo, *wi);
  } else {
    *pdf = 0;
    return {};
  }
}

// Refraction BSDF //

Spectrum RefractionBSDF::f(const Vector3D& wo, const Vector3D& wi) {
  return Spectrum();
}

Spectrum RefractionBSDF::sample_f(const Vector3D& wo, Vector3D* wi, float* pdf) {
  return Spectrum();
}

// Glass BSDF //

Spectrum GlassBSDF::f(const Vector3D& wo, const Vector3D& wi) {
  return {};
}

Spectrum GlassBSDF::sample_f(const Vector3D& wo, Vector3D* wi, float* pdf) {
  // 1.4
  // Compute Fresnel coefficient and either reflect or refract based on it.

  if (refract(wo, wi, ior)) {
    float eta = (wo.z >= 0) ? 1 / ior : ior;
    auto r0 = (float) pow((1 - eta) / (1 + eta), 2);
    auto r = r0 + (1 - r0) * (float) pow(1 - abs(wo.z), 5);
    if (coin_flip(r)) { // reflection
      reflect(wo, wi);
      *pdf = r;
      return r * reflectance / abs_cos_theta(*wi);
    } else { // refraction
      *pdf = 1 - r;
      return (1 - r) * transmittance / abs_cos_theta(*wi) / eta / eta;
    }
  } else { // total internal refraction, treat as mirror
    reflect(wo, wi);
    *pdf = 1;
    return reflectance / abs_cos_theta(*wi);
  }
}

void BSDF::reflect(const Vector3D& wo, Vector3D* wi) {
  // 1.1
  // Implement reflection of wo about normal (0,0,1) and store result in wi.
  *wi = {-wo.x, -wo.y, wo.z};
}

bool BSDF::refract(const Vector3D& wo, Vector3D* wi, float ior) {
  // 1.3
  // Use Snell's Law to refract wo surface and store result ray in wi.
  // Return false if refraction does not occur due to total internal reflection
  // and true otherwise. When dot(wo,n) is positive, then wo corresponds to a
  // ray entering the surface through vacuum.
  float eta = (wo.z >= 0) ? 1 / ior : ior;
  auto discriminant = (float) (1 - eta * eta * (1 - wo.z * wo.z));
  if (discriminant < 0) // total internal reflectio
    return false;
  else {
    *wi = {-eta * wo.x, -eta * wo.y, -wo.z / abs(wo.z) * sqrt(discriminant)};
    return true;
  }
}

// Emission BSDF //

Spectrum EmissionBSDF::f(const Vector3D& wo, const Vector3D& wi) {
  return Spectrum();
}

Spectrum EmissionBSDF::sample_f(const Vector3D& wo, Vector3D* wi, float* pdf) {
  *pdf = 1.0 / PI;
  *wi = sampler.get_sample(pdf);
  return Spectrum();
}

} // namespace CGL
