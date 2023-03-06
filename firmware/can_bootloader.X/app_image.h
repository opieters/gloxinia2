#ifndef __APP_IMAGE_H__
#define __APP_IMAGE_H__

#include <xc.h>
#include <stdbool.h>

typedef struct app_image_s
{
    uint32_t start_address;
} app_image_t;

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Initialize the bootloader application (not the image)
     *
     * @details This function sets the default values of the bootloader. It is
     * not used to initialise the system, see system_initialise for that.
     */
    void bootloader_init(void);

    /**
     * @brief Run the bootloader application
     */
    void bootloader_run(void);

#ifdef __cplusplus
}
#endif

#endif
