/**
 * @file r2_score.hpp
 * @author Bisakh Mondal
 *
 * The R^2 (Coefficient of determination) regression metric.
 *
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#ifndef MLPACK_CORE_CV_METRICS_R2SCORE_HPP
#define MLPACK_CORE_CV_METRICS_R2SCORE_HPP

#include <mlpack/core.hpp>

namespace mlpack {
namespace cv {

/**
 * The R2Score is a metric of performance for regression algorithms
 * that represents the proportion of variance (of y) that has been
 * explained by the independent variables in the model. It provides
 * an indication of goodness of fit and therefore a measure of how
 * well unseen samples are likely to be predicted by the model,
 * through the proportion of explained variance.
 * As R2Score is dataset dependent it can have wide range of values,
 * best possible score is @f$R^2 =1.0@f$, and it can be negative too for an
 * arbitraryly worse model. For a model which predicts exactly the expected
 * value of y, disregarding the input features, gets a R2Score equals to 0.0.
 * If a model predicts @f$ \hat{y}_i $@f of the @f$ i $@f-th sample for a true
 * @f$ y_i $@f for total n samples, the R2Score is calculated by
 * @f{eqnarray*}{
 * R^{2} \left( y, \hat{y} \right) &=& 1-\frac{\sum_{i=1}^{n}
 * \left( y_i - \hat{y_i} \right)^2 }
 * {\sum_{i=1}^{n} \left( y_i - \bar{y}\right)^2}\\
 * @f}
 * where @f$ \bar{y} = frac{1}{y}\sum_{i=1}^{n} y_i $@f.
 * For example, a model having R2Score = 0.85, explains 85 \% variability of
 * the response data around its mean.
 */
class R2Score
{
 public:
  /**
   * Run prediction and calculate the R squared error.
   *
   * @param model A regression model.
   * @param data Column-major data containing test items.
   * @param responses Ground truth (correct) target values for the test items,
   *     should be either a row vector or a column-major matrix.
   */
  template<typename MLAlgorithm, typename DataType, typename ResponsesType>
  static double Evaluate(MLAlgorithm& model,
                         const DataType& data,
                         const ResponsesType& responses);

  /**
   * Information for hyper-parameter tuning code. It indicates that we want
   * to maximize the measurement.
   */
  static const bool NeedsMinimization = false;
};

} // namespace cv
} // namespace mlpack

// Include implementation.
#include "r2_score_impl.hpp"

#endif
