#define LEGACY_DOT_C
#define PZ_COMPAT
#include "pz.h"

extern ttk_gc pz_root_gc;
static TWindow *pz_last_window;
static char *pz_next_header;

void pz_old_event_handler (t_GR_EVENT *ev) 
{
    int sdir = 0;
    if (ttk_windows->w->focus) {
	if (ev->keystroke.ch == 'r')
	    sdir = 1;
	else if (ev->keystroke.ch == 'l')
	    sdir = -1;
	
	switch (ev->type) {
	case GR_EVENT_TYPE_KEY_DOWN:
	    if (sdir)
		ttk_windows->w->focus->scroll (ttk_windows->w->focus, sdir);
	    else
		ttk_windows->w->focus->down (ttk_windows->w->focus, ev->keystroke.ch);
	    break;
	case GR_EVENT_TYPE_KEY_UP:
	    if (!sdir)
		ttk_windows->w->focus->button (ttk_windows->w->focus, ev->keystroke.ch, 0);
	    break;
	}
    }
}

void pz_draw_header (char *str) 
{
    if (pz_last_window)
	ttk_window_set_title (pz_last_window, str);
    else
	pz_next_header = str;
}

typedef struct 
{
    void (*draw)();
    int (*key)(GR_EVENT *);
} legacy_data;

#define _MAKETHIS legacy_data *data = (legacy_data *)this->data

void pz_legacy_construct_GR_EVENT (GR_EVENT *ev, int type, int arg) 
{
    ev->type = type;
    if (ev->type == GR_EVENT_TYPE_KEY_UP || ev->type == GR_EVENT_TYPE_KEY_DOWN) {
	if (arg == TTK_BUTTON_ACTION)
	    arg = '\r';
	ev->keystroke.ch = arg;
	ev->keystroke.scancode = 0;
    }
}
void pz_legacy_draw (TWidget *this, ttk_surface srf) 
{
    _MAKETHIS;
    data->draw(); // it'll be on the window we returned from pz_new_window
}
int pz_legacy_button (TWidget *this, int button, int time)
{
    GR_EVENT ev;
    _MAKETHIS;
    pz_legacy_construct_GR_EVENT (&ev, GR_EVENT_TYPE_KEY_UP, button);
    this->dirty++;
    return data->key (&ev);
}
int pz_legacy_down (TWidget *this, int button)
{
    GR_EVENT ev;
    _MAKETHIS;
    pz_legacy_construct_GR_EVENT (&ev, GR_EVENT_TYPE_KEY_DOWN, button);
    this->dirty++;
    return data->key (&ev);
}
int pz_legacy_scroll (TWidget *this, int dir)
{
#ifdef IPOD
#define SPER 4
    static int sofar = 0;
    sofar += dir;
    if (sofar > -SPER && sofar < SPER) return 0;
    dir = sofar / SPER;
    sofar -= SPER*dir;
#endif

    GR_EVENT ev;
    int key = 'r';
    int ret = 0;
    _MAKETHIS;

    if (dir < 0) {
	key = 'l';
	dir = -dir;
    }
    
    while (dir) {
	pz_legacy_construct_GR_EVENT (&ev, GR_EVENT_TYPE_KEY_DOWN, key);
	ret |= data->key (&ev);
	pz_legacy_construct_GR_EVENT (&ev, GR_EVENT_TYPE_KEY_UP, key);
	ret |= data->key (&ev);
	dir--;
    }
    this->dirty++;
    return ret;
}
int pz_legacy_timer (TWidget *this)
{
    GR_EVENT ev;
    _MAKETHIS;
    pz_legacy_construct_GR_EVENT (&ev, GR_EVENT_TYPE_TIMER, 0);
    this->dirty++;
    return data->key (&ev);
}
void pz_legacy_destroy (TWidget *this)
{
    free (this->data);
}


// Make widget 0 by 0 -- many old games draw to window only when they need to.
// ttk_run() blanks a WxH region before calling draw(), and draw() might
// only draw a few things.
TWidget *pz_new_legacy_widget (void (*do_draw)(), int (*do_keystroke)(GR_EVENT *))
{
    TWidget *ret = ttk_new_widget (0, 0);
    ret->w = 0;
    ret->h = 0;
    ret->data = calloc (1, sizeof(legacy_data));
    ret->focusable = 1;
    ret->dirty = 1;
    ret->draw = pz_legacy_draw;
    ret->button = pz_legacy_button;
    ret->down = pz_legacy_down;
    ret->scroll = pz_legacy_scroll;
    ret->timer = pz_legacy_timer;
    ret->destroy = pz_legacy_destroy;

    legacy_data *data = (legacy_data *)ret->data;
    data->draw = do_draw;
    data->key = do_keystroke;

    return ret;
}

TWindow *pz_mh_legacy (ttk_menu_item *item) 
{
    TWindow *old = ttk_windows->w;
    void (*newwin)() = (void (*)())item->data;
    (*newwin)();
    if (ttk_windows->w != old) {
	item->sub = ttk_windows->w;
	return TTK_MENU_ALREADYDONE;
    } else {
	item->flags &= ~TTK_MENU_MADESUB;
	return TTK_MENU_DONOTHING;
    }
}

t_GR_WINDOW_ID pz_old_window (int x, int y, int w, int h, void(*do_draw)(void), int(*do_keystroke)(t_GR_EVENT * event))
{
    TWindow *ret = ttk_new_window();
    fprintf (stderr, "Legacy code alert!\n");
    ttk_fillrect (ret->srf, 0, 0, ret->w, ret->h, ttk_makecol (255, 255, 255));
    ret->x = x;
    ret->y = y;
    if ((y == HEADER_TOPLINE) || (y == HEADER_TOPLINE + 1))
	ret->y = ttk_screen->wy, ret->h = ttk_screen->h - ttk_screen->wy;
    else if (!y)
	ret->show_header = 0;
    ret->w = w;
    ret->h = h;
    ttk_add_widget (ret, pz_new_legacy_widget (do_draw, do_keystroke));

    pz_last_window = ret;

    if (pz_next_header) {
	ttk_window_set_title (ret, pz_next_header);
	pz_next_header = 0;
    }

    return ret;
}

void pz_old_close_window(t_GR_WINDOW_ID wid)
{
    ttk_hide_window (wid);
    // pick new top legacy window:
    if (ttk_windows->w->focus && ttk_windows->w->focus->draw == pz_legacy_draw)
	pz_last_window = ttk_windows->w; // pick new top window
    else
	pz_last_window = 0;
    wid->data = 0x12345678; // hey menu: free it & recreate
}

t_GR_GC_ID pz_get_gc(int copy) 
{
    return (copy? t_GrCopyGC (pz_root_gc) : pz_root_gc);
}
