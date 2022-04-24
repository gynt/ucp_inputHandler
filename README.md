# UCP Input Handler

This repository contains a module for the "Unofficial Crusader Patch Version 3" (UCP3), a modification for Stronghold Crusader.
This module exposes the possibility to add certain events to inputs, or re-define already present key combinations.


### Motivation and Plan

Stronghold Crusader actually offers multiple keyboard shortcuts for different actions, like selecting certain buildings or open specific menus.
However, all those functions are hard-coded into the executable. There is no way to change the associated keys.

The module intends to provide tools to allow the changing of input keys by providing a simple input handler that takes care of input events.
This is achieved by using the [winProcHandler](https://github.com/TheRedDaemon/ucp_winProcHandler) to intercept inputs and transforming them to events.
The user can now register certain events by name and bind them to specific inputs.
As a second functionality, the handler also allows to create an alias for an input.
Inputs that are not handled are given to the Crusader input function instead.
If the input was registered as an alias, it is transformed to the specific other input for the backend.

At the moment, only keyboard inputs are handled at all.
Additionally, the module includes a small fix regarding stuck arrow keys on focus switch.


### Usage

The module is part of the UCP3. Certain commits of the main branch of this repository are included as a git submodule.
It is therefore not needed to download additional content.

However, should issues or suggestions arise that are related to this module, feel free to add a new GitHub issue.
Support is currently only guaranteed for the western versions of Crusader 1.41 and Crusader Extreme 1.41.1-E.
Other, eastern versions of 1.41 might work.

The module has the [winProcHandler](https://github.com/TheRedDaemon/ucp_winProcHandler) as dependency and it registers its winProc function with an early **-50000** priority (smaller means earlier).


### C++-Functions

t the time of creation, C functions would need to be parsed through lua in the form of numbers. To make using the module easier, the header [inputHandlerHeader.h](ucp_inputHandler/ucp_inputHandler/inputHandlerHeader.h) can be copied into your project.  
It is used by calling the function *initModuleFunctions(lua_state * )* during the lua require call of the dll. It tries to receive the provided functions and returns *true* if successful. For this to work, winProcHandler needs to be a dependency.
The provided functions are the following:

* *bool LockKeyMap(const char\* keyMapName)*  
  A function of the module is to allow the selection of a key-map.
  If a key map is selected, only input events of the specific key map are handled.
  Not input is passed to Crusaders own handler.
  Maps are identified by name. A call to this function might create one, if it is not present.
  Returns *true* if the switch was successful. It will fail if the name of the default map ("") is given or if another map is already locked.

* *bool ReleaseKeyMap(const char\* keyMapName)*  
  Releases the key map, enabling the default map again. The given string needs to be the current map, or it will fail.
  Returns *true* if the switch was successful.

* *bool RegisterKeyComb(const char\* keyMapName, bool ctrl, bool shift, bool alt, unsigned char virtualKey, const char\* eventName)*  
  This function registers a key combination that should trigger a specific event.
  The first string indicates the key map the combination should be placed in.
  The following bools are for the status of the modifier keys, the char after that needs to be a virtual key code. Only keyboard key codes are handled.
  The last value is the name of the event that should be triggered. It returns *true* on success.

* *bool RegisterEvent(const char\* keyMapName, const char\* eventName, const char\* asciiTitle, std::function\<KeyEventFunction>\&& func)*  
  This call registers an event.
  The first string indicates the key map the event should be placed in. The binding with combinations only happens if both are in the same map.
  The following value is the actual name of the event used for binding.
  The ASCII title is currently unused, but it should receive a human readable name of the event with valid ASCII symbols.
  The final parameter should receive a std\::function that contains an KeyEventFunction (explained later).
  Due to the nature of std\::function, it could receive a raw function pointer, a lambda (with captures) or an std\::function object.
  Note, however, that the received object will be moved. *RegisterEvent* returns *true* if successful.

* *bool RegisterRawEvent(const char\* keyMapName, const char\* eventName, const char\* asciiTitle, KeyEventFunction\* func)*  
  Basically the same as *RegisterEvent*, but it receives a raw function pointer.
  This API is mostly included to provide an easier function to call in case one wants to create an event using assembly language in Lua.

*KeyEventFunction* (in code called *RawKeyEventFunc*, since the wrapped version is called *KeyEventFunc*) is the function that will be called in case of a key event.
It takes the following form:

* *bool KeyEventFunction(KeyEvent, int windowProcPrio, HWND winHandle)*  
  *windowProcPrio* and *winHandle* are not relevant to the event and should be ignored.
  They are needed for the re-translation to a windows event in case the input should pass to Crusaders handler.
  *KeyEvent* is the relevant part. At the moment it is an bit-field.
  Whether or not this changes depends on the bit order the compiler produces. If issues arise, it will change to a flag integer.
  The current struct contains the following values (in hopefully this order):

  * *unsigned char virtualKey : 8*  
    The virtual key that triggered this event. (8 bit)  
    Only if this key changes, the event is lifted. Changes of the modifier keys will reflect in the received values.

  * *KeyStatus status : 2*  
    The key status given as enum. (2 bit)  
    The following values are possible:

    * *RESET = 0*
    * *KEY_DOWN = 1*
    * *KEY_HOLD = 2*
    * *KEY_UP = 3*
    
    *RESET* is a special case and for example triggered if the key map changes, the game loses focus or Num-Lock is used.
    The event is not consumed by this module and does not receive a valid windowProc priority, HWND or any key states.
    The event should clean up and assume a stable state.
    This is usually only relevant if the event listens to the *KEY_UP* state, which might never arrive.
    Cases that can cause this should be handled by a *RESET* event.

  * *unsigned int ctrlActive : 1*  
    The status of CTRL. (1 bit)  
    This value might change while the event is active.

  * *unsigned int shiftActive : 1*  
    The status of SHIFT. (1 bit)  
    This value might change while the event is active.

  * *unsigned int altActive : 1*  
    The status of ALT. (1 bit)  
    This value might change while the event is active.

  The function should return *false* in most cases. *true* will only have an effect if the event receives a *KEY_HOLD*.
  In this case, the event is deactivated and the event handler is run again with the current input, but the key status is set to *KEY_DOWN*.
  This can be used to instantly switch to another event if a modifier key is lifted, for example.


### Lua-Functions

**TODO**

### Options

**TODO**
    

### Special Thanks

To all of the UCP Team, the [Ghidra project](https://github.com/NationalSecurityAgency/ghidra) and
of course to [Firefly Studios](https://fireflyworlds.com/), the creators of Stronghold Crusader.