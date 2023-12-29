#include "grid_type_controller.h"

#include <apps/i18n.h>

#include "images/cartesian_grid_icon.h"
#include "images/polar_grid_icon.h"
#include "interactive_curve_view_range.h"

using namespace Escher;

namespace Shared {

GridTypeController::GridTypeController(
    Escher::Responder *parentResponder,
    InteractiveCurveViewRange *interactiveCurveViewRange)
    : UniformSelectableListController<
          MenuCell<MessageTextView, EmptyCellWidget, TransparentImageView>, 2>(
          parentResponder, this),
      m_viewRange(interactiveCurveViewRange) {
  cell(0)->label()->setMessage(I18n::Message::CartesianGrid);
  cell(1)->label()->setMessage(I18n::Message::PolarGrid);

  cell(0)->accessory()->setImage(ImageStore::CartesianGridIcon);
  cell(1)->accessory()->setImage(ImageStore::PolarGridIcon);
}

const char *GridTypeController::title() {
  return I18n::translate(I18n::Message::GridType);
}

void GridTypeController::viewWillAppear() {
  selectRow(m_viewRange->gridType() == GridType::Cartesian ? 0 : 1);
}

bool GridTypeController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::OK || event == Ion::Events::EXE) {
    /* Update the view range before ViewWillAppear is
     * called. */
    m_viewRange->setGridType(selectedRow() == 0 ? GridType::Cartesian
                                                : GridType::Polar);
    stackController()->pop();
    return true;
  }
  if (event == Ion::Events::Back || event == Ion::Events::Left) {
    stackController()->pop();
    return true;
  }
  return false;
}

}  // namespace Shared
