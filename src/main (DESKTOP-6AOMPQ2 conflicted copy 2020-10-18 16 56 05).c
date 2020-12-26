#include "uart.h"

char str[255],cur_login[9],cur_paswd[9],rx;
int i, str_len;
const char login[9] = "root";
const char paswd[9] = "12345678";

//Обработчик прерываний UART0 RDA и CTI
void on_uart(void) __irq {
	enum UART_err_t err = OK;

	err = UART0_read_line(cur_login, 9);
	if (err)
		goto error;
	
	err = UART0_read_line(cur_paswd, 9);
	if (err)
		goto error;
	
	if ((memcmp(cur_login,login,strlen(login)) == 0)&&
		(memcmp(cur_paswd,paswd,strlen(paswd)) == 0)) {
		//Идентификация закончилась успешно!
		UART0_write_line("-> success");
	} else {
		//Идентификация закончилась неудачей!
		UART0_write_line("-> failure");
	}
	
	goto cleanup;

error:
	UART0_write_line("-> error on interrupt handler");
cleanup:
	VICVectAddr = 0; /*Сбросить VIC*/
}

int main(void) {
	assert(!UART0_reg_int_handler(on_uart));
	assert(!UART_init());
	for (;;){}
}
