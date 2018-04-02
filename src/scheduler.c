#include "scheduler.h"

struct sched_eventTable events[NUMEVENTS];

/* Declare event mutex flags, and declare and assign the buffer paramater
*  type structs.
*/
int32_t Flag_DMA_Chan3;
int32_t Flag_DMA_Chan4;
int32_t Flag_test;

int32_t Flag_bufferSize_uart;
int32_t Flag_bufferSize_spi;

struct buffer_fifo_u8 fifo_uartTx[1];
struct buffer_fifo_u16 fifo_spiTx[1];
struct buffer_fifo_u8 fifo_test[1];

buffer_param_t fifo_uartTx_param = 
    { .type = FIFO_U8T, .is= { .fifo_u8 = fifo_uartTx } };

buffer_param_t fifo_spiTx_param = 
    { .type = FIFO_U16T, .is= { .fifo_u16 = fifo_spiTx } };

buffer_param_t fifo_test_param = 
    { .type = FIFO_U8T, .is= { .fifo_u8 = fifo_test } };

volatile uint8_t fifo_uartTxData[B_SIZE_FIFO_U8T];
volatile uint8_t fifo_spiTxData[B_SIZE_FIFO_U16T];
volatile uint8_t fifo_testData[B_SIZE_TEST];

/* ******* Sched_Init *******
*  Initializes system task fixed rate scheduler.
*/
void Sched_init(void) {
	/* Initialize task target blocking signals */ 
	Sched_flagInit( &Flag_DMA_Chan4, 1 ); // flag for UART_tx DMA
	Sched_flagInit( &Flag_DMA_Chan3, 1 ); // flag for SPI_tx DMA
	Sched_flagInit( &Flag_test, 1 ); // test flag for test target

	/* Initialize blocking signal reflecting number of elements stored */ 
	Buffer_flagSizeInit( &Flag_bufferSize_uart );
	Buffer_flagSizeInit( &Flag_bufferSize_spi );

	/* Initialize buffer size setting */ 
	const uint16_t sizeUart = B_SIZE_FIFO_U8T;
	const uint16_t sizeSpi = B_SIZE_FIFO_U16T;
	const uint16_t sizeTest = B_SIZE_TEST;

	/* Initialize buffer_param objects providing: pointer to buffer_param_t, 
	*  number of elements, pointer to data array, blocking stored elements
	*  flag, and address of a target callback handler function.
	*/
	Buffer_init( &fifo_uartTx_param, sizeUart, fifo_uartTxData, 
				&Flag_bufferSize_uart, &Uart_dmaTxHandler );

	Buffer_init( &fifo_spiTx_param, sizeSpi, fifo_spiTxData, 
				&Flag_bufferSize_spi, &Spi_dmaTxHandler );

	Buffer_init( &fifo_test_param, sizeTest, fifo_testData, &Flag_test,
				&test_handler );

	/* Add events to the task mananger by providing: pointer to event function,
	*  fixed time interval, a queue parameter object, and a target signal flag.
	*/

	Sched_addEvent( &Uart_fifoTxEvent, 25, &fifo_uartTx_param, 
				&Flag_DMA_Chan4 );
	/*
	Sched_addEvent( &Spi_fifoTxEvent, 1, &fifo_spiTx_param, 
				&Flag_DMA_Chan3 );
	*/
	Sched_addEvent( &test_event, 10000, &fifo_test_param, &Flag_test );

}

// ******* Sched_flagInit *******
// Initialize a counting semaphore
//  Inputs: pointer to a semaphore
//          initial value of semaphore
// Outputs: none
void Sched_flagInit( int32_t *flagPt, int32_t value ) 
{
	(*flagPt) = value;

}

// ******* Sched_waitFlag *******
// Decrement semaphore, blocking task if less than zero.
//  Inputs: pointer to a counting semaphore
// Outputs: none
void Sched_flagWait( int32_t *flagPt ) 
{
	(*flagPt)--;

}

// ******* Sched_flagSignal *******
// Increment semaphore. Value > 0 indicates ready status.
//  Inputs: pointer to a counting semaphore
// Outputs: none
void Sched_flagSignal( int32_t *flagPt )
{
	(*flagPt)++;
}

// ******* Sched_addEvent *******
// Adds event to event management table
//  Inputs: pointer to a event function
//          period in cycles through the event queue
//          pointer to a fifo type
// Outputs: none
void Sched_addEvent( 
	void(*function)( buffer_param_t *buffer, int32_t *flagPt ),
	uint32_t period_cycles, buffer_param_t *buffer, int32_t *flagPt )
{
	int j;
	for ( j = 0; j < NUMEVENTS; j++ )
	{
		if ( !events[j].eventFunction )
		{
			events[j].eventFunction = function;
			events[j].interval = period_cycles;
			events[j].last = 0;
			events[j].buffer = buffer;
			events[j].flag = flagPt;
			break;
		}
	}
}

// ******* Sched_runEventManager *******
// Executes functions based on a series of conditions,
// including elapsed time since last run and busy signals.
//  Inputs: none
// Outputs: none
void Sched_runEventManager(void)
{
	cm_disable_interrupts();
	uint8_t j;
	uint32_t now, diff;
	
	for ( j = 0; j < NUMEVENTS; j++ )
	{
		now = Systick_timeGetCount();

		/* Number of program execution cycles since last execution.
		*/
		diff = Systick_timeDelta( events[j].last, now );

		/* if: reception conditions are true (at or past interval delta, 
		   task flag > 0, buffer size > 0), run event
		*/
		if ( ( diff >= events[j].interval ) && ( (*events[j].flag) ) 
				&& ( (*events[j].buffer->flagSize) ) )  
		{
			events[j].eventFunction( events[j].buffer, events[j].flag );
			events[j].last = now;

		}			
		
	}
	
	cm_enable_interrupts();
}

void test_event( buffer_param_t *buffer, int32_t *flagPt )
{
	gpio_toggle(GPIOB, GPIO8);
}

void test_handler( volatile void* data, uint8_t length )
{
	return;
}