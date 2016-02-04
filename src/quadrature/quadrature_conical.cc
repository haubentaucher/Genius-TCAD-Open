// $Id: quadrature_conical.C 3391 2009-05-26 03:50:35Z benkirk $

// The libMesh Finite Element Library.
// Copyright (C) 2002-2008 Benjamin S. Kirk, John W. Peterson, Roy H. Stogner

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


#include "quadrature_conical.h"
#include "quadrature_gauss.h"
#include "quadrature_jacobi.h"

// See also the files:
// quadrature_conical_2D.C
// quadrature_conical_3D.C
// for additional implementation.




// Constructor
QConical::QConical(const unsigned int d,
		   const Order o) : QBase(d,o)
{
}



// Destructor
QConical::~QConical()
{
}



// Builds and scales a Gauss rule and a Jacobi rule.
// Then combines them to compute points and weights
// of a 2D conical product rule.
void QConical::conical_product_tri(unsigned int p)
{
  // Be sure the underlying rule object was built with the same dimension as the
  // rule we are about to construct.
  assert (this->get_dim() == 2);

  QGauss  gauss1D(1,static_cast<Order>(_order+2*p));
  QJacobi jac1D(1,static_cast<Order>(_order+2*p),1,0);

  // The Gauss rule needs to be scaled to [0,1]
  std::pair<Real, Real> old_range(-1.0L, 1.0L);
  std::pair<Real, Real> new_range( 0.0L, 1.0L);
  gauss1D.scale(old_range,
		new_range);

  // Now construct the points and weights for the conical product rule.

  // Both rules should have the same number of points.
  assert(gauss1D.n_points() == jac1D.n_points());

  // Save the number of points as a convenient variable
  const unsigned int n_points = gauss1D.n_points();

  // Both rules should be between x=0 and x=1
  assert(gauss1D.qp(0)(0) >= 0.0); assert(gauss1D.qp(n_points-1)(0) <= 1.0);
  assert(jac1D.qp(0)(0)   >= 0.0); assert(jac1D.qp(n_points-1)(0) <= 1.0);

  // Resize the points and weights vectors
  _points.resize(n_points * n_points);
  _weights.resize(n_points * n_points);

  // Compute the conical product
  unsigned int gp = 0;
  for (unsigned int i=0; i<n_points; i++)
    for (unsigned int j=0; j<n_points; j++)
      {
	_points[gp](0) = jac1D.qp(j)(0);                          //s[j];
	_points[gp](1) = gauss1D.qp(i)(0) * (1.-jac1D.qp(j)(0)); //r[i]*(1.-s[j]);
	_weights[gp]   = gauss1D.w(i) * jac1D.w(j);              //A[i]*B[j];
	gp++;
      }
}




// Builds and scales a Gauss rule and a Jacobi rule.
// Then combines them to compute points and weights
// of a 3D conical product rule for the Tet.
void QConical::conical_product_tet(unsigned int p)
{
  // Be sure the underlying rule object was built with the same dimension as the
  // rule we are about to construct.
  assert (this->get_dim() == 3);

  QGauss  gauss1D(1,static_cast<Order>(_order+2*p));
  QJacobi jacA1D(1,static_cast<Order>(_order+2*p),1,0);
  QJacobi jacB1D(1,static_cast<Order>(_order+2*p),2,0);

  // The Gauss rule needs to be scaled to [0,1]
  std::pair<Real, Real> old_range(-1.0L, 1.0L);
  std::pair<Real, Real> new_range( 0.0L, 1.0L);
  gauss1D.scale(old_range,
		new_range);

  // Now construct the points and weights for the conical product rule.

  // All rules should have the same number of points
  assert(gauss1D.n_points() == jacA1D.n_points());
  assert(jacA1D.n_points()  == jacB1D.n_points());

  // Save the number of points as a convenient variable
  const unsigned int n_points = gauss1D.n_points();

  // All rules should be between x=0 and x=1
  assert(gauss1D.qp(0)(0) >= 0.0); assert(gauss1D.qp(n_points-1)(0) <= 1.0);
  assert(jacA1D.qp(0)(0)  >= 0.0); assert(jacA1D.qp(n_points-1)(0)  <= 1.0);
  assert(jacB1D.qp(0)(0)  >= 0.0); assert(jacB1D.qp(n_points-1)(0)  <= 1.0);

  // Resize the points and weights vectors
  _points.resize(n_points * n_points * n_points);
  _weights.resize(n_points * n_points * n_points);

  // Compute the conical product
  unsigned int gp = 0;
  for (unsigned int i=0; i<n_points; i++)
    for (unsigned int j=0; j<n_points; j++)
      for (unsigned int k=0; k<n_points; k++)
      {
	_points[gp](0) = jacB1D.qp(k)(0);                                                  //t[k];
	_points[gp](1) = jacA1D.qp(j)(0)  * (1.-jacB1D.qp(k)(0));                         //s[j]*(1.-t[k]);
	_points[gp](2) = gauss1D.qp(i)(0) * (1.-jacA1D.qp(j)(0)) * (1.-jacB1D.qp(k)(0)); //r[i]*(1.-s[j])*(1.-t[k]);
	_weights[gp]   = gauss1D.w(i)     * jacA1D.w(j)          * jacB1D.w(k);          //A[i]*B[j]*C[k];
	gp++;
      }
}





// Builds and scales a Gauss rule and a Jacobi rule.
// Then combines them to compute points and weights
// of a 3D conical product rule for the Pyramid.  The integral
// over the reference Tet can be written (in LaTeX notation) as:
//
// If := \int_0^1 dz \int_{-(1-z)}^{(1-z)} dy \int_{-(1-z)}^{(1-z)} f(x,y,z) dx (1)
//
// (Imagine a stack of infinitely thin squares which decrease in size as
//  you approach the apex.)  Under the transformation of variables:
//
// z=w
// y=(1-z)v
// x=(1-z)u,
//
// The Jacobian determinant of this transformation is |J|=(1-w)^2, and
// the integral itself is transformed to:
//
// If = \int_0^1 (1-w)^2 dw \int_{-1}^{1} dv \int_{-1}^{1} f((1-w)u, (1-w)v, w) du (2)
//
// The integral can now be approximated by the product of three 1D quadrature rules:
// A Jacobi rule with alpha==2, beta==0 in w, and Gauss rules in v and u.  In this way
// we can obtain 3D rules to any order for which the 1D rules exist.
void QConical::conical_product_pyramid(unsigned int p)
{
  // Be sure the underlying rule object was built with the same dimension as the
  // rule we are about to construct.
  assert (this->get_dim() == 3);

  QGauss  gauss1D(1,static_cast<Order>(_order+2*p));
  QJacobi jac1D(1,static_cast<Order>(_order+2*p),2,0);

  // These rules should have the same number of points
  assert(gauss1D.n_points() == jac1D.n_points());

  // Save the number of points as a convenient variable
  const unsigned int n_points = gauss1D.n_points();

  // Resize the points and weights vectors
  _points.resize(n_points * n_points * n_points);
  _weights.resize(n_points * n_points * n_points);

  // Compute the conical product
  unsigned int qp = 0;
  for (unsigned int i=0; i<n_points; ++i)
    for (unsigned int j=0; j<n_points; ++j)
      for (unsigned int k=0; k<n_points; ++k, ++qp)
      {
	const Real xi=gauss1D.qp(i)(0);
	const Real yj=gauss1D.qp(j)(0);
	const Real zk=jac1D.qp(k)(0);

	_points[qp](0) = (1.-zk) * xi;
	_points[qp](1) = (1.-zk) * yj;
	_points[qp](2) = zk;
	_weights[qp]   = gauss1D.w(i) * gauss1D.w(j) * jac1D.w(k);
      }


}
