#include "timer.h"

#include <assert.h>
#include <stdbool.h>

static bool timer0_initialized = false;

void
delay(time_t seconds)
{
	assert(timer0_initialized);
	T0MR0 = (time_t)(seconds * 1000);
	//Сбросить таймер
	T0TC = 0x00000000;
	//Запустить таймер
	T0TCR = 0x00000001;
	//Ожидаем окончания счета
	while (T0TCR&0x1);
}

void
timer0_init(void)
{
	assert(!timer0_initialized);
	//Предделитель таймера = 15000
	T0PR = 15000;
	//Сбросить счетчик и делитель
	T0TCR = 0x00000002;
	//При совпадении останавливаем, сбрасываем таймер
	T0MCR = 0x00000006;
	
	timer0_initialized = true;
}
