#include "../app.h"
#include "graph_controller.h"
#include <apps/apps_container_helper.h>
#include <apps/exam_mode_configuration.h>
#include <apps/shared/function_banner_delegate.h>
#include <apps/shared/poincare_helpers.h>
#include <poincare/preferences.h>
#include <poincare/print.h>
#include <poincare/layout_helper.h>
#include <poincare/serialization_helper.h>
#include <poincare/helpers.h>
#include <cmath>
#include <algorithm>

using namespace Poincare;
using namespace Shared;
using namespace Escher;

namespace Regression {

GraphController::GraphController(Responder * parentResponder, Escher::InputEventHandlerDelegate * inputEventHandlerDelegate, ButtonRowController * header, Shared::InteractiveCurveViewRange * interactiveRange, CurveViewCursor * cursor, int * selectedDotIndex, int * selectedCurveIndex, Store * store) :
  InteractiveCurveViewController(parentResponder, inputEventHandlerDelegate, header, interactiveRange, &m_view, cursor, I18n::Message::Regression, selectedCurveIndex),
  m_bannerView(this, inputEventHandlerDelegate, this),
  m_view(interactiveRange, store, m_cursor, &m_bannerView, &m_cursorView),
  m_store(store),
  m_graphOptionsController(this, inputEventHandlerDelegate, interactiveRange, m_store, m_cursor, this),
  m_curveSelectionController(this),
  m_selectedDotIndex(selectedDotIndex),
  m_selectedModelType((Model::Type)-1)
{
  interactiveRange->setDelegate(this);
}

I18n::Message GraphController::emptyMessage() {
  return I18n::Message::NoDataToPlot;
}

void GraphController::viewWillAppear() {
  m_cursorView.resetMemoization();
  /* At this point, some series might have been removed from the model. We need
   * to reinitialize the selected series index if the current selection is
   * either null (right after construction) or refering a removed series. */
  if (*m_selectedCurveIndex < 0 || *m_selectedCurveIndex >= numberOfCurves()) {
    *m_selectedCurveIndex = 0;
    m_selectedModelType = (Model::Type)-1;
  }

  /* Selected model type is memoized. The values differ in three cases:
   *  1) the very first time the graph view appears
   *  2) when the user selects another Model::Type for a series.
   *  3) selected series has been removed
   * where we decide to place the cursor at a default position. */
  Model::Type newSelectedModelType = regressionTypeOfCurve(*m_selectedCurveIndex);
  if (m_selectedModelType != newSelectedModelType) {
    m_selectedModelType = newSelectedModelType;
    initCursorParameters();
  }

  /* The following
   *   - calls initCursorParameters() if necessary,
   *   - reloads the bannerView and the curveView. */
  InteractiveCurveViewController::viewWillAppear();

  /* Since *m_selectedDotIndex is altered by initCursorParameters(),
   * the following must absolutely come at the end. */
  setRoundCrossCursorView();
}

void GraphController::didBecomeFirstResponder() {
  if (*m_selectedDotIndex == -1 && curveView()->hasFocus()) {
    setAbscissaInputAsFirstResponder();
  }
  Shared::InteractiveCurveViewController::didBecomeFirstResponder();
}

void GraphController::setAbscissaInputAsFirstResponder() {
  m_bannerView.abscissaValue()->setParentResponder(this);
  m_bannerView.abscissaValue()->setDelegates(textFieldDelegateApp(), this);
  Container::activeApp()->setFirstResponder(m_bannerView.abscissaValue());
}

Poincare::Context * GraphController::globalContext() const {
  return AppsContainerHelper::sharedAppsContainerGlobalContext();
}

// Private

KDCoordinate GraphController::CurveSelectionController::nonMemoizedRowHeight(int j) {
  if (j < 0 || j >= numberOfRows()) {
    return 0;
  }
  return KDFont::GlyphHeight(KDFont::Size::Large) + Metric::CellTopMargin + Metric::CellBottomMargin;
}

void GraphController::CurveSelectionController::willDisplayCellForIndex(HighlightCell * cell, int index) {
  int series = graphController()->seriesIndexFromCurveIndex(index);
  const char * name = Store::SeriesTitle(series);
  static_cast<CurveSelectionCellWithChevron *>(cell)->setColor(DoublePairStore::colorOfSeriesAtIndex(series));
  static_cast<CurveSelectionCellWithChevron *>(cell)->setLayout(LayoutHelper::String(name));
}

bool GraphController::buildRegressionExpression(char * buffer, size_t bufferSize, Model::Type modelType, int significantDigits, Poincare::Preferences::PrintFloatMode displayMode) const {
  double * coefficients = m_store->coefficientsForSeries(selectedSeriesIndex(), globalContext());
  Layout l = m_store->regressionModel(modelType)->equationLayout(coefficients, GlobalPreferences::sharedGlobalPreferences->yPredictedSymbol(), significantDigits, displayMode);
  int length = l.serializeForParsing(buffer, bufferSize);
  if (length >= static_cast<int>(bufferSize - 1) || length == 0) {
    return false;
  }
  length = SerializationHelper::ReplaceSystemParenthesesAndBracesByUserParentheses(buffer, length);
  return true;
}

// SimpleInteractiveCurveViewController

void GraphController::reloadBannerView() {
  const int selectedSeries = selectedSeriesIndex();
  const int significantDigits = Preferences::sharedPreferences->numberOfSignificantDigits();
  Poincare::Preferences::PrintFloatMode displayMode = Poincare::Preferences::sharedPreferences->displayMode();

  // If any coefficient is NAN, display that data is not suitable
  bool coefficientsAreDefined = m_store->coefficientsAreDefined(selectedSeries, globalContext());
  if (coefficientsAreDefined && *m_selectedDotIndex < 0 && curveIsScatterPlot(*m_selectedCurveIndex)) {
    // Regression model has been removed, reinitialize cursor
    initCursorParameters();
  }
  bool displayMean = (*m_selectedDotIndex == numberOfDotsOfCurve(*m_selectedCurveIndex));
  bool displayEquation = (*m_selectedDotIndex < 0);
  char buffer[k_bannerViewTextBufferSize];
  Model::Type modelType = regressionTypeOfCurve(*m_selectedCurveIndex);
  if (displayEquation && coefficientsAreDefined && buildRegressionExpression(buffer, k_bannerViewTextBufferSize, modelType, significantDigits, displayMode)) {
    // Regression equation fits in the banner, display it
    m_bannerView.setDisplayParameters(true, false, false);
    m_bannerView.otherView()->setText(buffer);
  } else if (displayMean) {
    // Display MeanDot before x= and y=
    m_bannerView.setDisplayParameters(true, true, !coefficientsAreDefined);
    m_bannerView.otherView()->setText(I18n::translate(I18n::Message::MeanDot));
  } else if (modelType == Model::Type::None && !ExamModeConfiguration::statsDiagnosticsAreForbidden()) {
    // Display correlation coefficient
    m_bannerView.setDisplayParameters(true, false, !coefficientsAreDefined);
    Poincare::Print::CustomPrintf(buffer, k_bannerViewTextBufferSize, "r=%*.*ed", m_store->correlationCoefficient(selectedSeries), displayMode, significantDigits);
    m_bannerView.otherView()->setText(buffer);
  } else {
    // Nothing else to display
    m_bannerView.setDisplayParameters(false, false, !coefficientsAreDefined);
    m_bannerView.otherView()->setText("");
  }

  /* Use "x=..." or "xmean=..." (\xCC\x85 represents the combining overline ' ̅')
   * if the mean dot is selected. Same with y. */
  double x = displayMean ? m_store->meanOfColumn(selectedSeries, 0) : m_cursor->x();
  Poincare::Print::CustomPrintf(buffer, Shared::BannerView::k_maxLengthDisplayed - 2, "%*.*ed", x, displayMode, significantDigits); // -2 for "x="
  m_bannerView.abscissaValue()->setText(buffer);
  m_bannerView.abscissaSymbol()->setText(displayMean ? "x\xCC\x85=" : "x=");

  double y = displayMean ? m_store->meanOfColumn(selectedSeries, 1) : m_cursor->y();
  Poincare::Print::CustomPrintf(
    buffer, k_bannerViewTextBufferSize, "%s=%*.*ed",
    (displayMean ? "y\xCC\x85" : (displayEquation ? GlobalPreferences::sharedGlobalPreferences->yPredictedSymbol() : "y")),
    y, displayMode, significantDigits);
  m_bannerView.ordinateView()->setText(buffer);

  m_bannerView.reload();
}

bool GraphController::moveCursorHorizontally(int direction, int scrollSpeed) {
  const int selectedSeries = selectedSeriesIndex();
  double x;
  double y;
  if (*m_selectedDotIndex >= 0) {
    int dotSelected = m_store->nextDot(selectedSeries, direction, *m_selectedDotIndex, !curveIsScatterPlot(*m_selectedCurveIndex));
    if (dotSelected >= 0) {
      x = dotAbscissa(*m_selectedCurveIndex, dotSelected);
      y = dotOrdinate(*m_selectedCurveIndex, dotSelected);
    } else {
      return false;
    }
    *m_selectedDotIndex = dotSelected;
  } else {
    double step = direction * scrollSpeed * static_cast<double>(interactiveCurveViewRange()->xGridUnit())/static_cast<double>(k_numberOfCursorStepsInGradUnit);
    x = m_cursor->x() + step;
    y = m_store->yValueForXValue(selectedSeries, x, globalContext());
  }
  m_cursor->moveTo(x, x, y);
  return true;
}

InteractiveCurveViewRange * GraphController::interactiveCurveViewRange() const {
  return App::app()->graphRange();
}

void GraphController::openMenuForCurveAtIndex(int curveIndex) {
  if (*m_selectedCurveIndex != curveIndex) {
    *m_selectedCurveIndex = curveIndex;
    Coordinate2D<double> xy = xyValues(curveIndex, m_cursor->t(), textFieldDelegateApp()->localContext());
    m_cursor->moveTo(m_cursor->t(), xy.x1(), xy.x2());
  }
  if (curveIsScatterPlot(*m_selectedCurveIndex)) {
    // Push regression controller directly
    RegressionController * controller = App::app()->regressionController();
    controller->setSeries(selectedSeriesIndex());
    controller->setDisplayedFromDataTab(false);
    stackController()->push(controller);
  } else {
    // Reset selected row. It is preserved when navigating in its submenus
    m_graphOptionsController.selectRow(0);
    stackController()->push(&m_graphOptionsController);
  }
}

// InteractiveCurveViewController
void GraphController::initCursorParameters(bool ignoreMargins) {
  if (curveIsScatterPlot(*m_selectedCurveIndex)) {
    *m_selectedDotIndex = 0;
  } else {
    *m_selectedDotIndex = numberOfDotsOfCurve(*m_selectedCurveIndex);
  }
  double x = dotAbscissa(*m_selectedCurveIndex, *m_selectedDotIndex);
  double y = dotOrdinate(*m_selectedCurveIndex, *m_selectedDotIndex);
  m_cursor->moveTo(x, x, y);
}

bool GraphController::selectedModelIsValid() const {
  uint8_t numberOfDots = numberOfDotsOfCurve(*m_selectedCurveIndex);
  return *m_selectedDotIndex < numberOfDots || (*m_selectedDotIndex == numberOfDots && !curveIsScatterPlot(*m_selectedCurveIndex));
}

Poincare::Coordinate2D<double> GraphController::selectedModelXyValues(double t) const {
  assert(selectedModelIsValid());
  if (*m_selectedDotIndex == -1) {
    return xyValues(*m_selectedCurveIndex, t, globalContext());
  }
  return Coordinate2D<double>(dotAbscissa(*m_selectedCurveIndex, *m_selectedDotIndex), dotOrdinate(*m_selectedCurveIndex, *m_selectedDotIndex));
}

bool GraphController::moveCursorVertically(int direction) {
  int selectedSeries = selectedSeriesIndex();
  Poincare::Context * context = globalContext();
  double x = m_cursor->x();
  double y = m_cursor->y();

  // Find the closest regression
  int selectedRegressionCurve = *m_selectedDotIndex == -1 ? *m_selectedCurveIndex : -1;
  int closestRegressionCurve = closestCurveIndexVertically(direction > 0, selectedRegressionCurve, context);

  // Find the closest dot
  int closesDotCurve = -1;
  int dotSelected = closestVerticalDot(direction, x, y, *m_selectedCurveIndex, *m_selectedDotIndex, &closesDotCurve, context);

  // Choose between selecting the regression or the dot
  bool validRegression = closestRegressionCurve > -1;
  bool validDot = dotSelected >= 0 && dotSelected <= numberOfDotsOfCurve(closesDotCurve);
  if (validRegression && validDot) {
    /* Compare the abscissa distances to select either the dot or the
     * regression. If they are equal, compare the ordinate distances. */
    double dotDistanceX = std::fabs(dotAbscissa(closesDotCurve, dotSelected) - x);
    if (dotDistanceX != 0) {
      // The regression X distance to the point is 0, so it is closer than the dot.
      validDot = false;
    } else {
      // Compare the y distances
      double regressionDistanceY = std::fabs(yValue(closestRegressionCurve, x, context) - y);
      double dotDistanceY = std::fabs(dotOrdinate(closesDotCurve, dotSelected) - y);
      if (regressionDistanceY <= dotDistanceY) {
        validDot = false;
      } else {
        validRegression = false;
      }
    }
  }

  assert(!validDot || !validRegression);

  /* The model should be up to date before setting the cursor view. */

  if (validRegression) {
    // Select the regression
    if (*m_selectedCurveIndex != closestRegressionCurve) {
      *m_selectedCurveIndex = closestRegressionCurve;
      selectedSeries = selectedSeriesIndex();
      // Reload so that the selected series is on top
      m_view.reload(false, true);
    }
    *m_selectedDotIndex = -1;
    setRoundCrossCursorView();
    m_cursor->moveTo(x, x, m_store->yValueForXValue(selectedSeries, x, context));
    setAbscissaInputAsFirstResponder();
    return true;
  }

  if (validDot) {
    // Select the dot
    if (*m_selectedCurveIndex != closesDotCurve) {
      *m_selectedCurveIndex = closesDotCurve;
      // Reload so that the selected curve is on top
      m_view.reload(false, true);
    }
    *m_selectedDotIndex = dotSelected;
    setRoundCrossCursorView();
    double x = dotAbscissa(*m_selectedCurveIndex, dotSelected);
    double y = dotOrdinate(*m_selectedCurveIndex, dotSelected);
    m_cursor->moveTo(x, x, y);
    // abscissa input must resolve first responder
    Container::activeApp()->setFirstResponder(this);
    return true;
  }

  // There was no suitable selection
  return false;
}

Coordinate2D<double> GraphController::xyValues(int curveIndex, double t, Poincare::Context * context, int subCurveIndex) const {
  return Coordinate2D<double>(t, yValue(curveIndex, t, context));
}

bool GraphController::suitableYValue(double y) const {
  return interactiveCurveViewRange()->yMin() <= y && y <= interactiveCurveViewRange()->yMax();
}

void GraphController::setRoundCrossCursorView() {
  /* At this point, the model (selected series and dot indices) should be up
   * to date. */
  bool round = *m_selectedDotIndex < 0;
  m_cursorView.setIsRing(!round);
  assert(selectedSeriesIndex() < static_cast<int>(Palette::numberOfDataColors()));
  m_cursorView.setColor(Palette::DataColor[selectedSeriesIndex()]);
}

Range2D GraphController::optimalRange(bool computeX, bool computeY, Range2D originalRange) const {
  Range1D xRange, yRange;
  const int nbOfCurves = numberOfCurves();
  for (int curve = 0; curve < nbOfCurves; curve++) {
    int numberOfDots = numberOfDotsOfCurve(curve);
    for (int dot = 0; dot < numberOfDots; dot++) {
      float x = dotAbscissa(curve, dot);
      xRange.extend(x, k_maxFloat);
      if (computeX || (originalRange.xMin() <= x && x <= originalRange.xMax())) {
        float y = dotOrdinate(curve, dot);
        yRange.extend(y, k_maxFloat);
      }
    }
  }
  Range2D result(computeX ? xRange : *originalRange.x(), computeY ? yRange : *originalRange.y());
  return Zoom::Sanitize(result, InteractiveCurveViewRange::NormalYXRatio(), k_maxFloat);
}

int GraphController::closestVerticalDot(int direction, double x, double y, int currentCurve, int currentDot, int * nextCurve, Poincare::Context * globalContext) {
  double nextX = INFINITY;
  double nextY = INFINITY;
  int nextDot = -1;
  const int nbOfCurves = numberOfCurves();
  for (int curve = 0; curve < nbOfCurves; curve++) {
    if (currentDot >= 0 && currentCurve == curve) {
      /* If the currentDot is valid, the next curve
       * should not be the current curve */
      continue;
    }
    int numberOfDots = numberOfDotsOfCurve(curve);
    float xMin = App::app()->graphRange()->xMin();
    float xMax = App::app()->graphRange()->xMax();
    bool displayMean = !curveIsScatterPlot(curve);
    for (int i = 0; i < numberOfDots + displayMean; i++) {
      double currentX = dotAbscissa(curve, i);
      double currentY = dotOrdinate(curve, i);
      if (xMin <= currentX && currentX <= xMax // The next dot is within the window abscissa bounds
          && (std::fabs(currentX - x) <= std::fabs(nextX - x)) // The next dot is the closest to x in abscissa
          && ((currentY > y && direction > 0) // The next dot is above/under y
            || (currentY < y && direction < 0)
            || (currentY == y
              && ((currentDot < 0 && direction > 0)|| ((direction < 0) == (curve > currentCurve)))))
          && (nextX != currentX // Edge case: if 2 dots have the same abscissa but different ordinates
            || ((currentY <= nextY) == (direction > 0))))
      {
        nextX = currentX;
        nextY = currentY;
        nextDot = i;
        *nextCurve = curve;
      }
    }
  }
  return nextDot;
}

double GraphController::dotCoordinate(int curveIndex, int dotIndex, int coordinate) const {
  assert(dotIndex != numberOfDotsOfCurve(curveIndex) || !curveIsScatterPlot(curveIndex));
  assert(0 <= dotIndex && dotIndex <= numberOfDotsOfCurve(curveIndex));
  const int seriesIndex = seriesIndexFromCurveIndex(curveIndex);
  return dotIndex < m_store->numberOfPairsOfSeries(seriesIndex) ? m_store->get(seriesIndex, coordinate, dotIndex) : m_store->meanOfColumn(seriesIndex, coordinate);
}

}
