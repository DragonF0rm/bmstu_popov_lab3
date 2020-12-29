#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>

#include "uart.h"
#include "timer.h"

#define CMD_TASK    "task"
#define CMD_EXIT    "exit"
#define CMDBUF_LEN  4

#define TASK_MIN 0
#define TASK_MAX 99

#define CMD_DELAY 1

#define arr_len(_arr) (sizeof(_arr) / sizeof((_arr)[0]))

bool listening = true;
int  current_task = 0;
char bintask_buf[17] = "\0"; //16-bit number + terminator
char hextask_buf[3]  = "\0"; //2-digit hex number + terminator

/* Forward declarations */
static
int
make_task(void);
static
int
dec_to_bin(int dec, char *buf, size_t buf_size);

void
uart_int(void) __irq;

static
int
make_task(void)
{
	return rand() % (TASK_MAX - TASK_MIN + 1) + TASK_MIN;
}

static
int
dec_to_bin(int dec, char *buf, size_t buf_size)
{
	assert(buf);
	assert(buf_size >= 2);

	size_t max_dec = (1 << buf_size) - 1;	
	if(dec > max_dec)
		return -1; //buffer can't handle it, it's too big
  
	size_t digits_num = buf_size - 1;
	for (size_t i = 0; i <= digits_num; i++) {
		buf[buf_size - 2 - i] = (dec & 1) ? '1' : '0';
		dec >>= 1;
	}
	
	buf[buf_size - 1] = '\0';
	return 0;
}

static bool flag_uart_int = false;

void
uart_int(void) __irq
{
	flag_uart_int = true;
}

static bool flag_handle_cmd = true;

void
handle_cmd_n_answer(void)
{
	while(!flag_uart_int);
	flag_uart_int = false;
	
	int answer = 0;
	UART0_int_disable();
	if(!flag_handle_cmd)
		goto handle_answer;
	
	char cmd_buf[CMDBUF_LEN+1] = "\0";
	int err = UART0_read_line(cmd_buf, arr_len(cmd_buf));
	if (err)
		goto error;

	if (!memcmp(cmd_buf, CMD_EXIT, CMDBUF_LEN)) {
		UART0_write_line("-> Good bye!\n");
		listening = false;
		goto cleanup;
	} else if (!memcmp(cmd_buf, CMD_TASK, CMDBUF_LEN)) {
		current_task = make_task();
		dec_to_bin(current_task, bintask_buf, arr_len(bintask_buf));
		UART0_write_line_fmt("-> Task: turn into hex %s\n", bintask_buf);
		flag_handle_cmd = false;
		goto cleanup;
	} else {
		goto error;
	}
	
error:
	UART0_write_line("-> unknown command\n");
cleanup:
	UART0_int_enable();
	return;
	
handle_answer:
	if (UART0_read_line(hextask_buf, arr_len(hextask_buf))) {
		UART0_write_line("-> unable to read answer");
		goto fail;
	}
	
	UART0_write_line("-> Answer check delay...");
	delay(CMD_DELAY);
	
	for (size_t i = 0; i < arr_len(hextask_buf); i++) {
		hextask_buf[i] = tolower(hextask_buf[i]);
	}
	
	answer = (int)strtol(hextask_buf, NULL, 16);
	UART0_write_line_fmt("-> Task: %d, answer: %d", current_task, answer); 
	if (answer != current_task)
		goto fail;
	
	UART0_write_line("-> correct\n");
	goto cleanup_answer;

fail:
	UART0_write_line("-> incorrect\n");
cleanup_answer:
	flag_handle_cmd = true;
	UART0_int_enable();
}

int
main(void)
{
	assert(!UART_init());
	timer0_init();
	srand(0);
	UART0_reg_int_handler(uart_int);
	UART0_int_enable();
	while(listening)
		handle_cmd_n_answer();
	exit(0);
}
