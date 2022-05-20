/************************************************************************/
/* includes                                                             */
/************************************************************************/

#include "ili9341.h"
#include "lvgl.h"
#include "touch/touch.h"
#include <asf.h>
#include <string.h>

// FONTE:
LV_FONT_DECLARE(dseg70)
LV_FONT_DECLARE(dseg50)
LV_FONT_DECLARE(dseg30)
/************************************************************************/
/* Variáveis Globais                                                    */
/************************************************************************/

// buttons
lv_obj_t *btn_on_off;
lv_obj_t *btn_menu;
lv_obj_t *btn_clock;
lv_obj_t *btn_temp_up;
lv_obj_t *btn_temp_down;

// labels
lv_obj_t *label_floor;
lv_obj_t *label_floor_frac;
lv_obj_t *label_clock;
lv_obj_t *label_ref_temp;

// global style
lv_style_t btn_style;

/************************************************************************/
/* LCD / LVGL                                                           */
/************************************************************************/

#define LV_HOR_RES_MAX (320)
#define LV_VER_RES_MAX (240)

/*A static or global variable to store the buffers*/
static lv_disp_draw_buf_t disp_buf;

/*Static or global buffer(s). The second buffer is optional*/
static lv_color_t buf_1[LV_HOR_RES_MAX * LV_VER_RES_MAX];
static lv_disp_drv_t disp_drv; /*A variable to hold the drivers. Must be static or global.*/
static lv_indev_drv_t indev_drv;

/************************************************************************/
/* RTOS                                                                 */
/************************************************************************/

#define TASK_LCD_STACK_SIZE (1024 * 6 / sizeof(portSTACK_TYPE))
#define TASK_LCD_STACK_PRIORITY (tskIDLE_PRIORITY)

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName) {
    printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
    for (;;) {
    }
}

extern void vApplicationIdleHook(void) {}

extern void vApplicationTickHook(void) {}

extern void vApplicationMallocFailedHook(void) {
    configASSERT((volatile void *)NULL);
}

/************************************************************************/
/* lvgl                                                                 */
/************************************************************************/

static void event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
    } else if (code == LV_EVENT_VALUE_CHANGED) {
        LV_LOG_USER("Toggled");
    }
}

static void up_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    char *c;
    int temp;
    if (code == LV_EVENT_CLICKED) {
        c = lv_label_get_text(label_ref_temp);
        temp = atoi(c);
        temp++;
        lv_label_set_text_fmt(label_ref_temp, "%02d", temp);
    }
}

static void down_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    char *c;
    int temp;
    if (code == LV_EVENT_CLICKED) {
        c = lv_label_get_text(label_ref_temp);
        temp = atoi(c);
        temp--;
        lv_label_set_text_fmt(label_ref_temp, "%02d", temp);
    }
}
void lv_ex_btn_1(void) {
    lv_obj_t *label;

    lv_obj_t *btn1 = lv_btn_create(lv_scr_act());
    lv_obj_add_event_cb(btn1, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -40);

    label = lv_label_create(btn1);
    lv_label_set_text(label, "Corsi");
    lv_obj_center(label);

    lv_obj_t *btn2 = lv_btn_create(lv_scr_act());
    lv_obj_add_event_cb(btn2, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 40);
    lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_height(btn2, LV_SIZE_CONTENT);

    label = lv_label_create(btn2);
    lv_label_set_text(label, "Toggle");
    lv_obj_center(label);
}

void lv_clock(void) {

    label_clock = lv_label_create(lv_scr_act());
    lv_obj_align(label_clock, LV_ALIGN_TOP_RIGHT, 0, 5);
    lv_obj_set_style_text_font(label_clock, &dseg30, LV_STATE_DEFAULT);
    //<> HARDCODED - temp
    lv_label_set_text_fmt(label_clock, "%02d:%02d", 17, 46);
}

void lv_ref_termostato(void) {
    label_ref_temp = lv_label_create(lv_scr_act());
    lv_obj_align_to(label_ref_temp, label_clock, LV_ALIGN_OUT_BOTTOM_MID, -5, 30);
    lv_obj_set_style_text_font(label_ref_temp, &dseg50, LV_STATE_DEFAULT);
    //<> HARDCODED - temp
    lv_label_set_text_fmt(label_ref_temp, "%02d", 22);
}
void lv_termostato(void) {

    label_floor = lv_label_create(lv_scr_act());
    lv_obj_align(label_floor, LV_ALIGN_LEFT_MID, 35, -25);
    lv_obj_set_style_text_font(label_floor, &dseg70, LV_STATE_DEFAULT);
    //<> HARDCODED - temp
    lv_label_set_text_fmt(label_floor, "%02d", 23);
}

void lv_termostato_frac(void) {

    label_floor_frac = lv_label_create(lv_scr_act());
    lv_obj_align_to(label_floor_frac, label_floor, LV_ALIGN_OUT_BOTTOM_RIGHT, 50, -25);
    lv_obj_set_style_text_font(label_floor_frac, &dseg30, LV_STATE_DEFAULT);
    //<> HARDCODED - temp frac
    lv_label_set_text_fmt(label_floor_frac, ".%1d", 4);
}
void lv_power_on_off(void) {

    lv_style_init(&btn_style);
    lv_style_set_bg_color(&btn_style, lv_color_black());
    // lv_style_set_border_color(&btn_style, lv_palette_main(LV_PALETTE_GREEN));
    // lv_style_set_border_width(&btn_style, 3);

    lv_obj_t *label_btn;
    btn_on_off = lv_btn_create(lv_scr_act());
    lv_obj_add_event_cb(btn_on_off, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btn_on_off, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    label_btn = lv_label_create(btn_on_off);
    lv_label_set_text(label_btn, "[ " LV_SYMBOL_POWER);
    lv_obj_center(label_btn);

    lv_obj_add_style(btn_on_off, &btn_style, 0);
    lv_obj_set_width(btn_on_off, 60);
    lv_obj_set_height(btn_on_off, 60);
}

void lv_menu(void) {

    lv_obj_t *label_btn;
    btn_menu = lv_btn_create(lv_scr_act());
    lv_obj_add_event_cb(btn_menu, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align_to(btn_menu, btn_on_off, LV_ALIGN_OUT_TOP_RIGHT, 20, 16);

    label_btn = lv_label_create(btn_menu);
    lv_label_set_text(label_btn, "| M |");
    lv_obj_center(label_btn);

    lv_obj_add_style(btn_menu, &btn_style, 0);
    lv_obj_set_width(btn_menu, 60);
    lv_obj_set_height(btn_menu, 60);
}

void lv_adjust_clock(void) {
    lv_obj_t *label_btn;
    btn_clock = lv_btn_create(lv_scr_act());
    lv_obj_add_event_cb(btn_clock, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align_to(btn_clock, btn_menu, LV_ALIGN_OUT_TOP_RIGHT, 20, 16);

    label_btn = lv_label_create(btn_clock);
    lv_label_set_text(label_btn, LV_SYMBOL_SETTINGS " ]");
    lv_obj_center(label_btn);

    lv_obj_add_style(btn_clock, &btn_style, 0);
    lv_obj_set_width(btn_clock, 60);
    lv_obj_set_height(btn_clock, 60);
}

void lv_btn_down(void) {
    lv_obj_t *label_btn;
    btn_temp_down = lv_btn_create(lv_scr_act());
    lv_obj_add_event_cb(btn_temp_down, down_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btn_temp_down, LV_ALIGN_BOTTOM_RIGHT, -10, 0);

    label_btn = lv_label_create(btn_temp_down);
    lv_label_set_text(label_btn, LV_SYMBOL_DOWN "  ]");
    lv_obj_center(label_btn);

    lv_obj_add_style(btn_temp_down, &btn_style, 0);
    lv_obj_set_width(btn_temp_down, 60);
    lv_obj_set_height(btn_temp_down, 60);
}

void lv_btn_up(void) {

    lv_obj_t *label_btn;
    btn_temp_up = lv_btn_create(lv_scr_act());
    lv_obj_add_event_cb(btn_temp_up, up_handler, LV_EVENT_ALL, NULL);
    lv_obj_align_to(btn_temp_up, btn_temp_down, LV_ALIGN_OUT_TOP_LEFT, -50, 16);

    label_btn = lv_label_create(btn_temp_up);
    lv_label_set_text(label_btn, "[ " LV_SYMBOL_UP " | ");
    lv_obj_center(label_btn);

    lv_obj_add_style(btn_temp_up, &btn_style, 0);
    lv_obj_set_width(btn_temp_up, 60);
    lv_obj_set_height(btn_temp_up, 60);
}

/************************************************************************/
/* TASKS                                                                */
/************************************************************************/

static void task_lcd(void *pvParameters) {
    int px, py;

    lv_power_on_off();
    lv_menu();
    lv_adjust_clock();
    lv_termostato();
    lv_termostato_frac();
    lv_clock();
    lv_ref_termostato();
    lv_btn_down();
    lv_btn_up();

    for (;;) {
        lv_tick_inc(50);
        lv_task_handler();
        vTaskDelay(50);
    }
}

/************************************************************************/
/* configs                                                              */
/************************************************************************/

static void configure_lcd(void) {
    /**LCD pin configure on SPI*/
    pio_configure_pin(LCD_SPI_MISO_PIO, LCD_SPI_MISO_FLAGS); //
    pio_configure_pin(LCD_SPI_MOSI_PIO, LCD_SPI_MOSI_FLAGS);
    pio_configure_pin(LCD_SPI_SPCK_PIO, LCD_SPI_SPCK_FLAGS);
    pio_configure_pin(LCD_SPI_NPCS_PIO, LCD_SPI_NPCS_FLAGS);
    pio_configure_pin(LCD_SPI_RESET_PIO, LCD_SPI_RESET_FLAGS);
    pio_configure_pin(LCD_SPI_CDS_PIO, LCD_SPI_CDS_FLAGS);

    ili9341_init();
    ili9341_backlight_on();
}

static void configure_console(void) {
    const usart_serial_options_t uart_serial_options = {
        .baudrate = USART_SERIAL_EXAMPLE_BAUDRATE,
        .charlength = USART_SERIAL_CHAR_LENGTH,
        .paritytype = USART_SERIAL_PARITY,
        .stopbits = USART_SERIAL_STOP_BIT,
    };

    /* Configure console UART. */
    stdio_serial_init(CONSOLE_UART, &uart_serial_options);

    /* Specify that stdout should not be buffered. */
    setbuf(stdout, NULL);
}

/************************************************************************/
/* port lvgl                                                            */
/************************************************************************/

void my_flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
    ili9341_set_top_left_limit(area->x1, area->y1);
    ili9341_set_bottom_right_limit(area->x2, area->y2);
    ili9341_copy_pixels_to_screen(color_p, (area->x2 + 1 - area->x1) * (area->y2 + 1 - area->y1));

    /* IMPORTANT!!!
     * Inform the graphics library that you are ready with the flushing*/
    lv_disp_flush_ready(disp_drv);
}

void my_input_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    int px, py, pressed;

    if (readPoint(&px, &py))
        data->state = LV_INDEV_STATE_PRESSED;
    else
        data->state = LV_INDEV_STATE_RELEASED;

    data->point.x = px;
    data->point.y = py;
}

void configure_lvgl(void) {
    lv_init();
    lv_disp_draw_buf_init(&disp_buf, buf_1, NULL, LV_HOR_RES_MAX * LV_VER_RES_MAX);

    lv_disp_drv_init(&disp_drv);       /*Basic initialization*/
    disp_drv.draw_buf = &disp_buf;     /*Set an initialized buffer*/
    disp_drv.flush_cb = my_flush_cb;   /*Set a flush callback to draw to the display*/
    disp_drv.hor_res = LV_HOR_RES_MAX; /*Set the horizontal resolution in pixels*/
    disp_drv.ver_res = LV_VER_RES_MAX; /*Set the vertical resolution in pixels*/

    lv_disp_t *disp;
    disp = lv_disp_drv_register(&disp_drv); /*Register the driver and save the created display objects*/

    /* Init input on LVGL */
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_input_read;
    lv_indev_t *my_indev = lv_indev_drv_register(&indev_drv);
}

/************************************************************************/
/* main                                                                 */
/************************************************************************/
int main(void) {
    /* board and sys init */
    board_init();
    sysclk_init();
    configure_console();

    /* LCd, touch and lvgl init*/
    configure_lcd();
    configure_touch();
    configure_lvgl();

    /* Create task to control oled */
    if (xTaskCreate(task_lcd, "LCD", TASK_LCD_STACK_SIZE, NULL, TASK_LCD_STACK_PRIORITY, NULL) != pdPASS) {
        printf("Failed to create lcd task\r\n");
    }

    /* Start the scheduler. */
    vTaskStartScheduler();

    while (1) {
    }
}
