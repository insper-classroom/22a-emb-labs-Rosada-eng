#include "conf_board.h"
#include <asf.h>

/************************************************************************/
/* BOARD CONFIG                                                         */
/************************************************************************/

#define BUT_PIO PIOA
#define BUT_PIO_ID ID_PIOA
#define BUT_PIO_PIN 11
#define BUT_PIO_PIN_MASK (1 << BUT_PIO_PIN)

#define LED_PIO PIOC
#define LED_PIO_ID ID_PIOC
#define LED_PIO_IDX 8
#define LED_IDX_MASK (1 << LED_PIO_IDX)

// BTN1
#define BTN1_PIO		PIOD
#define BTN1_PIO_ID		ID_PIOD
#define BTN1_PIO_IDX	28
#define BTN1_PIO_IDX_MASK (1u << BTN1_PIO_IDX)


#define USART_COM_ID ID_USART1
#define USART_COM USART1

/************************************************************************/
/* RTOS                                                                */
/************************************************************************/

#define TASK_LED_STACK_SIZE (4096 / sizeof(portSTACK_TYPE))
#define TASK_LED_STACK_PRIORITY (tskIDLE_PRIORITY)
#define TASK_BUT_STACK_SIZE (4096 / sizeof(portSTACK_TYPE))
#define TASK_BUT_STACK_PRIORITY (tskIDLE_PRIORITY)

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
                                          signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);

void BTN_init(Pio *pio, uint32_t btn_id, uint32_t btn_mask);
void BTN_interrupt_init(Pio *pio, uint32_t btn_id, uint32_t btn_mask, int prioridade); 
void btn1_callback(void);

/************************************************************************/
/* recursos RTOS                                                        */
/************************************************************************/

/** Semaforo a ser usado pela task led */
SemaphoreHandle_t xSemaphoreBut;

/** Queue for msg log send data */
QueueHandle_t xQueueLedFreq;
QueueHandle_t xQueueLedInc;

/************************************************************************/
/* prototypes local                                                     */
/************************************************************************/

void but_callback(void);
static void BUT_init(void);
void pin_toggle(Pio *pio, uint32_t mask);
static void USART1_init(void);
void LED_init(int estado);

/************************************************************************/
/* RTOS application funcs                                               */
/************************************************************************/

/**
 * \brief Called if stack overflow during execution
 */
extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
                                          signed char *pcTaskName) {
  printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
  /* If the parameters have been corrupted then inspect pxCurrentTCB to
   * identify which task has overflowed its stack.
   */
  for (;;) {
  }
}

/**
 * \brief This function is called by FreeRTOS idle task
 */
extern void vApplicationIdleHook(void) { pmc_sleep(SAM_PM_SMODE_SLEEP_WFI); }

/**
 * \brief This function is called by FreeRTOS each tick
 */
extern void vApplicationTickHook(void) {}

extern void vApplicationMallocFailedHook(void) {
  /* Called if a call to pvPortMalloc() fails because there is insufficient
  free memory available in the FreeRTOS heap.  pvPortMalloc() is called
  internally by FreeRTOS API functions that create tasks, queues, software
  timers, and semaphores.  The size of the FreeRTOS heap is set by the
  configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */

  /* Force an assert. */
  configASSERT((volatile void *)NULL);
}

/************************************************************************/
/* handlers / callbacks                                                 */
/************************************************************************/

void but_callback(void) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(xSemaphoreBut, &xHigherPriorityTaskWoken);
	  
}

void btn1_callback(void) {
	// Envia um incremento para fila Inc
	// OBS.: Deve-se utilizar ISR, para enviar de uma interrup????o
	uint32_t incTicks = 25;
	xQueueSendFromISR(xQueueLedInc, (void *)&incTicks, 10 );
}

/************************************************************************/
/* TASKS                                                                */
/************************************************************************/
static void task_but(void *pvParameters) {

  /* iniciliza botao */
  BUT_init();
   BTN_init(BTN1_PIO, BTN1_PIO_ID, BTN1_PIO_IDX_MASK);
   BTN_interrupt_init(BTN1_PIO, BTN1_PIO_ID, BTN1_PIO_IDX_MASK, 5);
   pio_handler_set(
   BTN1_PIO,
   BTN1_PIO_ID,
   BTN1_PIO_IDX_MASK,
   PIO_IT_FALL_EDGE,
   btn1_callback);
   
   
  uint32_t inc = 0;
  uint32_t delayTicks = 2000;

  for (;;) {
	if (xQueueReceive(xQueueLedInc,(void *) &inc, 10)) {
		/* chegou novo valor, atualiza delay com Incremento recebido ! */
		
		delayTicks -= inc;
		printf("Novo valor de Ticks: %d \n", delayTicks);
		  
		/* envia nova frequencia para a task_led */
		xQueueSend(xQueueLedFreq, (void *)&delayTicks, 10);
		printf("task_but: %d \n", delayTicks);
	} // if xQueue
  } // for
} // but_task

static void task_led(void *pvParameters) {

  LED_init(1);

  uint32_t msg = 0;
  uint32_t delayMs = 2000;

  /* tarefas de um RTOS n??o devem retornar */
  for (;;) {
    /* verifica se chegou algum dado na queue, e espera por 0 ticks */
    if (xQueueReceive(xQueueLedFreq, &msg, (TickType_t) 0)) {
      /* chegou novo valor, atualiza delay ! */
      /* aqui eu poderia verificar se msg faz sentido (se esta no range certo)
       */
      /* converte ms -> ticks */
      delayMs = msg / portTICK_PERIOD_MS;
      printf("task_led: %d \n", delayMs);
    }

    /* pisca LED */
    pin_toggle(LED_PIO, LED_IDX_MASK);

    /* suspende por delayMs */
    vTaskDelay(delayMs);
  }
}


/************************************************************************/
/* funcoes                                                              */
/************************************************************************/
void BTN_init(Pio *pio, uint32_t btn_id, uint32_t btn_mask) {
	pmc_enable_periph_clk(btn_id);
	pio_set_input(pio, btn_mask, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(pio, btn_mask, 60);
}

void BTN_interrupt_init(Pio *pio, uint32_t btn_id, uint32_t btn_mask, int prioridade) {
	pio_enable_interrupt(pio, btn_mask);
	NVIC_EnableIRQ(btn_id);
	NVIC_SetPriority(btn_id, prioridade);
}

/**
 * \brief Configure the console UART.
 */
static void configure_console(void) {
  const usart_serial_options_t uart_serial_options = {
      .baudrate = CONF_UART_BAUDRATE,
      .charlength = CONF_UART_CHAR_LENGTH,
      .paritytype = CONF_UART_PARITY,
      .stopbits = CONF_UART_STOP_BITS,
  };

  /* Configure console UART. */
  stdio_serial_init(CONF_UART, &uart_serial_options);

  /* Specify that stdout should not be buffered. */
  setbuf(stdout, NULL);
}

void pin_toggle(Pio *pio, uint32_t mask) {
  if (pio_get_output_data_status(pio, mask))
    pio_clear(pio, mask);
  else
    pio_set(pio, mask);
}

void LED_init(int estado){
	pmc_enable_periph_clk(LED_PIO_ID);
	pio_set_output(LED_PIO, LED_IDX_MASK, estado, 0, 0);
};


static void BUT_init(void) {
  // Configura PIO para lidar com o pino do bot??o como entrada
  // com pull-up
  pio_configure(BUT_PIO, PIO_INPUT, BUT_PIO_PIN_MASK, PIO_PULLUP);

  // Configura interrup????o no pino referente ao botao e associa
  // fun????o de callback caso uma interrup????o for gerada
  // a fun????o de callback ?? a: but_callback()
  pio_handler_set(BUT_PIO,
                  BUT_PIO_ID,
                  BUT_PIO_PIN_MASK,
                  PIO_IT_FALL_EDGE,
                  but_callback);

  // Ativa interrup????o e limpa primeira IRQ gerada na ativacao
  pio_enable_interrupt(BUT_PIO, BUT_PIO_PIN_MASK);
  pio_get_interrupt_status(BUT_PIO);
  
  // Configura NVIC para receber interrupcoes do PIO do botao
  // com prioridade 4 (quanto mais pr??ximo de 0 maior)
  NVIC_EnableIRQ(BUT_PIO_ID);
  NVIC_SetPriority(BUT_PIO_ID, 4); // Prioridade 4
}

/************************************************************************/
/* main                                                                 */
/************************************************************************/

/**
 *  \brief FreeRTOS Real Time Kernel example entry point.
 *
 *  \return Unused (ANSI-C compatibility).
 */
int main(void) {
  /* Initialize the SAM system */
  sysclk_init();
  board_init();
  configure_console();
	
  printf("Sys init ok \n");
  
 

  /* Attempt to create a semaphore. */
  xSemaphoreBut = xSemaphoreCreateBinary();
  if (xSemaphoreBut == NULL)
    printf("falha em criar o semaforo \n");

  /* cria queue com 32 "espacos" */
  /* cada espa??o possui o tamanho de um inteiro*/
  xQueueLedFreq = xQueueCreate(32, sizeof(uint32_t));
  if (xQueueLedFreq == NULL)
    printf("falha em criar a queue \n");
	
	/* Fila de incrementos */
	xQueueLedInc = xQueueCreate(100, sizeof(uint32_t));
	if (xQueueLedFreq == NULL)
	printf("falha em criar a queue \n");
	

  /* Create task to make led blink */
  if (xTaskCreate(task_led, "Led", TASK_LED_STACK_SIZE, NULL,
                  TASK_LED_STACK_PRIORITY, NULL) != pdPASS) {
    printf("Failed to create test led task\r\n");
  } else {
     printf("task led created \r\n");
	  
  }
  /* Create task to monitor processor activity */
  if (xTaskCreate(task_but, "BUT", TASK_BUT_STACK_SIZE, NULL,
                  TASK_BUT_STACK_PRIORITY, NULL) != pdPASS) {
    printf("Failed to create UartTx task\r\n");
  } else {
     printf("task led but \r\n");  
  }

  /* Start the scheduler. */
  vTaskStartScheduler();

  /* RTOS n??o deve chegar aqui !! */
  while (1) {
  }

  /* Will only get here if there was insufficient memory to create the idle
   * task. */
  return 0;
}
