#include "stdafx.h"
#include "ttd.h"

#include "window.h"
#include "gui.h"
#include "viewport.h"
#include "gfx.h"
#include "command.h"

static void ShowBuildDockStationPicker();
static void ShowBuildDocksDepotPicker();

static byte _ship_depot_direction;

static void CcBuildDocks(bool success, uint tile, uint32 p1, uint32 p2)
{
	if (success) {
		SndPlayTileFx(0, tile);
		ResetObjectToPlace();
	}
}

static void CcBuildCanal(bool success, uint tile, uint32 p1, uint32 p2)
{
	if (success) { SndPlayTileFx(0, tile); }
}


static void PlaceDocks_Dock(uint tile)
{
	DoCommandP(tile, 0, 0, CcBuildDocks, CMD_BUILD_DOCK | CMD_AUTO | CMD_MSG(STR_9802_CAN_T_BUILD_DOCK_HERE));
}

static void PlaceDocks_Depot(uint tile)
{
	DoCommandP(tile, _ship_depot_direction, 0, CcBuildDocks, CMD_BUILD_SHIP_DEPOT | CMD_AUTO | CMD_MSG(STR_3802_CAN_T_BUILD_SHIP_DEPOT));
}

static void PlaceDocks_Buoy(uint tile)
{
	DoCommandP(tile, 0, 0, CcBuildDocks, CMD_BUILD_BUOY | CMD_AUTO | CMD_MSG(STR_9835_CAN_T_POSITION_BUOY_HERE));
}

static void PlaceDocks_DemolishArea(uint tile)
{
	VpStartPlaceSizing(tile, VPM_X_AND_Y);
}

static void PlaceDocks_BuildCanal(uint tile)
{
	VpStartPlaceSizing(tile, VPM_X_OR_Y);
}

static void PlaceDocks_BuildLock(uint tile)
{
	DoCommandP(tile, 0, 0, CcBuildDocks, CMD_BUILD_LOCK | CMD_AUTO | CMD_MSG(STR_CANT_BUILD_LOCKS));
}


static void BuildDocksClick_Dock(Window *w)
{

	if (HandlePlacePushButton(w, 2, 0xE54, 3, PlaceDocks_Dock)) ShowBuildDockStationPicker();
}

static void BuildDocksClick_Depot(Window *w)
{
	if (HandlePlacePushButton(w, 3, 0x2D1, 1, PlaceDocks_Depot)) ShowBuildDocksDepotPicker();
}

static void BuildDocksClick_Buoy(Window *w)
{
	HandlePlacePushButton(w, 4, 0x2BE, 1, PlaceDocks_Buoy);
}

static void BuildDocksClick_Demolish(Window *w)
{
	HandlePlacePushButton(w, 5, ANIMCURSOR_DEMOLISH, 1, PlaceDocks_DemolishArea);
}

static void BuildDocksClick_Lower(Window *w)
{
	HandlePlacePushButton(w, 6, ANIMCURSOR_LOWERLAND, 2, PlaceProc_LowerLand);
}

static void BuildDocksClick_Raise(Window *w)
{
	HandlePlacePushButton(w, 7, ANIMCURSOR_RAISELAND, 2, PlaceProc_RaiseLand);
}

static void BuildDocksClick_Purchase(Window *w)
{
	HandlePlacePushButton(w, 8, 0x12B8, 1, PlaceProc_BuyLand);
}

static void BuildDocksClick_Canal(Window *w)
{
	HandlePlacePushButton(w, 9, SPR_OPENTTD_BASE + 11, 1, PlaceDocks_BuildCanal);
}

static void BuildDocksClick_Lock(Window *w)
{
	HandlePlacePushButton(w, 10, SPR_OPENTTD_BASE + 64, 1, PlaceDocks_BuildLock);
}

typedef void OnButtonClick(Window *w);
static OnButtonClick * const _build_docks_button_proc[] = {
	BuildDocksClick_Dock,
	BuildDocksClick_Depot,
	BuildDocksClick_Buoy,
	BuildDocksClick_Demolish,
	BuildDocksClick_Lower,
	BuildDocksClick_Raise,
	BuildDocksClick_Purchase,
	BuildDocksClick_Canal,
	BuildDocksClick_Lock,
};

static void BuildDocksToolbWndProc(Window *w, WindowEvent *e)
{
	switch(e->event) {
	case WE_PAINT:
		DrawWindowWidgets(w);
		break;

	case WE_CLICK: {
		if (e->click.widget - 2 >= 0) _build_docks_button_proc[e->click.widget - 2](w);
	} break;

	case WE_PLACE_OBJ:
		_place_proc(e->place.tile);
		break;	

	case WE_PLACE_DRAG: {
		VpSelectTilesWithMethod(e->place.pt.x, e->place.pt.y, e->place.userdata);
		return;
	}

	case WE_PLACE_MOUSEUP:
		if (e->click.pt.x != -1) {
			if (e->place.userdata == VPM_X_AND_Y)
				DoCommandP(e->place.tile, e->place.starttile, 0, CcPlaySound10, CMD_CLEAR_AREA | CMD_MSG(STR_00B5_CAN_T_CLEAR_THIS_AREA));
			else if(e->place.userdata == VPM_X_OR_Y)
				DoCommandP(e->place.tile, e->place.starttile, 0, CcBuildCanal, CMD_BUILD_CANAL | CMD_AUTO | CMD_MSG(STR_CANT_BUILD_CANALS));
		}
		break;

	case WE_ABORT_PLACE_OBJ:
		w->click_state = 0;
		SetWindowDirty(w);

		w = FindWindowById(WC_BUILD_STATION, 0);
		if (w != NULL) WP(w,def_d).close=true;

		w = FindWindowById(WC_BUILD_DEPOT, 0);
		if (w != NULL) WP(w,def_d).close=true;
		break;
	
	case WE_PLACE_PRESIZE: {
		uint tile_from, tile_to;

		tile_from = tile_to = e->place.tile;
		switch(GetTileSlope(tile_from, NULL)) {
		case 3: tile_to += TILE_XY(-1,0); break;
		case 6:	tile_to += TILE_XY(0,-1);	break;
		case 9:	tile_to += TILE_XY(0,1);	break;
		case 12:tile_to += TILE_XY(1,0);	break;
		}
		VpSetPresizeRange(tile_from, tile_to);
	} break;
	}
}

static const Widget _build_docks_toolb_widgets[] = {
{   WWT_CLOSEBOX,     7,     0,    10,     0,    13, STR_00C5,										STR_018B_CLOSE_WINDOW},
{    WWT_CAPTION,     7,    11,   197,     0,    13, STR_9801_DOCK_CONSTRUCTION,	STR_018C_WINDOW_TITLE_DRAG_THIS},
{      WWT_PANEL,     7,     0,    21,    14,    35, 746,													STR_981D_BUILD_SHIP_DOCK},
{      WWT_PANEL,     7,    22,    43,    14,    35, 748,													STR_981E_BUILD_SHIP_DEPOT_FOR_BUILDING},
{      WWT_PANEL,     7,    44,    65,    14,    35, 693,													STR_9834_POSITION_BUOY_WHICH_CAN},
{      WWT_PANEL,     7,    66,    87,    14,    35, 703,													STR_018D_DEMOLISH_BUILDINGS_ETC},
{      WWT_PANEL,     7,    88,   109,    14,    35, 695,													STR_018E_LOWER_A_CORNER_OF_LAND},
{      WWT_PANEL,     7,   110,   131,    14,    35, 694,													STR_018F_RAISE_A_CORNER_OF_LAND},
{      WWT_PANEL,     7,   132,   153,    14,    35, 4791,												STR_0329_PURCHASE_LAND_FOR_FUTURE},
{      WWT_PANEL,     7,   154,   175,    14,    35, SPR_OPENTTD_BASE+65,					STR_BUILD_CANALS_TIP},
{      WWT_PANEL,     7,   176,   197,    14,    35, SPR_CANALS_BASE+69,					STR_BUILD_LOCKS_TIP},
{      WWT_LAST},
};

static const WindowDesc _build_docks_toolbar_desc = {
	640-197, 22, 198, 36,
	WC_BUILD_TOOLBAR,0,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET,
	_build_docks_toolb_widgets,
	BuildDocksToolbWndProc
};

void ShowBuildDocksToolbar()
{
	DeleteWindowById(WC_BUILD_TOOLBAR, 0);
	AllocateWindowDesc(&_build_docks_toolbar_desc);
}

static void BuildDockStationWndProc(Window *w, WindowEvent *e)
{
	switch(e->event) {
	case WE_PAINT: {
		if (WP(w,def_d).close)
			return;
		DrawWindowWidgets(w);
		DrawStationCoverageAreaText(2, 15, (uint)-1);
	} break;

	case WE_CLICK: {
		if (e->click.widget == 0) {
			ResetObjectToPlace();
		}
	} break;

	case WE_MOUSELOOP: {
		if (WP(w,def_d).close) {
			DeleteWindow(w);
			return;
		}

		CheckRedrawStationCoverage(w);
		break;
	}
	}	
}

static const Widget _build_dock_station_widgets[] = {
{   WWT_CLOSEBOX,     7,     0,    10,     0,    13, STR_00C5,			STR_018B_CLOSE_WINDOW},
{    WWT_CAPTION,     7,    11,   147,     0,    13, STR_3068_DOCK,	STR_018C_WINDOW_TITLE_DRAG_THIS},
{      WWT_PANEL,     7,     0,   147,    14,    45, 0x0,						0},
{      WWT_LAST},
};

static const WindowDesc _build_dock_station_desc = {
	-1, -1, 148, 46,
	WC_BUILD_STATION,WC_BUILD_TOOLBAR,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET,
	_build_dock_station_widgets,
	BuildDockStationWndProc
};

static void ShowBuildDockStationPicker()
{
	AllocateWindowDesc(&_build_dock_station_desc);
}

static void UpdateDocksDirection()
{
	if (_ship_depot_direction != 0) {
		SetTileSelectSize(1, 2);
	} else {
		SetTileSelectSize(2, 1);
	}
}

static void BuildDocksDepotWndProc(Window *w, WindowEvent *e)
{
	switch(e->event) {
	case WE_PAINT:
		w->click_state = (1<<3) << _ship_depot_direction;
		DrawWindowWidgets(w);

		DrawShipDepotSprite(67, 35, 0);
		DrawShipDepotSprite(35, 51, 1);
		DrawShipDepotSprite(135, 35, 2);
		DrawShipDepotSprite(167, 51, 3);
		return;

	case WE_CLICK: {
		switch(e->click.widget) {
		case 0:
			ResetObjectToPlace();
			break;
		case 3:
		case 4:
			_ship_depot_direction = e->click.widget - 3;
			SndPlayFx(0x13);
			UpdateDocksDirection();
			SetWindowDirty(w);
			break;
		}
	} break;

	case WE_MOUSELOOP:
		if (WP(w,def_d).close)
			DeleteWindow(w);
		break;
	}
}

static const Widget _build_docks_depot_widgets[] = {
{   WWT_CLOSEBOX,     7,     0,    10,     0,    13, STR_00C5,												STR_018B_CLOSE_WINDOW},
{    WWT_CAPTION,     7,    11,   203,     0,    13, STR_3800_SHIP_DEPOT_ORIENTATION,	STR_018C_WINDOW_TITLE_DRAG_THIS},
{      WWT_PANEL,     7,     0,   203,    14,    85, 0x0,															0},
{      WWT_PANEL,    14,     3,   100,    17,    82, 0x0,															STR_3803_SELECT_SHIP_DEPOT_ORIENTATION},
{      WWT_PANEL,    14,   103,   200,    17,    82, 0x0,															STR_3803_SELECT_SHIP_DEPOT_ORIENTATION},
{      WWT_LAST},
};

static const WindowDesc _build_docks_depot_desc = {
	-1, -1, 204, 86,
	WC_BUILD_DEPOT,WC_BUILD_TOOLBAR,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET,
	_build_docks_depot_widgets,
	BuildDocksDepotWndProc
};


static void ShowBuildDocksDepotPicker()
{
	AllocateWindowDesc(&_build_docks_depot_desc);
	UpdateDocksDirection();
}


void InitializeDockGui()
{
	_ship_depot_direction = 0;
}
