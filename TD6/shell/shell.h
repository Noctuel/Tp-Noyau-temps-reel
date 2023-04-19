/*
 * shell.h
 *
 *  Created on: 7 juin 2019
 *      Author: laurent
 */

#ifndef INC_LIB_SHELL_SHELL_H_
#define INC_LIB_SHELL_SHELL_H_

#include <stdint.h>

#define UART_DEVICE huart1

struct h_shell_structure;

#define ARGC_MAX 8
#define BUFFER_SIZE 40
#define SHELL_FUNC_LIST_MAX_SIZE 64

void shell_init();
int shell_add(char c, int (* pfunc)(int argc, char ** argv), char * description);
int shell_run();
// commentaire documentation!
void shell_uart_receive_irq_cb(void);

#endif /* INC_LIB_SHELL_SHELL_H_ */

typedef struct h_shell_structure{
		int shell_func_list_size ;
		char print_buffer[BUFFER_SIZE];
		shell_func_t shell_func_list[SHELL_FUNC_LIST_MAX_SIZE];
		char cmd_buffer[BUFFER_SIZE];
} h_shell_t;


