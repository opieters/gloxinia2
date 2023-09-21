#ifndef __FILTERING_H__
#define	__FILTERING_H__

#include <xc.h>

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * Copy elements from row to column format.
     * 
     * Initially, samples are stored as A0, A1, A2, ... A0, A1, A2 ...
     * and are copied into a destination array: A0, A0, ... 
     * 
     * Note that only A0 is copied into the destination.
     * 
     * @param num_elements number of elements in the output array
     * @param dst destination address
     * @param src source address in EDS space
     * @param step number of values to skip (1 for no skip)
     */
    void vector_copy_jump(int num_elements, fractional* dst, __eds__ fractional* src, int step);

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif

