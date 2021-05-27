#ifndef APPS_PROBABILITY_GUI_PAGE_CONTROLLER_H
#define APPS_PROBABILITY_GUI_PAGE_CONTROLLER_H

#include <escher/selectable_list_view_controller.h>
#include <escher/stack_view_controller.h>
#include <escher/view_controller.h>

using namespace Escher;

namespace Probability {

/*
 * This templatized class adds some utils function to handle the parentResponder as a
 * StackViewController. To enforce correct typing, the specialization class must require a
 * StackViewController * be passed as argument to the constructor.
 */
template <typename T>
class PageController : public T {
  using T::T;

public:
  StackViewController * stackViewController() {
    return static_cast<StackViewController *>(T::parentResponder());
  }
  /* Pushes the given controller onto the StackViewController */
  void openPage(ViewController * nextPage, KDColor backgroundColor = Palette::PurpleBright,
                KDColor separatorColor = Palette::PurpleBright, KDColor textColor = KDColorWhite) {
    stackViewController()->push(nextPage, textColor, backgroundColor, separatorColor);
  }
};

/* A Page is a controller that requires a StackViewController as its parent
 * and can open subPages easily by pushing them to the StackViewController. */
class Page : public PageController<ViewController> {
public:
  Page(StackViewController * stackViewController) : PageController(stackViewController){};
};

/* Similar to a Page but for SelectableListViewController */
class SelectableListViewPage : public PageController<SelectableListViewController> {
public:
  SelectableListViewPage(StackViewController * stackViewController,
                         SelectableTableViewDelegate * tableDelegate = nullptr)
      : PageController(stackViewController, tableDelegate){};
};

}  // namespace Probability

#endif /* APPS_PROBABILITY_GUI_PAGE_CONTROLLER_H */
