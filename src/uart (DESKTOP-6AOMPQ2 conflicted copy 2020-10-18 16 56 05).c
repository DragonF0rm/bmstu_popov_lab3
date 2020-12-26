#include "uart.h"

typedef unsigned char byte;

#define UART0_read_ready U0LSR&0x01
#define UART0_write_ready U0LSR&0x20

bool UART_initialized = false;
unsigned UART0_interrupt_handler = 0;

static byte UART0_read_byte(void) {
	byte b = '\0';
	
	while(true) {
		if (UART0_read_ready) {
			b = U0RBR; //Прочитать байт и сбросить прерывание
			break;
		}
	}
	
	return b;
}

static void UART0_write_byte(byte b) {
	while(!UART0_write_ready);
	U0RBR = b;
}

static void UART0_new_line(void) {
	UART0_write_byte('\r');
	UART0_write_byte('\n');
}

enum UART_err_t UART0_read_line(char *buf, size_t buf_len) {
	size_t i = 0;
	char input = '\0';
	
	assert(buf);
	assert(buf_len > 0);
	
	do {
		if (i > buf_len-1) {
			buf[buf_len-1] = '\0';
			UART0_new_line();
			return ERR_BUFFER_OVERFLOW;
		}
		input = UART0_read_byte();
		if (input == '\r')
			input = '\n';
		UART0_write_byte(input); //echo
		buf[i] = input;
		i++;
	} while (buf[i-1] != '\n');
	buf[i] = '\0';
	return OK;
}

void UART0_write_line(const char *str) {
	size_t i = 0;
	
	assert(str);
	
	for (i=0; i<strlen(str); i++) {
		UART0_write_byte(str[i]);
	}
	
	UART0_new_line();
}

enum UART_err_t UART0_reg_int_handler(UART_interrupt_handler h) {
	assert(h);
	
	if (UART_initialized)
		return ERR_ALREADY_INITIALIZED;
	UART0_interrupt_handler = (unsigned) h;
	return OK;
}

enum UART_err_t UART_init (void) {
	if (UART_initialized)
		return ERR_ALREADY_INITIALIZED;
	//Разрешить альтернативные UART0 функции входов/выходов P0.2 и P0.3: TxD и RxD
	PINSEL0 = 0x00000050;
	//Установить параметры передачи: 8 бит, без контроля четности, 1 стоповый бит
	//+Разрешить запись делителя частоты CLK_UART0
	U0LCR = 0x00000083;
	//Установить делитель частоты на скорость 38400 при частоте CLK_UART0 = 15MHz
	U0DLL = 0x00000018;
	//Дополнительный делитель частоты (DivAddVal/MulVal + 1) = 0 + 1 = 1
	U0FDR = 0x00000010;
	//Фиксировать делитель частоты
	U0LCR = 0x00000003;
	//Программировать FIFO буфер на прием 8-ми байт.
	U0FCR = 0x00000081;
	//Разрешить прерывание по приему
	U0IER = 0x00000001;
	if (UART0_interrupt_handler) {
		//Записать адрес обработчика прерывания в таблицу векторов
		VICVectAddr6 = UART0_interrupt_handler;
		//Разрешить прерывания
		VICIntEnable |= 0x00000040;
	}
	UART_initialized = true;
	return OK;
}
