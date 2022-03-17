#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/
// LED
#define LED_PIO      PIOC
#define LED_PIO_ID   ID_PIOC
#define LED_IDX      8
#define LED_IDX_MASK (1 << LED_IDX)

// BUT
#define BUT_PIO		PIOA
#define BUT_PIO_ID	ID_PIOA  // 10
#define BUT_PIO_IDX	11
#define BUT_PIO_IDX_MASK (1u << BUT_PIO_IDX)

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
/* VAR globais                                                          */
/************************************************************************/

/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/

void LED_init(Pio *pio, uint32_t led_id, uint32_t led_mask, int initial_state);
void pisca_led(Pio *pio, uint32_t led_mask);

void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq, int prioridade_irq);
void pin_toggle(Pio *pio, uint32_t mask);

/************************************************************************/
/* Handlers                                                             */
/************************************************************************/
void TC1_Handler(void) {
	/**
	* Devemos indicar ao TC que a interrupção foi satisfeita.
	* Isso é realizado pela leitura do status do periférico
	**/
	volatile uint32_t status = tc_get_status(TC0, 1);

	/** Muda o estado do LED (pisca) **/
	pisca_led(LED1_PIO, LED1_PIO_IDX_MASK);  
}


/************************************************************************/
/* Funcoes                                                              */
/************************************************************************/
void LED_init(Pio *pio, uint32_t led_id, uint32_t led_mask, int initial_state) {
	pmc_enable_periph_clk(led_id);
	pio_set_output(pio, led_mask, initial_state, 0,0);
}

void pisca_led(Pio *pio, uint32_t led_mask) {
	// Altera o status do LED de 0 p/ 1 e vice-versa.
	if (pio_get_output_data_status(pio, led_mask)) {
		pio_clear(pio, led_mask);
	} else {
		pio_set(pio, led_mask);
	}
}

void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq, int prioridade_irq){
	uint32_t ul_div;
	uint32_t ul_tcclks;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();

	/* Configura o PMC */
	pmc_enable_periph_clk(ID_TC);

	/** Configura o TC para operar em  freq hz e interrupçcão no RC compare */
	tc_find_mck_divisor(freq, ul_sysclk, &ul_div, &ul_tcclks, ul_sysclk);
	tc_init(TC, TC_CHANNEL, ul_tcclks | TC_CMR_CPCTRG);
	tc_write_rc(TC, TC_CHANNEL, (ul_sysclk / ul_div) / freq);

	/* Configura NVIC*/
	NVIC_SetPriority(ID_TC, prioridade_irq);
	NVIC_EnableIRQ((IRQn_Type) ID_TC);
	tc_enable_interrupt(TC, TC_CHANNEL, TC_IER_CPCS);
}


/************************************************************************/
/* Main Code	                                                        */
/************************************************************************/
int main (void)
{
	board_init();
	sysclk_init();
	delay_init();
	
	//gfx_mono_ssd1306_init();
	//gfx_mono_draw_filled_circle(20, 16, 16, GFX_PIXEL_SET, GFX_WHOLE);
	//gfx_mono_draw_string("mundo", 50,16, &sysfont);
  
	// Configura LED como OUTPUT e estado inicial como OFF
	LED_init(LED1_PIO, LED1_PIO_ID, LED1_PIO_IDX_MASK, 1);
	LED_init(LED2_PIO, LED2_PIO_ID, LED2_PIO_IDX_MASK, 1);
	LED_init(LED3_PIO, LED3_PIO_ID, LED3_PIO_IDX_MASK, 1);
	
	// Inicializa Timer TC0, canal 1
	TC_init(TC0, ID_TC1, 1, 4, 4);
	// Inicializa contagem do TC0 no canal 1
	tc_start(TC0, 1);
	
	
	while(1) {

			
			
	}
}
