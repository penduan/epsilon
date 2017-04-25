#include "float_parameter_controller.h"
#include "../constant.h"
#include "../apps_container.h"
#include "text_field_delegate_app.h"
#include <assert.h>

using namespace Poincare;

namespace Shared {

FloatParameterController::FloatParameterController(Responder * parentResponder) :
  DynamicViewController(parentResponder)
{
}

void FloatParameterController::didBecomeFirstResponder() {
  if (selectableTableView()->selectedRow() >= 0) {
    int selectedRow = selectableTableView()->selectedRow();
    selectedRow = selectedRow >= numberOfRows() ? numberOfRows()-1 : selectedRow;
    int selectedColumn = selectableTableView()->selectedColumn();
    selectedColumn = selectedColumn >= numberOfColumns() ? numberOfColumns() - 1 : selectedColumn;
    selectableTableView()->selectCellAtLocation(selectedColumn, selectedRow);
  }
  app()->setFirstResponder(selectableTableView());
}

void FloatParameterController::viewWillAppear() {
  selectableTableView()->reloadData();
  if (selectableTableView()->selectedRow() == -1) {
    selectableTableView()->selectCellAtLocation(0, 0);
  } else {
    int selectedRow = selectableTableView()->selectedRow();
    selectedRow = selectedRow >= numberOfRows() ? numberOfRows()-1 : selectedRow;
    int selectedColumn = selectableTableView()->selectedColumn();
    selectedColumn = selectedColumn >= numberOfColumns() ? numberOfColumns() - 1 : selectedColumn;
    selectableTableView()->selectCellAtLocation(selectedColumn, selectedRow);
  }
}

void FloatParameterController::willExitResponderChain(Responder * nextFirstResponder) {
  if (parentResponder() == nullptr) {
    selectableTableView()->deselectTable();
    selectableTableView()->scrollToCell(0,0);
  }
}

bool FloatParameterController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::Back) {
    stackController()->pop();
    return true;
  }
  return false;
}

int FloatParameterController::typeAtLocation(int i, int j) {
  if (j == numberOfRows()-1) {
    return 0;
  }
  return 1;
}

int FloatParameterController::reusableCellCount(int type) {
  if (type == 0) {
    return 1;
  }
  return reusableParameterCellCount(type);
}

HighlightCell * FloatParameterController::reusableCell(int index, int type) {
  if (type == 0) {
    return m_okButton;
  }
  return reusableParameterCell(index, type);
}

KDCoordinate FloatParameterController::rowHeight(int j) {
  if (j == numberOfRows()-1) {
    return Metric::ParameterCellHeight+k_buttonMargin;
  }
  return Metric::ParameterCellHeight;
}

KDCoordinate FloatParameterController::cumulatedHeightFromIndex(int j) {
  if (j == numberOfRows()) {
    return j*Metric::ParameterCellHeight+k_buttonMargin;
  }
  return Metric::ParameterCellHeight*j;
}

int FloatParameterController::indexFromCumulatedHeight(KDCoordinate offsetY) {
  if (offsetY > numberOfRows()*Metric::ParameterCellHeight + k_buttonMargin) {
    return numberOfRows();
  }
  return (offsetY - 1) / Metric::ParameterCellHeight;
}

void FloatParameterController::willDisplayCellForIndex(HighlightCell * cell, int index) {
  if (index == numberOfRows()-1) {
    return;
  }
  MessageTableCellWithEditableText * myCell = (MessageTableCellWithEditableText *) cell;
  if (myCell->isEditing()) {
    return;
  }
  char buffer[Complex::bufferSizeForFloatsWithPrecision(Constant::LargeNumberOfSignificantDigits)];
  Complex::convertFloatToText(parameterAtIndex(index), buffer, Complex::bufferSizeForFloatsWithPrecision(Constant::LargeNumberOfSignificantDigits), Constant::LargeNumberOfSignificantDigits, Expression::FloatDisplayMode::Decimal);
  myCell->setAccessoryText(buffer);
}

bool FloatParameterController::textFieldShouldFinishEditing(TextField * textField, Ion::Events::Event event) {
  return (event == Ion::Events::Down && selectableTableView()->selectedRow() < numberOfRows()-1)
     || (event == Ion::Events::Up && selectableTableView()->selectedRow() > 0)
     || TextFieldDelegate::textFieldShouldFinishEditing(textField, event);
}

bool FloatParameterController::textFieldDidFinishEditing(TextField * textField, const char * text, Ion::Events::Event event) {
  AppsContainer * appsContainer = ((TextFieldDelegateApp *)app())->container();
  Context * globalContext = appsContainer->globalContext();
  float floatBody = Expression::approximate(text, *globalContext);
  if (isnan(floatBody) || isinf(floatBody)) {
    app()->displayWarning(I18n::Message::UndefinedValue);
    return false;
  }
  if (!setParameterAtIndex(selectableTableView()->selectedRow(), floatBody)) {
    return false;
  }
  willDisplayCellForIndex(selectableTableView()->selectedCell(), activeCell());
  selectableTableView()->reloadData();
  if (event == Ion::Events::EXE || event == Ion::Events::OK) {
    selectableTableView()->selectCellAtLocation(selectableTableView()->selectedColumn(), selectableTableView()->selectedRow()+1);
  } else {
    selectableTableView()->handleEvent(event);
  }
  return true;
}

bool FloatParameterController::textFieldDidReceiveEvent(::TextField * textField, Ion::Events::Event event) {
  if (event == Ion::Events::Backspace && !textField->isEditing()) {
    textField->setEditing(true);
    return true;
  }
  return TextFieldDelegate::textFieldDidReceiveEvent(textField, event);
}

void FloatParameterController::tableViewDidChangeSelection(SelectableTableView * t, int previousSelectedCellX, int previousSelectedCellY) {
  if (previousSelectedCellX == t->selectedColumn() && previousSelectedCellY == t->selectedRow()) {
    return;
  }
  if (previousSelectedCellY >= 0 && previousSelectedCellY < numberOfRows()-1) {
    MessageTableCellWithEditableText * myCell = (MessageTableCellWithEditableText *)t->cellAtLocation(previousSelectedCellX, previousSelectedCellY);
    myCell->setEditing(false);
  }
  if (t->selectedRow() == numberOfRows()-1) {
    Button * myNewCell = (Button *)t->selectedCell();
    app()->setFirstResponder(myNewCell);
    return;
  }
  if (t->selectedRow() >= 0) {
    MessageTableCellWithEditableText * myNewCell = (MessageTableCellWithEditableText *)t->selectedCell();
    app()->setFirstResponder(myNewCell);
  }
}

TextFieldDelegateApp * FloatParameterController::textFieldDelegateApp() {
  return (TextFieldDelegateApp *)app();
}

int FloatParameterController::activeCell() {
  return selectableTableView()->selectedRow();
}

void FloatParameterController::unloadView() {
  assert(m_okButton != nullptr);
  delete m_okButton;
  m_okButton = nullptr;
  DynamicViewController::unloadView();
}

StackViewController * FloatParameterController::stackController() {
  return (StackViewController *)parentResponder();
}

SelectableTableView * FloatParameterController::selectableTableView() {
  return (SelectableTableView *)view();
}

View * FloatParameterController::createView() {
  SelectableTableView * tableView = new SelectableTableView(this, this, 0, 1, Metric::CommonTopMargin, Metric::CommonRightMargin, Metric::CommonBottomMargin, Metric::CommonLeftMargin, this);
  assert(m_okButton == nullptr);
  m_okButton = new ButtonWithSeparator(tableView, okButtonText(), Invocation([](void * context, void * sender) {
    FloatParameterController * parameterController = (FloatParameterController *) context;
    parameterController->buttonAction();
  }, this));
  return tableView;
}

void FloatParameterController::buttonAction() {
  StackViewController * stack = stackController();
  stack->pop();
}

I18n::Message FloatParameterController::okButtonText() {
  return I18n::Message::Ok;
}

}
