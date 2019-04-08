/************************************************************************/
/* includes                                                             */
/************************************************************************/

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
#define LED_IDX      30
#define LED_IDX_MASK (1 << LED_IDX)

// Botão1
#define BUT1_PIO      PIOD
#define BUT1_PIO_ID   ID_PIOD
#define BUT1_IDX  28
#define BUT1_IDX_MASK (1 << BUT1_IDX)

// Botão2
#define BUT2_PIO      PIOC
#define BUT2_PIO_ID   ID_PIOC
#define BUT2_IDX  31
#define BUT2_IDX_MASK (1 << BUT2_IDX)

// Botão3
#define BUT3_PIO      PIOA
#define BUT3_PIO_ID   ID_PIOA
#define BUT3_IDX  19
#define BUT3_IDX_MASK (1 << BUT3_IDX)


/************************************************************************/
/* constants                                                            */
/************************************************************************/

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/

volatile Bool flag;
volatile int freq = 5;

/************************************************************************/
/* prototipos                                                           */
/************************************************************************/

void io_init();



/************************************************************************/
/* handler / callbacks                                                  */
/************************************************************************/

void but1_callback(){
	if(freq > 2){
		freq -= 1;
	}
}

void but2_callback(){
	if(flag){
		flag = false;
	}
	else{
		flag = true;
	}
}

void but3_callback(){
	freq += 1;
}


/************************************************************************/
/* funções                                                              */
/************************************************************************/

void io_init(void)
{

	// Configura led
	pmc_enable_periph_clk(ID_PIOA);
	pmc_enable_periph_clk(ID_PIOC);
	pmc_enable_periph_clk(ID_PIOD);
	
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_IDX_MASK, PIO_DEFAULT);
	pio_configure(BUT1_PIO, PIO_INPUT, BUT1_IDX_MASK, PIO_DEFAULT);
	pio_configure(BUT2_PIO, PIO_INPUT, BUT2_IDX_MASK, PIO_DEFAULT);
	pio_configure(BUT3_PIO, PIO_INPUT, BUT3_IDX_MASK, PIO_DEFAULT);
	
	pio_set_output(LED_PIO, LED_IDX_MASK, 0, 0, 0);
	
	pio_set_input(BUT1_PIO, BUT1_IDX_MASK, PIO_DEFAULT);
	pio_set_input(BUT2_PIO, BUT2_IDX_MASK, PIO_DEFAULT);
	pio_set_input(BUT3_PIO, BUT3_IDX_MASK, PIO_DEFAULT);
	pio_pull_up(BUT1_PIO, BUT1_IDX_MASK, 1);
	pio_pull_up(BUT2_PIO, BUT2_IDX_MASK, 1);
	pio_pull_up(BUT3_PIO, BUT3_IDX_MASK, 1);

	// Configura interrupção no pino referente ao botao e associa
	// função de callback caso uma interrupção for gerada
	// a função de callback é a: but_callback()
	pio_handler_set(BUT1_PIO, BUT1_PIO_ID, BUT1_IDX_MASK, PIO_IT_FALL_EDGE, but1_callback);
	pio_handler_set(BUT2_PIO, BUT2_PIO_ID, BUT2_IDX_MASK, PIO_IT_FALL_EDGE, but2_callback);
	pio_handler_set(BUT3_PIO, BUT3_PIO_ID, BUT3_IDX_MASK, PIO_IT_FALL_EDGE, but3_callback);

	// Ativa interrupção
	pio_enable_interrupt(BUT1_PIO, BUT1_IDX_MASK);
	pio_enable_interrupt(BUT2_PIO, BUT2_IDX_MASK);
	pio_enable_interrupt(BUT3_PIO, BUT3_IDX_MASK);

	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(ID_PIOA);
	NVIC_SetPriority(ID_PIOA, 4); // Prioridade 4
	NVIC_EnableIRQ(ID_PIOC);
	NVIC_SetPriority(ID_PIOC, 4); // Prioridade 4
	NVIC_EnableIRQ(ID_PIOD);
	NVIC_SetPriority(ID_PIOD, 4); // Prioridade 4
}

void pisca_led(){
	int t = 1000/freq;
	pio_clear(LED_PIO, LED_IDX_MASK);
	delay_ms(t);
	pio_set(LED_PIO, LED_IDX_MASK);
	delay_ms(t);
}

/************************************************************************/
/* Main                                                                 */
/************************************************************************/

int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */

	board_init();
	sysclk_init();
	
	WDT->WDT_MR = WDT_MR_WDDIS;

	delay_init();

	gfx_mono_ssd1306_init();
	gfx_mono_draw_filled_circle(20, 16, 16, GFX_PIXEL_SET, GFX_WHOLE);
	gfx_mono_draw_string("mund", 50,16, &sysfont);

	io_init();
	
	/* Insert application code here, after the board has been initialized. */
	while(1) {
		while(flag){
			pisca_led();
		}
		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
	}
}
