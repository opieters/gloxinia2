#ifndef __FILTERING_H__
#define	__FILTERING_H__

#include <xc.h>

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

        void vector_copy_jump(int num_elements, fractional* dst, fractional* src, int kump);

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif

