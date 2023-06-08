# arduino-string (string_asukiaaa)

This library supplies useful functions to manage string on Arduino.

## Usage

See [an example project for string](./examples/padString/padString.ino).

Include
```c
#include <string_asukiaaa.h>
```

padStart and padEnd
```c
// strA becomes "0000000123"
String strA = string_asukiaaa::padStart("123", 10, '0'));
// strB becomes "bb--------"
String strB = string_asukiaaa::padEnd("bb", 10, '-'));
```

padNumStart and padNumEnd
```c
// strA becomes "0000000123"
String strA = string_asukiaaa::padNumStart(123, 10, '0'));
// strB becomes "99--------"
String strB = string_asukiaaa::padNumEnd(99, 10, '-'));
```

yesNo and trueFalse
```c
string_asukiaaa::yesNo(true); // returns "yes"
string_asukiaaa::trueFalse(false): // returns "false"
```

## License

MIT

## Reference

- [Add padStart and padEnd | ArduinoCore-avr](https://github.com/arduino/ArduinoCore-avr/pull/392)
