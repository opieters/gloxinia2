#include <utilities.h>

char print_buffer[DEBUG_BUFFER_DEPTH][DEBUG_BUFFER_LENGTH];
uint8_t print_buffer_idx = 0;

void clear_buffer(uint8_t* buffer, size_t length)
{
    for(size_t i = 0; i < length; i++)
    {
        buffer[i] = 0;
    }
}

bool buffer_is_empty(uint8_t *buffer, size_t length)
{
    for(int j = 0; j < length; j++)
    {
        if(buffer[j] != 0)
            return false;
    }
    return true;
}