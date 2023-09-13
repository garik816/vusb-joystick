#ifndef _EasyHID_h
#define _EasyHID_h

#include <Arduino.h>
#include "HIDPrivate.h"

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif

class EasyHID {
public:                         // Класс HID
    void begin(void);           // Инициализация шины USB
    void end(void);             // Отключение шины USB
    void tick(void);            // Поллинг шины (вызывать не реже чем раз в 10мс!)
    bool isConnected(void);     // Статус шины
    bool isNumLock(void);       // Проверка numLock
    bool isCapsLock(void);      // Проверка capsLock
    bool isScrollLock(void);    // Проверка scrollLock
};

extern EasyHID HID;
#endif