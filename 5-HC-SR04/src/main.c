#include <asf.h>
#include <string.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"



/************************************************************************/
/* defines                                                              */
/************************************************************************/
#include "pins_config.h"


/************************************************************************/
/* VAR globais                                                          */
/************************************************************************/



/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/
void set_trigger();

void PIN_output_init(Pio *pio, uint32_t pin_id, uint32_t pin_mask, int initial_state);
void PIN_input_init(Pio *pio, uint32_t btn_id, uint32_t btn_mask);
void BTN_interrupt_init(Pio *pio, uint32_t btn_id, uint32_t btn_mask, void (*callback)(void), uint32_t IRQ_type, int prioridade); 

void pisca_led(Pio *pio, uint32_t led_mask);


/************************************************************************/
/* Handlers && Callbacks                                                */
/************************************************************************/
void set_trigger() {
	pio_set(TRIGGER_PIO, TRIGGER_PIO_IDX_MASK);
	delay_us(10);
	pio_clear(TRIGGER_PIO, TRIGGER_PIO_IDX_MASK);
}


/************************************************************************/
/* Funcoes                                                              */
/************************************************************************/
void PIN_output_init(Pio *pio, uint32_t pin_id, uint32_t pin_mask, int initial_state) {
	pmc_enable_periph_clk(pin_id);
	pio_set_output(pio, pin_mask, initial_state, 0,0);
}

void PIN_input_init(Pio *pio, uint32_t btn_id, uint32_t btn_mask) {
	pmc_enable_periph_clk(btn_id);
	pio_set_input(pio, btn_mask, PIO_DEBOUNCE); // REMOVE PULL_UP
	pio_set_debounce_filter(pio, btn_mask, 60);
}

void BTN_interrupt_init(Pio *pio, uint32_t btn_id, uint32_t btn_mask, void (*callback)(void), uint32_t IRQ_type, int prioridade) {
	pio_enable_interrupt(pio, btn_mask);
	NVIC_EnableIRQ(btn_id);
	NVIC_SetPriority(btn_id, prioridade);
	
	pio_handler_set(
	pio,
	btn_id,
	btn_mask, 
	IRQ_type,
	*callback);
}

void pisca_led(Pio *pio, uint32_t led_mask) {
	// Altera o status do LED de 0 p/ 1 e vice-versa.
	if (pio_get_output_data_status(pio, led_mask)) {
		pio_clear(pio, led_mask);
	} else {
		pio_set(pio, led_mask);
	}
}






/************************************************************************/
/* Main Code	                                                        */
/************************************************************************/
int main (void)
{
	board_init();
	sysclk_init();
	delay_init();

	  // Init OLED
	  gfx_mono_ssd1306_init();
  
	// Configura pinos TRIGGER e LEDs como SAÍDA
	PIN_output_init(LED_PIO, LED_PIO_ID, LED_PIO_IDX_MASK, 0);
	PIN_output_init(TRIGGER_PIO, TRIGGER_PIO_ID, TRIGGER_PIO_IDX, 0);
	
	// Configura pino ECHO como ENTRADA
	PIN_input_init(ECHO_PIO, ECHO_PIO_ID, ECHO_PIO_IDX_MASK);
	
	// Configura BTN 1 como Input e Interrupção para gerar pulso em Trigger
	PIN_input_init(BTN1_PIO, BTN1_PIO_ID, BTN1_PIO_IDX_MASK);
	BTN_interrupt_init(BTN1_PIO, BTN1_PIO_ID, BTN1_PIO_IDX_MASK, &set_trigger, PIO_IT_FALL_EDGE, 3); // Previous Song
	
		
	//gfx_mono_draw_string(playlist[current_song]->name, 0, 0, &sysfont);
	//sprintf(display, "%d/%d", current_song+1, NUMBER_OF_SONGS);
	//gfx_mono_draw_string(display, 0, 16, &sysfont);
		
	while(1) {
			
		
	
	
	} // end while
}
