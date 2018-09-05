#include <poincare/layout_node.h>
#include <poincare/horizontal_layout_node.h>
#include <poincare/layout_cursor.h>
#include <poincare/layout_reference.h>
#include <poincare/matrix_layout_node.h>
#include <ion/display.h>

namespace Poincare {

// Rendering

void LayoutNode::draw(KDContext * ctx, KDPoint p, KDColor expressionColor, KDColor backgroundColor) {
  for (LayoutNode * l : children()) {
    l->draw(ctx, p, expressionColor, backgroundColor);
  }
  render(ctx, absoluteOrigin().translatedBy(p), expressionColor, backgroundColor);
}

KDPoint LayoutNode::origin() {
  LayoutNode * p = parent();
  if (p->isUninitialized()) {
    return absoluteOrigin();
  } else {
    return KDPoint(absoluteOrigin().x() - p->absoluteOrigin().x(),
        absoluteOrigin().y() - p->absoluteOrigin().y());
  }
}

KDPoint LayoutNode::absoluteOrigin() {
  LayoutNode * p = parent();
  if (!m_positioned) {
    if (!p->isUninitialized()) {
      m_frame.setOrigin(p->absoluteOrigin().translatedBy(p->positionOfChild(this)));
    } else {
      m_frame.setOrigin(KDPointZero);
    }
    m_positioned = true;
  }
  return m_frame.origin();
}

KDSize LayoutNode::layoutSize() {
  if (!m_sized) {
    m_frame.setSize(computeSize());
    m_sized = true;
  }
  return m_frame.size();
}

KDCoordinate LayoutNode::baseline() {
  if (!m_baselined) {
    m_baseline = computeBaseline();
    m_baselined = true;
  }
  return m_baseline;
}

void LayoutNode::invalidAllSizesPositionsAndBaselines() {
  m_sized = false;
  m_positioned = false;
  m_baselined = false;
  for (LayoutNode * l : children()) {
    l->invalidAllSizesPositionsAndBaselines();
  }
}

// Tree navigation
void LayoutNode::moveCursorUp(LayoutCursor * cursor, bool * shouldRecomputeLayout, bool equivalentPositionVisited) {
  moveCursorVertically(VerticalDirection::Up, cursor, shouldRecomputeLayout, equivalentPositionVisited);
}

void LayoutNode::moveCursorDown(LayoutCursor * cursor, bool * shouldRecomputeLayout, bool equivalentPositionVisited) {
  moveCursorVertically(VerticalDirection::Down, cursor, shouldRecomputeLayout, equivalentPositionVisited);
}

void LayoutNode::moveCursorUpInDescendants(LayoutCursor * cursor, bool * shouldRecomputeLayout) {
  return moveCursorInDescendantsVertically(VerticalDirection::Up, cursor, shouldRecomputeLayout);
}

void LayoutNode::moveCursorDownInDescendants(LayoutCursor * cursor, bool * shouldRecomputeLayout) {
  return moveCursorInDescendantsVertically(VerticalDirection::Down, cursor, shouldRecomputeLayout);
}

LayoutCursor LayoutNode::equivalentCursor(LayoutCursor * cursor) {
  // Only HorizontalLayout may have no parent, and it overloads this method
  assert(!parent()->isUninitialized());
  return (cursor->layoutReference().node() == this) ? parent()->equivalentCursor(cursor) : LayoutCursor();
}

// Tree modification

void LayoutNode::deleteBeforeCursor(LayoutCursor * cursor) {
  int indexOfPointedLayout = indexOfChild(cursor->layoutNode());
  if (indexOfPointedLayout >= 0) {
    // Case: The pointed layout is a child. Move Left.
    assert(cursor->position() == LayoutCursor::Position::Left);
    bool shouldRecomputeLayout = false;
    cursor->moveLeft(&shouldRecomputeLayout);
    return;
  }
  assert(cursor->layoutNode() == this);
  LayoutNode * p = parent();
  // Case: this is the pointed layout.
  if (p->isUninitialized()) {
    // Case: No parent. Return.
    return;
  }
  if (cursor->position() == LayoutCursor::Position::Left) {
    // Case: Left. Ask the parent.
    p->deleteBeforeCursor(cursor);
    return;
  }
  assert(cursor->position() == LayoutCursor::Position::Right);
  // Case: Right. Delete the layout (or replace it with an EmptyLayout).
  LayoutRef(p).removeChild(LayoutRef(this), cursor);
  // WARNING: Do no use "this" afterwards
}

bool LayoutNode::willRemoveChild(LayoutNode * l, LayoutCursor * cursor, bool force) {
  if (!force) {
    LayoutRef(this).replaceChildWithEmpty(LayoutRef(l), cursor);
    return false;
  }
  return true;
}

// Other
bool LayoutNode::canBeOmittedMultiplicationLeftFactor() const {
  /* WARNING: canBeOmittedMultiplicationLeftFactor is true when and only when
   * isCollapsable is true too. If isCollapsable changes, it might not be the
   * case anymore so make sure to modify this function if needed. */
  int numberOfOpenParentheses = 0;
  return isCollapsable(&numberOfOpenParentheses, true);
}

bool LayoutNode::canBeOmittedMultiplicationRightFactor() const {
  /* WARNING: canBeOmittedMultiplicationLeftFactor is true when and only when
   * isCollapsable is true and isVerticalOffset is false. If one of these
   * functions changes, it might not be the case anymore so make sure to modify
   * canBeOmittedMultiplicationRightFactor if needed. */
  int numberOfOpenParentheses = 0;
  return isCollapsable(&numberOfOpenParentheses, false) && !isVerticalOffset();
}

// TreeNode

TreeNode * LayoutNode::uninitializedStaticNode() const {
  return LayoutReference().node();
}

// Private

void LayoutNode::moveCursorVertically(VerticalDirection direction, LayoutCursor * cursor, bool * shouldRecomputeLayout, bool equivalentPositionVisited) {
  if (!equivalentPositionVisited) {
    LayoutCursor cursorEquivalent = equivalentCursor(cursor);
    if (cursorEquivalent.isDefined()) {
      cursor->setLayoutReference(cursorEquivalent.layoutReference());
      cursor->setPosition(cursorEquivalent.position());
      if (direction == VerticalDirection::Up) {
        cursor->layoutNode()->moveCursorUp(cursor, shouldRecomputeLayout, true);
      } else {
        cursor->layoutNode()->moveCursorDown(cursor, shouldRecomputeLayout, true);
      }
      return;
    }
  }
  LayoutNode * p = parent();
  if (p->isUninitialized()) {
    cursor->setLayoutReference(LayoutReference());
    return;
  }
  if (direction == VerticalDirection::Up) {
    p->moveCursorUp(cursor, shouldRecomputeLayout, true);
  } else {
    p->moveCursorDown(cursor, shouldRecomputeLayout, true);
  }
}

void LayoutNode::moveCursorInDescendantsVertically(VerticalDirection direction, LayoutCursor * cursor, bool * shouldRecomputeLayout) {
  LayoutNode * childResult = static_cast<LayoutNode *>(uninitializedStaticNode());
  LayoutNode ** childResultPtr = &childResult;
  LayoutCursor::Position resultPosition = LayoutCursor::Position::Left;
  /* The distance between the cursor and its next position cannot be greater
   * than this initial value of score. */
  int resultScore = Ion::Display::Width*Ion::Display::Width + Ion::Display::Height*Ion::Display::Height;

  scoreCursorInDescendantsVertically(direction, cursor, shouldRecomputeLayout, childResultPtr, &resultPosition, &resultScore);

  // If there is a valid result
  LayoutRef resultRef(childResult);
  if (!(*childResultPtr)->isUninitialized()) {
    *shouldRecomputeLayout = childResult->addGreySquaresToAllMatrixAncestors();
    // WARNING: Do not use "this" afterwards
  }
  cursor->setLayoutReference(resultRef);
  cursor->setPosition(resultPosition);
}

void LayoutNode::scoreCursorInDescendantsVertically (
    VerticalDirection direction,
    LayoutCursor * cursor,
    bool * shouldRecomputeLayout,
    LayoutNode ** childResult,
    void * resultPosition,
    int * resultScore)
{
  LayoutCursor::Position * castedResultPosition = static_cast<LayoutCursor::Position *>(resultPosition);
  KDPoint cursorMiddleLeft = cursor->middleLeftPoint();
  bool layoutIsUnderOrAbove = direction == VerticalDirection::Up ? m_frame.isAbove(cursorMiddleLeft) : m_frame.isUnder(cursorMiddleLeft);
  bool layoutContains = m_frame.contains(cursorMiddleLeft);

  if (layoutIsUnderOrAbove) {
    // Check the distance to a Left cursor.
    int currentDistance = LayoutCursor(this, LayoutCursor::Position::Left).middleLeftPoint().squareDistanceTo(cursorMiddleLeft);
    if (currentDistance <= *resultScore ){
      *childResult = this;
      *castedResultPosition = LayoutCursor::Position::Left;
      *resultScore = currentDistance;
    }

    // Check the distance to a Right cursor.
    currentDistance = LayoutCursor(this, LayoutCursor::Position::Right).middleLeftPoint().squareDistanceTo(cursorMiddleLeft);
    if (currentDistance < *resultScore) {
      *childResult = this;
      *castedResultPosition = LayoutCursor::Position::Right;
      *resultScore = currentDistance;
    }
  }
  if (layoutIsUnderOrAbove || layoutContains) {
    for (LayoutNode * c : children()) {
      c->scoreCursorInDescendantsVertically(direction, cursor, shouldRecomputeLayout, childResult, castedResultPosition, resultScore);
    }
  }
}

bool LayoutNode::changeGreySquaresOfAllMatrixAncestors(bool add) {
  bool changedSquares = false;
  LayoutRef currentAncestor = LayoutRef(parent());
  while (!currentAncestor.isUninitialized()) {
    if (currentAncestor.isMatrix()) {
      if (add) {
        MatrixLayoutRef(static_cast<MatrixLayoutNode *>(currentAncestor.node())).addGreySquares();
      } else {
        MatrixLayoutRef(static_cast<MatrixLayoutNode *>(currentAncestor.node())).removeGreySquares();
      }
      changedSquares = true;
    }
    currentAncestor = currentAncestor.parent();
  }
  return changedSquares;
}

}
