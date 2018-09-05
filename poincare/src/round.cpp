#include <poincare/round.h>
#include <poincare/undefined.h>
#include <poincare/rational.h>
#include <poincare/power.h>
#include <assert.h>
#include <cmath>

namespace Poincare {

LayoutReference RoundNode::createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  return LayoutHelper::Prefix(Round(this), floatDisplayMode, numberOfSignificantDigits, name());
}

Expression RoundNode::shallowReduce(Context & context, Preferences::AngleUnit angleUnit) {
  return Round(this).shallowReduce(context, angleUnit);
}

template<typename T>
Evaluation<T> RoundNode::templatedApproximate(Context& context, Preferences::AngleUnit angleUnit) const {
  Evaluation<T> f1Input = childAtIndex(0)->approximate(T(), context, angleUnit);
  Evaluation<T> f2Input = childAtIndex(1)->approximate(T(), context, angleUnit);
  T f1 = f1Input.toScalar();
  T f2 = f2Input.toScalar();
  if (std::isnan(f2) || f2 != std::round(f2)) {
    return Complex<T>::Undefined();
  }
  T err = std::pow(10, std::floor(f2));
  return Complex<T>(std::round(f1*err)/err);
}

Expression Round::shallowReduce(Context & context, Preferences::AngleUnit angleUnit) {
  {
    Expression e = Expression::defaultShallowReduce(context, angleUnit);
    if (e.isUndefined()) {
      return e;
    }
  }
#if MATRIX_EXACT_REDUCING
  if (childAtIndex(0).type() == ExpressionNode::Type::Matrix || childAtIndex(1).type() == ExpressionNode::Type::Matrix) {
    return Undefined();
  }
#endif
  /* We reduce only round(Rational, Rational). We do not reduce
   * round(Float, Float) which is equivalent to what is done in approximate. */
  if (childAtIndex(0).type() == ExpressionNode::Type::Rational && childAtIndex(1).type() == ExpressionNode::Type::Rational) {
    Rational r1 = childAtIndex(0).convert<Rational>();
    Rational r2 = childAtIndex(1).convert<Rational>();
    if (!r2.integerDenominator().isOne()) {
      Expression result = Undefined();
      replaceWithInPlace(result);
      return result;
    }
    const Rational ten(10);
    if (Power::RationalExponentShouldNotBeReduced(ten, r2)) {
      return *this;
    }
    Rational err = Rational::IntegerPower(ten, r2.signedIntegerNumerator());
    Rational mult = Rational::Multiplication(r1, err);
    IntegerDivision d = Integer::Division(mult.signedIntegerNumerator(), mult.integerDenominator());
    Integer rounding = d.quotient;
    if (Rational::NaturalOrder(Rational(d.remainder, mult.integerDenominator()), Rational(1,2)) >= 0) {
      rounding = Integer::Addition(rounding, Integer(1));
    }
    Rational result = Rational::Multiplication(rounding, Rational::IntegerPower(Rational(1,10), r2.signedIntegerNumerator()));
    if (result.numeratorOrDenominatorIsInfinity()) {
      return *this;
    }
    replaceWithInPlace(result);
    return result;
  }
  return *this;
}

}


