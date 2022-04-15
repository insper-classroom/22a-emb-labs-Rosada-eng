#ifndef pins_config
#define pins_config

// ECHO (X) -- fio laranja (PA13)
#define ECHO_PIO			PIOA
#define ECHO_PIO_ID			ID_PIOA
#define ECHO_PIO_IDX		13
#define ECHO_PIO_IDX_MASK	(1u << ECHO_PIO_IDX)


// TRIGGER (Y) -- fio amarelo (PC19)
#define TRIGGER_PIO			PIOC
#define TRIGGER_PIO_ID		ID_PIOC
#define TRIGGER_PIO_IDX		19
#define TRIGGER_PIO_IDX_MASK (1u << TRIGGER_PIO_IDX)

// LED
#define LED_PIO      PIOC
#define LED_PIO_ID   ID_PIOC
#define LED_PIO_IDX      8
#define LED_PIO_IDX_MASK (1 << LED_PIO_IDX)

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


#endif