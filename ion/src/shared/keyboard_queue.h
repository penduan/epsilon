#ifndef ION_SHARED_KEYBOARD_QUEUE_H
#define ION_SHARED_KEYBOARD_QUEUE_H

#include <ion/keyboard.h>
#include <stddef.h>

namespace Ion {
namespace Keyboard {

class Queue {
public:
  Queue() : m_begin(0), m_end(0) {}
  static Queue * sharedQueue();
  void push(Ion::Keyboard::State s);
  Ion::Keyboard::State pop(); // crashes if empty
  bool isEmpty() const { return m_begin == m_end; }
private:
  static constexpr size_t k_maximalNumberOfStates = 5;
  // Ring buffer
  Ion::Keyboard::State m_states[k_maximalNumberOfStates];
  size_t m_begin;
  size_t m_end;
};

}
}

#endif
