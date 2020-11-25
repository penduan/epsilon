#include <quiz.h>
#include <string.h>
#include <assert.h>
#include <apps/shared/global_context.h>
#include "../model/model.h"
#include "../regression_context.h"
#include "../store.h"
#include <poincare/helpers.h>
#include <poincare/test/helper.h>

using namespace Poincare;
using namespace Regression;

/* The data was generated by choosing X1 and the coefficients of the regression,
 * then filling Y1 with the regression formula + random()/10. */

void setRegressionPoints(Regression::Store * store, int series, int numberOfPoints, double * xi, double * yi = nullptr) {
  for (int i = 0; i < numberOfPoints; i++) {
    store->set(xi[i], series, 0, i);
    if (yi != nullptr) {
      store->set(yi[i], series, 1, i);
    }
  }
}

void assert_regression_is(double * xi, double * yi, int numberOfPoints, Model::Type modelType, double * trueCoefficients, double trueR2) {
  int series = 0;
  Regression::Store store;

  setRegressionPoints(&store, series, numberOfPoints, xi, yi);
  store.setSeriesRegressionType(series, modelType);
  Shared::GlobalContext globalContext;
  RegressionContext context(&store, &globalContext);

  double precision = 1e-2;
  // When trueCoefficients = 0, a DBL_EPSILON reference ensures that the only accepted errors are due to double approximations
  double reference = 1e6 * DBL_EPSILON;

  // Compute and compare the coefficients
  double * coefficients = store.coefficientsForSeries(series, &context);
  int numberOfCoefs = store.modelForSeries(series)->numberOfCoefficients();
  for (int i = 0; i < numberOfCoefs; i++) {
    quiz_assert(IsApproximatelyEqual(coefficients[i], trueCoefficients[i], precision, reference));
  }

  // Compute and check r2 value and sign
  double r2 = store.determinationCoefficientForSeries(series, &globalContext);
  quiz_assert(r2 <= 1.0 && (r2 >= 0.0 || modelType == Model::Type::Proportional));
  quiz_assert(IsApproximatelyEqual(r2, trueR2, precision, reference));
}

QUIZ_CASE(linear_regression) {
  double x[] = {1.0, 8.0, 14.0, 79.0};
  double y[] = {-3.581, 20.296, 40.676, 261.623};
  double coefficients[] = {3.4, -7.0};
  double r2 = 1.0;
  assert_regression_is(x, y, 4, Model::Type::Linear, coefficients, r2);
}

QUIZ_CASE(linear_regression2) {
  double x[] = {-5.0, 2.0, 4.0, 5.6, 9.0};
  double y[] = {22.0, 1.0, 13.0, 28.36, 78.0};
  double coefficients[] = {3.31824, 18.1191};
  double r2 = 0.343;
  assert_regression_is(x, y, 5, Model::Type::Linear, coefficients, r2);
}

QUIZ_CASE(proportional_regression) {
  double x[] = {7.0, 5.0, 1.0, 9.0, 3.0};
  double y[] = {-41.4851, -29.62186, -6.454245, -53.4976, -18.03325};
  double coefficients[] = {-5.89};
  double r2 = 0.9999648161902982;
  assert_regression_is(x, y, 5, Model::Type::Proportional, coefficients, r2);
}

QUIZ_CASE(proportional_regression2) {
  constexpr int numberOfPoints = 4;
  double x[numberOfPoints] = {5.0, 2.0, 3.0, 4.0};
  double y[numberOfPoints] = {10.0, 6.0, 7.0, 8.0};
  double coefficients[] = {2.12963963};
  double r2 = 0.53227513227513223;
  assert_regression_is(x, y, numberOfPoints, Model::Type::Proportional, coefficients, r2);
}

QUIZ_CASE(proportional_regression3) {
  constexpr int numberOfPoints = 4;
  double x[numberOfPoints] = {1.0, 2.0, 3.0, 4.0};
  double y[numberOfPoints] = {0.0, 0.0, 0.0, 0.0};
  double coefficients[] = {0.0};
  double r2 = 1.0;
  assert_regression_is(x, y, numberOfPoints, Model::Type::Proportional, coefficients, r2);
}

QUIZ_CASE(proportional_regression4) {
  constexpr int numberOfPoints = 3;
  double x[numberOfPoints] = {-1.0, 0.0, 1.0};
  double y[numberOfPoints] = {1.0, 1.0, 1.0};
  double coefficients[] = {0.0};
  // Y is constant, and proportional regression cannot fit it, R2 is null.
  double r2 = 0.0;
  assert_regression_is(x, y, numberOfPoints, Model::Type::Proportional, coefficients, r2);
}

QUIZ_CASE(proportional_regression5) {
  constexpr int numberOfPoints = 3;
  double x[numberOfPoints] = {-1.0, 0.0, 1.0};
  double y[numberOfPoints] = {1.0, 1.01, 1.0};
  double coefficients[] = {0.0};
  /* In this case, proportional regression performed poorly compared to a
   * constant regression, R2 is negative. */
  double r2 = -45300.5;
  assert_regression_is(x, y, numberOfPoints, Model::Type::Proportional, coefficients, r2);
}

QUIZ_CASE(quadratic_regression) {
  double x[] = {-34.0, -12.0, 5.0, 86.0, -2.0};
  double y[] = {-8241.389, -1194.734, -59.163, - 46245.39, -71.774};
  double coefficients[] = {-6.50001, 21.3004, -3.15799};
  double r2 = 1.0;
  assert_regression_is(x, y, 5, Model::Type::Quadratic, coefficients, r2);
}

QUIZ_CASE(cubic_regression) {
  double x[] = {-3.0, -2.8, -1.0, 0.0, 12.0};
  double y[] = {691.261, 566.498, 20.203, -12.865, -34293.21};
  double coefficients[] = {-21.2015, 16.0141, 4.14522, -12.8658};
  double r2 = 1.0;
  assert_regression_is(x, y, 5, Model::Type::Cubic, coefficients, r2);
}

QUIZ_CASE(quartic_regression) {
  double x[] = {1.6, 3.5, 3.5, -2.8, 6.4, 5.3, 2.9, -4.8, -5.7, 3.1};
  double y[] = {-112.667, -1479.824, -1479.805, 1140.276, -9365.505, -5308.355, -816.925, 5554.007, 9277.107, -1009.874};
  double coefficients[] = {0.59998, -42.9998, 21.5015, 3.09232, -0.456824};
  double r2 = 1.0;
  assert_regression_is(x, y, 10, Model::Type::Quartic, coefficients, r2);
}

QUIZ_CASE(logarithmic_regression) {
  double x[] = {0.2, 0.5, 5, 7};
  double y[] = {-11.952, -9.035, -1.695, -0.584};
  double coefficients[] = {3.19383, -6.81679};
  double r2 = 0.999994;
  assert_regression_is(x, y, 4, Model::Type::Logarithmic, coefficients, r2);
}

QUIZ_CASE(exponential_regression) {
  double x[] = {5.5, 5.6, 5.7, 5.8, 5.9, 6.0};
  double y[] = {-276.842, -299.956, -324.933, -352.0299, -381.314, -413.0775};
  double coefficients[] = {-3.4, 0.8};
  double r2 = 1.0;
  assert_regression_is(x, y, 6, Model::Type::Exponential, coefficients, r2);
}

QUIZ_CASE(exponential_regression2) {
  double x[] = {0, 1, 2, 3};
  double y[] = {3000, 3315.513, 3664.208, 4049.576};
  double coefficients[] = {3000, .1};
  double r2 = 1.0;
  assert_regression_is(x, y, 4, Model::Type::Exponential, coefficients, r2);
}

QUIZ_CASE(exponential_regression3) {
  double x[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  double y[] = {-1, -.3678794, -.1353353, -.04978707, -.01831564, -.006737947, -.002478752, -.000911882, -.0003354626, -.0001234098, -.00004539993};
  double coefficients[] = {-1, -1};
  double r2 = 0.9999999999999992;
  assert_regression_is(x, y, 11, Model::Type::Exponential, coefficients, r2);

  // TODO : This data produce a wrong fit currently
  // double x2[] = {1.0, 2.0, 3.0, 4.0};
  // double y2[] = {2.0, 3.0, 4.0, 1.0};
  // double coefficients2[] = {2.905, -0.0606857};
  // double r22 = 0.838388;
  // assert_regression_is(x2, y2, 4, Model::Type::Exponential, coefficients2, r22);
}

QUIZ_CASE(power_regression) {
  double x[] = {1.0, 50.0, 34.0, 67.0, 20.0};
  double y[] = {71.860, 2775514, 979755.1, 6116830, 233832.9};
  double coefficients[] = {71.8, 2.7};
  double r2 = 1.0;
  assert_regression_is(x, y, 5, Model::Type::Power, coefficients, r2);

  // TODO : This data produce a wrong fit currently
  // double x2[] = {1.0, 2.0, 3.0, 4.0};
  // double y2[] = {2.0, 3.0, 4.0, 1.0};
  // double coefficients2[] = {2.54948, -0.0247463};
  // double r22 = 0.833509;
  // assert_regression_is(x2, y2, 4, Model::Type::Power, coefficients2, r22);
}

QUIZ_CASE(trigonometric_regression) {
  Preferences::AngleUnit previousAngleUnit = Preferences::sharedPreferences()->angleUnit();
  double r2 = 0.9994216;
  double x[] = {1, 31, 61, 91, 121, 151, 181, 211, 241, 271, 301, 331, 361};
  double y[] = {9.24, 10.05, 11.33, 12.72, 14.16, 14.98, 15.14, 14.41, 13.24, 11.88, 10.54, 9.48, 9.19};
  double coefficients[] = {2.9723, 0.016780, -1.3067, 12.146};
  int numberOfPoints = 13;

  // TODO : Ensure unicity with trigonometric coefficients.
  Poincare::Preferences::sharedPreferences()->setAngleUnit(Poincare::Preferences::AngleUnit::Radian);
  // a*sin(b*x+c)+d = -a*sin(b*x+c+π)+d
  double coefficientsRad[] = {-coefficients[0], coefficients[1], coefficients[2] + M_PI, coefficients[3]};
  assert_regression_is(x, y, numberOfPoints, Model::Type::Trigonometric, coefficientsRad, r2);

  Poincare::Preferences::sharedPreferences()->setAngleUnit(Poincare::Preferences::AngleUnit::Degree);
  double radToDeg = 180.0 / M_PI;
  // a*sin(b*x+c)+d = a*sin(b*x+c+2π)+d
  double coefficientsDeg[] = {coefficients[0], coefficients[1] * radToDeg, (coefficients[2] - 2.0 * M_PI) * radToDeg, coefficients[3]};
  assert_regression_is(x, y, numberOfPoints, Model::Type::Trigonometric, coefficientsDeg, r2);

  Poincare::Preferences::sharedPreferences()->setAngleUnit(Poincare::Preferences::AngleUnit::Gradian);
  double radToGrad = 200.0 / M_PI;
  // a*sin(b*x+c)+d = a*sin(b*x+c+2π)+d
  double coefficientsGrad[] = {coefficients[0], coefficients[1] * radToGrad, (coefficients[2] - 2.0 * M_PI) * radToGrad, coefficients[3]};
  assert_regression_is(x, y, numberOfPoints, Model::Type::Trigonometric, coefficientsGrad, r2);

  Poincare::Preferences::sharedPreferences()->setAngleUnit(previousAngleUnit);
}


QUIZ_CASE(logistic_regression) {
  /* This data was generated without the random error, otherwise it did not pass
   * the test. */
  double x1[] = {2.3, 5.6, 1.1, 4.3};
  double y1[] = {3.948, 4.694, 2.184, 4.656};
  double coefficients1[] = {6, 1.5, 4.7};
  double r21 = 0.9999999917270119;
  assert_regression_is(x1, y1, 4, Model::Type::Logistic, coefficients1, r21);

  // This data produced a wrong fit before
  double x2[] = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
  double y2[] = {5.0, 9.0, 40.0, 64.0, 144.0, 200.0, 269.0, 278.0, 290.0, 295.0};
  double coefficients2[] = {64.9, 1.0, 297.4};
  double r22 = 0.9984396821656006;
  assert_regression_is(x2, y2, 10, Model::Type::Logistic, coefficients2, r22);

  // TODO : This data produce a wrong fit currently
  // double x3[] = {1.0, 3.0, 4.0, 6.0, 8.0};
  // double y3[] = {4.0, 4.0, 0.0, 58.0, 5.0};
  // No source of truth for coefficient, r2 should at least be positive.
  // double coefficients3[] = {-0.1, -0.4, -4};
  // double r23 = 0.75;
  // assert_regression_is(x3, y3, 5, Model::Type::Logistic, coefficients3, r23);
}

// Testing column and regression calculation

void assert_column_calculations_is(double * xi, int numberOfPoints, double trueMean, double trueSum, double trueSquaredSum, double trueStandardDeviation, double trueVariance) {
  int series = 0;
  Regression::Store store;

  setRegressionPoints(&store, series, numberOfPoints, xi);

  // Compute and compare the regression calculations metrics
  double mean = store.meanOfColumn(series,0);
  double sum = store.sumOfColumn(series,0);
  double squaredSum = store.squaredValueSumOfColumn(series,0);
  double standardDeviation = store.standardDeviationOfColumn(series,0);
  double variance = store.varianceOfColumn(series,0);

  // Check that squaredSum, standardDeviation and variance are positive
  quiz_assert(squaredSum >= 0.0);
  quiz_assert(standardDeviation >= 0.0);
  quiz_assert(variance >= 0.0);

  double precision = 1e-3;
  // When the expected value is 0, the expected coefficient must be negligible against reference.
  // The least likely value to be null is trueSquaredSum
  double reference = trueSquaredSum;

  quiz_assert(IsApproximatelyEqual(variance, trueVariance, precision, reference));
  quiz_assert(IsApproximatelyEqual(squaredSum, trueSquaredSum, precision, reference));

  // adapt the reference
  reference = std::sqrt(trueSquaredSum);

  quiz_assert(IsApproximatelyEqual(mean, trueMean, precision, reference));
  quiz_assert(IsApproximatelyEqual(sum, trueSum, precision, reference));
  quiz_assert(IsApproximatelyEqual(standardDeviation, trueStandardDeviation, precision, reference));
}

QUIZ_CASE(column_calculation) {
  double x[] = {2.3, 5.6, 1.1, 4.3};
  double mean = 3.325;
  double sum = 13.3;
  double squaredSum = 56.35;
  double standardDeviation = 1.741228;
  double variance = 3.031875;
  assert_column_calculations_is(x, 4, mean, sum, squaredSum, standardDeviation, variance);
}

QUIZ_CASE(constant_column_calculation) {
  // This data produced a negative variance before
  double x[] = {-996.85840734641, -996.85840734641, -996.85840734641};
  double mean = -996.85840734641;
  double sum = -2990.57522203923;
  double squaredSum = 2981180.0528916633;
  double standardDeviation = 0.0;
  double variance = 0.0;
  assert_column_calculations_is(x, 3, mean, sum, squaredSum, standardDeviation, variance);
}

void assert_regression_calculations_is(double * xi, double * yi, int numberOfPoints, double trueCovariance, double trueProductSum, double trueR) {
  int series = 0;
  Regression::Store store;

  setRegressionPoints(&store, series, numberOfPoints, xi, yi);

  double precision = 1e-3;

  // Compute and compare the regression calculations metrics
  double covariance = store.covariance(series);
  double productSum = store.columnProductSum(series);

  // trueProductSum and trueCovariance are using each other as reference
  // By construction, they often have a close value with a numberOfPoints factor
  quiz_assert(IsApproximatelyEqual(covariance, trueCovariance, precision, trueProductSum / numberOfPoints));
  quiz_assert(IsApproximatelyEqual(productSum, trueProductSum, precision, trueCovariance * numberOfPoints));

  // When trueR = 0, a DBL_EPSILON reference ensures that the only accepted errors are due to double approximations
  // sqrt is used because the R is computed from sqrt(V1*V0)
  double reference = 100.0 * std::sqrt(DBL_EPSILON);

  double r = store.correlationCoefficient(series);
  quiz_assert(r >= 0.0);
  quiz_assert(IsApproximatelyEqual(r, trueR, precision, reference));
}

QUIZ_CASE(regression_calculation) {
  double x[] = {1.0, 50.0, 34.0, 67.0, 20.0};
  double y[] = {71.860, 2775514, 979755.1, 6116830.0, 233832.9};
  double covariance = 4.7789036e7;
  double productSum = 586591713.26;
  double r = 0.919088;
  assert_regression_calculations_is(x, y, 5, covariance, productSum, r);
}
