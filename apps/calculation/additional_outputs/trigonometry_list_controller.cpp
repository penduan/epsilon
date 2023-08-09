#include "trigonometry_list_controller.h"

#include <apps/shared/expression_display_permissions.h>
#include <apps/shared/poincare_helpers.h>
#include <poincare/layout_helper.h>
#include <poincare/trigonometry.h>
#include <poincare_expressions.h>

#include "../app.h"

using namespace Poincare;
using namespace Shared;

namespace Calculation {

Expression TrigonometryListController::ExtractExactAngleFromDirectTrigo(
    Expression input, Expression exactOutput) {
  /* Trigonometry additional results are displayed if either input or output is
   * a direct function. Indeed, we want to capture both cases:
   * - > input: cos(60)
   *   > output: 1/2
   * - > input: 2cos(2) - cos(2)
   *   > output: cos(2)
   * However if the result is complex, it is treated as a complex result. */
  Preferences* preferences = Preferences::sharedPreferences;
  assert(!exactOutput.isScalarComplex(preferences));
  Context* context = App::app()->localContext();
  Expression directTrigoFunction;
  if (Trigonometry::isDirectTrigonometryFunction(exactOutput)) {
    directTrigoFunction = exactOutput;
  } else if (Trigonometry::isDirectTrigonometryFunction(input) &&
             !input.deepIsSymbolic(
                 context, SymbolicComputation::DoNotReplaceAnySymbol)) {
    /* Do not display trigonometric additional informations, in case the symbol
     * value is later modified/deleted in the storage and can't be retrieved.
     * Ex: 0->x; tan(x); 3->x; => The additional results of tan(x) become
     * inconsistent. And if x is deleted, it crashes. */
    directTrigoFunction = input;
  } else {
    return Expression();
  }
  assert(!directTrigoFunction.isUninitialized() &&
         !directTrigoFunction.isUndefined());
  Expression exactAngle = directTrigoFunction.childAtIndex(0);
  assert(!exactAngle.isUninitialized() && !exactAngle.isUndefined());
  Expression unit;
  PoincareHelpers::CloneAndReduceAndRemoveUnit(&exactAngle, context,
                                               ReductionTarget::User, &unit);
  if (!unit.isUninitialized()) {
    if (!unit.isPureAngleUnit()) {
      return Expression();
    }
    assert(static_cast<Unit&>(unit).representative() ==
           Unit::k_angleRepresentatives + Unit::k_radianRepresentativeIndex);
    /* After a reduction, all angle units are converted to radians, so we
     * convert exactAngle again here to fit the angle unit that will be used
     * in reductions below. */
    exactAngle = Multiplication::Builder(
        exactAngle,
        Trigonometry::UnitConversionFactor(Preferences::AngleUnit::Radian,
                                           preferences->angleUnit()));
  }
  // The angle must be real.
  if (!std::isfinite(
          PoincareHelpers::ApproximateToScalar<double>(exactAngle, context))) {
    return Expression();
  }
  assert(!exactAngle.isUninitialized() && !exactAngle.isUndefined());
  return exactAngle;
}

void TrigonometryListController::computeAdditionalResults(
    Expression input, Expression exactOutput, Expression approximateOutput) {
  assert((m_directTrigonometry &&
          Calculation::HasDirectTrigoAdditionalResults(input, exactOutput)) ||
         (!m_directTrigonometry &&
          Calculation::HasInverseTrigoAdditionalResults(input, exactOutput)));

  Preferences* preferences = Preferences::sharedPreferences;
  Preferences::AngleUnit userAngleUnit = preferences->angleUnit();
  Context* context = App::app()->localContext();
  size_t index = 0;

  Expression period = Multiplication::Builder(
      Rational::Builder(2),
      Trigonometry::PiExpressionInAngleUnit(userAngleUnit));

  // Find the angle
  Expression exactAngle, approximateAngle;
  if (m_directTrigonometry) {
    exactAngle = ExtractExactAngleFromDirectTrigo(input, exactOutput);
    approximateAngle = Expression();
  } else {
    exactAngle = exactOutput;
    approximateAngle = approximateOutput;
    assert(!approximateAngle.isUninitialized() &&
           !approximateAngle.isUndefined());
    if (approximateAngle.isPositive(context) == TrinaryBoolean::False) {
      // If the approximate angle is in [-π, π], set it in [0, 2π]
      approximateAngle = Addition::Builder(period.clone(), approximateAngle);
    }
  }
  assert(!exactAngle.isUninitialized() && !exactAngle.isUndefined());

  /* Set exact angle in [0, 2π].
   * Use the reduction of frac part to compute modulo. */
  Expression simplifiedAngle;
  simplifiedAngle = Multiplication::Builder(
      FracPart::Builder(Division::Builder(exactAngle, period.clone())),
      period.clone());
  PoincareHelpers::CloneAndSimplify(&simplifiedAngle, context,
                                    ReductionTarget::User);

  /* Approximate the angle if:
   * - The reduction failed
   * - The fractional part could not be reduced (because the angle is not a
   * multiple of pi)
   * - Displaying the exact expression is forbidden. */
  if (simplifiedAngle.isUninitialized() ||
      simplifiedAngle.recursivelyMatches(
          [](const Expression e, Context* context) {
            return e.type() == ExpressionNode::Type::FracPart;
          }) ||
      ExpressionDisplayPermissions::ShouldOnlyDisplayApproximation(
          exactAngle, simplifiedAngle, approximateAngle, context)) {
    if (m_directTrigonometry) {
      assert(approximateAngle.isUninitialized());
      /* Do not approximate the FracPart, which could lead to truncation error
       * for large angles (e.g. frac(1e17/2pi) = 0). Instead find the angle with
       * the same sine and cosine. */
      approximateAngle =
          ArcCosine::Builder(Cosine::Builder(exactAngle.clone()));
      /* acos has its values in [0,π[, use the sign of the sine to find the
       * right semicircle. */
      if (PoincareHelpers::ApproximateToScalar<double>(
              Sine::Builder(exactAngle), context, preferences) < 0) {
        approximateAngle =
            Subtraction::Builder(period.clone(), approximateAngle);
      }
    }
    assert(!approximateAngle.isUninitialized());
    approximateAngle = PoincareHelpers::Approximate<double>(
        approximateAngle, context, preferences);
    exactAngle = approximateAngle;
    m_isStrictlyEqual[index] = false;
  } else {
    exactAngle = simplifiedAngle;
    m_isStrictlyEqual[index] = true;
  }

  m_layouts[index] = LayoutHelper::String("θ");

  Expression exactAngleWithUnit = Multiplication::Builder(
      exactAngle.clone(),
      Unit::Builder(
          UnitNode::AngleRepresentative::DefaultRepresentativeForAngleUnit(
              userAngleUnit)));

  Expression radians = Unit::Builder(Unit::k_angleRepresentatives +
                                     Unit::k_radianRepresentativeIndex);
  m_exactLayouts[index] = getLayoutFromExpression(
      UnitConvert::Builder(exactAngleWithUnit.clone(), radians), context,
      preferences);

  Expression degrees = Unit::Builder(Unit::k_angleRepresentatives +
                                     Unit::k_degreeRepresentativeIndex);
  m_approximatedLayouts[index] = getLayoutFromExpression(
      UnitConvert::Builder(exactAngleWithUnit.clone(), degrees), context,
      preferences);

  Expression theta = Symbol::Builder(k_symbol);
  setLineAtIndex(++index, Cosine::Builder(theta),
                 Cosine::Builder(exactAngle.clone()), context, preferences);
  updateIsStrictlyEqualAtIndex(index, context);
  setLineAtIndex(++index, Sine::Builder(theta),
                 Sine::Builder(exactAngle.clone()), context, preferences);
  updateIsStrictlyEqualAtIndex(index, context);
  setLineAtIndex(++index, Tangent::Builder(theta),
                 Tangent::Builder(exactAngle.clone()), context, preferences);
  updateIsStrictlyEqualAtIndex(index, context);

  // Set illustration
  /* m_model ask for a float angle but we compute the angle in double and then
   * cast it to float because approximation in float can overflow during the
   * computation. The angle should be between 0 and 2*pi so the approximation in
   * double is castable in float. */
  float angle = static_cast<float>(PoincareHelpers::ApproximateToScalar<double>(
      approximateAngle.isUninitialized() ? exactAngle : approximateAngle,
      context));
  // Convert angle to radians
  if (userAngleUnit != Preferences::AngleUnit::Radian) {
    angle = angle * M_PI / Trigonometry::PiInAngleUnit(userAngleUnit);
  }
  assert(std::isfinite(angle));
  assert(0 <= angle && angle < 2 * M_PI + Float<float>::EpsilonLax());
  m_model.setAngle(angle);
  setShowIllustration(true);
}

void TrigonometryListController::updateIsStrictlyEqualAtIndex(
    int index, Context* context) {
  if (m_approximatedLayouts[index].isUninitialized()) {
    /* Only one layout is displayed, so there is no equal sign. */
    return;
  }
  char approximateBuffer[::Constant::MaxSerializedExpressionSize];
  m_approximatedLayouts[index].serializeForParsing(
      approximateBuffer, ::Constant::MaxSerializedExpressionSize);
  if (strcmp(approximateBuffer, Undefined::Name()) == 0) {
    // Hide exact result if approximation is undef (e.g tan(1.5707963267949))
    m_exactLayouts[index] = Layout();
    return;
  }
  char exactBuffer[::Constant::MaxSerializedExpressionSize];
  m_exactLayouts[index].serializeForParsing(
      exactBuffer, ::Constant::MaxSerializedExpressionSize);
  assert(strcmp(exactBuffer, approximateBuffer) != 0);
  m_isStrictlyEqual[index] = Expression::ExactAndApproximateExpressionsAreEqual(
      Expression::Parse(exactBuffer, context),
      Expression::Parse(approximateBuffer, context));
}

void TrigonometryListController::fillCellForRow(Escher::HighlightCell* cell,
                                                int row) {
  if (typeAtRow(row) == k_expressionCellType) {
    int expressionIndex = row - showIllustration();
    assert(0 <= expressionIndex && expressionIndex < k_numberOfExpressionRows);
    static_cast<AdditionnalResultCell*>(cell)
        ->label()
        ->setExactAndApproximateAreStriclyEqual(
            m_isStrictlyEqual[expressionIndex]);
  }
  return IllustratedExpressionsListController::fillCellForRow(cell, row);
}

KDCoordinate TrigonometryListController::nonMemoizedRowHeight(int row) {
  if (typeAtRow(row) == k_illustrationCellType) {
    return k_illustrationHeight;
  }
  return IllustratedExpressionsListController::nonMemoizedRowHeight(row);
}

I18n::Message TrigonometryListController::messageAtIndex(int index) {
  if (index == 0) {
    return I18n::Message::AngleInZeroTwoPi;
  }
  return I18n::Message::Default;
}

}  // namespace Calculation
