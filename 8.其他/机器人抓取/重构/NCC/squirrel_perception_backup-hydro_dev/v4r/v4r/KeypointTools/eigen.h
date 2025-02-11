/*
 * Software License Agreement (BSD License)
 *
 *  Point Cloud Library (PCL) - www.pointclouds.org
 *  Copyright (c) 2010-2012, Willow Garage, Inc.
 *  Copyright (C) 2010 Gael Guennebaud <gael.guennebaud@inria.fr>
 *  Copyright (C) 2009 Hauke Heibel <hauke.heibel@gmail.com>
 *  Copyright (c) 2012-, Open Perception, Inc.
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the copyright holder(s) nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id$
 *
 */
#ifndef KP_COMMON_EIGEN_H_
#define KP_COMMON_EIGEN_H_

#ifndef NOMINMAX
#define NOMINMAX
#endif

#if defined __GNUC__
#  pragma GCC system_header
#elif defined __SUNPRO_CC
#  pragma disable_warn
#endif

#include <cmath>
#include <stdint.h>

#include <Eigen/StdVector>
#include <Eigen/Core>
#include <Eigen/Eigenvalues>
#include <Eigen/Geometry>
#include <Eigen/SVD>
#include <Eigen/LU>
#include <Eigen/Dense>
#include <Eigen/Eigenvalues>

namespace kp
{
  /** \brief Compute the roots of a quadratic polynom x^2 + b*x + c = 0
    * \param[in] b linear parameter
    * \param[in] c constant parameter
    * \param[out] roots solutions of x^2 + b*x + c = 0
    */
  template<typename Scalar, typename Roots> inline void
  computeRoots2 (const Scalar& b, const Scalar& c, Roots& roots)
  {
    roots (0) = Scalar (0);
    Scalar d = Scalar (b * b - 4.0 * c);
    if (d < 0.0) // no real roots!!!! THIS SHOULD NOT HAPPEN!
      d = 0.0;

    Scalar sd = ::std::sqrt (d);

    roots (2) = 0.5f * (b + sd);
    roots (1) = 0.5f * (b - sd);
  }

  /** \brief computes the roots of the characteristic polynomial of the input matrix m, which are the eigenvalues
    * \param[in] m input matrix
    * \param[out] roots roots of the characteristic polynomial of the input matrix m, which are the eigenvalues
    */
  template<typename Matrix, typename Roots> inline void
  computeRoots (const Matrix& m, Roots& roots)
  {
    typedef typename Matrix::Scalar Scalar;

    // The characteristic equation is x^3 - c2*x^2 + c1*x - c0 = 0.  The
    // eigenvalues are the roots to this equation, all guaranteed to be
    // real-valued, because the matrix is symmetric.
    Scalar c0 =            m (0, 0) * m (1, 1) * m (2, 2)
            + Scalar (2) * m (0, 1) * m (0, 2) * m (1, 2)
                         - m (0, 0) * m (1, 2) * m (1, 2)
                         - m (1, 1) * m (0, 2) * m (0, 2)
                         - m (2, 2) * m (0, 1) * m (0, 1);
    Scalar c1 = m (0, 0) * m (1, 1) -
                m (0, 1) * m (0, 1) +
                m (0, 0) * m (2, 2) -
                m (0, 2) * m (0, 2) +
                m (1, 1) * m (2, 2) -
                m (1, 2) * m (1, 2);
    Scalar c2 = m (0, 0) + m (1, 1) + m (2, 2);


    if (fabs (c0) < Eigen::NumTraits<Scalar>::epsilon ())// one root is 0 -> quadratic equation
      computeRoots2 (c2, c1, roots);
    else
    {
      const Scalar s_inv3 = Scalar (1.0 / 3.0);
      const Scalar s_sqrt3 = std::sqrt (Scalar (3.0));
      // Construct the parameters used in classifying the roots of the equation
      // and in solving the equation for the roots in closed form.
      Scalar c2_over_3 = c2*s_inv3;
      Scalar a_over_3 = (c1 - c2 * c2_over_3) * s_inv3;
      if (a_over_3 > Scalar (0))
        a_over_3 = Scalar (0);

      Scalar half_b = Scalar (0.5) * (c0 + c2_over_3 * (Scalar (2) * c2_over_3 * c2_over_3 - c1));

      Scalar q = half_b * half_b + a_over_3 * a_over_3*a_over_3;
      if (q > Scalar (0))
        q = Scalar (0);

      // Compute the eigenvalues by solving for the roots of the polynomial.
      Scalar rho = std::sqrt (-a_over_3);
      Scalar theta = std::atan2 (std::sqrt (-q), half_b) * s_inv3;
      Scalar cos_theta = std::cos (theta);
      Scalar sin_theta = std::sin (theta);
      roots (0) = c2_over_3 + Scalar (2) * rho * cos_theta;
      roots (1) = c2_over_3 - rho * (cos_theta + s_sqrt3 * sin_theta);
      roots (2) = c2_over_3 - rho * (cos_theta - s_sqrt3 * sin_theta);

      // Sort in increasing order.
      if (roots (0) >= roots (1))
        std::swap (roots (0), roots (1));
      if (roots (1) >= roots (2))
      {
        std::swap (roots (1), roots (2));
        if (roots (0) >= roots (1))
          std::swap (roots (0), roots (1));
      }

      if (roots (0) <= 0) // eigenval for symetric positive semi-definite matrix can not be negative! Set it to 0
        computeRoots2 (c2, c1, roots);
    }
  }

  /** \brief determine the smallest eigenvalue and its corresponding eigenvector
    * \param[in] mat input matrix that needs to be symmetric and positive semi definite
    * \param[out] eigenvalue the smallest eigenvalue of the input matrix
    * \param[out] eigenvector the corresponding eigenvector to the smallest eigenvalue of the input matrix
    * \ingroup common
    */
  template <typename Matrix, typename Vector> inline void
  eigen22 (const Matrix& mat, typename Matrix::Scalar& eigenvalue, Vector& eigenvector)
  {
    // if diagonal matrix, the eigenvalues are the diagonal elements
    // and the eigenvectors are not unique, thus set to Identity
    if (fabs(mat.coeff (1)) <= std::numeric_limits<typename Matrix::Scalar>::min ())
    {
      if (mat.coeff (0) < mat.coeff (2))
      {
        eigenvalue = mat.coeff (0);
        eigenvector [0] = 1.0;
        eigenvector [1] = 0.0;
      }
      else
      {
        eigenvalue = mat.coeff (2);
        eigenvector [0] = 0.0;
        eigenvector [1] = 1.0;
      }
      return;
    }
    
    // 0.5 to optimize further calculations
    typename Matrix::Scalar trace = static_cast<typename Matrix::Scalar> (0.5) * (mat.coeff (0) + mat.coeff (3));
    typename Matrix::Scalar determinant = mat.coeff (0) * mat.coeff (3) - mat.coeff (1) * mat.coeff (1);

    typename Matrix::Scalar temp = trace * trace - determinant;

    if (temp < 0)
      temp = 0;

    eigenvalue = trace - ::std::sqrt (temp);
    
    eigenvector [0] = - mat.coeff (1);
    eigenvector [1] = mat.coeff (0) - eigenvalue;
    eigenvector.normalize ();
  }

  /** \brief determine the smallest eigenvalue and its corresponding eigenvector
    * \param[in] mat input matrix that needs to be symmetric and positive semi definite
    * \param[out] eigenvectors the corresponding eigenvector to the smallest eigenvalue of the input matrix
    * \param[out] eigenvalues the smallest eigenvalue of the input matrix
    * \ingroup common
    */
  template <typename Matrix, typename Vector> inline void
  eigen22 (const Matrix& mat, Matrix& eigenvectors, Vector& eigenvalues)
  {
    // if diagonal matrix, the eigenvalues are the diagonal elements
    // and the eigenvectors are not unique, thus set to Identity
    if (fabs(mat.coeff (1)) <= std::numeric_limits<typename Matrix::Scalar>::min ())
    {
      if (mat.coeff (0) < mat.coeff (3))
      {
        eigenvalues.coeffRef (0) = mat.coeff (0);
        eigenvalues.coeffRef (1) = mat.coeff (3);
        eigenvectors.coeffRef (0) = 1.0;
        eigenvectors.coeffRef (1) = 0.0;
        eigenvectors.coeffRef (2) = 0.0;
        eigenvectors.coeffRef (3) = 1.0;        
      }
      else
      {
        eigenvalues.coeffRef (0) = mat.coeff (3);
        eigenvalues.coeffRef (1) = mat.coeff (0);
        eigenvectors.coeffRef (0) = 0.0;
        eigenvectors.coeffRef (1) = 1.0;
        eigenvectors.coeffRef (2) = 1.0;
        eigenvectors.coeffRef (3) = 0.0;        
      }
      return;
    }

    // 0.5 to optimize further calculations
    typename Matrix::Scalar trace = static_cast<typename Matrix::Scalar> (0.5) * (mat.coeff (0) + mat.coeff (3));
    typename Matrix::Scalar determinant = mat.coeff (0) * mat.coeff (3) - mat.coeff (1) * mat.coeff (1);

    typename Matrix::Scalar temp = trace * trace - determinant;

    if (temp < 0)
      temp = 0;
    else
      temp = ::std::sqrt (temp);

    eigenvalues.coeffRef (0) = trace - temp;
    eigenvalues.coeffRef (1) = trace + temp;

    // either this is in a row or column depending on RowMajor or ColumnMajor
    eigenvectors.coeffRef (0) = - mat.coeff (1);
    eigenvectors.coeffRef (2) = mat.coeff (0) - eigenvalues.coeff (0);
    typename Matrix::Scalar norm = static_cast<typename Matrix::Scalar> (1.0) / 
                                   static_cast<typename Matrix::Scalar> (::std::sqrt (eigenvectors.coeffRef (0) * eigenvectors.coeffRef (0) + eigenvectors.coeffRef (2) * eigenvectors.coeffRef (2)));
    eigenvectors.coeffRef (0) *= norm;
    eigenvectors.coeffRef (2) *= norm;
    eigenvectors.coeffRef (1) = eigenvectors.coeffRef (2);
    eigenvectors.coeffRef (3) = -eigenvectors.coeffRef (0);
  }

  /** \brief determines the corresponding eigenvector to the given eigenvalue of the symmetric positive semi definite input matrix
    * \param[in] mat symmetric positive semi definite input matrix
    * \param[in] eigenvalue the eigenvalue which corresponding eigenvector is to be computed
    * \param[out] eigenvector the corresponding eigenvector for the input eigenvalue
    * \ingroup common
    */
  template<typename Matrix, typename Vector> inline void
  computeCorrespondingEigenVector (const Matrix& mat, const typename Matrix::Scalar& eigenvalue, Vector& eigenvector)
  {
    typedef typename Matrix::Scalar Scalar;
    // Scale the matrix so its entries are in [-1,1].  The scaling is applied
    // only when at least one matrix entry has magnitude larger than 1.

    Scalar scale = mat.cwiseAbs ().maxCoeff ();
    if (scale <= std::numeric_limits<Scalar>::min ())
      scale = Scalar (1.0);

    Matrix scaledMat = mat / scale;

    scaledMat.diagonal ().array () -= eigenvalue / scale;

    Vector vec1 = scaledMat.row (0).cross (scaledMat.row (1));
    Vector vec2 = scaledMat.row (0).cross (scaledMat.row (2));
    Vector vec3 = scaledMat.row (1).cross (scaledMat.row (2));

    Scalar len1 = vec1.squaredNorm ();
    Scalar len2 = vec2.squaredNorm ();
    Scalar len3 = vec3.squaredNorm ();

    if (len1 >= len2 && len1 >= len3)
      eigenvector = vec1 / std::sqrt (len1);
    else if (len2 >= len1 && len2 >= len3)
      eigenvector = vec2 / std::sqrt (len2);
    else
      eigenvector = vec3 / std::sqrt (len3);
  }
  
  /** \brief determines the eigenvector and eigenvalue of the smallest eigenvalue of the symmetric positive semi definite input matrix
    * \param[in] mat symmetric positive semi definite input matrix
    * \param[out] eigenvalue smallest eigenvalue of the input matrix
    * \param[out] eigenvector the corresponding eigenvector for the input eigenvalue
    * \note if the smallest eigenvalue is not unique, this function may return any eigenvector that is consistent to the eigenvalue.
    * \ingroup common
    */
  template<typename Matrix, typename Vector> inline void
  eigen33 (const Matrix& mat, typename Matrix::Scalar& eigenvalue, Vector& eigenvector)
  {
    typedef typename Matrix::Scalar Scalar;
    // Scale the matrix so its entries are in [-1,1].  The scaling is applied
    // only when at least one matrix entry has magnitude larger than 1.

    Scalar scale = mat.cwiseAbs ().maxCoeff ();
    if (scale <= std::numeric_limits<Scalar>::min ())
      scale = Scalar (1.0);

    Matrix scaledMat = mat / scale;

    Vector eigenvalues;
    computeRoots (scaledMat, eigenvalues);

    eigenvalue = eigenvalues (0) * scale;

    scaledMat.diagonal ().array () -= eigenvalues (0);

    Vector vec1 = scaledMat.row (0).cross (scaledMat.row (1));
    Vector vec2 = scaledMat.row (0).cross (scaledMat.row (2));
    Vector vec3 = scaledMat.row (1).cross (scaledMat.row (2));

    Scalar len1 = vec1.squaredNorm ();
    Scalar len2 = vec2.squaredNorm ();
    Scalar len3 = vec3.squaredNorm ();

    if (len1 >= len2 && len1 >= len3)
      eigenvector = vec1 / std::sqrt (len1);
    else if (len2 >= len1 && len2 >= len3)
      eigenvector = vec2 / std::sqrt (len2);
    else
      eigenvector = vec3 / std::sqrt (len3);
  }

  /** \brief determines the eigenvalues of the symmetric positive semi definite input matrix
    * \param[in] mat symmetric positive semi definite input matrix
    * \param[out] evals resulting eigenvalues in ascending order
    * \ingroup common
    */
  template<typename Matrix, typename Vector> inline void
  eigen33 (const Matrix& mat, Vector& evals)
  {
    typedef typename Matrix::Scalar Scalar;
    Scalar scale = mat.cwiseAbs ().maxCoeff ();
    if (scale <= std::numeric_limits<Scalar>::min ())
      scale = Scalar (1.0);

    Matrix scaledMat = mat / scale;
    computeRoots (scaledMat, evals);
    evals *= scale;
  }

  /** \brief determines the eigenvalues and corresponding eigenvectors of the symmetric positive semi definite input matrix
    * \param[in] mat symmetric positive semi definite input matrix
    * \param[out] evecs resulting eigenvalues in ascending order
    * \param[out] evals corresponding eigenvectors in correct order according to eigenvalues
    * \ingroup common
    */
  template<typename Matrix, typename Vector> inline void
  eigen33 (const Matrix& mat, Matrix& evecs, Vector& evals)
  {
    typedef typename Matrix::Scalar Scalar;
    // Scale the matrix so its entries are in [-1,1].  The scaling is applied
    // only when at least one matrix entry has magnitude larger than 1.

    Scalar scale = mat.cwiseAbs ().maxCoeff ();
    if (scale <= std::numeric_limits<Scalar>::min ())
      scale = Scalar (1.0);

    Matrix scaledMat = mat / scale;

    // Compute the eigenvalues
    computeRoots (scaledMat, evals);

    if ((evals (2) - evals (0)) <= Eigen::NumTraits<Scalar>::epsilon ())
    {
      // all three equal
      evecs.setIdentity ();
    }
    else if ((evals (1) - evals (0)) <= Eigen::NumTraits<Scalar>::epsilon () )
    {
      // first and second equal
      Matrix tmp;
      tmp = scaledMat;
      tmp.diagonal ().array () -= evals (2);

      Vector vec1 = tmp.row (0).cross (tmp.row (1));
      Vector vec2 = tmp.row (0).cross (tmp.row (2));
      Vector vec3 = tmp.row (1).cross (tmp.row (2));

      Scalar len1 = vec1.squaredNorm ();
      Scalar len2 = vec2.squaredNorm ();
      Scalar len3 = vec3.squaredNorm ();

      if (len1 >= len2 && len1 >= len3)
        evecs.col (2) = vec1 / std::sqrt (len1);
      else if (len2 >= len1 && len2 >= len3)
        evecs.col (2) = vec2 / std::sqrt (len2);
      else
        evecs.col (2) = vec3 / std::sqrt (len3);

      evecs.col (1) = evecs.col (2).unitOrthogonal ();
      evecs.col (0) = evecs.col (1).cross (evecs.col (2));
    }
    else if ((evals (2) - evals (1)) <= Eigen::NumTraits<Scalar>::epsilon () )
    {
      // second and third equal
      Matrix tmp;
      tmp = scaledMat;
      tmp.diagonal ().array () -= evals (0);

      Vector vec1 = tmp.row (0).cross (tmp.row (1));
      Vector vec2 = tmp.row (0).cross (tmp.row (2));
      Vector vec3 = tmp.row (1).cross (tmp.row (2));

      Scalar len1 = vec1.squaredNorm ();
      Scalar len2 = vec2.squaredNorm ();
      Scalar len3 = vec3.squaredNorm ();

      if (len1 >= len2 && len1 >= len3)
        evecs.col (0) = vec1 / std::sqrt (len1);
      else if (len2 >= len1 && len2 >= len3)
        evecs.col (0) = vec2 / std::sqrt (len2);
      else
        evecs.col (0) = vec3 / std::sqrt (len3);

      evecs.col (1) = evecs.col (0).unitOrthogonal ();
      evecs.col (2) = evecs.col (0).cross (evecs.col (1));
    }
    else
    {
      Matrix tmp;
      tmp = scaledMat;
      tmp.diagonal ().array () -= evals (2);

      Vector vec1 = tmp.row (0).cross (tmp.row (1));
      Vector vec2 = tmp.row (0).cross (tmp.row (2));
      Vector vec3 = tmp.row (1).cross (tmp.row (2));

      Scalar len1 = vec1.squaredNorm ();
      Scalar len2 = vec2.squaredNorm ();
      Scalar len3 = vec3.squaredNorm ();
#ifdef _WIN32
      Scalar *mmax = new Scalar[3];
#else
      Scalar mmax[3];
#endif
      unsigned int min_el = 2;
      unsigned int max_el = 2;
      if (len1 >= len2 && len1 >= len3)
      {
        mmax[2] = len1;
        evecs.col (2) = vec1 / std::sqrt (len1);
      }
      else if (len2 >= len1 && len2 >= len3)
      {
        mmax[2] = len2;
        evecs.col (2) = vec2 / std::sqrt (len2);
      }
      else
      {
        mmax[2] = len3;
        evecs.col (2) = vec3 / std::sqrt (len3);
      }

      tmp = scaledMat;
      tmp.diagonal ().array () -= evals (1);

      vec1 = tmp.row (0).cross (tmp.row (1));
      vec2 = tmp.row (0).cross (tmp.row (2));
      vec3 = tmp.row (1).cross (tmp.row (2));

      len1 = vec1.squaredNorm ();
      len2 = vec2.squaredNorm ();
      len3 = vec3.squaredNorm ();
      if (len1 >= len2 && len1 >= len3)
      {
        mmax[1] = len1;
        evecs.col (1) = vec1 / std::sqrt (len1);
        min_el = len1 <= mmax[min_el] ? 1 : min_el;
        max_el = len1 > mmax[max_el] ? 1 : max_el;
      }
      else if (len2 >= len1 && len2 >= len3)
      {
        mmax[1] = len2;
        evecs.col (1) = vec2 / std::sqrt (len2);
        min_el = len2 <= mmax[min_el] ? 1 : min_el;
        max_el = len2 > mmax[max_el] ? 1 : max_el;
      }
      else
      {
        mmax[1] = len3;
        evecs.col (1) = vec3 / std::sqrt (len3);
        min_el = len3 <= mmax[min_el] ? 1 : min_el;
        max_el = len3 > mmax[max_el] ? 1 : max_el;
      }

      tmp = scaledMat;
      tmp.diagonal ().array () -= evals (0);

      vec1 = tmp.row (0).cross (tmp.row (1));
      vec2 = tmp.row (0).cross (tmp.row (2));
      vec3 = tmp.row (1).cross (tmp.row (2));

      len1 = vec1.squaredNorm ();
      len2 = vec2.squaredNorm ();
      len3 = vec3.squaredNorm ();
      if (len1 >= len2 && len1 >= len3)
      {
        mmax[0] = len1;
        evecs.col (0) = vec1 / std::sqrt (len1);
        min_el = len3 <= mmax[min_el] ? 0 : min_el;
        max_el = len3 > mmax[max_el] ? 0 : max_el;
      }
      else if (len2 >= len1 && len2 >= len3)
      {
        mmax[0] = len2;
        evecs.col (0) = vec2 / std::sqrt (len2);
        min_el = len3 <= mmax[min_el] ? 0 : min_el;
        max_el = len3 > mmax[max_el] ? 0 : max_el;
      }
      else
      {
        mmax[0] = len3;
        evecs.col (0) = vec3 / std::sqrt (len3);
        min_el = len3 <= mmax[min_el] ? 0 : min_el;
        max_el = len3 > mmax[max_el] ? 0 : max_el;
      }

      unsigned mid_el = 3 - min_el - max_el;
      evecs.col (min_el) = evecs.col ((min_el + 1) % 3).cross ( evecs.col ((min_el + 2) % 3) ).normalized ();
      evecs.col (mid_el) = evecs.col ((mid_el + 1) % 3).cross ( evecs.col ((mid_el + 2) % 3) ).normalized ();
#ifdef _WIN32
      delete [] mmax;
#endif
    }
    // Rescale back to the original size.
    evals *= scale;
  }

  /** \brief Calculate the inverse of a 2x2 matrix
    * \param[in] matrix matrix to be inverted
    * \param[out] inverse the resultant inverted matrix
    * \note only the upper triangular part is taken into account => non symmetric matrices will give wrong results
    * \return determinant of the original matrix => if 0 no inverse exists => result is invalid
    * \ingroup common
    */
  template<typename Matrix> inline typename Matrix::Scalar
  invert2x2 (const Matrix& matrix, Matrix& inverse)
  {
    typedef typename Matrix::Scalar Scalar;
    Scalar det = matrix.coeff (0) * matrix.coeff (3) - matrix.coeff (1) * matrix.coeff (2) ;

    if (det != 0)
    {
      //Scalar inv_det = Scalar (1.0) / det;
      inverse.coeffRef (0) =   matrix.coeff (3);
      inverse.coeffRef (1) = - matrix.coeff (1);
      inverse.coeffRef (2) = - matrix.coeff (2);
      inverse.coeffRef (3) =   matrix.coeff (0);
      inverse /= det;
    }
    return det;
  }

  /** \brief Calculate the inverse of a 3x3 symmetric matrix.
    * \param[in] matrix matrix to be inverted
    * \param[out] inverse the resultant inverted matrix
    * \note only the upper triangular part is taken into account => non symmetric matrices will give wrong results
    * \return determinant of the original matrix => if 0 no inverse exists => result is invalid
    * \ingroup common
    */
  template<typename Matrix> inline typename Matrix::Scalar
  invert3x3SymMatrix (const Matrix& matrix, Matrix& inverse)
  {
    typedef typename Matrix::Scalar Scalar;
    // elements
    // a b c
    // b d e
    // c e f
    //| a b c |-1             |   fd-ee    ce-bf   be-cd  |
    //| b d e |    =  1/det * |   ce-bf    af-cc   bc-ae  |
    //| c e f |               |   be-cd    bc-ae   ad-bb  |

    //det = a(fd-ee) + b(ec-fb) + c(eb-dc)

    Scalar fd_ee = matrix.coeff (4) * matrix.coeff (8) - matrix.coeff (7) * matrix.coeff (5);
    Scalar ce_bf = matrix.coeff (2) * matrix.coeff (5) - matrix.coeff (1) * matrix.coeff (8);
    Scalar be_cd = matrix.coeff (1) * matrix.coeff (5) - matrix.coeff (2) * matrix.coeff (4);

    Scalar det = matrix.coeff (0) * fd_ee + matrix.coeff (1) * ce_bf + matrix.coeff (2) * be_cd;

    if (det != 0)
    {
      //Scalar inv_det = Scalar (1.0) / det;
      inverse.coeffRef (0) = fd_ee;
      inverse.coeffRef (1) = inverse.coeffRef (3) = ce_bf;
      inverse.coeffRef (2) = inverse.coeffRef (6) = be_cd;
      inverse.coeffRef (4) = (matrix.coeff (0) * matrix.coeff (8) - matrix.coeff (2) * matrix.coeff (2));
      inverse.coeffRef (5) = inverse.coeffRef (7) = (matrix.coeff (1) * matrix.coeff (2) - matrix.coeff (0) * matrix.coeff (5));
      inverse.coeffRef (8) = (matrix.coeff (0) * matrix.coeff (4) - matrix.coeff (1) * matrix.coeff (1));
      inverse /= det;
    }
    return det;
  }

  /** \brief Calculate the inverse of a general 3x3 matrix.
    * \param[in] matrix matrix to be inverted
    * \param[out] inverse the resultant inverted matrix
    * \return determinant of the original matrix => if 0 no inverse exists => result is invalid
    * \ingroup common
    */
  template<typename Matrix> inline typename Matrix::Scalar
  invert3x3Matrix (const Matrix& matrix, Matrix& inverse)
  {
    typedef typename Matrix::Scalar Scalar;

    //| a b c |-1             |   ie-hf    hc-ib   fb-ec  |
    //| d e f |    =  1/det * |   gf-id    ia-gc   dc-fa  |
    //| g h i |               |   hd-ge    gb-ha   ea-db  |
    //det = a(ie-hf) + d(hc-ib) + g(fb-ec)

    Scalar ie_hf = matrix.coeff (8) * matrix.coeff (4) - matrix.coeff (7) * matrix.coeff (5);
    Scalar hc_ib = matrix.coeff (7) * matrix.coeff (2) - matrix.coeff (8) * matrix.coeff (1);
    Scalar fb_ec = matrix.coeff (5) * matrix.coeff (1) - matrix.coeff (4) * matrix.coeff (2);
    Scalar det = matrix.coeff (0) * (ie_hf) + matrix.coeff (3) * (hc_ib) + matrix.coeff (6) * (fb_ec) ;

    if (det != 0)
    {
      inverse.coeffRef (0) = ie_hf;
      inverse.coeffRef (1) = hc_ib;
      inverse.coeffRef (2) = fb_ec;
      inverse.coeffRef (3) = matrix.coeff (6) * matrix.coeff (5) - matrix.coeff (8) * matrix.coeff (3);
      inverse.coeffRef (4) = matrix.coeff (8) * matrix.coeff (0) - matrix.coeff (6) * matrix.coeff (2);
      inverse.coeffRef (5) = matrix.coeff (3) * matrix.coeff (2) - matrix.coeff (5) * matrix.coeff (0);
      inverse.coeffRef (6) = matrix.coeff (7) * matrix.coeff (3) - matrix.coeff (6) * matrix.coeff (4);
      inverse.coeffRef (7) = matrix.coeff (6) * matrix.coeff (1) - matrix.coeff (7) * matrix.coeff (0);
      inverse.coeffRef (8) = matrix.coeff (4) * matrix.coeff (0) - matrix.coeff (3) * matrix.coeff (1);

      inverse /= det;
    }
    return det;
  }

  template<typename Matrix> inline typename Matrix::Scalar
  determinant3x3Matrix (const Matrix& matrix)
  {
    // result is independent of Row/Col Major storage!
    return matrix.coeff (0) * (matrix.coeff (4) * matrix.coeff (8) - matrix.coeff (5) * matrix.coeff (7)) +
           matrix.coeff (1) * (matrix.coeff (5) * matrix.coeff (6) - matrix.coeff (3) * matrix.coeff (8)) +
           matrix.coeff (2) * (matrix.coeff (3) * matrix.coeff (7) - matrix.coeff (4) * matrix.coeff (6)) ;
  }
  
  /** \brief Get the unique 3D rotation that will rotate \a z_axis into (0,0,1) and \a y_direction into a vector
    * with x=0 (or into (0,1,0) should \a y_direction be orthogonal to \a z_axis)
    * \param[in] z_axis the z-axis
    * \param[in] y_direction the y direction
    * \param[out] transformation the resultant 3D rotation
    * \ingroup common
    */
  inline void
  getTransFromUnitVectorsZY (const Eigen::Vector3f& z_axis, 
                             const Eigen::Vector3f& y_direction,
                             Eigen::Affine3f& transformation);

  /** \brief Get the unique 3D rotation that will rotate \a z_axis into (0,0,1) and \a y_direction into a vector
    * with x=0 (or into (0,1,0) should \a y_direction be orthogonal to \a z_axis)
    * \param[in] z_axis the z-axis
    * \param[in] y_direction the y direction
    * \return the resultant 3D rotation
    * \ingroup common
    */
  inline Eigen::Affine3f
  getTransFromUnitVectorsZY (const Eigen::Vector3f& z_axis, 
                             const Eigen::Vector3f& y_direction);

  /** \brief Get the unique 3D rotation that will rotate \a x_axis into (1,0,0) and \a y_direction into a vector
    * with z=0 (or into (0,1,0) should \a y_direction be orthogonal to \a z_axis)
    * \param[in] x_axis the x-axis
    * \param[in] y_direction the y direction
    * \param[out] transformation the resultant 3D rotation
    * \ingroup common
    */
  inline void
  getTransFromUnitVectorsXY (const Eigen::Vector3f& x_axis, 
                             const Eigen::Vector3f& y_direction,
                             Eigen::Affine3f& transformation);

  /** \brief Get the unique 3D rotation that will rotate \a x_axis into (1,0,0) and \a y_direction into a vector
    * with z=0 (or into (0,1,0) should \a y_direction be orthogonal to \a z_axis)
    * \param[in] x_axis the x-axis
    * \param[in] y_direction the y direction
    * \return the resulting 3D rotation
    * \ingroup common
    */
  inline Eigen::Affine3f
  getTransFromUnitVectorsXY (const Eigen::Vector3f& x_axis, 
                             const Eigen::Vector3f& y_direction);

  /** \brief Get the unique 3D rotation that will rotate \a z_axis into (0,0,1) and \a y_direction into a vector
    * with x=0 (or into (0,1,0) should \a y_direction be orthogonal to \a z_axis)
    * \param[in] y_direction the y direction
    * \param[in] z_axis the z-axis
    * \param[out] transformation the resultant 3D rotation
    * \ingroup common
    */
  inline void
  getTransformationFromTwoUnitVectors (const Eigen::Vector3f& y_direction, 
                                       const Eigen::Vector3f& z_axis,
                                       Eigen::Affine3f& transformation);

  /** \brief Get the unique 3D rotation that will rotate \a z_axis into (0,0,1) and \a y_direction into a vector
    * with x=0 (or into (0,1,0) should \a y_direction be orthogonal to \a z_axis)
    * \param[in] y_direction the y direction
    * \param[in] z_axis the z-axis
    * \return transformation the resultant 3D rotation
    * \ingroup common
    */
  inline Eigen::Affine3f
  getTransformationFromTwoUnitVectors (const Eigen::Vector3f& y_direction, 
                                       const Eigen::Vector3f& z_axis);

  /** \brief Get the transformation that will translate \a orign to (0,0,0) and rotate \a z_axis into (0,0,1)
    * and \a y_direction into a vector with x=0 (or into (0,1,0) should \a y_direction be orthogonal to \a z_axis)
    * \param[in] y_direction the y direction
    * \param[in] z_axis the z-axis
    * \param[in] origin the origin
    * \param[in] transformation the resultant transformation matrix
    * \ingroup common
    */
  inline void
  getTransformationFromTwoUnitVectorsAndOrigin (const Eigen::Vector3f& y_direction, 
                                                const Eigen::Vector3f& z_axis,
                                                const Eigen::Vector3f& origin, 
                                                Eigen::Affine3f& transformation);

  /** \brief Extract the Euler angles (XYZ-convention) from the given transformation
    * \param[in] t the input transformation matrix
    * \param[in] roll the resulting roll angle
    * \param[in] pitch the resulting pitch angle
    * \param[in] yaw the resulting yaw angle
    * \ingroup common
    */
  inline void
  getEulerAngles (const Eigen::Affine3f& t, float& roll, float& pitch, float& yaw);

  /** Extract x,y,z and the Euler angles (XYZ-convention) from the given transformation
    * \param[in] t the input transformation matrix
    * \param[out] x the resulting x translation
    * \param[out] y the resulting y translation
    * \param[out] z the resulting z translation
    * \param[out] roll the resulting roll angle
    * \param[out] pitch the resulting pitch angle
    * \param[out] yaw the resulting yaw angle
    * \ingroup common
    */
  inline void
  getTranslationAndEulerAngles (const Eigen::Affine3f& t, 
                                float& x, float& y, float& z,
                                float& roll, float& pitch, float& yaw);

  /** \brief Create a transformation from the given translation and Euler angles (XYZ-convention)
    * \param[in] x the input x translation
    * \param[in] y the input y translation
    * \param[in] z the input z translation
    * \param[in] roll the input roll angle
    * \param[in] pitch the input pitch angle
    * \param[in] yaw the input yaw angle
    * \param[out] t the resulting transformation matrix
    * \ingroup common
    */
  template <typename Scalar> inline void
  getTransformation (Scalar x, Scalar y, Scalar z, Scalar roll, Scalar pitch, Scalar yaw, 
                     Eigen::Transform<Scalar, 3, Eigen::Affine> &t);

  inline void
  getTransformation (float x, float y, float z, float roll, float pitch, float yaw, 
                     Eigen::Affine3f &t)
  {
    return (getTransformation<float> (x, y, z, roll, pitch, yaw, t));
  }

  inline void
  getTransformation (double x, double y, double z, double roll, double pitch, double yaw, 
                     Eigen::Affine3d &t)
  {
    return (getTransformation<double> (x, y, z, roll, pitch, yaw, t));
  }

  /** \brief Create a transformation from the given translation and Euler angles (XYZ-convention)
    * \param[in] x the input x translation
    * \param[in] y the input y translation
    * \param[in] z the input z translation
    * \param[in] roll the input roll angle
    * \param[in] pitch the input pitch angle
    * \param[in] yaw the input yaw angle
    * \return the resulting transformation matrix
    * \ingroup common
    */
  inline Eigen::Affine3f
  getTransformation (float x, float y, float z, float roll, float pitch, float yaw);

  /** \brief Write a matrix to an output stream
    * \param[in] matrix the matrix to output
    * \param[out] file the output stream
    * \ingroup common
    */
  template <typename Derived> void
  saveBinary (const Eigen::MatrixBase<Derived>& matrix, std::ostream& file);

  /** \brief Read a matrix from an input stream
    * \param[out] matrix the resulting matrix, read from the input stream
    * \param[in,out] file the input stream
    * \ingroup common
    */
  template <typename Derived> void
  loadBinary (Eigen::MatrixBase<Derived> const& matrix, std::istream& file);

// PCL_EIGEN_SIZE_MIN_PREFER_DYNAMIC gives the min between compile-time sizes. 0 has absolute priority, followed by 1,
// followed by Dynamic, followed by other finite values. The reason for giving Dynamic the priority over
// finite values is that min(3, Dynamic) should be Dynamic, since that could be anything between 0 and 3.
#define KP_EIGEN_SIZE_MIN_PREFER_DYNAMIC(a,b) ((int (a) == 0 || int (b) == 0) ? 0 \
                           : (int (a) == 1 || int (b) == 1) ? 1 \
                           : (int (a) == Eigen::Dynamic || int (b) == Eigen::Dynamic) ? Eigen::Dynamic \
                           : (int (a) <= int (b)) ? int (a) : int (b))

  /** \brief Returns the transformation between two point sets. 
    * The algorithm is based on: 
    * "Least-squares estimation of transformation parameters between two point patterns",
    * Shinji Umeyama, PAMI 1991, DOI: 10.1109/34.88573
    *
    * It estimates parameters \f$ c, \mathbf{R}, \f$ and \f$ \mathbf{t} \f$ such that
    * \f{align*}
    *   \frac{1}{n} \sum_{i=1}^n \vert\vert y_i - (c\mathbf{R}x_i + \mathbf{t}) \vert\vert_2^2
    * \f}
    * is minimized.
    *
    * The algorithm is based on the analysis of the covariance matrix
    * \f$ \Sigma_{\mathbf{x}\mathbf{y}} \in \mathbb{R}^{d \times d} \f$
    * of the input point sets \f$ \mathbf{x} \f$ and \f$ \mathbf{y} \f$ where
    * \f$d\f$ is corresponding to the dimension (which is typically small).
    * The analysis is involving the SVD having a complexity of \f$O(d^3)\f$
    * though the actual computational effort lies in the covariance
    * matrix computation which has an asymptotic lower bound of \f$O(dm)\f$ when
    * the input point sets have dimension \f$d \times m\f$.
    *
    * \param[in] src Source points \f$ \mathbf{x} = \left( x_1, \hdots, x_n \right) \f$
    * \param[in] dst Destination points \f$ \mathbf{y} = \left( y_1, \hdots, y_n \right) \f$.
    * \param[in] with_scaling Sets \f$ c=1 \f$ when <code>false</code> is passed. (default: false)
    * \return The homogeneous transformation 
    * \f{align*}
    *   T = \begin{bmatrix} c\mathbf{R} & \mathbf{t} \\ \mathbf{0} & 1 \end{bmatrix}
    * \f}
    * minimizing the resudiual above. This transformation is always returned as an
    * Eigen::Matrix.
    */
  template <typename Derived, typename OtherDerived> 
  typename Eigen::internal::umeyama_transform_matrix_type<Derived, OtherDerived>::type
  umeyama (const Eigen::MatrixBase<Derived>& src, const Eigen::MatrixBase<OtherDerived>& dst, bool with_scaling = false);
}

#include "eigen.hpp"

#if defined __SUNPRO_CC
#  pragma enable_warn
#endif

#endif  //KP_COMMON_EIGEN_H_
