#include <asf.h>
#include "conf_board.h"

/************************************************************************/
/* DEFINES                                                              */
/************************************************************************/

#define TASK_MONITOR_STACK_SIZE            (2048/sizeof(portSTACK_TYPE))
#define TASK_MONITOR_STACK_PRIORITY        (tskIDLE_PRIORITY)
#define TASK_LED_STACK_SIZE                (1024/sizeof(portSTACK_TYPE))
#define TASK_LED_STACK_PRIORITY            (configMAX_PRIORITIES - 1)

#define BUT_PIO           PIOA
#define BUT_PIO_ID        10
#define BUT_IDX       11u
#define BUT_IDX_MASK  (1u << BUT_IDX)

/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
		signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);
static void task_monitor(void *pvParameters);
static void task_led(void *pvParameters);
void but_callback(void);
void io_init(void);

/************************************************************************/
/* Globals                                                           */
/************************************************************************/

SemaphoreHandle_t xSemaphore;

/************************************************************************/
/* Handlers                                                             */
/************************************************************************/

void but_callback(void){
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	printf("but_callback \n");
	xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);
	printf("semafaro tx \n");
}

/************************************************************************/
/* Tasks                                                              */
/************************************************************************/

static void task_monitor(void *pvParameters)
{
	static portCHAR szList[256];
	UNUSED(pvParameters);

	for (;;) {
		printf("--- task ## %u", (unsigned int)uxTaskGetNumberOfTasks());
		vTaskList((signed portCHAR *)szList);
		printf(szList);
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
}

static void task_led(void *pvParameters)
{
	/* We are using the semaphore for synchronization so we create a binary
        semaphore rather than a mutex.  We must make sure that the interrupt
        does not attempt to use the semaphore before it is created! */
	xSemaphore = xSemaphoreCreateBinary();

	/* devemos iniciar a interrupcao no pino somente apos termos alocado
           os recursos (no caso semaforo), nessa funcao inicializamos 
           o botao e seu callback*/
	io_init();
	
	if (xSemaphore == NULL)
	printf("falha em criar o semaforo \n");
	
	/* Block for 500ms. */
	for (;;) {
		if( xSemaphoreTake(xSemaphore, ( TickType_t ) 500) == pdTRUE ){
			LED_Toggle(LED0);
		}
	}
}

/************************************************************************/
/* Funcoes                                                              */
/************************************************************************/

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
		signed char *pcTaskName)
{
	printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
	/* If the parameters have been corrupted then inspect pxCurrentTCB to
	 * identify which task has overflowed its stack.
	 */
	for (;;) {
	}
}

extern void vApplicationIdleHook(void)
{
	pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
}

extern void vApplicationTickHook(void)
{
}

extern void vApplicationMallocFailedHook(void)
{
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */

	/* Force an assert. */
	configASSERT( ( volatile void * ) NULL );
}

static void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
#if (defined CONF_UART_CHAR_LENGTH)
		.charlength = CONF_UART_CHAR_LENGTH,
#endif
		.paritytype = CONF_UART_PARITY,
#if (defined CONF_UART_STOP_BITS)
		.stopbits = CONF_UART_STOP_BITS,
#endif
	};

	/* Configure console UART. */
	stdio_serial_init(CONF_UART, &uart_serial_options);

	/* Specify that stdout should not be buffered. */
#if defined(__GNUC__)
	setbuf(stdout, NULL);
#else
	/* Already the case in IAR's Normal DLIB default configuration: printf()
	 * emits one character at a time.
	 */
#endif
}

void io_init(void)
{
	// Inicializa clock do periférico PIO responsavel pelo botao
	pmc_enable_periph_clk(BUT_PIO_ID);

	// Configura PIO para lidar com o pino do botão como entrada
	// com pull-up
	pio_configure(BUT_PIO, PIO_INPUT, BUT_IDX_MASK, PIO_PULLUP);

	// Configura interrupção no pino referente ao botao e associa
	// função de callback caso uma interrupção for gerada
	// a função de callback é a: but_callback()
	pio_handler_set(BUT_PIO,
	BUT_PIO_ID,
	BUT_IDX_MASK,
	PIO_IT_FALL_EDGE,
	but_callback);

	// Ativa interrupção
	pio_enable_interrupt(BUT_PIO, BUT_IDX_MASK);

	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(BUT_PIO_ID);
	NVIC_SetPriority(BUT_PIO_ID, 4); // Prioridade 4
}


/************************************************************************/
/* Main Code	                                                        */
/************************************************************************/

int main(void)
{
	/* Initialize the SAM system */
	sysclk_init();
	board_init();

	/* Initialize the console uart */
	configure_console();

	/* Output demo information. */
	printf("-- Freertos Example --\n\r");
	printf("-- %s\n\r", BOARD_NAME);
	printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);


	/* Create task to monitor processor activity */
	if (xTaskCreate(task_monitor, "Monitor", TASK_MONITOR_STACK_SIZE, NULL,
			TASK_MONITOR_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create Monitor task\r\n");
	}

	/* Create task to make led blink */
	if (xTaskCreate(task_led, "Led", TASK_LED_STACK_SIZE, NULL,
			TASK_LED_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create test led task\r\n");
	}

	/* Start the scheduler. */
	vTaskStartScheduler();

	/* Will only get here if there was insufficient memory to create the idle task. */
	return 0;
}
