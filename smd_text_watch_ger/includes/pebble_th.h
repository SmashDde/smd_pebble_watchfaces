/** \file
 * Functions in libpblth.a.
 */
#ifndef _pblth_h_
#define _pblth_h_

#include <stdint.h>
#include "pebble_os.h"


extern int
__attribute__((__format__(printf, 2, 3)))
pebble_sprintf(
	char * buf,
	const char * fmt,
	...
);


extern void
text_layer_setup(
	Window * window,
	TextLayer * layer,
	GRect frame,
	GFont font
);


void
monospace_text(
	GContext * ctx,
	const char * buf,
	GFont font,
	GPoint xy,
	int x_size,
	int y_size
);

#endif
