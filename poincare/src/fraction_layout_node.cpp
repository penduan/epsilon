#include <poincare/fraction_layout_node.h>
#include <poincare/empty_layout_node.h>
#include <poincare/horizontal_layout_node.h>
#include <poincare/layout_helper.h>
#include <poincare/serialization_helper.h>
#include <ion/charset.h>
#include <escher/metric.h>
#include <assert.h>

namespace Poincare {

static inline KDCoordinate max(KDCoordinate x, KDCoordinate y) { return x > y ? x : y; }

void FractionLayoutNode::moveCursorLeft(LayoutCursor * cursor, bool * shouldRecomputeLayout) {
   if (cursor->position() == LayoutCursor::Position::Left
       && ((!numeratorLayout()->isUninitialized() && cursor->layoutNode() == numeratorLayout())
         || (!denominatorLayout()->isUninitialized() && cursor->layoutNode() == denominatorLayout())))
  {
    // Case: Left of the numerator or the denominator. Go Left of the fraction.
    cursor->setLayoutNode(this);
    return;
  }
  assert(cursor->layoutNode() == this);
  // Case: Right. Go to the denominator.
  if (cursor->position() == LayoutCursor::Position::Right) {
    assert(!denominatorLayout()->isUninitialized());
    cursor->setLayoutNode(denominatorLayout());
    cursor->setPosition(LayoutCursor::Position::Right);
    return;
  }
  // Case: Left. Ask the parent.
  assert(cursor->position() == LayoutCursor::Position::Left);
  LayoutNode * parentNode = parent();
  if (!parentNode->isUninitialized()) {
    parentNode->moveCursorLeft(cursor, shouldRecomputeLayout);
  }
}

void FractionLayoutNode::moveCursorRight(LayoutCursor * cursor, bool * shouldRecomputeLayout) {
   if (cursor->position() == LayoutCursor::Position::Right
       && ((!numeratorLayout()->isUninitialized() && cursor->layoutNode() == numeratorLayout())
         || (!denominatorLayout()->isUninitialized() && cursor->layoutNode() == denominatorLayout())))
  {
    // Case: Right of the numerator or the denominator. Go Right of the fraction.
    cursor->setLayoutNode(this);
    return;
  }
  assert(cursor->layoutNode() == this);
  if (cursor->position() == LayoutCursor::Position::Left) {
    // Case: Left. Go to the numerator.
    assert(!numeratorLayout()->isUninitialized());
    cursor->setLayoutNode(numeratorLayout());
    return;
  }
  // Case: Right. Ask the parent.
  assert(cursor->position() == LayoutCursor::Position::Right);
  LayoutNode * parentNode = parent();
  if (!parentNode->isUninitialized()) {
    parentNode->moveCursorRight(cursor, shouldRecomputeLayout);
  }
}

void FractionLayoutNode::moveCursorUp(LayoutCursor * cursor, bool * shouldRecomputeLayout, bool equivalentPositionVisited) {
  if (!denominatorLayout()->isUninitialized() && cursor->layoutNode()->hasAncestor(denominatorLayout(), true)) {
    // If the cursor is inside denominator, move it to the numerator.
    assert(!numeratorLayout()->isUninitialized());
    numeratorLayout()->moveCursorUpInDescendants(cursor, shouldRecomputeLayout);
    return;
  }
  if (cursor->layoutNode() == this) {
    // If the cursor is Left or Right, move it to the numerator.
    assert(!numeratorLayout()->isUninitialized());
    cursor->setLayoutNode(numeratorLayout());
    return;
  }
  LayoutNode::moveCursorUp(cursor, shouldRecomputeLayout, equivalentPositionVisited);
}

void FractionLayoutNode::moveCursorDown(LayoutCursor * cursor, bool * shouldRecomputeLayout, bool equivalentPositionVisited) {
  if (!numeratorLayout()->isUninitialized() && cursor->layoutNode()->hasAncestor(numeratorLayout(), true)) {
    // If the cursor is inside numerator, move it to the denominator.
    assert(!denominatorLayout()->isUninitialized());
    denominatorLayout()->moveCursorDownInDescendants(cursor, shouldRecomputeLayout);
    return;
  }
  if (cursor->layoutNode() == this){
    // If the cursor is Left or Right, move it to the denominator.
    assert(!denominatorLayout()->isUninitialized());
    cursor->setLayoutNode(denominatorLayout());
    return;
  }
  LayoutNode::moveCursorDown(cursor, shouldRecomputeLayout, equivalentPositionVisited);
}

void FractionLayoutNode::deleteBeforeCursor(LayoutCursor * cursor) {
  if (cursor->layoutNode() == denominatorLayout()) {
    /* Case: Left of the denominator. Replace the fraction with a horizontal
     * juxtaposition of the numerator and the denominator. */
    LayoutRef thisRef = LayoutRef(this);
    assert(cursor->position() == LayoutCursor::Position::Left);
    if (numeratorLayout()->isEmpty() && denominatorLayout()->isEmpty()) {
      /* Case: Numerator and denominator are empty. Move the cursor and replace
       * the fraction with an empty layout. */
      thisRef.replaceWith(EmptyLayoutRef(), cursor);
      // WARNING: Do no use "this" afterwards
      return;
    }
    /* Else, replace the fraction with a juxtaposition of the numerator and
     * denominator. Place the cursor in the middle of the juxtaposition, which
     * is right of the numerator. */
    LayoutRef numeratorRef = LayoutRef(numeratorLayout());
    LayoutRef denominatorRef = LayoutRef(denominatorLayout());
    thisRef.replaceChildWithGhostInPlace(numeratorRef);
    // WARNING: Do no use "this" afterwards
    thisRef.replaceChildWithGhostInPlace(denominatorRef);
    thisRef.replaceWithJuxtapositionOf(numeratorRef, denominatorRef, cursor, true);
    return;
  }

  if (cursor->layoutNode() == this && cursor->position() == LayoutCursor::Position::Right) {
    // Case: Right. Move Right of the denominator.
    cursor->setLayoutNode(denominatorLayout());
    return;
  }
  LayoutNode::deleteBeforeCursor(cursor);
}

int FractionLayoutNode::serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  if (bufferSize == 0) {
    return -1;
  }
  buffer[bufferSize-1] = 0;
  int numberOfChar = 0;
  if (numberOfChar >= bufferSize-1) { return bufferSize-1;}

  int idxInParent = -1;
  LayoutNode * p = parent();
  if (!p->isUninitialized()) {
    idxInParent = p->indexOfChild(this);
  }

  // Add a multiplication if omitted.
  if (idxInParent > 0 && p->isHorizontal() && p->childAtIndex(idxInParent - 1)->canBeOmittedMultiplicationLeftFactor()) {
    buffer[numberOfChar++] = Ion::Charset::MiddleDot;
    if (numberOfChar >= bufferSize-1) { return bufferSize-1;}
  }

  bool addParenthesis = false;
  if (idxInParent >= 0 && idxInParent < (p->numberOfChildren() - 1) && p->isHorizontal() && p->childAtIndex(idxInParent + 1)->isVerticalOffset()) {
    addParenthesis = true;
    // Add parenthesis
    buffer[numberOfChar++] = '(';
    if (numberOfChar >= bufferSize-1) { return bufferSize-1;}
  }

  // Write the content of the fraction
  numberOfChar += SerializationHelper::Infix(this, buffer+numberOfChar, bufferSize-numberOfChar, floatDisplayMode, numberOfSignificantDigits, "/");
  if (numberOfChar >= bufferSize-1) { return bufferSize-1; }

  if (addParenthesis) {
    // Add parenthesis
    buffer[numberOfChar++] = ')';
    if (numberOfChar >= bufferSize-1) { return bufferSize-1;}
  }

  // Add a multiplication if omitted.
  if (idxInParent >= 0 && idxInParent < (p->numberOfChildren() - 1) && p->isHorizontal() && p->childAtIndex(idxInParent + 1)->canBeOmittedMultiplicationRightFactor()) {
    buffer[numberOfChar++] = Ion::Charset::MiddleDot;
    if (numberOfChar >= bufferSize-1) { return bufferSize-1;}
  }

  buffer[numberOfChar] = 0;
  return numberOfChar;
}

LayoutNode * FractionLayoutNode::layoutToPointWhenInserting() {
  if (numeratorLayout()->isEmpty()){
    return numeratorLayout();
  }
  if (denominatorLayout()->isEmpty()){
    return denominatorLayout();
  }
  return this;
}

void FractionLayoutNode::didCollapseSiblings(LayoutCursor * cursor) {
  if (cursor != nullptr) {
    cursor->setLayoutNode(denominatorLayout());
    cursor->setPosition(LayoutCursor::Position::Left);
  }
}

KDSize FractionLayoutNode::computeSize() {
  KDCoordinate width = max(numeratorLayout()->layoutSize().width(), denominatorLayout()->layoutSize().width())
    + 2*Metric::FractionAndConjugateHorizontalOverflow+2*Metric::FractionAndConjugateHorizontalMargin;
  KDCoordinate height = numeratorLayout()->layoutSize().height()
    + k_fractionLineMargin + k_fractionLineHeight + k_fractionLineMargin
    + denominatorLayout()->layoutSize().height();
  return KDSize(width, height);
}

KDCoordinate FractionLayoutNode::computeBaseline() {
  return numeratorLayout()->layoutSize().height() + k_fractionLineMargin + k_fractionLineHeight;
}

KDPoint FractionLayoutNode::positionOfChild(LayoutNode * child) {
  KDCoordinate x = 0;
  KDCoordinate y = 0;
  if (child == numeratorLayout()) {
    x = (KDCoordinate)((layoutSize().width() - numeratorLayout()->layoutSize().width())/2);
  } else if (child == denominatorLayout()) {
    x = (KDCoordinate)((layoutSize().width() - denominatorLayout()->layoutSize().width())/2);
    y = (KDCoordinate)(numeratorLayout()->layoutSize().height() + 2*k_fractionLineMargin + k_fractionLineHeight);
  } else {
    assert(false);
  }
  return KDPoint(x, y);
}

void FractionLayoutNode::render(KDContext * ctx, KDPoint p, KDColor expressionColor, KDColor backgroundColor) {
  KDCoordinate fractionLineY = p.y() + numeratorLayout()->layoutSize().height() + k_fractionLineMargin;
  ctx->fillRect(KDRect(p.x()+Metric::FractionAndConjugateHorizontalMargin, fractionLineY, layoutSize().width()-2*Metric::FractionAndConjugateHorizontalMargin, k_fractionLineHeight), expressionColor);
}

}
