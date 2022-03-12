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

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/
volatile char led_flag;
volatile char btn2_flag;
volatile char btn3_flag;

volatile char increment_freq;
volatile char decrement_freq_fast;
volatile char decrement_freq_slow;

char str[16];

int delay; // 1 / 30 * 1000 = 33 ms (30 hz)

/************************************************************************/
/* prototype                                                            */
/************************************************************************/
void io_init(void);
void pisca_led(int n, int t);
//void but_callback(void);
void btn1_callback(void);
void btn2_callback(void);
void btn3_callback(void);

/************************************************************************/
/* handler / callbacks                                                  */
/************************************************************************/

//void but_callback(void) {
	//led_flag = 1;
//}

void btn1_callback(void) {
	if (pio_get(BTN1_PIO, PIO_INPUT, BTN1_PIO_IDX_MASK)) {
		// BTN 1 HIGH --> apertou e soltou
		increment_freq = 1;
		decrement_freq_fast = 0;
		
	} else {
		// BTN 1 LOW --> segurando o botão
		decrement_freq_fast = 1;
	}	
}

void btn2_callback(void) {
	if (led_flag) {
		led_flag = 0;	
	} else {
		led_flag = 1;
	}
}

void btn3_callback(void) {
	decrement_freq_slow = 1;
}


/************************************************************************/
/* funções                                                              */
/************************************************************************/

// pisca led N vez no periodo T
void pisca_led(int n, int t){
	// Desenha barra de status
	gfx_mono_draw_horizontal_line(0, 20, 4*(n-1), GFX_PIXEL_SET);
	for (int i = 0; i < n ; i ++) {
		pio_clear(LED_PIO, LED_IDX_MASK);
		delay_ms(t/2);
		pio_set(LED_PIO, LED_IDX_MASK);
		delay_ms(t/2);
		
		// Apaga parte da barra
		gfx_mono_draw_horizontal_line(0, 20, 4*i, GFX_PIXEL_CLR);
		if (led_flag == 0){
			break;
		}
	}
	
}


// *********** INIT *****************
void io_init(void)
{
	// Initialize the board clock
	sysclk_init();

	// Desativa WatchDog Timer
	WDT->WDT_MR = WDT_MR_WDDIS;
	  
	// Enable CLOCK para todos periféricos
	pmc_enable_periph_clk(LED_PIO_ID);
	//pmc_enable_periph_clk(BUT_PIO_ID);
	pmc_enable_periph_clk(BTN1_PIO_ID);
	pmc_enable_periph_clk(BTN2_PIO_ID);
	pmc_enable_periph_clk(BTN3_PIO_ID);
	
	// Configura Pinos do PIO como INPUT com pull-up e debounce (BTN1, BTN2, BTN3)
	//pio_set_input(BUT_PIO, BUT_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_input(BTN1_PIO, BTN1_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_input(BTN2_PIO, BTN2_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_input(BTN3_PIO, BTN3_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	
	// Configura o Pino do LED como OUTPUT
	//pio_set_output(LED_PIO, LED_IDX_MASK, PIO_DEFAULT, 0,0);
	// Inicia LED apagado
	//pio_set(LED_PIO, LED_IDX_MASK);
	
	// ajusta o debounce
	//pio_set_debounce_filter(BUT_PIO, BUT_PIO_IDX_MASK, 60);
	pio_set_debounce_filter(BTN1_PIO, BTN1_PIO_IDX_MASK, 60);
	pio_set_debounce_filter(BTN2_PIO, BTN2_PIO_IDX_MASK, 60);
	pio_set_debounce_filter(BTN3_PIO, BTN3_PIO_IDX_MASK, 60);
	
	// Configura IRQ
	//pio_enable_interrupt(BUT_PIO, BUT_PIO_IDX_MASK);
	pio_enable_interrupt(BTN1_PIO, BTN1_PIO_IDX_MASK);
	pio_enable_interrupt(BTN2_PIO, BTN2_PIO_IDX_MASK);
	pio_enable_interrupt(BTN3_PIO, BTN3_PIO_IDX_MASK);
	
	// Pega status de interrupção
	//pio_get_interrupt_status(BUT_PIO);
	pio_get_interrupt_status(BTN1_PIO);
	pio_get_interrupt_status(BTN2_PIO);
	pio_get_interrupt_status(BTN3_PIO);
		
	// Configura NVIC para receber interrupções do IRQ com sua respectiva Prioridade 
	//NVIC_EnableIRQ(BUT_PIO_ID);
	NVIC_EnableIRQ(BTN1_PIO_ID);
	NVIC_EnableIRQ(BTN2_PIO_ID);
	NVIC_EnableIRQ(BTN3_PIO_ID);
	
	//NVIC_SetPriority(BUT_PIO_ID, 4); 
	NVIC_SetPriority(BTN1_PIO_ID, 3); 
	NVIC_SetPriority(BTN2_PIO_ID, 1); 
	NVIC_SetPriority(BTN3_PIO_ID, 3); // BTN3 e BUT compartilham o msm PIO 
	
	// Configura led
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_IDX_MASK, PIO_DEFAULT);

	// Configura handler (hardware) para o SW
	// começa a piscar LED
	//pio_handler_set(
		//BUT_PIO,
		//BUT_PIO_ID,
		//BUT_PIO_IDX_MASK,
		//PIO_IT_FALL_EDGE,
		//but_callback);
		
	pio_handler_set( 
		BTN1_PIO,
		BTN1_PIO_ID,
		BTN1_PIO_IDX_MASK,
		PIO_IT_EDGE,
		btn1_callback);
		
	pio_handler_set(
		BTN2_PIO,
		BTN2_PIO_ID,
		BTN2_PIO_IDX_MASK,
		PIO_IT_FALL_EDGE,
		btn2_callback);
		
	pio_handler_set(
		BTN3_PIO,
		BTN3_PIO_ID,
		BTN3_PIO_IDX_MASK,
		PIO_IT_FALL_EDGE,
		btn3_callback);
		
	// Configura a frequência inicial (1/delay)
	delay = 50;

}




int main (void)
{
	board_init();
	io_init();
	delay_init();
	
	// Init OLED
	gfx_mono_ssd1306_init();
	
	
	sprintf(str, "f= %d Hz", 1000 / delay);
	gfx_mono_draw_string(str, 10,0, &sysfont);
	
  /* Insert application code here, after the board has been initialized. */
	while(1) {
			// Escreve na tela um circulo e um texto
			//for(int i=70;i<=120;i+=2){
				//gfx_mono_draw_rect(i, 5, 2, 10, GFX_PIXEL_SET);
				//delay_ms(10);
			//}
			//
			//for(int i=120;i>=70;i-=2){
				//gfx_mono_draw_rect(i, 5, 2, 10, GFX_PIXEL_CLR);
				//delay_ms(10);
			//}
			//
			
			if (increment_freq) {
				delay -= 50;
				if (delay <= 0) {
					delay = 50;
				}
				sprintf(str, "f= %d Hz", 1000 / delay);
				gfx_mono_draw_string(str, 10,0, &sysfont);
				increment_freq = 0;
			} else if (decrement_freq_fast) {
				delay += 50;
				sprintf(str, "f= %d Hz  ", 1000 / delay);
				gfx_mono_draw_string(str, 10,0, &sysfont);
			} else if (decrement_freq_slow) {
				delay += 50;
				sprintf(str, "f= %d Hz  ", 1000 / delay);
				gfx_mono_draw_string(str, 10,0, &sysfont);
				decrement_freq_slow = 0;
			}
			
			if (led_flag) {
				pisca_led(30, delay);
				
			}
			
	
	}
}
