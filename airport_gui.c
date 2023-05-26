#include "stdafx.h"
#include "ttd.h"

#include "window.h"
#include "gui.h"
#include "viewport.h"
#include "gfx.h"
#include "command.h"
#include "vehicle.h"
#include "station.h"
#include "airport.h"

static byte _selected_airport_type;

static void ShowBuildAirportPicker();


static void CcBuildAirport(bool success, uint tile, uint32 p1, uint32 p2)
{
	if (success) {
		SndPlayTileFx(0x1D, tile);
		ResetObjectToPlace();
	}
}

static void PlaceAirport(uint tile)
{
	DoCommandP(tile, _selected_airport_type, 0, CcBuildAirport, CMD_BUILD_AIRPORT | CMD_AUTO | CMD_NO_WATER | CMD_MSG(STR_A001_CAN_T_BUILD_AIRPORT_HERE));
}

static void PlaceAir_DemolishArea(uint tile)
{
	VpStartPlaceSizing(tile, 4);
}


static void BuildAirClick_Airport(Window *w)
{
	if (HandlePlacePushButton(w, 2, 0xAA4, 1, PlaceAirport)) ShowBuildAirportPicker();
}

static void BuildAirClick_Demolish(Window *w)
{
	HandlePlacePushButton(w, 3, ANIMCURSOR_DEMOLISH, 1, PlaceAir_DemolishArea);
}

static void BuildAirClick_Lower(Window *w)
{
	HandlePlacePushButton(w, 4, ANIMCURSOR_LOWERLAND, 2, PlaceProc_LowerLand);
}

static void BuildAirClick_Raise(Window *w)
{
	HandlePlacePushButton(w, 5, ANIMCURSOR_RAISELAND, 2, PlaceProc_RaiseLand);
}

static void BuildAirClick_Purchase(Window *w)
{
	HandlePlacePushButton(w, 6, 0x12B8, 1, PlaceProc_BuyLand);
}

typedef void OnButtonClick(Window *w);
static OnButtonClick * const _build_air_button_proc[] = {
	BuildAirClick_Airport,
	BuildAirClick_Demolish,
	BuildAirClick_Lower,
	BuildAirClick_Raise,
	BuildAirClick_Purchase,
};

static void BuildAirToolbWndProc(Window *w, WindowEvent *e)
{
	switch(e->event) {
	case WE_PAINT:
		DrawWindowWidgets(w);
		break;

	case WE_CLICK:
		if (e->click.widget-2 >= 0)
			_build_air_button_proc[e->click.widget - 2](w);		
		break;

	case WE_PLACE_OBJ:
		_place_proc(e->place.tile);
		break;	

	case WE_PLACE_DRAG: {
		VpSelectTilesWithMethod(e->place.pt.x, e->place.pt.y, e->place.userdata);
		return;
	}

	case WE_PLACE_MOUSEUP:
		if (e->place.pt.x != -1) {
			DoCommandP(e->place.tile, e->place.starttile, 0, CcPlaySound10, CMD_CLEAR_AREA | CMD_MSG(STR_00B5_CAN_T_CLEAR_THIS_AREA));
		}
		break;

	case WE_ABORT_PLACE_OBJ:
		w->click_state = 0;
		SetWindowDirty(w);
		w = FindWindowById(WC_BUILD_STATION, 0);
		if (w != 0)
			WP(w,def_d).close = true;
		break;
	}
}

static const Widget _air_toolbar_widgets[] = {
{   WWT_CLOSEBOX,     7,     0,    10,     0,    13, STR_00C5,										STR_018B_CLOSE_WINDOW},
{    WWT_CAPTION,     7,    11,   129,     0,    13, STR_A000_AIRPORT_CONSTRUCT,	STR_018C_WINDOW_TITLE_DRAG_THIS},
{      WWT_PANEL,     7,     0,    41,    14,    35, 0x2E8,												STR_A01E_BUILD_AIRPORT},
{      WWT_PANEL,     7,    42,    63,    14,    35, 0x2BF,												STR_018D_DEMOLISH_BUILDINGS_ETC},
{      WWT_PANEL,     7,    64,    85,    14,    35, 0x2B7,												STR_018E_LOWER_A_CORNER_OF_LAND},
{      WWT_PANEL,     7,    86,   107,    14,    35, 0x2B6,												STR_018F_RAISE_A_CORNER_OF_LAND},
{      WWT_PANEL,     7,   108,   129,    14,    35, 0x12B7,											STR_0329_PURCHASE_LAND_FOR_FUTURE},
{      WWT_LAST},
};


static const WindowDesc _air_toolbar_desc = {
	510, 22, 130, 36,
	WC_BUILD_TOOLBAR,0,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET,
	_air_toolbar_widgets,
	BuildAirToolbWndProc
};

void ShowBuildAirToolbar()
{
	DeleteWindowById(WC_BUILD_TOOLBAR, 0);
	AllocateWindowDescFront(&_air_toolbar_desc, 0);
}

static void BuildAirportPickerWndProc(Window *w, WindowEvent *e)
{
	switch(e->event) {
	case WE_PAINT: {
		int sel;

		if (WP(w,def_d).close)
			return;
		w->disabled_state = 0;

		sel = _selected_airport_type;
		// FIXME -- BuildAirportPickerWndProc - set availability of airports by year, instead of airplane
		if (!(_avail_aircraft & 1)) { w->disabled_state |= (1<<3); if (sel == AT_SMALL) sel = AT_LARGE; }
		if (!(_avail_aircraft & 2)) {	w->disabled_state |= (1<<4); if (sel == AT_LARGE) sel = AT_SMALL; }
		if (!(_avail_aircraft & 4)) {	w->disabled_state |= (1<<5); }	// heliport
		// 1980-1-1 is --> 21915
		// 1990-1-1 is --> 25568
		if (_date < 21915) {w->disabled_state |= (1<<6);}	// metropilitan airport 1980
		if (_date < 25568) {w->disabled_state |= (1<<7);}	// international airport 1990
		_selected_airport_type = sel;
		// select default the coverage area to 'Off' (8)
		w->click_state = ((1<<3) << sel) | ((1<<8) << _station_show_coverage);
		SetTileSelectSize(_airport_size_x[sel],_airport_size_y[sel]);
		if (_station_show_coverage)	SetTileSelectBigSize(-4, -4, 8, 8);

		DrawWindowWidgets(w);
    // strings such as 'Size' and 'Coverage Area'
		DrawStringCentered(74, 16, STR_305B_SIZE, 0);
		DrawStringCentered(74, 78, STR_3066_COVERAGE_AREA_HIGHLIGHT, 0);
		DrawStationCoverageAreaText(2, 104, (uint)-1);		
		break;
	}

	case WE_CLICK: {
		switch(e->click.widget) {
		case 0:
			ResetObjectToPlace();
			break;
		case 3: case 4: case 5: case 6: case 7:
			_selected_airport_type = e->click.widget - 3;
			SndPlayFx(0x13);
			SetWindowDirty(w);
			break;
		case 8: case 9:
			_station_show_coverage = e->click.widget - 8;
			SndPlayFx(0x13);
			SetWindowDirty(w);
			break;
		}
	} break;

	case WE_MOUSELOOP: {
		if (WP(w,def_d).close) {
			DeleteWindow(w);
			return;
		}

		CheckRedrawStationCoverage(w);
	} break;
	}
}

static const Widget _build_airport_picker_widgets[] = {
{   WWT_CLOSEBOX,     7,     0,    10,     0,    13, STR_00C5,										STR_018B_CLOSE_WINDOW},
{    WWT_CAPTION,     7,    11,   147,     0,    13, STR_3001_AIRPORT_SELECTION,	STR_018C_WINDOW_TITLE_DRAG_THIS},
{      WWT_PANEL,     7,     0,   147,    14,   130, 0x0,													0},
{WWT_NODISTXTBTN,    14,     2,    73,    27,    38, STR_3059_SMALL,							STR_3058_SELECT_SIZE_TYPE_OF_AIRPORT},
{WWT_NODISTXTBTN,    14,    74,   145,    27,    38, STR_305A_LARGE,							STR_3058_SELECT_SIZE_TYPE_OF_AIRPORT},
{WWT_NODISTXTBTN,    14,     2,   145,    63,    74, STR_306B_HELIPORT,						STR_3058_SELECT_SIZE_TYPE_OF_AIRPORT},
{WWT_NODISTXTBTN,    14,     2,   145,    39,    50, STR_305AA_LARGE,	  					STR_3058_SELECT_SIZE_TYPE_OF_AIRPORT},
{WWT_NODISTXTBTN,    14,     2,   145,    51,    62, STR_305AB_LARGE,	  					STR_3058_SELECT_SIZE_TYPE_OF_AIRPORT},
{   WWT_CLOSEBOX,    14,    14,    73,    88,    98, STR_02DB_OFF,								STR_3065_DON_T_HIGHLIGHT_COVERAGE},
{   WWT_CLOSEBOX,    14,    74,   133,    88,    98, STR_02DA_ON,									STR_3064_HIGHLIGHT_COVERAGE_AREA},
{      WWT_LAST},
};

static const WindowDesc _build_airport_desc = {
	-1, -1, 148, 131, // height, 130+1
	WC_BUILD_STATION,WC_BUILD_TOOLBAR,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET,
	_build_airport_picker_widgets,
	BuildAirportPickerWndProc
};

static void ShowBuildAirportPicker()
{
	AllocateWindowDesc(&_build_airport_desc);
}

void InitializeAirportGui()
{
	_selected_airport_type = AT_SMALL;
	_last_built_aircraft_depot_tile = 0;
}
