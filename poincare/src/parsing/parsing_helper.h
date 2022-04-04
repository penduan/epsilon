#ifndef POINCARE_PARSING_HELPER_H
#define POINCARE_PARSING_HELPER_H

#include <poincare_expressions.h>

namespace Poincare {

class ParsingHelper {

public:

  /* The method GetReservedFunction passes through the successive
   * entries of the reserved functions array in order to determine
   * whether a token corresponds to an entry. The static constexpr
   * s_reservedFunctionsUpperBound marks the end of the array. */
  static const Expression::FunctionHelper * const * GetReservedFunction(const char * name, size_t nameLength);
  static const Expression::FunctionHelper * const * ReservedFunctionsUpperBound() { return s_reservedFunctionsUpperBound; }

  static bool IsSpecialIdentifierName(const char * name, size_t nameLength);


private:
  constexpr static const char * s_specialIdentifierNames[] = {
    Symbol::k_ans,
    Symbol::k_ansLowerCase,
    Infinity::Name(),
    Undefined::Name(),
    Nonreal::Name(),
    "u",
    "v",
    "w"
  };
  constexpr static int k_numberOfSpecialIdentifiers = sizeof(s_specialIdentifierNames) / sizeof(const char *);

   // The array of reserved functions' helpers
  static constexpr const Expression::FunctionHelper * s_reservedFunctions[] = {
    // Ordered according to name and numberOfChildren
    &AbsoluteValue::s_functionHelper,
    &ArcCosine::s_functionHelper,
    &HyperbolicArcCosine::s_functionHelper,
    &ArcCotangent::s_functionHelper,
    &ArcCosecant::s_functionHelper,
    &ComplexArgument::s_functionHelper,
    &ArcSecant::s_functionHelper,
    &ArcSine::s_functionHelper,
    &HyperbolicArcSine::s_functionHelper,
    &ArcTangent::s_functionHelper,
    &HyperbolicArcTangent::s_functionHelper,
    &BinomCDF::s_functionHelper,
    &BinomialCoefficient::s_functionHelper,
    &BinomPDF::s_functionHelper,
    &Ceiling::s_functionHelper,
    &Conjugate::s_functionHelper,
    &Cosine::s_functionHelper,
    &HyperbolicCosine::s_functionHelper,
    &Cotangent::s_functionHelper,
    &VectorCross::s_functionHelper,
    &Cosecant::s_functionHelper,
    &Dependency::s_functionHelper,
    &Determinant::s_functionHelper,
    &Derivative::s_functionHelper,
    &MatrixDimension::s_functionHelper,
    &VectorDot::s_functionHelper,
    &Factor::s_functionHelper,
    &Floor::s_functionHelper,
    &FracPart::s_functionHelper,
    &GreatCommonDivisor::s_functionHelper,
    &MatrixIdentity::s_functionHelper,
    &ImaginaryPart::s_functionHelper,
    &Integral::s_functionHelper,
    &InvBinom::s_functionHelper,
    &MatrixInverse::s_functionHelper,
    &InvNorm::s_functionHelper,
    &LeastCommonMultiple::s_functionHelper,
    &NaperianLogarithm::s_functionHelper,
    &CommonLogarithm::s_functionHelper,
    &Logarithm::s_functionHelper,
    &VectorNorm::s_functionHelper,
    &NormCDF::s_functionHelper,
    &NormCDF2::s_functionHelper,
    &NormPDF::s_functionHelper,
    &PermuteCoefficient::s_functionHelper,
    &Product::s_functionHelper,
    &DivisionQuotient::s_functionHelper,
    &Randint::s_functionHelper,
    &Random::s_functionHelper,
    &RealPart::s_functionHelper,
    &MatrixRowEchelonForm::s_functionHelper,
    &DivisionRemainder::s_functionHelper,
    &NthRoot::s_functionHelper,
    &Round::s_functionHelper,
    &MatrixReducedRowEchelonForm::s_functionHelper,
    &Secant::s_functionHelper,
    &SignFunction::s_functionHelper,
    &Sine::s_functionHelper,
    &HyperbolicSine::s_functionHelper,
    &Sum::s_functionHelper,
    &Tangent::s_functionHelper,
    &HyperbolicTangent::s_functionHelper,
    &MatrixTrace::s_functionHelper,
    &MatrixTranspose::s_functionHelper,
    &SquareRoot::s_functionHelper
  };

constexpr static const Expression::FunctionHelper * const * s_reservedFunctionsUpperBound = s_reservedFunctions + (sizeof(s_reservedFunctions)/sizeof(Expression::FunctionHelper *));

};

}

#endif
