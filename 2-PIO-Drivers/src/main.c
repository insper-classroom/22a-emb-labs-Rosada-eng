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
#define HALF_FREQUENCY  160000

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

// CONFIGURAÇÕES DEFAULT DOS PINOS:
#define _PIO_DEFAULT		(0u << 0)
#define _PIO_PULLUP			(1u << 0)		// padrão: pullup ativado
#define _PIO_DEGLITCH		(1u << 1)		// padrão: deglitch ativado
#define _PIO_DEBOUNCE		(1u << 3)		// padrão: debounce ativado
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
void _pio_set(Pio *p_pio, const uint32_t ul_mask);
void _pio_clear(Pio *p_pio, const uint32_t ul_mask);
void _pio_pull_up(Pio *p_pio, const uint32_t ul_mask, const uint32_t ul_pull_up_enable);
void _pio_set_input(Pio *p_pio, const uint32_t ul_mask, const uint32_t ul_attribute);
void _pio_set_output(Pio *p_pio, const uint32_t ul_mask,const uint32_t ul_default_level,const uint32_t ul_multidrive_enable,const uint32_t ul_pull_up_enable);
int _pio_get(Pio *p_pio, const pio_type_t ul_type, const uint32_t ul_mask);
void _delay_ms(uint32_t ms);

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
	_pio_set_input(BUT_PIO, BUT_PIO_IDX_MASK, _PIO_PULLUP | _PIO_DEBOUNCE); // Inicializa o Botão (SW300) com input
	
	
	/* Configura os pinos dos botões como INPUT */
	_pio_set_input(BTN1_PIO, BTN1_PIO_IDX_MASK, _PIO_PULLUP | _PIO_DEBOUNCE);
	_pio_set_input(BTN2_PIO, BTN2_PIO_IDX_MASK, _PIO_PULLUP | _PIO_DEBOUNCE);
	_pio_set_input(BTN3_PIO, BTN3_PIO_IDX_MASK, _PIO_PULLUP | _PIO_DEBOUNCE);
	
	/* Configura os pinos dos LEDs como OUTPUT */
	pio_set_output(LED1_PIO, LED1_PIO_IDX_MASK, 0,0,0);
	pio_set_output(LED2_PIO, LED2_PIO_IDX_MASK, 0,0,0);
	pio_set_output(LED3_PIO, LED3_PIO_IDX_MASK, 0,0,0);
	
	/* Ajusta pull_up */
	//_pio_pull_up(BUT_PIO, BUT_PIO_IDX_MASK, PIO_DEFAULT); // Ajusta BTN com pull_up
	//_pio_pull_up(BTN1_PIO, BTN1_PIO_IDX_MASK, 1);
	//_pio_pull_up(BTN2_PIO, BTN2_PIO_IDX_MASK, 1);
	//_pio_pull_up(BTN3_PIO, BTN3_PIO_IDX_MASK, 1);
}


/************************************************************************/
/* Main                                                                 */
/************************************************************************/
void _delay_ms(uint32_t ms) {
	for (uint32_t i=0; i < (ms* HALF_FREQUENCY) ; i ++ ){
		asm("nop");
	}
}

void pisca(Pio * Pio_ID_led, uint32_t led_mask) {
	 int i;
	 for(i=0; i<5; i++) {
		 // LED apagado
		 _pio_set(Pio_ID_led, led_mask);
		 _delay_ms(200);
		 
		 // LED aceso
		 _pio_clear(Pio_ID_led, led_mask);
		 _delay_ms(200);
	 }
	
}

void _pio_set(Pio *p_pio, const uint32_t ul_mask){
	p_pio->PIO_SODR = ul_mask;
}

void _pio_clear(Pio *p_pio, const uint32_t ul_mask) {
	p_pio->PIO_CODR = ul_mask;
}

void _pio_pull_up(Pio *p_pio, const uint32_t ul_mask, const uint32_t ul_pull_up_enable) {
	if (ul_pull_up_enable) {
		p_pio->PIO_PUER = ul_mask;
	} else {
		p_pio->PIO_PUDR = ul_mask;
	}	
}

void _pio_set_input(Pio *p_pio, const uint32_t ul_mask, const uint32_t ul_attribute) {
	// Configura pino como input (set output = 0)
	p_pio->PIO_CODR = ul_mask;
	
	// Checa bit de pull up:
	if (ul_attribute & (1u << 0)) {
		// Configura pull_up:
		_pio_pull_up(p_pio, ul_mask, 1);
	}
	if (ul_attribute & (1u << 3)) {
		// Configura debounce
		p_pio->PIO_IFSCER = ul_mask;
	}
}

void _pio_set_output(Pio *p_pio, const uint32_t ul_mask, 
					const uint32_t ul_default_level,
					const uint32_t ul_multidrive_enable,
					const uint32_t ul_pull_up_enable) {
		
		// Configura para ser controlado pelo PIO
		p_pio->PIO_PER = ul_mask;
		
		// Configura pino como OUTPUT
		p_pio->PIO_OER = ul_mask;
		
		// Configura a saída inicial (0 ou 1)
		if (ul_default_level) {
			_pio_set(p_pio, ul_mask);
		} else {
			_pio_clear(p_pio, ul_mask);
		}
		
		// Configura o Multidrive
		if (ul_multidrive_enable) {
			p_pio->PIO_MDER = ul_mask;
		} else {
			p_pio->PIO_MDDR = ul_mask;
		}
		
		// se ul_pull_up_enable = 1, ativa pull up. Caso contrário, não ativa.
		_pio_pull_up(p_pio, ul_mask, ul_pull_up_enable);
	}
	
int _pio_get(Pio *p_pio, const pio_type_t ul_type, const uint32_t ul_mask) {
	if (ul_type == PIO_OUTPUT_0) {
		if (p_pio->PIO_ODSR & ul_mask) {
			return 1;
		} else {
			return 0;
		}
	} else if (ul_type == PIO_INPUT) {
		if (p_pio->PIO_PDSR & ul_mask) {
			return 1;
	} else 	{
		return 0;
	}
	} else {
		return 0;
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
	  _pio_set(LED1_PIO, LED1_PIO_IDX_MASK);
	  _pio_set(LED2_PIO, LED2_PIO_IDX_MASK);
	  _pio_set(LED3_PIO, LED3_PIO_IDX_MASK); 
	  
	 int pressed1 = _pio_get(BTN1_PIO, PIO_INPUT, BTN1_PIO_IDX_MASK);
	 if (pressed1 == 0) {
		pisca(LED1_PIO, LED1_PIO_IDX_MASK);
	 } else {
		 _pio_set(LED1_PIO, LED1_PIO_IDX_MASK);
	 }
	 
	int pressed2 = _pio_get(BTN2_PIO, PIO_INPUT, BTN2_PIO_IDX_MASK);
	if (pressed2 == 0) {
		pisca(LED2_PIO, LED2_PIO_IDX_MASK);
		} else {
		_pio_set(LED2_PIO, LED2_PIO_IDX_MASK);
	}
	 
	 int pressed3 = _pio_get(BTN3_PIO, PIO_INPUT, BTN3_PIO_IDX_MASK);
	 if (pressed3 == 0) {
		pisca(LED3_PIO, LED3_PIO_IDX_MASK);
		 } else {
		 _pio_set(LED3_PIO, LED3_PIO_IDX_MASK);
	 }
	
  }
  return 0;
}
