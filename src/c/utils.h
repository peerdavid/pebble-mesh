#ifndef UTILS_H
#define UTILS_H

#include <pebble.h>
#include "config.h"

#define ORIG_ICON_SIZE 25
#define ICON_SIZE 32

extern void scale_pdc(GDrawCommandImage *image, int original_size, int new_size);
extern void load_pdc_icon(GDrawCommandImage **icon, uint32_t resource_id);

#endif // UTILS_H