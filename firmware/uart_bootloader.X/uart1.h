#ifndef _UART1_H
#define _UART1_H

/**
  Section: Included Files
*/

#include <stdbool.h>
#include <stdint.h>
#include "bootloader_config.h"
#include "bootloader.h"


#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif

/**
  Section: UART1 Driver Routines
*/
/**
  @Summary
    Initializes the UART

  @Description
    This routine initializes the UART module.
    This routine must be called before any other UART routine 
    is called.
    
  @Preconditions
    None.

  @Returns
    None.

  @Param
    None.

  @Comment
    
*/
void uart1_init(void);

void uart1_deactivate(void);


/**
 * 
 * @return 
 */
void bootloader_copy_message(const bootloader_message_t* m_src, bootloader_message_t* m_dst);

/**
 * 
 * @return 
 */
void uart1_tx_message(bootloader_message_t* m);

/**
 * 
 * 
 * @return 
 */
void uart1_wait_tx(void);

/**
  @Summary
    Read a byte of data from the UART1

  @Description
    This routine reads a byte of data from the UART1.  It will
    block until a byte of data is available.  If you do not wish to block, call 
    the UART1_IsTxReady() function to check to see if there is
    data available to read first.

  @Preconditions
    UART1_Initialize function should have been called 
    before calling this function. 

  @Param
    None.

  @Returns
    A data byte received by the driver.
*/
uint8_t UART1_Read(void);

/**
  @Summary
    Writes a byte of data to the UART1

  @Description
    This routine writes a byte of data to the UART1. This function
    will block if this transmit buffer is currently full until the transmit 
    buffer becomes available.  If you do not wish to block, call the
    UART1_IsTxReady() function to check on the transmit
    availability.

  @Preconditions
    UART1_Initialize() function should have been called
    before calling this function.

  @Param
    byte - data to be written
*/
void UART1_Write(uint8_t byte);

/**
  @Description
    Indicates of there is data available to read.

  @Returns
    true if byte can be read.
    false if byte can't be read right now.
*/
bool UART1_IsRxReady(void);

/**
  @Description
    Indicates if a byte can be written.
 
 @Returns
    true if byte can be written.
    false if byte can't be written right now.
*/
bool UART1_IsTxReady(void);

/**
  @Description
    Indicates if all bytes have been transferred.
 
 @Returns
    true if all bytes transfered.
    false if there is still data pending to transfer.
*/
bool UART1_IsTxDone(void);

#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif

#endif  // _UART1_H

