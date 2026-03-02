#ifndef UTILS_H
#define UTILS_H

#include <pebble.h>
#include "config.h"

extern void scale_pdc(GDrawCommandImage *image, int original_size, int new_size);
void load_pdc_icon(GDrawCommandImage **icon, uint32_t resource_id, int orig_icon_size, int target_icon_size);

#endif // UTILS_H