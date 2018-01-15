#include <escher/expression_view_with_cursor.h>
#include <kandinsky/rect.h>
#include <assert.h>

using namespace Poincare;

ExpressionViewWithCursor::ExpressionViewWithCursor(ExpressionLayout * expressionLayout) :
  m_cursor(),
  m_expressionView(),
  m_cursorView(),
  m_isEditing(false)
{
  m_cursor.setPointedExpressionLayout(expressionLayout);
  m_cursor.setPosition(ExpressionLayoutCursor::Position::Right);
  m_expressionView.setExpressionLayout(expressionLayout);
}

void ExpressionViewWithCursor::cursorPositionChanged() {
  layoutCursorSubview();
}

KDRect ExpressionViewWithCursor::cursorRect() {
  return m_cursorView.frame();
}

KDSize ExpressionViewWithCursor::minimalSizeForOptimalDisplay() const {
  KDSize expressionViewSize = m_expressionView.minimalSizeForOptimalDisplay();
  return KDSize(expressionViewSize.width()+1, m_expressionView.minimalSizeForOptimalDisplay().height()); // +1 for the cursor
}

View * ExpressionViewWithCursor::subviewAtIndex(int index) {
  assert(index >= 0 && index < 2);
  View * m_views[] = {&m_expressionView, &m_cursorView};
  return m_views[index];
}

void ExpressionViewWithCursor::layoutSubviews() {
  m_expressionView.setFrame(bounds());
  layoutCursorSubview();
}

void ExpressionViewWithCursor::layoutCursorSubview() {
  KDPoint expressionViewOrigin = m_expressionView.drawingOrigin();
  KDPoint cursoredExpressionViewOrigin = m_cursor.pointedExpressionLayout()->absoluteOrigin();
  KDCoordinate cursorX = expressionViewOrigin.x() + cursoredExpressionViewOrigin.x();
  if (m_cursor.position() == ExpressionLayoutCursor::Position::Right) {
    cursorX += m_cursor.pointedExpressionLayout()->size().width();
  }
  KDPoint cursorTopLeftPosition(cursorX, expressionViewOrigin.y() + cursoredExpressionViewOrigin.y() + m_cursor.pointedExpressionLayout()->baseline()-m_cursor.cursorHeight()/2);
  m_cursorView.setFrame(KDRect(cursorTopLeftPosition, 1, m_cursor.cursorHeight()));
}
