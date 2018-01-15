#ifndef ESCHER_EDITABLE_EXPRESSION_VIEW_H
#define ESCHER_EDITABLE_EXPRESSION_VIEW_H

#include <escher/scrollable_view.h>
#include <escher/expression_view_with_cursor.h>
#include <escher/editable_expression_view_delegate.h>
#include <poincare/expression_layout_cursor.h>

class EditableExpressionView : public ScrollableView, public ScrollViewDataSource {
public:
  EditableExpressionView(Responder * parentResponder, Poincare::ExpressionLayout * expressionLayout, EditableExpressionViewDelegate * delegate = nullptr);
  void setDelegate(EditableExpressionViewDelegate * delegate) { m_delegate = delegate; }
  ExpressionViewWithCursor * expressionViewWithCursor() { return &m_expressionViewWithCursor; }
  bool isEditing() const;
  void setEditing(bool isEditing);
  void scrollToCursor();

  /* Responder */
  Toolbox * toolbox() override;
  bool handleEvent(Ion::Events::Event event) override;

  bool editableExpressionViewShouldFinishEditing(Ion::Events::Event event);

  /* Callback for MathToolbox */
  void insertLayoutAtCursor(Poincare::ExpressionLayout * layout, Poincare::ExpressionLayout * pointedLayout);

  /* View */
  KDSize minimalSizeForOptimalDisplay() const override;

private:
  bool privateHandleEvent(Ion::Events::Event event);
  void cursorOrLayoutChanged();
  ExpressionViewWithCursor m_expressionViewWithCursor;
  EditableExpressionViewDelegate * m_delegate;
};

#endif
