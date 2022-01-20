
#ifndef INPUT_HANDLER_HEADER
#define INPUT_HANDLER_HEADER

#include <functional>

namespace InputHandlerHeader
{
  enum class KeyStatus
  {
    NONE = -1, // unused for events, only used in managing
    RESET = 0,
    KEY_DOWN = 1,
    KEY_HOLD = 2,
    KEY_UP = 3
  };

  struct KeyEvent
  {
    KeyStatus status : 2;
  };

  using KeyEventFunc = std::function<bool(KeyEvent)>;
}

#endif //INPUT_HANDLER_HEADER