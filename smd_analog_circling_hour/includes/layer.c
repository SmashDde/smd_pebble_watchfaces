/** \file
 * Text layer management.
 */

#include "pebble_th.h"

void
text_layer_setup(
	Window * window,
	TextLayer * layer,
	GRect frame,
	GFont font
)
{
	text_layer_init(layer, frame);
	text_layer_set_text(layer, "");
	text_layer_set_text_color(layer, GColorWhite);
	text_layer_set_background_color(layer, GColorClear);
	text_layer_set_font(layer, font);
        layer_add_child(&window->layer, &layer->layer);
}


void
monospace_text(
	GContext * ctx,
	const char * buf,
	GFont font,
	GPoint xy,
	int x_size,
	int y_size
)
{
	while (1)
	{
		const char c = *buf++;
		char str[2] = { c, '\0' };

		if (c == '\0')
			break;

		graphics_text_draw(
			ctx,
			str,
			font,
			GRect(xy.x, xy.y, x_size, y_size),
			GTextOverflowModeTrailingEllipsis,
			GTextAlignmentRight,
			NULL
		);

		xy.x += x_size;
	}
}
