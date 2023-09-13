#include <Arduino.h>
#include "EasyHID.h"

EasyHID HID;
void EasyHID::begin(void){
#if (defined (__AVR_ATtiny48__) || defined (__AVR_ATtiny88__))
	pinMode(1, INPUT); 		
    pinMode(2, INPUT);
	
#elif defined (__AVR_ATtiny167__)
	pinMode(3, INPUT); 		
    pinMode(4, INPUT);
	
#elif (defined (__AVR_ATmega48P__) || defined (__AVR_ATmega88P__) ||  \
defined (__AVR_ATmega168P__) || defined (__AVR_ATmega168__) || \
defined (__AVR_ATmega328P__)) || defined (__AVR_ATmega328__)
	pinMode(4, INPUT); 		
    pinMode(2, INPUT); 
	
#elif defined (__AVR_ATmega8__)
	DDRD &=	~(1 << PD2);
	PORTD &= ~(1 << PD2);
	DDRD &= ~(1 << PD4);
	PORTD &= ~(1 << PD4);
#endif
	
    cli();					// Запрет работы прерываний	
    usbDeviceDisconnect();	// Ложный дисконнект usb
    _delay_ms(250);			// Небольшая задержка
    usbDeviceConnect(); 	// Коннект usb 
    usbInit();				// Инициализация usb 
    sei();					// Разрешение работы прерываний	
}

void EasyHID::end(void){	// Отключение usb
     cli();					// Запрет работы прерываний	
	 usbDeviceDisconnect();	// Дисконнект usb
	 sei();					// Разрешение работы прерываний	
}

void EasyHID::tick(void){	
    usbPoll();
}