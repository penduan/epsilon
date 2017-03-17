#include <ion.h>
#include "console.h"

/* This file implements a serial console.
 * We use a 115200 8N1 serial port */

namespace Ion {
namespace Console {

char readChar() {
  while (Device::UARTPort.SR()->getRXNE() == 0) {
  }
  return (char)Device::UARTPort.DR()->get();
}

void writeChar(char c) {
  while (Device::UARTPort.SR()->getTXE() == 0) {
  }
  Device::UARTPort.DR()->set(c);
}

}
}

namespace Ion {
namespace Console {
namespace Device {

void init() {
  RCC.APB2ENR()->setUSART1EN(true);

  for(const GPIOPin & g : Pins) {
    g.group().MODER()->setMode(g.pin(), GPIO::MODER::Mode::AlternateFunction);
    g.group().AFR()->setAlternateFunction(g.pin(), GPIO::AFR::AlternateFunction::AF7);
  }

  UARTPort.CR1()->setUE(true);
  UARTPort.CR1()->setTE(true);
  UARTPort.CR1()->setRE(true);

  /* We need to set the baud rate of the UART port.
   * This is set relative to the APB2 clock, which runs at 96 MHz.
   *
   * The baud rate is set by the following equation:
   * BaudRate = fAPB2/(16*USARTDIV), where USARTDIV is a divider.
   * In other words, USARDFIV = fAPB2/(16*BaudRate). All frequencies in Hz.
   *
   * In our case, fAPB2 = 96 MHz, so USARTDIV = 52.08333
   * DIV_MANTISSA = 52
   * DIV_FRAC = 16*0.083333 = 1.33 = 1
   */
  UARTPort.BRR()->setDIV_MANTISSA(52);
  UARTPort.BRR()->setDIV_FRAC(1);
}

void shutdown() {
  for(const GPIOPin & g : Pins) {
    g.group().MODER()->setMode(g.pin(), GPIO::MODER::Mode::Analog);
    g.group().PUPDR()->setPull(g.pin(), GPIO::PUPDR::Pull::None);
  }
}

}
}
}
