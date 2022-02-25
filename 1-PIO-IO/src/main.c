/**
 * 5 semestre - Eng. da Computação - Insper
 * Rafael Corsi - rafael.corsi@insper.edu.br
 *
 * Projeto 0 para a placa SAME70-XPLD
 *
 * Objetivo :
 *  - Introduzir ASF e HAL
 *  - Configuracao de clock
 *  - Configuracao pino In/Out
 *
 * Material :
 *  - Kit: ATMEL SAME70-XPLD - ARM CORTEX M7
 */

/************************************************************************/
/* includes                                                             */
/************************************************************************/

#include "asf.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/

#define LED_PIO		PIOC       // Associa automaticamente ao ID do PIOC    
#define LED_PIO_ID	ID_PIOC 
#define LED_PIO_IDX   8    
#define LED_PIO_IDX_MASK  (1 << LED_PIO_IDX)

// Defines para o botão
#define BUT_PIO		PIOA
#define BUT_PIO_ID	ID_PIOA  // 10
#define BUT_PIO_IDX	11
#define BUT_PIO_IDX_MASK (1u << BUT_PIO_IDX)


/*
######### BOTÕES / LED externos 

# BTN 1 -- pino 9 -- PD28
# BTN 2 -- pino 3 -- PC31
# BTN 3 -- pino 4 -- PA19

# LED 1 -- pino7 -- PA0
# LED 2 -- pino8 -- PC30
# LED 3 -- pino6 -- PB2

*/

// BTN1
#define BTN1_PIO		PIOD
#define BTN1_PIO_ID		ID_PIOD
#define BTN1_PIO_IDX	28
#define BTN1_PIO_IDX_MASK (1u << BTN1_PIO_IDX)

// BTN2
#define BTN2_PIO		PIOC
#define BTN2_PIO_ID		ID_PIOC
#define BTN2_PIO_IDX	31
#define BTN2_PIO_IDX_MASK (1u << BTN2_PIO_IDX)

// BTN3
#define BTN3_PIO		PIOA
#define BTN3_PIO_ID		ID_PIOA
#define BTN3_PIO_IDX	19
#define BTN3_PIO_IDX_MASK (1u << BTN3_PIO_IDX)

// LED1
#define LED1_PIO		PIOA
#define LED1_PIO_ID		ID_PIOA
#define LED1_PIO_IDX	0
#define LED1_PIO_IDX_MASK (1u << LED1_PIO_IDX)

// LED2
#define LED2_PIO		PIOC
#define LED2_PIO_ID		ID_PIOC
#define LED2_PIO_IDX	30
#define LED2_PIO_IDX_MASK (1u << LED2_PIO_IDX)

// LED3
#define LED3_PIO		PIOB
#define LED3_PIO_ID		ID_PIOB
#define LED3_PIO_IDX	2
#define LED3_PIO_IDX_MASK (1u << LED3_PIO_IDX)

/************************************************************************/
/* constants                                                            */
/************************************************************************/

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/

/************************************************************************/
/* prototypes                                                           */
/************************************************************************/

void init(void);
void pisca(Pio * Pio_ID_led, uint32_t led_mask);

/************************************************************************/
/* interrupcoes                                                         */
/************************************************************************/

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

// Função de inicialização do uC
void init(void)
{
	// Initialize the board clock
	sysclk_init();

	// Desativa WatchDog Timer
	WDT->WDT_MR = WDT_MR_WDDIS;
	
	
	/******  Ativa os periféricos desejados, liberando o CLOCK p/ eles ******/
	pmc_enable_periph_clk(LED_PIO_ID); // LED principal
	pmc_enable_periph_clk(BUT_PIO_ID); // Botão SW300
	
	pmc_enable_periph_clk(BTN1_PIO_ID); // Botão externo 1
	pmc_enable_periph_clk(BTN2_PIO_ID); // Botão externo 2
	pmc_enable_periph_clk(BTN3_PIO_ID); // Botão externo 3
	
	pmc_enable_periph_clk(LED1_PIO_ID); // LED externo 1
	pmc_enable_periph_clk(LED2_PIO_ID); // LED externo 2
	pmc_enable_periph_clk(LED3_PIO_ID); // LED externo 3
	
	
	/******  Configura os pinos como INPUT / OUTPUT  ******/
	pio_set_output(LED_PIO, LED_PIO_IDX_MASK, 0, 0, 0); // Inicializa o PIOC8 como saída
	pio_set_input(BUT_PIO, BUT_PIO_IDX_MASK, PIO_DEFAULT); // Inicializa o Botão (SW300) com input
	
	
	/* Configura os pinos dos botões como INPUT */
	pio_set_input(BTN1_PIO, BTN1_PIO_IDX_MASK, PIO_DEFAULT);
	pio_set_input(BTN2_PIO, BTN2_PIO_IDX_MASK, PIO_DEFAULT);
	pio_set_input(BTN3_PIO, BTN3_PIO_IDX_MASK, PIO_DEFAULT);
	
	/* Configura os pinos dos LEDs como OUTPUT */
	pio_set_output(LED1_PIO, LED1_PIO_IDX_MASK, 0,0,0);
	pio_set_output(LED2_PIO, LED2_PIO_IDX_MASK, 0,0,0);
	pio_set_output(LED3_PIO, LED3_PIO_IDX_MASK, 0,0,0);
	
	/* Ajusta pull_up */
	pio_pull_up(BUT_PIO, BUT_PIO_IDX_MASK, PIO_DEFAULT); // Ajusta BTN com pull_up
	pio_pull_up(BTN1_PIO, BTN1_PIO_IDX_MASK, 1);
	pio_pull_up(BTN2_PIO, BTN2_PIO_IDX_MASK, 1);
	pio_pull_up(BTN3_PIO, BTN3_PIO_IDX_MASK, 1);
}


/************************************************************************/
/* Main                                                                 */
/************************************************************************/


void pisca(Pio * Pio_ID_led, uint32_t led_mask) {
	 int i;
	 for(i=0; i<5; i++) {
		 // LED apagado
		 pio_set(Pio_ID_led, led_mask);
		 delay_ms(500);
		 
		 // LED aceso
		 pio_clear(Pio_ID_led, led_mask);
		 delay_ms(500);
	 }
	
}
// Funcao principal chamada na inicalizacao do uC.
int main(void)
{
  init();

  // super loop
  // aplicacoes embarcadas não devem sair do while(1).
  while (1)
  {
	  pio_set(LED1_PIO, LED1_PIO_IDX_MASK);
	  pio_set(LED2_PIO, LED2_PIO_IDX_MASK);
	  pio_set(LED3_PIO, LED3_PIO_IDX_MASK); 
	  
	 int pressed1 = pio_get(BTN1_PIO, PIO_INPUT, BTN1_PIO_IDX_MASK);
	 if (pressed1 == 0) {
		pisca(LED1_PIO, LED1_PIO_IDX_MASK);
	 } else {
		 pio_set(LED1_PIO, LED1_PIO_IDX_MASK);
	 }
	 
	int pressed2 = pio_get(BTN2_PIO, PIO_INPUT, BTN2_PIO_IDX_MASK);
	if (pressed2 == 0) {
		pisca(LED2_PIO, LED2_PIO_IDX_MASK);
		} else {
		pio_set(LED2_PIO, LED2_PIO_IDX_MASK);
	}
	 
	 int pressed3 = pio_get(BTN3_PIO, PIO_INPUT, BTN3_PIO_IDX_MASK);
	 if (pressed3 == 0) {
		pisca(LED3_PIO, LED3_PIO_IDX_MASK);
		 } else {
		 pio_set(LED3_PIO, LED3_PIO_IDX_MASK);
	 }
	
  }
  return 0;
}
