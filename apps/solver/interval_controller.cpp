#include "interval_controller.h"

#include <apps/i18n.h>
#include <assert.h>
#include <string.h>

#include "app.h"

using namespace Escher;
using namespace Shared;

namespace Solver {

IntervalController::IntervalController(Responder *parentResponder)
    : SingleRangeController(parentResponder, &m_confirmPopUpController),
      m_confirmPopUpController(Invocation::Builder<IntervalController>(
          [](IntervalController *controller, void *sender) {
            controller->stackController()->pop();
            return true;
          },
          this)) {
  m_okButton.setMessage(I18n::Message::ResolveEquation);
}

const char *IntervalController::title() {
  return I18n::translate(I18n::Message::SearchInverval);
}

bool IntervalController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::Left &&
      typeAtRow(selectedRow()) != k_autoCellType) {
    // Bypass the Left == Back of single range controller
    return false;
  }
  return SingleRangeController::handleEvent(event);
}

I18n::Message IntervalController::parameterMessage(int index) const {
  return index == 0 ? I18n::Message::XMin : I18n::Message::XMax;
}

float IntervalController::limit() const {
  return SystemOfEquations::k_maxFloat;
}

void IntervalController::extractParameters() {
  SystemOfEquations *system = App::app()->system();
  m_rangeParam = system->approximateSolvingRange();
  m_autoParam = system->autoApproximateSolvingRange();
}

void IntervalController::confirmParameters() {
  SystemOfEquations *system = App::app()->system();
  system->setApproximateSolvingRange(m_rangeParam);
  system->setAutoApproximateSolvingRange(m_autoParam);
}

bool IntervalController::parametersAreDifferent() {
  SystemOfEquations *system = App::app()->system();
  return m_rangeParam != system->approximateSolvingRange() ||
         m_autoParam != system->autoApproximateSolvingRange();
}

void IntervalController::setAutoRange() {
  m_rangeParam = SystemOfEquations::k_defaultAutoApproximateSolvingRange;
}

void IntervalController::pop(bool onConfirmation) {
  if (onConfirmation) {
    App::app()->system()->approximateSolve(App::app()->localContext());
  }
  StackViewController *stack = stackController();
  stack->pop();
}

}  // namespace Solver
