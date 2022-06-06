#ifndef POINCARE_MIXED_FRACTION_H
#define POINCARE_MIXED_FRACTION_H

#include <poincare/expression.h>

namespace Poincare {

class MixedFractionNode final : public ExpressionNode  {
public:

  // TreeNode
  size_t size() const override { return sizeof(MixedFractionNode); }
  int numberOfChildren() const override { return 2; }
#if POINCARE_TREE_LOG
  void logNodeName(std::ostream & stream) const override {
    stream << "MixedFraction";
  }
#endif

  // Properties
  Type type() const override { return Type::MixedFraction; }
  Sign sign(Context * context) const override { return childAtIndex(0)->sign(context); }
private:
  // Simplification
  Expression shallowReduce(ReductionContext reductionContext) override;
  LayoutShape leftLayoutShape() const override { return LayoutShape::Integer; };
  LayoutShape rightLayoutShape() const override { return LayoutShape::Fraction; }
  // Layout
  Layout createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override;
  int serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override;
  // Evaluation
  Evaluation<float> approximate(SinglePrecision p, ApproximationContext approximationContext) const override {
    return templateApproximate<float>(approximationContext);
  }
  Evaluation<double> approximate(DoublePrecision p, ApproximationContext approximationContext) const override {
    return templateApproximate<double>(approximationContext);
  }
  template <typename T> Evaluation<T> templateApproximate(ApproximationContext approximationContext) const;
};

class MixedFraction final : public Expression {
public:
  MixedFraction(const MixedFractionNode * n) : Expression(n) {}
  static MixedFraction Builder(Expression integerPart, Expression fractionPart) { return TreeHandle::FixedArityBuilder<MixedFraction, MixedFractionNode>({integerPart, fractionPart}); }
  Expression shallowReduce(ExpressionNode::ReductionContext context);

};

}

#endif

