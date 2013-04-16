/** \file
 * Word clock, with german time.
 * word clock source adapted from https://bitbucket.org/hudson/pebble
 *
 * ALMOST UNTESTED CODE DUE TO LIBPEBBLE NOT WORKING HERE AND IOS APP NOT YET APPROVED
 *
 * 2013 SmashD // smashd.de
 */
#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "pebble_th.h"
#include "layer.c"

#define UUID { 0x5D, 0xBE, 0xBB, 0x58, 0x3C, 0xF4, 0x4C, 0xAF, 0xB4, 0xAC, 0x02, 0x44, 0xA2, 0x99, 0x99, 0x99 },
PBL_APP_INFO(
	UUID,
	"Text DE",
	"smashd.de",
	3, 1, // Version
	RESOURCE_ID_IMAGE_MENU_ICON,
	APP_INFO_WATCH_FACE
);



static Window window;
static GFont font_small;
static GFont font_thin;
static GFont font_thick;

typedef struct
{
	TextLayer layer;
	PropertyAnimation anim;
	const char * text;
	const char * old_text;
} word_t;

static word_t min_word;
static word_t rel_word;
static word_t hour_word;
static word_t ampm_word;

int small_font_int_rel=0;
int small_font_int_ampm=0;

static const char *nums[] = {
	"",
	"eins",
	"zwei",
	"drei",
	"vier",
	"fuenf",
	"sechs",
	"sieben",
	"acht",
	"neun",
	"zehn",
	"elf",
	"zwoelf",
	"dreizehn",
	"vierzehn",
	"fuenfzehn",
	"sechzehn",
	"siebzehn",
	"achtzehn",
	"neunzehn",
	"zwanzig"
};

static const char *nums_und[] = {
	//for 31 - 39 because idk how to combine string* :P
	"",
	"ein und",
	"zwei und",
	"drei und",
	"vier und",
	"fuenf und",
	"sechs und",
	"siebe nund",
	"acht und",
	"neun und"
};

static const char *
min_string(
	int i
)
{
	return nums[i];
}

static const char *
min_string_und(
	int i
)
{
	return nums_und[i];
}


static const char *
hour_string(
	int h
)
{
	// only if it is actually midnight, not "before midnight"
	if ( (h == 0) || (h == 24) )
		return "mitter-\nnacht";

	if (h == 12)
		return "mittag";

	if (h < 12)
		return nums[h];
	else
		return nums[h - 12];
}


static void
german_format(
	int hour,
	int min
)
{
	small_font_int_rel = 0;
	if (min == 0)
	{
		min_word.text = "";
		rel_word.text = "";
		hour_word.text = hour_string(hour);
	} else
	if (min < 15)
	{
		// over the hour
		min_word.text = min_string(min);
		rel_word.text = "nach";
		hour_word.text = hour_string(hour);
	} else
	if (min == 15)
	{
		// over the hour
		min_word.text = "zwanzig";
		rel_word.text = "nach";
		hour_word.text = hour_string(hour);
	} else
	if (min < 30)
	{
		// over the kwart
		min_word.text = min_string(30 - min);
		rel_word.text = "vor halb";
		hour++;
		hour_word.text = hour_string(hour);
	} else
	if (min == 30)
	{
		// just the half
		min_word.text = "";
		rel_word.text = "halb";
		hour++;
		hour_word.text = hour_string(hour);
	} else
	if (min < 40)
	{
		// 31 - 39
		min_word.text = min_string_und(min - 30);
		rel_word.text = "dreissig nach";
		//hour++;
		hour_word.text = hour_string(hour);
		small_font_int_rel = 1;
	} else
	if (min == 40)
	{
		min_word.text = min_string(60 - min);
		rel_word.text = "vor";
		hour++;
		hour_word.text = hour_string(hour);
	} else
	if (min < 45)
	{
		// 41 - 44
		min_word.text = "viertel";
		rel_word.text = "vor";
		hour++;
		hour_word.text = hour_string(hour);
	} else
	if (min == 45)
	{
		// just the kwart
		min_word.text = "";
		rel_word.text = "viertel vor";
		hour++;
		hour_word.text = hour_string(hour);
	} else
	if (min < 60)
	{
		// over the kwart
		min_word.text = min_string(60 - min);
		rel_word.text = "vor";
		hour++;
		hour_word.text = hour_string(hour);
	}


	// at midnight and noon do not display an am/pm notation
	// but don't say "before midnight"
	if (hour == 0 || hour == 12)
	{
		// nothing to do
		ampm_word.text = "";
		small_font_int_ampm = 0;
	} else
	if (hour < 6)
		ampm_word.text = "nachts";
	else
	if (hour < 9)
		ampm_word.text = "frueh";
	else
	if ( (hour < 11) || (hour == 11 && min <= 30) )
		ampm_word.text = "vormittags";
	else
	if ( (hour < 13) || (hour == 13 && min <= 59) )
		ampm_word.text = "morgens";
	else
	if ( (hour < 17) || (hour == 17 && min <= 59) )
	{	
		ampm_word.text = "nachmittags";
		small_font_int_ampm = 1;
	}
	else
	if ( (hour < 21) || (hour == 21 && min <=59) )
		ampm_word.text = "abends";
	else
	if (hour <= 24)
		ampm_word.text = "nachts";
}


static void
update_word(
	word_t * const word
)
{
	text_layer_set_text(&word->layer, word->text);
	if (word->text != word->old_text)
		animation_schedule(&word->anim.animation);
}


/** Called once per minute */
static void
handle_tick(
	AppContextRef ctx,
	PebbleTickEvent * const event
)
{
	(void) ctx;
	const PblTm * const ptm = event->tick_time;

	int hour = ptm->tm_hour;
	int min = ptm->tm_min;

	ampm_word.old_text = ampm_word.text;
	hour_word.old_text = hour_word.text;
	rel_word.old_text = rel_word.text;
	min_word.old_text = min_word.text;

	german_format(hour,  min);

/*
	string_format_time(
		time_buffer,
		sizeof(time_buffer),
		"%H:%M",
		event->tick_time
	);
*/

	update_word(&ampm_word);
	update_word(&hour_word);
	update_word(&rel_word);
	update_word(&min_word);
}


static void
text_layer(
	word_t * word,
	GRect frame,
	GFont font
)
{
	text_layer_setup(&window, &word->layer, frame, font);

	GRect frame_right = frame;
	frame_right.origin.x = 150;

	property_animation_init_layer_frame(
		&word->anim,
		&word->layer.layer,
		&frame_right,
		&frame
	);

	animation_set_duration(&word->anim.animation, 1250);
	animation_set_curve(&word->anim.animation, AnimationCurveEaseIn);
}


static void
handle_init(
	AppContextRef ctx
)
{
	(void) ctx;

	window_init(&window, "Main");
	window_stack_push(&window, true);
	window_set_background_color(&window, GColorBlack);

//	resource_init_current_app(&RESOURCES);
	resource_init_current_app(&APP_RESOURCES);
	
	int y = 15;
	int h = 30;

	font_small = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ARIAL_22));
	font_thin = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ARIAL_28));
	font_thick = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ARIAL_BLACK_30));

	// Stack top to bottom.  Note that the hour can take up
	// two rows at midnight.
	if (small_font_int_ampm == 1)
		text_layer(&ampm_word, GRect(4, y + 3*h, 144, h+8), font_small);
	else
		text_layer(&ampm_word, GRect(4, y + 3*h, 144, h+8), font_thin);

	text_layer(&hour_word, GRect(4, y + 2*h-2, 144, 2*h+8), font_thick);

	if (small_font_int_rel == 1)
		text_layer(&rel_word, GRect(4, y + 34, 144, h+8), font_small);
	else
		text_layer(&rel_word, GRect(4, y + 1*h, 144, h+8), font_thin);

	text_layer(&min_word, GRect(4, y + 0*h, 144, h+8), font_thin);

}


static void
handle_deinit(
	AppContextRef ctx
)
{
	(void) ctx;

	fonts_unload_custom_font(font_small);
	fonts_unload_custom_font(font_thin);
	fonts_unload_custom_font(font_thick);
}


void
pbl_main(
	void * const params
)
{
	PebbleAppHandlers handlers = {
		.init_handler	= &handle_init,
		.deinit_handler = &handle_deinit,
		.tick_info	= {
			.tick_handler = &handle_tick,
			.tick_units = MINUTE_UNIT,
		},
	};

	app_event_loop(params, &handlers);
}
