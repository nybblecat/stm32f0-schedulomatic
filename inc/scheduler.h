#ifndef SCHEDULER_H_
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/f0/nvic.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/cortex.h>

#include "systick.h"
#include "buffer.h"

#define NUMEVENTS 2

// Signaling flags
extern int32_t Flag_DMA_Chan3;
extern int32_t Flag_DMA_Chan4;
extern int32_t Flag_test;

// Buffer parameter initialization stubs
extern buffer_param_t fifo_uartTx_param;
extern buffer_param_t fifo_spiTx_param;

// Scheduler event table
struct sched_eventTable {
	// event function is executed in the event scheduler loop
	void(*eventFunction)( buffer_param_t *buffer, int32_t *flagPt ); 
	buffer_param_t *buffer; // pointer to data queue related to the particular event.
	int32_t *flag; // pointer to an initialized semaphore for event signaling.
	uint32_t interval; // how often the manager function will be called.
	uint32_t last; // time of last execution.
};

// ******* Sched_Init *******
// Initializes event scheduling manager
//  Inputs: none
// Outputs: none
void Sched_init(void);

// ******* Sched_flagInit *******
// Initialize a counting semaphore
//  Inputs: pointer to a semaphore
//          initial value of semaphore
// Outputs: none
void Sched_flagInit( int32_t *semaPt, int32_t value );

// ******* Sched_flagWait *******
// Decrement semaphore, blocking task if less than zero
//  Inputs: pointer to a counting semaphore
// Outputs: none
void Sched_flagWait( int32_t *semaPt );

// ******* Sched_flagSignal *******
// Increment semaphore
//  Inputs: pointer to a counting semaphore
// Outputs: none
void Sched_flagSignal( int32_t *semaPt );

// ******* Sched_addEvent *******
// Adds event to event management table
//  Inputs: pointer to a event function
//          period in milliseconds
//          pointer to a fifo type
// Outputs: none
void Sched_addEvent( 
	void(*function)( buffer_param_t *buffer, int32_t *flagPt ),
	uint32_t period_cycles, 
	buffer_param_t *buffer, 
	int32_t *flagPt );

// ******* Sched_runEventManager *******
// Runs scheduler event manager
//  Inputs: none
// Outputs: none
void Sched_runEventManager(void);

// ******* Uart_fifoTxEvent *******
// Periodic event that manages the UART transmit queue.
//  Inputs: fifo_t pointer, signal flag
// Outputs: none
extern void Uart_fifoTxEvent( buffer_param_t *buffer, int32_t *flagPt );

// ******* Uart_dmaTxHandler *******
// Copies a series of data from a memory address to the serial peripheral DMA
// transmission channel.
//  Inputs: pointer to a contiguous block of data, the number of bytes to copy
// Outputs: none
extern void Uart_dmaTxHandler( volatile void* data, uint8_t length );

// ******* Spi_fifoTxEvent *******
// Periodic event that manages the SPI transmission queue.
// Executed from the event scheduler.
//  Inputs: buffer_fifo_t pointer, signal flag
// Outputs: none
extern void Spi_fifoTxEvent( buffer_param_t *buffer, int32_t *flagPt );

// ******* Spi_dmaTxHandler *******
// Copies data from a memory address to the SPI peripheral DMA transmission 
// channel.
//  Inputs: pointer to a contiguous block of data, the number of bytes to copy
// Outputs: none
extern void Spi_dmaTxHandler( volatile void* data, uint8_t length );

void test_event( buffer_param_t *buffer, int32_t *flagPt );

void test_handler( volatile void* data, uint8_t length );

#define SCHEDULER_H_ 1
#endif