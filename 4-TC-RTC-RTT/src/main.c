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

typedef struct  {
	uint32_t year;
	uint32_t month;
	uint32_t day;
	uint32_t week;
	uint32_t hour;
	uint32_t minute;
	uint32_t second;
} calendar;

/************************************************************************/
/* VAR globais                                                          */
/************************************************************************/
volatile char btn1_flag = 0;			// Flag do botão 1 --> seta o alarme e abaixa flag
volatile char rtc_alarm_flag = 0;		// Flag do alarme  --> Faz o LED piscar por 10seg e abaixa flag
volatile char increment_sec_flag = 0;		// Flag do alarme, ativada a cada 1 seg --> incrementa calendar.seg

uint32_t current_hour, current_min, current_sec;
uint32_t current_year, current_month, current_day, current_week;

char display[8];
/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/

void LED_init(Pio *pio, uint32_t led_id, uint32_t led_mask, int initial_state);
void BTN_init(Pio *pio, uint32_t btn_id, uint32_t btn_mask);
void BTN_interrupt_init(Pio *pio, uint32_t btn_id, uint32_t btn_mask, int prioridade); 

void pisca_led(Pio *pio, uint32_t led_mask);
void show_clock_on_display(int x, int y);

void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq, int prioridade_irq);
static void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource);
void RTC_init(Rtc *rtc, uint32_t id_rtc, calendar t, uint32_t irq_type);

void btn1_callback(void);

/************************************************************************/
/* Handlers && Callbacks                                                */
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

void RTT_Handler(void) {
	uint32_t ul_status;

	/* Get RTT status - ACK */
	ul_status = rtt_get_status(RTT);

	/* IRQ due to Alarm */
	if ((ul_status & RTT_SR_ALMS) == RTT_SR_ALMS) {
		RTT_init(0.25, 0, RTT_MR_RTTINCIEN);
	}
	
	/* IRQ due to Time has changed */
	if ((ul_status & RTT_SR_RTTINC) == RTT_SR_RTTINC) {
		pisca_led(LED2_PIO, LED2_PIO_IDX_MASK);    // BLINK Led
	}

}

void btn1_callback(void) {
	btn1_flag = 1;
}

void RTC_Handler(void) {
	uint32_t ul_status = rtc_get_status(RTC);
	
	/* seccond tick */
	if ((ul_status & RTC_SR_SEC) == RTC_SR_SEC) {
		// o código para irq de segundo vem aqui
		increment_sec_flag = 1;
		
	}
	
	/* Time or date alarm */
	if ((ul_status & RTC_SR_ALARM) == RTC_SR_ALARM) {
		// Ativa flag de resultado do alarme
		rtc_alarm_flag = 1;
	}

	rtc_clear_status(RTC, RTC_SCCR_SECCLR);
	rtc_clear_status(RTC, RTC_SCCR_ALRCLR);
	rtc_clear_status(RTC, RTC_SCCR_ACKCLR);
	rtc_clear_status(RTC, RTC_SCCR_TIMCLR);
	rtc_clear_status(RTC, RTC_SCCR_CALCLR);
	rtc_clear_status(RTC, RTC_SCCR_TDERRCLR);
}


/************************************************************************/
/* Funcoes                                                              */
/************************************************************************/
void LED_init(Pio *pio, uint32_t led_id, uint32_t led_mask, int initial_state) {
	pmc_enable_periph_clk(led_id);
	pio_set_output(pio, led_mask, initial_state, 0,0);
}

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

void pisca_led(Pio *pio, uint32_t led_mask) {
	// Altera o status do LED de 0 p/ 1 e vice-versa.
	if (pio_get_output_data_status(pio, led_mask)) {
		pio_clear(pio, led_mask);
	} else {
		pio_set(pio, led_mask);
	}
}

void show_clock_on_display(int x, int y) {
	sprintf(display, "%02lu:%02lu:%02lu", current_hour, current_min, current_sec);
	gfx_mono_draw_string(display, x, y, &sysfont);
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
	if (rttIRQSource & (RTT_MR_RTTINCIEN | RTT_MR_ALMIEN))
	rtt_enable_interrupt(RTT, rttIRQSource);
	else
	rtt_disable_interrupt(RTT, RTT_MR_RTTINCIEN | RTT_MR_ALMIEN);
	
}

void RTC_init(Rtc *rtc, uint32_t id_rtc, calendar t, uint32_t irq_type) {
	/* Configura o PMC */
	pmc_enable_periph_clk(ID_RTC);

	/* Default RTC configuration, 24-hour mode */
	rtc_set_hour_mode(rtc, 0);

	/* Configura data e hora manualmente */
	rtc_set_date(rtc, t.year, t.month, t.day, t.week);
	rtc_set_time(rtc, t.hour, t.minute, t.second);

	/* Configure RTC interrupts */
	NVIC_DisableIRQ(id_rtc);
	NVIC_ClearPendingIRQ(id_rtc);
	NVIC_SetPriority(id_rtc, 1);
	NVIC_EnableIRQ(id_rtc);

	/* Ativa interrupcao via alarme */
	rtc_enable_interrupt(rtc,  irq_type);
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
	  //gfx_mono_draw_filled_circle(20, 16, 16, GFX_PIXEL_SET, GFX_WHOLE);
	  //gfx_mono_draw_string("ble", 50,16, &sysfont);

  
	// Configura LED como OUTPUT e estado inicial como OFF
	LED_init(LED1_PIO, LED1_PIO_ID, LED1_PIO_IDX_MASK, 1);
	LED_init(LED2_PIO, LED2_PIO_ID, LED2_PIO_IDX_MASK, 1);
	LED_init(LED3_PIO, LED3_PIO_ID, LED3_PIO_IDX_MASK, 1);
	
	// Configura BTN 1 como Input
	// Configura interrupção no BTN 1
	BTN_init(BTN1_PIO, BTN1_PIO_ID, BTN1_PIO_IDX_MASK);
	BTN_interrupt_init(BTN1_PIO, BTN1_PIO_ID, BTN1_PIO_IDX_MASK, 4);
	pio_handler_set(
		BTN1_PIO,
		BTN1_PIO_ID,
		BTN1_PIO_IDX_MASK,
		PIO_IT_FALL_EDGE,
		btn1_callback);
	
	// Inicializa Timer TC0, canal 1
	// Inicializa contagem do TC0 no canal 1
	TC_init(TC0, ID_TC1, 1, 4, 4);
	tc_start(TC0, 1);
	
	// Inicializa RTT como incrementador com freq = 0.25 hz
	RTT_init(0.25, 0, RTT_MR_RTTINCIEN);
	
	// Inicialia variáveis de calendário para utilizar no RTC:
	// Leitura dos valores atuais do RTC:
	rtc_get_time(RTC, &current_hour, &current_min, &current_sec);
	rtc_get_date(RTC, &current_year, &current_month, &current_day, &current_week);
	// configura RTC como alarme e como interrupção a cada SEG
	calendar rtc_initial = {2022, 3, 17, 20, 45, 32};
	RTC_init(RTC, ID_RTC, rtc_initial, RTC_IER_ALREN | RTC_IER_SECEN);
	

	
	
		
	
	while(1) {
		
		
	// if flag_btn1: --> Aciona rtc_alarm_flag => Seta alarme para t+20seg
	if (btn1_flag) {
		
		rtc_get_time(RTC, &current_hour, &current_min, &current_sec);
		// Configura alarme do RTC para +20seg.
		rtc_set_date_alarm(RTC, 1, current_month, 1, current_day);
		if (current_sec + 20 >= 60) {
			rtc_set_time_alarm(RTC, 1, current_hour, 1, current_min +1, 1, current_sec + 20 -60);
		} else {
			rtc_set_time_alarm(RTC, 1, current_hour, 1, current_min, 1, current_sec + 20);
		}
		
		
		// Abaixa flag
		btn1_flag = 0;
	}
	
	if (rtc_alarm_flag) {
		rtc_alarm_flag = 0;
		for (int i=0; i<100; i++) {
			pisca_led(LED3_PIO, LED3_PIO_IDX_MASK);
			delay_ms(100);
			}
		
	}
	
	if (increment_sec_flag) {
		current_sec += 1;
		if (current_sec == 60) {
			current_min += 1;
			current_sec = 0;
		}
		if (current_min == 60) {
			current_hour += 1;
			current_min = 0;
		}
		if (current_hour == 24) {
			current_day += 1;
			current_hour = 0;
		}
		increment_sec_flag = 0;
	}
	
	// show clock on display
	show_clock_on_display(30, 10);
	//pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
	
	}
}
