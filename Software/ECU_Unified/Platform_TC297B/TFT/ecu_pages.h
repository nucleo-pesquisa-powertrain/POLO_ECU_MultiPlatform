#ifndef ECU_PAGES_H
#define ECU_PAGES_H

#include "Ifx_Types.h"

void ECU_Pages_Init(void);    /* Show splash screen, init page 0 */
void ECU_Pages_Update(void);  /* Called every 100ms: update current page + check navigation */

#endif /* ECU_PAGES_H */
