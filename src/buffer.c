#include "buffer.h"

// ******* Buffer_Init *******
// Initializes a buffer parameter structure, preparing it for usage.
// Inputs: Pointer to buffer_param_t.
//		   Function pointer called by the event scheduler to handle queue 
//		   processing.
// Ouputs: None
void Buffer_init( buffer_param_t *buffer, uint16_t size, 
	volatile uint8_t *data, int32_t *flagSize, 
	void(*handler)( volatile void *data, uint8_t length ) )
{
	switch ( buffer->type )
	{
		case FIFO_U8T:
		{
			buffer->is.fifo_u8->data = data;
			buffer->is.fifo_u8->getIndex = 0;
			buffer->is.fifo_u8->putIndex = 0;
			buffer->is.fifo_u8->size = size;
			buffer->is.fifo_u8->handler_function = handler;
			buffer->flagSize = flagSize;

		}

		case FIFO_U16T:
		{
			buffer->is.fifo_u16->getPt = (uint16_t*)&buffer->is.fifo_u16->data[0];
			buffer->is.fifo_u16->putPt = (uint16_t*)&buffer->is.fifo_u16->data[0];
			buffer->is.fifo_u16->handler_function = handler;
			buffer->flagSize = flagSize;

		}
	}

}

// ******* Buffer_put *******
// Public function that appends data to a specified buffer based on the
// buffer parameter structure provided.
//  Inputs: pointer to data, pointer to a buffer_param_t, data length
// Outputs: number of elements inserted successfully
uint16_t Buffer_put( volatile void *in_buf, 
	buffer_param_t *buffer, 
	uint16_t length )
{
	uint16_t num_queued;

	cm_disable_interrupts();
	/*
	*  Call the specialized helper function corresponding to the supplied 
	*  buffer parameters.
	*/
	switch ( buffer->type )
	{

		case FIFO_U8T:
			num_queued = buffer_fifo_u8_put( in_buf, buffer->is.fifo_u8, length );
			Buffer_flagSizeAdd( buffer->flagSize, num_queued );
			cm_enable_interrupts();
			return num_queued; // return number of data added to the buffer
			break;


		case FIFO_U16T:
			num_queued = buffer_fifo_u16_put( in_buf, buffer->is.fifo_u16, length );
			Buffer_flagSizeAdd( buffer->flagSize, num_queued );
			//Test_in( &a_test_table, num_queued );
			cm_enable_interrupts();
			return num_queued; // return number of data added to the buffer
			break;
	}

	cm_enable_interrupts();
	return 0;
}

// ******* Buffer_get *******
// Public function that retrieves and removes data from a buffer corresponding
// to the supplied buffer parameter structure.
//  Inputs: data pointer, buffer_param_t pointer, and number of elements to
//          read.
// Outputs: number of elements read into the data pointer.
uint16_t Buffer_get( volatile void *out_buf, 
	buffer_param_t *buffer, 
	uint16_t length )
{
	uint16_t num_read;

	cm_disable_interrupts();
	/*
	*  Call the specialized helper function corresponding to the supplied 
	*  buffer parameters.
	*/
	switch ( buffer->type )
	{

		case FIFO_U8T:
			num_read = buffer_fifo_u8_get( out_buf, buffer->is.fifo_u8, length );
			Buffer_flagSizeSub( buffer->flagSize, num_read );
			cm_enable_interrupts();
			return num_read;
			break;

		case FIFO_U16T:

			num_read = buffer_fifo_u16_get( out_buf, buffer->is.fifo_u16, length );
			Buffer_flagSizeSub( buffer->flagSize, num_read );
			//Test_out( &a_test_table, num_read );
			cm_enable_interrupts();
			return num_read;
			break;

	}

	cm_enable_interrupts();
	return 0;
}

// ******* buffer_fifo_u8_put *******
// Private function that appends data to a supplied buffer.
//  Inputs: pointer to data, pointer to a buffer_fifo_t, data length
// Outputs: number of elements inserted successfully
uint16_t buffer_fifo_u8_put( volatile void *in_buf, 
	struct buffer_fifo_u8 *b_u8t, 
	uint16_t length )
{

	uint16_t j;
	uint16_t nextPutIndex;
	volatile uint8_t *p;

	p = in_buf;

	for ( j = 0; j < length; j++ )
	{
		/* Testing for buffer overflow
		*  First handle wrapping if we've reached the buffer size. Check to see
		*  if there's space in the queue: full condition reached when the
		*  advancing putIndex meets the getIndex. We avoid corrupting the queue
		*  by writing to its index variables only after this test passes.
		*/ 

		nextPutIndex = b_u8t->putIndex + 1;

		if ( nextPutIndex == ( b_u8t->size - 1 ) )
		{
			nextPutIndex = 0;
		}
		
		if ( nextPutIndex == b_u8t->getIndex )
		{
			return j; // no vacancy, return number of elements fetched
		}

		b_u8t->data[b_u8t->putIndex] = *p++;
		// Set putIndex to the next element
		b_u8t->putIndex = nextPutIndex;
		
	}
	return j; // return number of data added to the buffer
}

// ******* buffer_fifo_u8_get *******
// Private function to retrieve and remove data from a specified buffer.
//  Inputs: data pointer, buffer_fifo_t pointer, and number of elements to read
// Outputs: number of elements read into the data pointer.
uint16_t buffer_fifo_u8_get( volatile void *out_buf, 
	struct buffer_fifo_u8 *b_u8t, 
	uint16_t length )
{

	uint16_t j;
	volatile uint8_t *p;

	p = out_buf;

	for ( j = 0; j < length; j++ ) 
	{	
		// Check for get & put index collision
		if ( b_u8t->getIndex != b_u8t->putIndex )
		{
			*p++ = b_u8t->data[b_u8t->getIndex];

			b_u8t->getIndex += 1;

			// Check for index wrap-around
			if ( b_u8t->getIndex == ( b_u8t->size - 1 ) )
			{
				b_u8t->getIndex = 0;
			}

		}
		else
		{

			// Nothing left, return number of elements retrieved.
			return j;
		}
	}

	return j;
}

// ******* buffer_fifo_u16_put *******
// Private function that appends data to a supplied buffer.
//  Inputs: pointer to data, pointer to a buffer_fifo_t, data length
// Outputs: number of elements inserted successfully
uint16_t buffer_fifo_u16_put( volatile void *in_buf, 
	struct buffer_fifo_u16 *b_u16t, 
	uint16_t length )
{

	uint16_t j;
	uint16_t *nextPutPt;

	volatile uint16_t *p;

	p = in_buf;

	for ( j = 0; j < length; j++ )
	{
		/* Testing for buffer overflow
		*  First handle wrapping if we've reached the buffersize. Check to see
		*  if there's space in the queue: full condition reached when the
		*  advancing putPt meets the getPt. 
		*/ 

		nextPutPt = (uint16_t*)( b_u16t->putPt + 1 );

		if ( nextPutPt == (uint16_t*)&b_u16t->data[B_SIZE_FIFO_U16T] ) 
		{
			nextPutPt = (uint16_t*)&b_u16t->data[0];
		}
		
		if ( nextPutPt == b_u16t->getPt )
		{

			Uart_send( "x", 1 );
			return j; // no vacancy, return number of elements fetched
		}

		(*b_u16t->putPt) = *p++;
		// Set putPt address to the next element
		b_u16t->putPt = nextPutPt;
		
	}

	return j; // return number of data added to the queue
}

// ******* buffer_fifo_u16_get *******
// Private function to retrieve and remove data from a specified buffer.
//  Inputs: data pointer, buffer_fifo_t pointer, and number of elements to read
// Outputs: number of elements read into the data pointer.
uint16_t buffer_fifo_u16_get( volatile void *out_buf, 
	struct buffer_fifo_u16 *b_u16t, 
	uint16_t length )
{

	uint16_t j;
	volatile uint16_t *p;

	p = out_buf;

	for ( j = 0; j < length; j++ ) 
	{	
		// Check for get & put pointer collision
		if ( b_u16t->getPt != b_u16t->putPt )
		{

			*p++ = (*b_u16t->getPt);

			b_u16t->getPt = (uint16_t*)&b_u16t->getPt[1];

			// Check for pointer wrap-around
			if ( b_u16t->getPt == (uint16_t*)&b_u16t->data[B_SIZE_FIFO_U16T] )
			{
				b_u16t->getPt = (uint16_t*)&b_u16t->data[0];
			}

		}
		else
		{
			// Nothing left, return number of elements retrieved.
			Uart_send( "j", 1 );
			return j;
		}
	}

	return j;
}

// ******* Buffer_flagSizeInit *******
// Initialize a buffer size counting flag
//  Inputs: pointer to a buffer size flag
// Outputs: none
void Buffer_flagSizeInit( int32_t *flagPt )
{

	(*flagPt) = 0;

}


// ******* Buffer_flagSizeAdd *******
// Decrement semaphore, blocking task if less than zero
//  Inputs: pointer to a flag, number of elements to add
// Outputs: none
void Buffer_flagSizeAdd( int32_t *flagPt, uint16_t num_elements )
{

	(*flagPt) += num_elements;

}

// ******* Buffer_flagSizeSub *******
// Subtract  buffer size flag
//  Inputs: pointer to a flag, number of elements to substract
// Outputs: none
void Buffer_flagSizeSub( int32_t *flagPt, uint16_t num_elements )
{

	(*flagPt) -= num_elements;

	if ( (*flagPt) < 0 )
	{
		(*flagPt) = 0;
	}

}