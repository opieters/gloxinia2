#ifndef __SPI_H__
#define	__SPI_H__

#include <xc.h> 
#include <utilities.h>


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief SPI status
    */
    typedef enum {
        SPI_TRANSFER_PENDING,  ///< SPI transfer pending
        SPI_TRANSFER_DONE,     ///< SPI transfer done
        SPI_TRANSFER_ONGOING,  ///< SPI transfer ongoing
        N_SPI_STATUS_T         ///< Number of SPI status types
    } spi_status_t;
    
    /**
     * @brief SPI message structure
    */
    typedef struct {
        uint16_t* write_data;       ///< Pointer to the data to be written
        uint16_t* read_data;        ///< Pointer to the data to be read
        uint8_t data_length;        ///< Length of the data to be written/read
        pin_t cs_pin;               ///< Chip select pin
        spi_status_t status;        ///< SPI status
    } spi_message_t;
    

    /// @brief SPI module selector
    typedef enum {
        SPI_MODULE_SELECTOR_1,      ///< SPI module 1
        SPI_MODULE_SELECTOR_2,      ///< SPI module 2
        SPI_MODULE_SELECTOR_3,      ///< SPI module 3
        //SPI_MODULE_SELECTOR_4,
    } spi_module_selector_t;
    
    
    spi_message_t* spi_init_message(spi_message_t* m, uint16_t* write_data, uint16_t* read_data, uint8_t length, const pin_t* pin);
    
    /// @brief SPI1 module initialisation
    void spi1_init(void);

    /// @brief SPI2 module initialisation
    void spi2_init(void);

    /// @brief SPI3 module initialisation
    void spi3_init(void);
    
    /**
     * @brief Send message in SPI1
     * 
     * @param m: pointer to the message to be sent
     */
    void spi1_send_message(spi_message_t* m);

    /**
     * @brief Send message in SPI2
     * 
     * @param m: pointer to the message to be sent
     */
    void spi2_send_message(spi_message_t* m);

    /**
     * @brief Send message in SPI2
     * 
     * @param m: pointer to the message to be sent
     */
    void spi3_send_message(spi_message_t* m);
    
    /// @brief spi handler type (function pointer)
    typedef void (*spi_handler_t)(spi_message_t*);
    
    /**
     * @brief Send SPI message using software SPI
     * 
     * @param sck: pointer to the SCK pin
     * @param sdo: pointer to the SDO pin
     * @param sdi: pointer to the SDI pin
     * @param cs: pointer to the CS pin
     * @param tx_data: pointer to the data to be sent
     * @param rx_data: pointer to the data to be received
     * @param n: number of bytes to be sent/received
     * @param period: period of the clock in us
    */
    void spi_software_send(pin_t* sck, pin_t* sdo, pin_t* sdi, pin_t* cs, uint8_t* tx_data, uint8_t* rx_data, uint8_t n, uint16_t period);
    
    /**
     * @brief Get SPI handler for a specific SPI module
     * 
     * @param spi_module: SPI module selector
    */
    spi_handler_t spi_get_handler(const spi_module_selector_t spi_module);

#ifdef	__cplusplus
}
#endif

#endif	/* __SPI_H__ */

