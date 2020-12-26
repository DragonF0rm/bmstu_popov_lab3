#include <LPC23xx.H>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

enum UART_err_t {
	OK = 0,
	ERR_ALREADY_INITIALIZED = 1,
	ERR_BUFFER_OVERFLOW = 2
};

enum UART_err_t UART0_read_line(char *buf, size_t buf_len);
void UART0_write_line(const char *str);

typedef void (*UART_interrupt_handler)(void) __irq;

enum UART_err_t UART0_reg_int_handler(UART_interrupt_handler h);
enum UART_err_t UART_init (void);
