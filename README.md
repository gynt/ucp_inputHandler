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
Since the CHAR messages are generated before reaching the handler function, text input functions normally.
However, issues can happen if aliases are set that point to keys that interact with text, like *Enter*.

At the moment, only keyboard inputs are handled at all.
Additionally, the module includes a small fix regarding stuck arrow keys on focus switch.


### Usage

The module is part of the UCP3. Certain commits of the main branch of this repository are included as a git submodule.
It is therefore not needed to download additional content.

However, should issues or suggestions arise that are related to this module, feel free to add a new GitHub issue.
Support is currently only guaranteed for the western versions of Crusader 1.41 and Crusader Extreme 1.41.1-E.
Other, eastern versions of 1.41 might work.

The module has the [winProcHandler](https://github.com/TheRedDaemon/ucp_winProcHandler) as dependency and it registers its winProc function with an early **-50000** priority (smaller means earlier).


### C++-Exports

At the time of creation, C functions would need to be parsed through lua in the form of numbers. To make using the module easier, the header [inputHandlerHeader.h](ucp_inputHandler/ucp_inputHandler/inputHandlerHeader.h) can be copied into your project.
It is used by calling the function *initModuleFunctions(lua_state * )* during the lua require call of the dll. It tries to receive the provided functions and returns *true* if successful. For this to work, inputHandler needs to be a dependency of your module.
The provided functions are the following:

* *bool LockKeyMap(const char\* keyMapName)*  
  A function of the module is to allow the selection of a key-map.
  If a key map is selected, only input events of the specific key map are handled.
  No input is passed to Crusaders own handler.
  Maps are identified by name. A call to this function might create one, if it is not present.
  Returns *true* if the switch was successful. It will fail if the name of the default map ("") is given or if another map is already locked.

* *bool ReleaseKeyMap(const char\* keyMapName)*  
  Releases the key map, enabling the default map again. The given string needs to be the current map, or it will fail.
  Returns *true* if the switch was successful.

* *bool RegisterKeyComb(const char\* keyMapName, bool ctrl, bool shift, bool alt, unsigned char virtualKey, const char\* eventName)*  
  This function registers a key combination that should trigger a specific event.
  The first string indicates the key map the combination should be placed in.
  The following bools are for the status of the modifier keys, the char after that needs to be a virtual key code. Only keyboard key codes are handled.
  The last value is the name of the event that should be triggered. The function returns *true* on success.

* *bool RegisterEvent(const char\* keyMapName, const char\* eventName, const char\* asciiTitle, std::function\<KeyEventFunction>\&& func)*  
  This call registers an event.
  The first string indicates the key map the event should be placed in. The binding with combinations only happens if both are in the same map.
  The following value is the actual name of the event used for binding.
  The ASCII title is currently unused, but it should receive a human readable name of the event with valid ASCII symbols.
  The final parameter should receive a std\::function that contains a KeyEventFunction (explained later).
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
  *KeyEvent* is the relevant part. At the moment it is a bit-field.
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


### Lua-Exports

The Lua exports are parameters and functions accessible through the module object.


##### Enums:

* *DEFAULT_KEY_MAP*  
  Name of the default key map (`""`).
  Should not be changed.

* *status*  
  Enum table of the key states.
  The enums are `RESET`, `KEY_DOWN`, `KEY_HOLD` and `KEY_UP`.
  Should not be changed.

* *keys*  
  Enum table of the virtual key codes. This table maps names to virtual key codes of windows.
  They are too much to list here. The table can be found almost at the top of [init.lua](init.lua).
  Should not be changed.

* *invKeys*  
  Inverse enum table of the virtual key codes. Maps the virtual key codes of windows to the names used by this module.
  Should not be changed.

* *modifier*  
  Enum table of the virtual key codes of the modifiers. This tables maps names to virtual key codes of windows.
  The enums are SHIFT, CONTROL and ALT.
  Should not be changed.

* *invModifier*  
  Inverse enum table of the virtual key codes of the modifiers.
  Should not be changed.


##### Functions:

* *bool LockKeyMap(string keyMapName)*  
  Same as the C++-function.
  A function of the module is to allow the selection of a key-map.
  If a key map is selected, only input events of the specific key map are handled.
  No input is passed to Crusaders own handler.
  Maps are identified by name. A call to this function might create one, if it is not present.
  Returns *true* if the switch was successful. It will fail if the name of the default map ("") is given or if another map is already locked.

* *bool ReleaseKeyMap(string keyMapName)*  
  Same as the C++-function.
  Releases the key map, enabling the default map again. The given string needs to be the current map, or it will fail.
  Returns *true* if the switch was successful.

* *bool RegisterEvent(string keyMapName, string eventName, string asciiTitle, LuaKeyEventFunction funcToCall)*  
  Similar to the C++ function, this call registers an event.
  The first string indicates the key map the event should be placed in. The binding with combinations only happens if both are in the same map.
  The following value is the actual name of the event used for binding.
  The ASCII title is currently unused, but it should receive a human readable name of the event with valid ASCII symbols.
  The final parameter should receive a lua function of a specific structure explained in [Types and Structures](#types-and-structures).
  *RegisterEvent* returns *true* if successful.
 
* *bool RegisterKeyComb(string keyMapName, bool ctrlActive, bool shiftActive, bool altActive, int virtualKey, string eventName)*  
  Identical to the C++ function, this call registers a key combination that should trigger a specific event.
  The first string indicates the key map the combination should be placed in.
  The following bools are for the status of the modifier keys, the int after that needs to be a virtual key code. For easier use, the key enums can be used. Only keyboard key codes are handled.
  The last value is the name of the event that should be triggered. This function returns *true* on success.

* *bool RegisterKeyCombStr(string keyMapName, string eventName, KeyString keyStr)*  
  Does the same as *RegisterKeyComb*, but receives the map name and event name first.
  The last value is a specific string describing a key combination explained in [Types and Structures](#types-and-structures).

* *bool RegisterKeyAlias(KeyString ofStr, KeyString isStr)*  
  Allows to register an alias for a string combination. If the key combination `isStr` is used, the handler of Crusader receives `ofStr`.
  Returns *true* on success.
  Note, that these alias functions exist on the same level as other events.
  If an alias key combination is bound to a function, the alias is overwritten.
  Aliases are only created for the default key map.


##### Types and Structures

* *KeyString*  
  A key string is a short way to write a key combination. The structure uses the enums defined in the enums structures.
  The longest possible version would be `CONTROL+SHIFT+ALT+"keyEnum"`.
  The existence of an modifier enum in the string indicates that it should be pressed.
  The shortest would therefore be `"keyEnum"`. Only one of each modifiers and only one key enum are allowed.
  The key enum needs to be the last. This structure is also used for the options config.

* *bool LuaKeyEventFunction(LuaKeyEvent event)*  
  This function in very similar to the C++ version of it, but it only receives the event object.
  Once again, returning *true* only has an effect if the current key status is key hold, in which case the key handler is called again with a KEY_DOWN status, but still the current key types.
  This can be used to react to changed modifiers, which do not lift the key event.
  
* *LuaKeyEvent*  
  The event object received by the *LuaKeyEventFunction* has several functions that can be used:

  * *bool ctrlActive()*  
    If CTRL is pressed.

  * *bool shiftActive()*  
    If SHIFT is pressed.

  * *bool altActive()*  
    If ALT is pressed.

  * *int virtualKeyNum()*  
    Returns the virtual key number of the pressed main key.

  * *KeyStatusEnum statusNum()*  
    Likely the most important.
    Returns the number indicating the current key event status.


### Options

The module provides two main options for the UCP3 config: one to set alias key combinations and one to bind combinations to events.
The following list will briefly explain the structure.

* **alias** - Used to define alias.
  * Aliases are defined as key-value pairs under the **alias** option using KeyStrings.
    The key is the alias that is typed by the user, the value the combination received by the key handler of Crusader. Examples:  
    "CONTROL+B": "B"  
    "B": "CONTROL+B"  
    These would switch the barracks shortcut, so that pressing "B" would not jump to the barracks and only "CONTROL+B" would.
    Note that these changes simply trick Crusaders handler and some rare combinations might cause issues.


* **functions** - Configure key combinations for functions that may be registered.
  * The **functions** option requires two levels under it. First, the key map is given via name. Example:  
    "": 
    * `""` is the name of the default key map. Under this key, the functions key combinations are now defined as key-value pairs.
      The key is the key combination as KeyString, the value the name of the event function to call. Examples:  
      "CONTROL+SHIFT+ALT+SPACE": secretMsgCpp  
      "SHIFT+CONTROL+SPACE": secretMsgLua  
      Both of these would trigger debug messages on key presses.
      The Cpp one prints one for every status, the Lua one only on KEY_DOWN.
    

### Special Thanks

To all of the UCP Team, the [Ghidra project](https://github.com/NationalSecurityAgency/ghidra) and
of course to [Firefly Studios](https://fireflyworlds.com/), the creators of Stronghold Crusader.
