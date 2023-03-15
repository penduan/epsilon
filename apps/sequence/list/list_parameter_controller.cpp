#include "list_parameter_controller.h"

#include <apps/i18n.h>

#include "../../shared/poincare_helpers.h"
#include "../app.h"
#include "list_controller.h"

using namespace Poincare;
using namespace Shared;
using namespace Escher;

namespace Sequence {

ListParameterController::ListParameterController(
    Escher::InputEventHandlerDelegate *inputEventHandlerDelegate,
    ListController *listController)
    : Shared::ListParameterController(listController,
                                      I18n::Message::SequenceColor,
                                      I18n::Message::DeleteSequence, this),
      m_typeCell(I18n::Message::SequenceType),
      m_initialRankCell(&m_selectableListView, inputEventHandlerDelegate, this,
                        I18n::Message::FirstTermIndex),
      m_typeParameterController(this, listController, Metric::CommonTopMargin,
                                Metric::CommonRightMargin,
                                Metric::CommonBottomMargin,
                                Metric::CommonLeftMargin) {}

const char *ListParameterController::title() {
  return I18n::translate(I18n::Message::SequenceOptions);
}

bool ListParameterController::textFieldShouldFinishEditing(
    AbstractTextField *textField, Ion::Events::Event event) {
  return event == Ion::Events::Down || event == Ion::Events::Up ||
         TextFieldDelegate::textFieldShouldFinishEditing(textField, event);
}

bool ListParameterController::textFieldDidFinishEditing(
    AbstractTextField *textField, const char *text, Ion::Events::Event event) {
  double floatBody =
      textFieldDelegateApp()->parseInputtedFloatValue<double>(text);
  if (textFieldDelegateApp()->hasUndefinedValue(floatBody)) {
    return false;
  }
  int index = std::floor(floatBody);
  if (index < 0 || index > Shared::Sequence::k_maxInitialRank) {
    Container::activeApp()->displayWarning(I18n::Message::ForbiddenValue);
    return false;
  }
  sequence()->setInitialRank(index);
  App::app()->snapshot()->updateInterval();
  // Invalidate sequence context cache when changing sequence type
  App::app()->localContext()->resetCache();
  m_selectableListView.reloadCell(selectedRow());
  m_selectableListView.handleEvent(event);
  return true;
}

void ListParameterController::listViewDidChangeSelectionAndDidScroll(
    SelectableListView *l, int previousSelectedRow,
    bool withinTemporarySelection) {
  if (withinTemporarySelection || previousSelectedRow == l->selectedRow()) {
    return;
  }
  if (previousSelectedRow == 1) {
    MessageTableCellWithEditableText *myCell =
        (MessageTableCellWithEditableText *)l->cell(previousSelectedRow);
    if (myCell) {
      myCell->setEditing(false);
    }
    Container::activeApp()->setFirstResponder(&m_selectableListView);
  }
  if (l->selectedRow() == 1) {
    MessageTableCellWithEditableText *myNewCell =
        (MessageTableCellWithEditableText *)l->selectedCell();
    Container::activeApp()->setFirstResponder(myNewCell);
  }
}

HighlightCell *ListParameterController::cell(int index) {
  assert(0 <= index && index < numberOfRows());
  HighlightCell *const cells[] = {&m_typeCell, &m_initialRankCell,
                                  &m_enableCell, &m_colorCell, &m_deleteCell};
  return cells[index];
}

void ListParameterController::willDisplayCellForIndex(HighlightCell *cell,
                                                      int index) {
  Shared::ListParameterController::willDisplayCellForIndex(cell, index);
  if (cell == &m_typeCell && !m_record.isNull()) {
    m_typeCell.setLayout(sequence()->definitionName());
  }
  if (cell == &m_initialRankCell && !m_record.isNull()) {
    MessageTableCellWithEditableText *myCell =
        (MessageTableCellWithEditableText *)cell;
    if (myCell->isEditing()) {
      return;
    }
    char buffer[Shared::Sequence::k_initialRankNumberOfDigits + 1];
    Poincare::Integer(sequence()->initialRank())
        .serialize(buffer, Shared::Sequence::k_initialRankNumberOfDigits + 1);
    myCell->setAccessoryText(buffer);
  }
}

bool ListParameterController::handleEvent(Ion::Events::Event event) {
  HighlightCell *cell = selectedCell();
  if (cell == &m_typeCell && m_typeCell.ShouldEnterOnEvent(event)) {
    m_typeParameterController.setRecord(m_record);
    static_cast<StackViewController *>(parentResponder())
        ->push(&m_typeParameterController);
    return true;
  }
  if (cell == &m_enableCell && m_enableCell.enterOnEvent(event)) {
    App::app()->localContext()->resetCache();
    function()->setActive(!function()->isActive());
    resetMemoization();
    m_selectableListView.reloadData();
    return true;
  }
  return Shared::ListParameterController::handleEvent(event);
}

}  // namespace Sequence
