#include <asf.h>
#include <time.h>
#include <string.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"
/*
TODO: desenhar gráfico: 9 espaços p/ gráfico + 3 numeros + 1 espaço

*/
/* Botão 1 */
#define BUT1_PIO				PIOD
#define BUT1_PIO_ID				ID_PIOD
#define BUT1_PIO_IDX			28
#define BUT1_PIO_IDX_MASK		(1u << BUT1_PIO_IDX)
#define BUT1_PRIORITY			4

/* TRIG - Y */
#define TRIG_PIO				PIOC
#define TRIG_PIO_ID				ID_PIOC
#define TRIG_PIO_IDX			13
#define TRIG_PIO_IDX_MASK	    (1 << TRIG_PIO_IDX)

/* ECHO - Pino X */
#define ECHO_PIO				PIOA
#define ECHO_PIO_ID				ID_PIOA
#define ECHO_PIO_IDX			4
#define ECHO_PIO_IDX_MASK		(1 << ECHO_PIO_IDX)
#define ECHO_PRIORITY			4

volatile char echo_flag;
volatile char but1_flag = 0;


volatile float freq = (float) 1/(0.000058*2);
volatile float delta_t = 0;
volatile float ultimas_medidas[2] = {0.00, 0.00};
	
void echo_callback(void);
void but1_callback(void);
void io_init(void);
void display_oled(freq);
static void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource);

static void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource) {

	uint16_t pllPreScale = (int) (((float) 32768) / freqPrescale);
	
	rtt_sel_source(RTT, false);
	rtt_init(RTT, pllPreScale);
	
	if (rttIRQSource & RTT_MR_ALMIEN) {
		uint32_t ul_previous_time;
		ul_previous_time = rtt_read_timer_value(RTT);
		while (ul_previous_time == rtt_read_timer_value(RTT));
		rtt_write_alarm_time(RTT, IrqNPulses+ul_previous_time);
	}

	/* config NVIC */
	NVIC_DisableIRQ(RTT_IRQn);
	NVIC_ClearPendingIRQ(RTT_IRQn);
	NVIC_SetPriority(RTT_IRQn, 4);
	NVIC_EnableIRQ(RTT_IRQn);

	/* Enable RTT interrupt */
	if (rttIRQSource & (RTT_MR_RTTINCIEN | RTT_MR_ALMIEN)) {
		rtt_enable_interrupt(RTT, rttIRQSource);
	} else {
		rtt_disable_interrupt(RTT, RTT_MR_RTTINCIEN | RTT_MR_ALMIEN);
	}
	
	
}

void but1_callback(void){
	but1_flag = 1;
}

void echo_callback(void) {
	if (echo_flag) {
		delta_t = rtt_read_timer_value(RTT);
	}
	else {
		RTT_init(freq, 0, 0);
	}
	echo_flag = !echo_flag;
}

void calculate_distance() {
	float distance;
	// delta_t e freq global
	if (delta_t) {
		distance = (float) (340*delta_t*100.0)/(2.0*freq);
	} else {
		distance = 0;
	}
	
	ultimas_medidas[1] = ultimas_medidas[0];
	ultimas_medidas[0] = distance;
	
	display_oled();
	
}

void display_oled() {
	char str_distancia[20];
	char str_last_d1[6];
	char str_last_d2[6];
	
	// erro ou acima do alcance
	if (((rtt_get_status(RTT) & RTT_SR_ALMS) == RTT_SR_ALMS) || (ultimas_medidas[0] >= 400)) {
		gfx_mono_generic_draw_filled_rect(0, 0, 127, 31, GFX_PIXEL_CLR);
		sprintf(str_distancia, "Erro de Leitura");
		gfx_mono_draw_string(str_distancia, 0,0, &sysfont);
		delay_ms(1000);
		gfx_mono_generic_draw_filled_rect(0, 0, 127, 31, GFX_PIXEL_CLR);
	}
	else {
		sprintf(str_distancia, "%2.2f cm", ultimas_medidas[0]);
		gfx_mono_draw_string(str_distancia, 0, 0, &sysfont);
		
		int d1 = (int) ultimas_medidas[0] * 88 / 400 ;
		int d2 = (int) ultimas_medidas[1] * 88 / 400 ;
		
		gfx_mono_generic_draw_filled_rect(0, 16, d1 , 5, GFX_PIXEL_SET);
		gfx_mono_generic_draw_filled_rect(0, 22, d2 , 5, GFX_PIXEL_SET);
			
	}
}

void io_init(void) {
	
	board_init();
	
	sysclk_init();

	WDT->WDT_MR = WDT_MR_WDDIS;
	
	pmc_enable_periph_clk(BUT1_PIO_ID);
	pmc_enable_periph_clk(TRIG_PIO);
	pmc_enable_periph_clk(ECHO_PIO);
	
	// configura input e output
	pio_set_input(ECHO_PIO,ECHO_PIO_IDX_MASK,PIO_DEFAULT);
	pio_configure(TRIG_PIO, PIO_OUTPUT_0,TRIG_PIO_IDX_MASK, PIO_DEFAULT);
	pio_configure(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK, PIO_DEBOUNCE | PIO_PULLUP);
	pio_set_debounce_filter(BUT1_PIO, BUT1_PIO_IDX_MASK, 60);
	
	// configura interrupções
	pio_handler_set(ECHO_PIO, ECHO_PIO_ID, ECHO_PIO_IDX_MASK, PIO_IT_EDGE, echo_callback);
	pio_enable_interrupt(ECHO_PIO, ECHO_PIO_IDX_MASK);
	pio_get_interrupt_status(ECHO_PIO);
	NVIC_EnableIRQ(ECHO_PIO_ID);
	NVIC_SetPriority(ECHO_PIO_ID, ECHO_PRIORITY);
	
	pio_handler_set(BUT1_PIO, BUT1_PIO_ID, BUT1_PIO_IDX_MASK, PIO_IT_FALL_EDGE, but1_callback);
	pio_enable_interrupt(BUT1_PIO, BUT1_PIO_IDX_MASK);
	pio_get_interrupt_status(BUT1_PIO);
	NVIC_EnableIRQ(BUT1_PIO_ID);
	NVIC_SetPriority(BUT1_PIO_ID, BUT1_PRIORITY);
}

int main (void)
{
	io_init();

	delay_init();

	gfx_mono_ssd1306_init();
	gfx_mono_draw_string("apertePMedir", 0,0, &sysfont);
	
	while(1) {
		if (but1_flag) {
			but1_flag = 0;
			pio_set(TRIG_PIO, TRIG_PIO_IDX_MASK);
			delay_us(10);
			pio_clear(TRIG_PIO, TRIG_PIO_IDX_MASK);
			gfx_mono_draw_filled_rect(0, 0, 128, 32, GFX_PIXEL_CLR);
			calculate_distance();
		}
		
		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
	}
}