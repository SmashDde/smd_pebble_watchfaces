/** \file
  Mix of digital and analog time where the digital hour circles around the big digital minute.
 
  2013 SmashD // smashd.de
 
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
 
#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#include "pebble_th.h"
#include "layer.c"

#define MY_UUID {0x13, 0x37, 0x13, 0x37, 0x13, 0x37, 0x13, 0x37, 0xAB, 0xCD, 0xDC, 0xBA, 0x11, 0x22, 0x33, 0x44}
PBL_APP_INFO(MY_UUID, "Circling Hours", "smashd.de", 1, 10, RESOURCE_ID_IMAGE_MENU_ICON, APP_INFO_WATCH_FACE);

Window window;

BmpContainer background_image_container;

RotBmpContainer hour_image_container_1;
RotBmpContainer hour_image_container_2;
RotBmpContainer hour_image_container_3;
RotBmpContainer hour_image_container_4;
RotBmpContainer hour_image_container_5;
RotBmpContainer hour_image_container_6;
RotBmpContainer hour_image_container_7;
RotBmpContainer hour_image_container_8;
RotBmpContainer hour_image_container_9;
RotBmpContainer hour_image_container_10;
RotBmpContainer hour_image_container_11;
RotBmpContainer hour_image_container_12;

static GFont font_minutes;

typedef struct
{
	TextLayer layer;
	PropertyAnimation anim;
	const char * text;
	const char * old_text;
} word_t;
static word_t font_minutes_word;
static word_t hours_word;
char * first_run;

/* -------------- TODO: Remove this and use Public API ! ------------------- */

// from src/core/util/misc.h

#define MAX(a,b) (((a)>(b))?(a):(b))

// From src/fw/ui/rotate_bitmap_layer.c

//! newton's method for floor(sqrt(x)) -> should always converge
static int32_t integer_sqrt(int32_t x) {
  if (x < 0) {
    ////    PBL_LOG(LOG_LEVEL_ERROR, "Looking for sqrt of negative number");
    return 0;
  }

  int32_t last_res = 0;
  int32_t res = (x + 1)/2;
  while (last_res != res) {
    last_res = res;
    res = (last_res + x / last_res) / 2;
  }
  return res;
}

void rot_bitmap_set_src_ic(RotBitmapLayer *image, GPoint ic) {
  image->src_ic = ic;

  // adjust the frame so the whole image will still be visible
  const int32_t horiz = MAX(ic.x, abs(image->bitmap->bounds.size.w - ic.x));
  const int32_t vert = MAX(ic.y, abs(image->bitmap->bounds.size.h - ic.y));

  GRect r = layer_get_frame(&image->layer);
  //// const int32_t new_dist = integer_sqrt(horiz*horiz + vert*vert) * 2;
  const int32_t new_dist = (integer_sqrt(horiz*horiz + vert*vert) * 2) + 1; //// Fudge to deal with non-even dimensions--to ensure right-most and bottom-most edges aren't cut off.

  r.size.w = new_dist;
  r.size.h = new_dist;
  layer_set_frame(&image->layer, r);

  r.origin = GPoint(0, 0);
  ////layer_set_bounds(&image->layer, r);
  image->layer.bounds = r;

  image->dest_ic = GPoint(new_dist / 2, new_dist / 2);

  layer_mark_dirty(&(image->layer));
}

/* ------------------------------------------------------------------------- */


void set_hand_angle(RotBmpContainer *hand_image_cont, unsigned int hand_angle) {

  signed short x_fudge = 0;
  signed short y_fudge = 0;


  hand_image_cont->layer.rotation =  TRIG_MAX_ANGLE * hand_angle / 360;

  //
  // Due to rounding/centre of rotation point/other issues of fitting
  // square pixels into round holes by the time hands get to 6 and 9
  // o'clock there's off-by-one pixel errors.
  //
  // The `x_fudge` and `y_fudge` values enable us to ensure the hands
  // look centred on the minute marks at those points. (This could
  // probably be improved for intermediate marks also but they're not
  // as noticable.)
  //
  // I think ideally we'd only ever calculate the rotation between
  // 0-90 degrees and then rotate again by 90 or 180 degrees to
  // eliminate the error.
  //
  if (hand_angle >= 180) {
    x_fudge = -1;
  }
  if ( (hand_angle > 225) && (hand_angle < 315) ) {
    x_fudge = -2;
  }
  x_fudge++;

  // (144 = screen width, 168 = screen height)
  hand_image_cont->layer.layer.frame.origin.x = (144/2) - (hand_image_cont->layer.layer.frame.size.w/2) + x_fudge;
  hand_image_cont->layer.layer.frame.origin.y = (168/2) - (hand_image_cont->layer.layer.frame.size.h/2) + y_fudge;

  layer_mark_dirty(&hand_image_cont->layer.layer);
}


void update_hand_positions() {

  PblTm t;
  get_time(&t);
	unsigned int hand_angle_precalc = ((t.tm_hour % 12) * 30) + (t.tm_min/2);

//if hour changed OR it's the first run
if ( (t.tm_min == 0) || (strcmp(first_run, "true") == 0) )
	{	
	
	  // Set up layers for the hour, first hide them
		layer_set_hidden(&hour_image_container_1.layer.layer, true);
		layer_set_hidden(&hour_image_container_2.layer.layer, true);
		layer_set_hidden(&hour_image_container_3.layer.layer, true);
		layer_set_hidden(&hour_image_container_4.layer.layer, true);
		layer_set_hidden(&hour_image_container_5.layer.layer, true);
		layer_set_hidden(&hour_image_container_6.layer.layer, true);
		layer_set_hidden(&hour_image_container_7.layer.layer, true);
		layer_set_hidden(&hour_image_container_8.layer.layer, true);
		layer_set_hidden(&hour_image_container_9.layer.layer, true);
		layer_set_hidden(&hour_image_container_10.layer.layer, true);
		layer_set_hidden(&hour_image_container_11.layer.layer, true);
		layer_set_hidden(&hour_image_container_12.layer.layer, true);
	
	
	//it's the first run
		if (strcmp(first_run, "true") != 0)
	  {
			//animation, quirky
/*			psleep(2000);
	 		layer_set_hidden(&hour_image_container_1.layer.layer, false);

	    const int max = hand_angle_precalc;
	    int i;
	    for(i = 0; i < max; i++) {
				if (i < 45)
					psleep(40);
				if (i < 135)
					psleep(10);
				if (i < 225)
					psleep(10);
				if (i < 359)
					psleep(10);
				set_hand_angle(&hour_image_container_1, i);
			}
*/
	    //for end
	  }
	
	
	/***DEBUG***/
	//	layer_set_hidden(&hour_image_container_10.layer.layer, false);
	//	set_hand_angle(&hour_image_container_3, 90);
	//	set_hand_angle(&hour_image_container_3, 180);
	//	set_hand_angle(&hour_image_container_3, 270);
	
	
	/*********************
	VISIBILITY
				layer_set_hidden(&hour_image_container_1.layer.layer, false);
	*/
	  if ( (t.tm_hour==1) || (t.tm_hour==13) )
	  	{ 
	  		layer_set_hidden(&hour_image_container_1.layer.layer, false);
			} else
	  if ( (t.tm_hour==2) || (t.tm_hour==14) )
	  	{ 
	  		layer_set_hidden(&hour_image_container_2.layer.layer, false);
			} else
	  if ( (t.tm_hour==3) || (t.tm_hour==15) )
	  	{ 
	  		layer_set_hidden(&hour_image_container_3.layer.layer, false);
			} else
	  if ( (t.tm_hour==4) || (t.tm_hour==16) )
	  	{ 
	  		layer_set_hidden(&hour_image_container_4.layer.layer, false);
			} else
	  if ( (t.tm_hour==5) || (t.tm_hour==17) )
	  	{ 
	  		layer_set_hidden(&hour_image_container_5.layer.layer, false);
			} else
	  if ( (t.tm_hour==6) || (t.tm_hour==18) )
	  	{ 
	  		layer_set_hidden(&hour_image_container_6.layer.layer, false);
			} else
	  if ( (t.tm_hour==7) || (t.tm_hour==19) )
	  	{ 
	  		layer_set_hidden(&hour_image_container_7.layer.layer, false);
			} else
	  if ( (t.tm_hour==8) || (t.tm_hour==20) )
	  	{ 
	  		layer_set_hidden(&hour_image_container_8.layer.layer, false);
			} else
	  if ( (t.tm_hour==9) || (t.tm_hour==21) )
	  	{ 
	  		layer_set_hidden(&hour_image_container_9.layer.layer, false);
			} else
	  if ( (t.tm_hour==10) || (t.tm_hour==22) )
	  	{ 
	  		layer_set_hidden(&hour_image_container_10.layer.layer, false);
			} else
	  if ( (t.tm_hour==11) || (t.tm_hour==23) )
	  	{ 
	  		layer_set_hidden(&hour_image_container_11.layer.layer, false);
			} else
	  if ( (t.tm_hour==12) || (t.tm_hour==24) || (t.tm_hour==0) )
	  	{ 
	  		layer_set_hidden(&hour_image_container_12.layer.layer, false);
			}
	}

//ALWAYS rotate the current hour_image_layer
/*********************
ROTATION
			set_hand_angle(&hour_image_container_1, hand_angle_precalc);
*/
  if ( (t.tm_hour==1) || (t.tm_hour==13) )
  	{ 
			set_hand_angle(&hour_image_container_1, hand_angle_precalc);
		} else
  if ( (t.tm_hour==2) || (t.tm_hour==14) )
  	{ 
			set_hand_angle(&hour_image_container_2, hand_angle_precalc);
		} else
  if ( (t.tm_hour==3) || (t.tm_hour==15) )
  	{ 
			set_hand_angle(&hour_image_container_3, hand_angle_precalc);
		} else
  if ( (t.tm_hour==4) || (t.tm_hour==16) )
  	{ 
			set_hand_angle(&hour_image_container_4, hand_angle_precalc);
		} else
  if ( (t.tm_hour==5) || (t.tm_hour==17) )
  	{ 
			set_hand_angle(&hour_image_container_5, hand_angle_precalc);
		} else
  if ( (t.tm_hour==6) || (t.tm_hour==18) )
  	{ 
			set_hand_angle(&hour_image_container_6, hand_angle_precalc);
		} else
  if ( (t.tm_hour==7) || (t.tm_hour==19) )
  	{ 
			set_hand_angle(&hour_image_container_7, hand_angle_precalc);
		} else
  if ( (t.tm_hour==8) || (t.tm_hour==20) )
  	{ 
			set_hand_angle(&hour_image_container_8, hand_angle_precalc);
		} else
  if ( (t.tm_hour==9) || (t.tm_hour==21) )
  	{ 
			set_hand_angle(&hour_image_container_9, hand_angle_precalc);
		} else
  if ( (t.tm_hour==10) || (t.tm_hour==22) )
  	{ 
			set_hand_angle(&hour_image_container_10, hand_angle_precalc);
		} else
  if ( (t.tm_hour==11) || (t.tm_hour==23) )
  	{ 
			set_hand_angle(&hour_image_container_11, hand_angle_precalc);
		} else
  if ( (t.tm_hour==12) || (t.tm_hour==24) || (t.tm_hour==0) )
  	{ 
			set_hand_angle(&hour_image_container_12, hand_angle_precalc);
		}


}

static void
text_layer(
	word_t * word,
	GRect frame,
	GFont font
)
{
	text_layer_setup(&window, &word->layer, frame, font);

	GRect frame_before = frame;
	frame_before.origin.y = 18;

	property_animation_init_layer_frame(
		&word->anim,
		&word->layer.layer,
		&frame_before,
		&frame
	);

	animation_set_duration(&word->anim.animation, 500);
	animation_set_curve(&word->anim.animation, AnimationCurveEaseIn);
}


static void
update_word(
	word_t * const word
)
{
	text_layer_set_text(&word->layer, word->text);
  text_layer_set_background_color(&word->layer,GColorClear);
  text_layer_set_text_color(&word->layer,GColorWhite);
/*
  text_layer_set_background_color(&word->layer,GColorWhite);
  text_layer_set_text_color(&word->layer,GColorBlack);
*/

/*	
	if (strcmp(word->text, "") == 0)
		//layer_set_hidden(&word->layer.layer, true);
		layer_set_hidden(min_word.layer, true);
	else
		layer_set_hidden(&word->layer.layer, false);
*/
	
	if (word->text != word->old_text)
		animation_schedule(&word->anim.animation);
}



void handle_tick(AppContextRef ctx, PebbleTickEvent *t) {
  (void)t;

	PblTm currentTime;
	static char timeTextM[] = "00";
	static char timeTextH[] = "00";

  first_run = "false";
	font_minutes_word.old_text = font_minutes_word.text;
	hours_word.old_text = hours_word.text;
	
	get_time(&currentTime);
	
	string_format_time(timeTextM, sizeof(timeTextM), "%M", &currentTime);
	font_minutes_word.text = timeTextM;

	string_format_time(timeTextH, sizeof(timeTextH), "%H", &currentTime);
	hours_word.text = timeTextH;
	
	update_word(&font_minutes_word);
  update_hand_positions();
}





void handle_init(AppContextRef ctx) {
  (void)ctx;

  window_init(&window, "Circling Hours");
  window_stack_push(&window, true);
	window_set_background_color(&window, GColorBlack);

  resource_init_current_app(&APP_RESOURCES);

	first_run = "true";
	hours_word.old_text = hours_word.text;

  bmp_init_container(RESOURCE_ID_IMAGE_HOUR_BG, &background_image_container);
  layer_add_child(&window.layer, &background_image_container.layer.layer);


//	font_minutes = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ARIAL_BLACK_50));
//	text_layer(&font_minutes_word, GRect(0, 40, 143, 100), font_minutes);
/*  add to resource_map.json:
  {
				"type":"font",
				"defName":"FONT_ARIAL_BLACK_50",
				"file":"fonts/Arial-Black.ttf"
  }
*/

// much less space needed because font is built in
//	font_minutes = fonts_get_system_font(FONT_KEY_GOTHAM_42_MEDIUM_NUMBERS);
	font_minutes = fonts_get_system_font(FONT_KEY_GOTHAM_42_BOLD);
	text_layer(&font_minutes_word, GRect(0, 58, 143, 50), font_minutes);

	text_layer_set_text_alignment(&font_minutes_word.layer, GTextAlignmentCenter);
	
	rotbmp_init_container(RESOURCE_ID_IMAGE_HOUR_1, &hour_image_container_1);
	rotbmp_init_container(RESOURCE_ID_IMAGE_HOUR_2, &hour_image_container_2);
	rotbmp_init_container(RESOURCE_ID_IMAGE_HOUR_3, &hour_image_container_3);
	rotbmp_init_container(RESOURCE_ID_IMAGE_HOUR_4, &hour_image_container_4);
	rotbmp_init_container(RESOURCE_ID_IMAGE_HOUR_5, &hour_image_container_5);
	rotbmp_init_container(RESOURCE_ID_IMAGE_HOUR_6, &hour_image_container_6);
	rotbmp_init_container(RESOURCE_ID_IMAGE_HOUR_7, &hour_image_container_7);
	rotbmp_init_container(RESOURCE_ID_IMAGE_HOUR_8, &hour_image_container_8);
	rotbmp_init_container(RESOURCE_ID_IMAGE_HOUR_9, &hour_image_container_9);
	rotbmp_init_container(RESOURCE_ID_IMAGE_HOUR_10, &hour_image_container_10);
	rotbmp_init_container(RESOURCE_ID_IMAGE_HOUR_11, &hour_image_container_11);
	rotbmp_init_container(RESOURCE_ID_IMAGE_HOUR_12, &hour_image_container_12);

	//GCompOpAssign, GCompOpAssignInverted, GCompOpOr, GCompOpAnd, GCompOpClear
	GCompOp mode = GCompOpAssign;
  hour_image_container_1.layer.compositing_mode = mode;
  hour_image_container_2.layer.compositing_mode = mode;
  hour_image_container_3.layer.compositing_mode = mode;
  hour_image_container_4.layer.compositing_mode = mode;
  hour_image_container_5.layer.compositing_mode = mode;
  hour_image_container_6.layer.compositing_mode = mode;
  hour_image_container_7.layer.compositing_mode = mode;
  hour_image_container_8.layer.compositing_mode = mode;
  hour_image_container_9.layer.compositing_mode = mode;
  hour_image_container_10.layer.compositing_mode = mode;
  hour_image_container_11.layer.compositing_mode = mode;
  hour_image_container_12.layer.compositing_mode = mode;

	//GPoint pointxy = GPoint(-73, -75);
	GPoint pointxy = GPoint(14, 70);
  rot_bitmap_set_src_ic(&hour_image_container_1.layer, pointxy);
  rot_bitmap_set_src_ic(&hour_image_container_2.layer, pointxy);
  rot_bitmap_set_src_ic(&hour_image_container_3.layer, pointxy);
  rot_bitmap_set_src_ic(&hour_image_container_4.layer, pointxy);
  rot_bitmap_set_src_ic(&hour_image_container_5.layer, pointxy);
  rot_bitmap_set_src_ic(&hour_image_container_6.layer, pointxy);
  rot_bitmap_set_src_ic(&hour_image_container_7.layer, pointxy);
  rot_bitmap_set_src_ic(&hour_image_container_8.layer, pointxy);
  rot_bitmap_set_src_ic(&hour_image_container_9.layer, pointxy);
  rot_bitmap_set_src_ic(&hour_image_container_10.layer, pointxy);
  rot_bitmap_set_src_ic(&hour_image_container_11.layer, pointxy);
  rot_bitmap_set_src_ic(&hour_image_container_12.layer, pointxy);

  layer_add_child(&window.layer, &hour_image_container_1.layer.layer);
  layer_add_child(&window.layer, &hour_image_container_2.layer.layer);
  layer_add_child(&window.layer, &hour_image_container_3.layer.layer);
  layer_add_child(&window.layer, &hour_image_container_4.layer.layer);
  layer_add_child(&window.layer, &hour_image_container_5.layer.layer);
  layer_add_child(&window.layer, &hour_image_container_6.layer.layer);
  layer_add_child(&window.layer, &hour_image_container_7.layer.layer);
  layer_add_child(&window.layer, &hour_image_container_8.layer.layer);
  layer_add_child(&window.layer, &hour_image_container_9.layer.layer);
  layer_add_child(&window.layer, &hour_image_container_10.layer.layer);
  layer_add_child(&window.layer, &hour_image_container_11.layer.layer);
  layer_add_child(&window.layer, &hour_image_container_12.layer.layer);


  update_hand_positions();


}



void handle_deinit(AppContextRef ctx) {
  (void)ctx;

  bmp_deinit_container(&background_image_container);
  rotbmp_deinit_container(&hour_image_container_1);
  rotbmp_deinit_container(&hour_image_container_2);
  rotbmp_deinit_container(&hour_image_container_3);
  rotbmp_deinit_container(&hour_image_container_4);
  rotbmp_deinit_container(&hour_image_container_5);
  rotbmp_deinit_container(&hour_image_container_6);
  rotbmp_deinit_container(&hour_image_container_7);
  rotbmp_deinit_container(&hour_image_container_8);
  rotbmp_deinit_container(&hour_image_container_9);
  rotbmp_deinit_container(&hour_image_container_10);
  rotbmp_deinit_container(&hour_image_container_11);
  rotbmp_deinit_container(&hour_image_container_12);

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


