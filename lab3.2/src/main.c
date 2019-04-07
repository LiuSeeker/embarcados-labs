/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

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

// Botão1
#define BUT1_PIO      PIOD
#define BUT1_PIO_ID   16
#define BUT1_IDX  28
#define BUT1_IDX_MASK (1 << BUT1_IDX)

// Botão2
#define BUT2_PIO      PIOC
#define BUT2_PIO_ID   12
#define BUT2_IDX  31
#define BUT2_IDX_MASK (1 << BUT2_IDX)

// Botão3
#define BUT3_PIO      PIOA
#define BUT3_PIO_ID   10
#define BUT3_IDX  19
#define BUT3_IDX_MASK (1 << BUT3_IDX)

// LED1 
#define LED1_PIO      PIOA
#define LED1_PIO_ID   10
#define LED1_IDX  0
#define LED1_IDX_MASK (1 << LED1_IDX)

// LED2
#define LED2_PIO      PIOC
#define LED2_PIO_ID   12
#define LED2_IDX  30
#define LED2_IDX_MASK (1 << LED2_IDX)

// LED3
#define LED3_PIO      PIOB
#define LED3_PIO_ID   11
#define LED3_IDX  2
#define LED3_IDX_MASK (1 << LED3_IDX)

/************************************************************************/
/* funções                                                              */
/************************************************************************/

void io_init(void)
{

	// Configura led
	pmc_enable_periph_clk(LED1_PIO_ID);
	pio_configure(LED1_PIO, PIO_OUTPUT_0, LED1_IDX_MASK, PIO_DEFAULT);
	pmc_enable_periph_clk(LED2_PIO_ID);
	pio_configure(LED2_PIO, PIO_OUTPUT_0, LED2_IDX_MASK, PIO_DEFAULT);
	pmc_enable_periph_clk(LED3_PIO_ID);
	pio_configure(LED3_PIO, PIO_OUTPUT_0, LED3_IDX_MASK, PIO_DEFAULT);

	// Inicializa clock do periférico PIO responsavel pelo botao
	pmc_enable_periph_clk(BUT1_PIO_ID);
	pmc_enable_periph_clk(BUT2_PIO_ID);
	pmc_enable_periph_clk(BUT3_PIO_ID);

	// Configura PIO para lidar com o pino do botão como entrada
	// com pull-up
	pio_configure(BUT1_PIO, PIO_INPUT, BUT1_IDX_MASK, PIO_PULLUP);
	pio_configure(BUT2_PIO, PIO_INPUT, BUT2_IDX_MASK, PIO_PULLUP);
	pio_configure(BUT3_PIO, PIO_INPUT, BUT3_IDX_MASK, PIO_PULLUP);

	// Configura interrupção no pino referente ao botao e associa
	// função de callback caso uma interrupção for gerada
	// a função de callback é a: but_callback()
	pio_handler_set(BUT1_PIO, BUT1_PIO_ID, BUT1_IDX_MASK, PIO_IT_RISE_EDGE, but1_callback);
	pio_handler_set(BUT2_PIO, BUT2_PIO_ID, BUT2_IDX_MASK, PIO_IT_RISE_EDGE, but2_callback);
	pio_handler_set(BUT3_PIO, BUT3_PIO_ID, BUT3_IDX_MASK, PIO_IT_RISE_EDGE, but3_callback);


	// Ativa interrupção
	pio_enable_interrupt(BUT1_PIO, BUT1_IDX_MASK);
	pio_enable_interrupt(BUT2_PIO, BUT2_IDX_MASK);
	pio_enable_interrupt(BUT3_PIO, BUT3_IDX_MASK);

	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(BUT1_PIO_ID);
	NVIC_SetPriority(BUT1_PIO_ID, 4); // Prioridade 4
	NVIC_EnableIRQ(BUT2_PIO_ID);
	NVIC_SetPriority(BUT2_PIO_ID, 4); // Prioridade 4
	NVIC_EnableIRQ(BUT3_PIO_ID);
	NVIC_SetPriority(BUT3_PIO_ID, 4); // Prioridade 4
}

/************************************************************************/
/* Main                                                                 */
/************************************************************************/

int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */

	board_init();
	sysclk_init();

	delay_init();
	
	// Desativa watchdog
	WDT->WDT_MR = WDT_MR_WDDIS;
	
	// configura botao com interrupcao
	io_init();

	gfx_mono_ssd1306_init();
	gfx_mono_draw_filled_circle(20, 16, 16, GFX_PIXEL_SET, GFX_WHOLE);
	gfx_mono_draw_string("mundo", 50,16, &sysfont);


  /* Insert application code here, after the board has been initialized. */
	while(1) {
		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
		
		

	}
}
