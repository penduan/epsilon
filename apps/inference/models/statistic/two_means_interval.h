#ifndef INFERENCE_MODELS_STATISTIC_TWO_MEANS_INTERVAL_H
#define INFERENCE_MODELS_STATISTIC_TWO_MEANS_INTERVAL_H

#include "interval.h"
#include "two_means_statistic.h"

namespace Inference {

class TwoMeansInterval : public Interval, public TwoMeansStatistic {
 public:
  using TwoMeansStatistic::TwoMeansStatistic;

  SignificanceTestType significanceTestType() const override {
    return SignificanceTestType::TwoMeans;
  }
  I18n::Message title() const override {
    return TwoMeans::Title(twoMeansType(this));
  }
  bool initializeDistribution(DistributionType distributionType) override {
    return TwoMeans::IntervalInitializeDistribution(this, distributionType);
  }
  int numberOfAvailableDistributions() const override {
    return TwoMeans::NumberOfAvailableDistributions();
  }
  I18n::Message distributionTitle() const override {
    return TwoMeans::DistributionTitle();
  }
  void initParameters() override { TwoMeans::InitIntervalParameters(this); }
  bool authorizedParameterAtIndex(double p, int i) const override {
    return Inference::authorizedParameterAtIndex(p, i) &&
           TwoMeans::AuthorizedParameterAtIndex(twoMeansType(this), i, p);
  }
  void setParameterAtIndex(double p, int index) override {
    p = TwoMeans::ProcessParamaterForIndex(p, index);
    Interval::setParameterAtIndex(p, index);
  }
  const char* estimateSymbol() const override {
    return TwoMeans::EstimateSymbol();
  }
  Poincare::Layout estimateLayout() const override {
    return TwoMeans::EstimateLayout(&m_estimateLayout);
  }
  I18n::Message estimateDescription() override {
    return TwoMeans::EstimateDescription();
  }

 private:
  bool validateInputs() override {
    return TwoMeans::ValidateInputs(twoMeansType(this), m_params);
  }
  int numberOfStatisticParameters() const override {
    return TwoMeans::NumberOfParameters();
  }
  Shared::ParameterRepresentation paramRepresentationAtIndex(
      int i) const override {
    return TwoMeans::ParameterRepresentationAtIndex(twoMeansType(this), i);
  }
  double* parametersArray() override { return m_params; }
  void privateCompute() override {
    TwoMeans::ComputeInterval(twoMeansType(this), this);
  }

  double m_params[TwoMeans::k_numberOfParams];
};

class TwoMeansTInterval : public TwoMeansInterval {
 public:
  using TwoMeansInterval::TwoMeansInterval;

  DistributionType distributionType() const override {
    return DistributionType::T;
  }
};

class PooledTwoMeansTInterval : public TwoMeansInterval {
 public:
  using TwoMeansInterval::TwoMeansInterval;

  DistributionType distributionType() const override {
    return DistributionType::TPooled;
  }
};

class TwoMeansZInterval : public TwoMeansInterval {
 public:
  using TwoMeansInterval::TwoMeansInterval;

  DistributionType distributionType() const override {
    return DistributionType::Z;
  }
};

}  // namespace Inference

#endif
