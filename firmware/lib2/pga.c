#include <xc.h> 
#include <libpic30.h>
#include <pga.h>
#include <spi.h>
#include <utilities.h>


void pga_init(pga_config_t* config){
    // configure nCS pin as output 
    CLEAR_BIT(config->cs_pin.tris_r, config->cs_pin.n);
    SET_BIT(config->cs_pin.lat_r, config->cs_pin.n);
    
    __delay_ms(1);
    
    pga_update_status(config, PGA_STATUS_OFF);
}

void pga_update_status(pga_config_t* config, pga_status_t status){
    spi_message_t m;
    uint16_t write_buffer[1], read_buffer[1];
    
    config->status = status;
    
    m.cs_pin = config->cs_pin;
    
    m.write_data = write_buffer;
    m.read_data = read_buffer;
    m.data_length = 1;
    
    switch(status){
        case PGA_STATUS_ON:
            // enable device
            write_buffer[0] = 0xE100;
            
            m.status = SPI_TRANSFER_PENDING;
            config->spi_message_handler(&m);
            
            // send the switch channel command
            write_buffer[0] = 0x2A00 
                    | (config->gain << 4) 
                    | config->channel;
            m.status = SPI_TRANSFER_PENDING;
            config->spi_message_handler(&m);
            
            
            break;
        case PGA_STATUS_OFF:
        case PGA_STATUS_ERROR:
            // disable device
            
            write_buffer[0] = 0xE1F1;
            
            m.status = SPI_TRANSFER_PENDING;
            config->spi_message_handler(&m);
            break;
        default:
            break;
    }

    
}

