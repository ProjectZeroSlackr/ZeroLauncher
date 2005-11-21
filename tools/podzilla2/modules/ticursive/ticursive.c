/*
 * Copyright (C) 2005 Jonathan Bettencourt (jonrelay)
 *
 * Cursive concept by Peter Burk, implemented by jonrelay
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 *  /!\ Warning! This is NOT the same as legacy cursive.c!
 */

#include "pz.h"
#include <string.h>

static PzModule * module;

const char * ti_cursive_records[] = {
	"l", "-", "_",
	"ll", "l", "L",
	"lll", "c", "C",
	"llll", "o", "O",
	"lllll", "0", ")",
	"llllo", "q", "Q",
	"llllr", "a", "A",
	"lllo", "g", "G",
	"llloo", "\\", "|",
	"lllr", "u", "U",
	"lllro", "b", "B",
	"llo", "6", "^",
	"lloo", "(", "[",
	"llooo", "[", "{",
	"llor", "4", "$",
	"llorr", "9", "(",
	"lo", "h", "H",
	"lol", "w", "W",
	"lolo", "=", "+",
	"lorr", "5", "%",
	"lor", "s", "S",
	"loro", "8", "*",
	"lro", "k", "K",
	"lrrr", "n", "N",
	"o", "x", "X",
	"ol", "v", "V",
	"oll", "\1", "\1",
	"olll", "e", "E",
	"oo", ".", ">",
	"ooo", ":", "\"",
	"ooor", ";", "\'",
	"oor", ",", "<",
	"or", "y", "Y",
	"orr", "\2", "\2",
	"orrr", "\x0A", "\x0A",
	"r", "1", "!",
	"ro", "t", "T",
	"rol", "z", "Z",
	"ror", "i", "I",
	"rorr", "3", "#",
	"rr", "r", "R",
	"rrl", "~", "`",
	"rro", "f", "F",
	"rrol", "2", "@",
	"rroo", ")", "]",
	"rrooo", "]", "}",
	"rror", "7", "&",
	"rrr", "j", "J",
	"rrrlo", "?", "/",
	"rrro", "p", "P",
	"rrroo", "/", "?",
	"rrrr", "d", "D",
	"rrrro", "m", "M"
};
const int ti_cursive_record_count = 51;

static char ti_cursive_buffer[6];
static int ti_cursive_buffer_pos = 0;
static int ti_cursive_shift = 0;

extern TWidget * ti_create_tim_widget(int ht, int wd);

void ti_cursive_reset(void)
{
	ti_cursive_buffer[0] = 0;
	ti_cursive_buffer_pos = 0;
	ti_cursive_shift = 0;
}

char ti_cursive_get_char(void)
{
	int i;
	for (i=0; i<ti_cursive_record_count; i++) {
		if (strcmp(ti_cursive_buffer, ti_cursive_records[i*3]) == 0) {
			if (ti_cursive_shift != 0) {
				return ti_cursive_records[i*3+2][0];
			} else {
				return ti_cursive_records[i*3+1][0];
			}
		}
	}
	return 0;
}

void ti_cursive_push(void)
{
	char c;
	c = ti_cursive_get_char();
	if (c != 0) { ttk_input_char(c); }
	ti_cursive_buffer[0] = 0;
	ti_cursive_buffer_pos = 0;
}

void ti_cursive_draw(TWidget * wid, ttk_surface srf)
{
	char s[2];
	int x, y, w;
	x = wid->x;
	w = wid->w;
	y = wid->y + 8 - ttk_text_height(ttk_menufont)/2;
	
	ttk_fillrect(srf, wid->x, wid->y, wid->x+wid->w, wid->y+wid->h, ti_ap_get(0));
	ttk_text(srf, ttk_menufont, x+2, y, ti_ap_get(1), ti_cursive_buffer);
	s[0] = ti_cursive_get_char();
	s[1] = 0;
	if (s[0]>32) {
		ttk_text(srf, ttk_menufont, x+w-40, y, ti_ap_get(1), s);
	} else {
		switch(s[0]) {
		case 0:
			break;
		case 32:
			ttk_text(srf, ttk_menufont, x+w-40, y, ti_ap_get(1), "space");
			break;
		case 1:
			ttk_text(srf, ttk_menufont, x+w-40, y, ti_ap_get(1), "left");
			break;
		case 2:
			ttk_text(srf, ttk_menufont, x+w-40, y, ti_ap_get(1), "right");
			break;
		case 8:
			ttk_text(srf, ttk_menufont, x+w-40, y, ti_ap_get(1), "bksp");
			break;
		case 127:
			ttk_text(srf, ttk_menufont, x+w-40, y, ti_ap_get(1), "del");
			break;
		case 10:
		case 13:
			ttk_text(srf, ttk_menufont, x+w-40, y, ti_ap_get(1), "nwln");
			break;
		default:
			ttk_text(srf, ttk_menufont, x+w-40, y, ti_ap_get(1), "??");
			break;
		}
	}
	if (ti_cursive_shift != 0) {
		ttk_text(srf, ttk_menufont, x+w/2, y, ti_ap_get(1), "2nd");
	}
}

int ti_cursive_down(TWidget * wid, int btn)
{
	switch (btn) {
	case TTK_BUTTON_PLAY:
		ttk_widget_set_timer(wid, 0);
		ti_cursive_push();
		wid->dirty=1;
		break;
	case TTK_BUTTON_MENU:
		ttk_input_end();
		break;
	case TTK_BUTTON_PREVIOUS:
		ttk_input_char(TTK_INPUT_BKSP);
		break;
	case TTK_BUTTON_NEXT:
		ttk_input_char(32);
		break;
	default:
		return TTK_EV_UNUSED;
		break;
	}
	return TTK_EV_CLICK;
}

int ti_cursive_scroll(TWidget * wid, int dir)
{
	TTK_SCROLLMOD (dir,24) /* DO NOT change this value! */
	
	ttk_widget_set_timer(wid, 0);
	if (dir<0) {
		if (ti_cursive_buffer_pos < 5) {
			ti_cursive_buffer[ti_cursive_buffer_pos] = 'l';
			ti_cursive_buffer_pos++;
			ti_cursive_buffer[ti_cursive_buffer_pos] = 0;
		}
	} else {
		if (ti_cursive_buffer_pos < 5) {
			ti_cursive_buffer[ti_cursive_buffer_pos] = 'r';
			ti_cursive_buffer_pos++;
			ti_cursive_buffer[ti_cursive_buffer_pos] = 0;
		}
	}
	wid->dirty=1;
	ttk_widget_set_timer(wid, 1000);
	return TTK_EV_CLICK;
}

int ti_cursive_button(TWidget * wid, int btn, int t)
{
	if (btn != TTK_BUTTON_ACTION) return TTK_EV_UNUSED;
	if (t>1000) return TTK_EV_UNUSED;
	ttk_widget_set_timer(wid, 0);
	if (ti_cursive_buffer_pos < 5) {
		ti_cursive_buffer[ti_cursive_buffer_pos] = 'o';
		ti_cursive_buffer_pos++;
		ti_cursive_buffer[ti_cursive_buffer_pos] = 0;
	}
	wid->dirty=1;
	ttk_widget_set_timer(wid, 1000);
	return TTK_EV_CLICK;
}

int ti_cursive_held(TWidget * wid, int btn)
{
	if (btn != TTK_BUTTON_ACTION) return TTK_EV_UNUSED;
	ttk_widget_set_timer(wid, 0);
	if (ti_cursive_shift != 0) {
		ti_cursive_shift = 0;
	} else {
		ti_cursive_shift = 1;
	}
	wid->dirty=1;
	ttk_widget_set_timer(wid, 1000);
	return TTK_EV_CLICK;
}

int ti_cursive_timer(TWidget * wid)
{
	ttk_widget_set_timer(wid, 0);
	ti_cursive_push();
	wid->dirty=1;
	return 0;
}

TWidget * ti_cursive_create()
{
	TWidget * wid = ti_create_tim_widget(15, 0);
	wid->draw = ti_cursive_draw;
	wid->down = ti_cursive_down;
	wid->button = ti_cursive_button;
	wid->held = ti_cursive_held;
	wid->scroll = ti_cursive_scroll;
	wid->timer = ti_cursive_timer;
	ti_cursive_reset();
	return wid;
}

void ti_cursive_init()
{
	module = pz_register_module("ticursive", 0);
	ti_register(ti_cursive_create, ti_cursive_create, "Cursive", 5);
}

PZ_MOD_INIT(ti_cursive_init)
