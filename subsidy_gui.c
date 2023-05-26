#include "stdafx.h"
#include "ttd.h"
#include "window.h"
#include "gui.h"
#include "station.h"
#include "industry.h"
#include "town.h"
#include "player.h"
#include "gfx.h"
#include "economy.h"

static void HandleSubsidyClick(int y)
{			
	Subsidy *s;
	int num,offs;
	TileIndex xy;

	if (y < 0)
		return;

	num = 0;
	for(s=_subsidies; s != endof(_subsidies); s++) {
		if (s->cargo_type != 0xFF && s->age < 12) {
			y -= 10;
			if (y < 0) goto handle_click;
			num++;
		}
	}

	if (num == 0) {
		y -= 10;
		if (y < 0) return;
	}

	y -= 11;
	if (y < 0) return;

	for(s=_subsidies; s != endof(_subsidies); s++) {
		if (s->cargo_type != 0xFF && s->age >= 12) {
			y -= 10;
			if (y < 0) goto handle_click;
		}
	}
	return;
	
handle_click:

	/* determine from coordinate for subsidy and try to scroll to it */
	offs = s->from;
	if (s->age >= 12) {
		xy = DEREF_STATION(offs)->xy;
	} else if (s->cargo_type == CT_PASSENGERS || s->cargo_type == CT_MAIL) {
		xy = DEREF_TOWN(offs)->xy;
	} else {
		xy = _industries[offs].xy;
		
	}
	if (!ScrollMainWindowToTile(xy)) {
		/* otherwise determine to coordinate for subsidy and scroll to it */
		offs = s->to;
		if (s->age >= 12) {
			xy = DEREF_STATION(offs)->xy;
		} else if (s->cargo_type == CT_PASSENGERS || s->cargo_type == CT_MAIL || s->cargo_type == CT_GOODS || s->cargo_type == CT_FOOD) {
			xy = DEREF_TOWN(offs)->xy;
		} else {
			xy = _industries[offs].xy;
		}
		ScrollMainWindowToTile(xy);
	}
}

static void DrawSubsidiesWindow(Window *w)
{
	YearMonthDay ymd;
	Subsidy *s;
	int x,xt,y,num,x2;
	Player *p;

	DrawWindowWidgets(w);

	ConvertDayToYMD(&ymd, _date);

	y = 15;
	x = 1;
	DrawString(x, y, STR_2026_SUBSIDIES_ON_OFFER_FOR, 0);
	y += 10;
	num = 0;

	for(s=_subsidies; s != endof(_subsidies); s++) {
		if (s->cargo_type != 0xFF && s->age < 12) {
			SetupSubsidyDecodeParam(s, 1);
			x2 = DrawString(x+2, y, STR_2027_FROM_TO, 0);
			
			SET_DPARAM16(0, _date - ymd.day + 384 - s->age * 32);
			DrawString(x2, y, STR_2028_BY, 0);
			y += 10;
			num++;
		}
	}

	if (num == 0) {
		DrawString(x+2, y, STR_202A_NONE, 0);
		y += 10;
	}

	DrawString(x, y+1, STR_202B_SERVICES_ALREADY_SUBSIDISED, 0);
	y += 10;
	num = 0;

	for(s=_subsidies; s != endof(_subsidies); s++) {
		if (s->cargo_type != 0xFF && s->age >= 12) {
			SetupSubsidyDecodeParam(s, 1);

			p = DEREF_PLAYER(DEREF_STATION(s->to)->owner);
			SET_DPARAM16(3, p->name_1);
			SET_DPARAM32(4, p->name_2);

			xt = DrawString(x+2, y, STR_202C_FROM_TO, 0);

			SET_DPARAM16(0, _date - ymd.day + 768 - s->age * 32);
			DrawString(xt, y, STR_202D_UNTIL, 0);
			y += 10;
			num++;
		}
	}

	if (num == 0) {
		DrawString(x+2, y, STR_202A_NONE, 0);
	}
}

static void SubsidiesListWndProc(Window *w, WindowEvent *e)
{
	switch(e->event) {
	case WE_PAINT: DrawSubsidiesWindow(w); break;
	case WE_CLICK: {
		switch(e->click.widget) {
		case 2: HandleSubsidyClick(e->click.pt.y - 25); break;
		}
	}
	break;
	}
}

static const Widget _subsidies_list_widgets[] = {
{   WWT_CLOSEBOX,    13,     0,    10,     0,    13, STR_00C5, STR_018B_CLOSE_WINDOW},
{    WWT_CAPTION,    13,    11,   629,     0,    13, STR_2025_SUBSIDIES, STR_018C_WINDOW_TITLE_DRAG_THIS},
{      WWT_PANEL,    13,     0,   629,    14,   126, 0x0, STR_01FD_CLICK_ON_SERVICE_TO_CENTER},
{      WWT_LAST},
};

static const WindowDesc _subsidies_list_desc = {
	-1, -1, 630, 127,
	WC_SUBSIDIES_LIST,0,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET,
	_subsidies_list_widgets,
	SubsidiesListWndProc
};


void ShowSubsidiesList()
{
	AllocateWindowDescFront(&_subsidies_list_desc, 0);
}
