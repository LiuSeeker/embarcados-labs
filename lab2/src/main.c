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

#define LED_PIO           PIOC
#define LED_PIO_ID        12
#define LED_PIO_IDX       8u
#define LED_PIO_IDX_MASK  (1u << LED_PIO_IDX)

#define BUT_PIO           PIOA
#define BUT_PIO_ID        10
#define BUT_PIO_IDX       11u
#define BUT_PIO_IDX_MASK  (1u << BUT_PIO_IDX)

/*  Default pin configuration (no attribute). */
#define _PIO_DEFAULT             (0u << 0)
/*  The internal pin pull-up is active. */
#define _PIO_PULLUP              (1u << 0)
/*  The internal glitch filter is active. */
#define _PIO_DEGLITCH            (1u << 1)
/*  The pin is open-drain. */
#define _PIO_OPENDRAIN           (1u << 2)
/*  The internal debouncing filter is active. */
#define _PIO_DEBOUNCE            (1u << 3)

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

/************************************************************************/
/* interrupcoes                                                         */
/************************************************************************/

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

// Função de inicialização do uC
void init(void)
{
	sysclk_init();
	
	WDT->WDT_MR = WDT_MR_WDDIS;
	
	pmc_enable_periph_clk(LED_PIO_ID);
	pmc_enable_periph_clk(BUT_PIO_ID);
	
	pio_set_output(LED_PIO, LED_PIO_IDX_MASK, 0, 0, 0);
	
	pio_set_input(BUT_PIO, BUT_PIO_IDX_MASK, _PIO_DEFAULT);
	_pio_pull_up(BUT_PIO, BUT_PIO_IDX_MASK, 1);
	
}


void _pio_set(Pio *p_pio, const uint32_t ul_mask)
{
	p_pio->PIO_SODR = ul_mask;
}
	
void _pio_clear(Pio *p_pio, const uint32_t ul_mask)
{
	p_pio->PIO_CODR = ul_mask;
}

void _pio_pull_up(Pio *p_pio, const uint32_t ul_mask, const uint32_t ul_pull_up_enable){
	if(ul_pull_up_enable == 1){
		p_pio->PIO_PUER = ul_mask;
	}
	else{
		p_pio->PIO_PUDR = ul_mask;
	}
}

void _pio_set_input(Pio *p_pio, const uint32_t ul_mask, const uint32_t ul_attribute)
{
	if(ul_attribute&_PIO_DEGLITCH){
		p_pio->PIO_IFSCSR = (0u<<0);
	}
	if(ul_attribute&_PIO_DEBOUNCE){
		p_pio->PIO_IFSCSR = (1u<<0);
	}
	
	p_pio->PIO_IFER = ul_mask;
	
}

/************************************************************************/
/* Main                                                                 */
/************************************************************************/

// Funcao principal chamada na inicalizacao do uC.
int main(void)
{
  init();

  // super loop
  // aplicacoes embarcadas não devem sair do while(1).
  while (1)
  {
	if(!pio_get(BUT_PIO, PIO_INPUT, BUT_PIO_IDX_MASK)){
		for(int i = 0; i < 5; i++){
			_pio_clear(PIOC, LED_PIO_IDX_MASK);    // Coloca 0 no pino do LED
			delay_ms(200);                   // Delay por software de 200 ms	
			_pio_set(PIOC, LED_PIO_IDX_MASK);
			delay_ms(200);                   // Delay por software de 200 ms
		}
	}
	_pio_set(PIOC, LED_PIO_IDX_MASK);      // Coloca 1 no pino LED
  }
  return 0;
}
