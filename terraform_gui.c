#include "stdafx.h"
#include "ttd.h"

#include "window.h"
#include "gui.h"
#include "viewport.h"
#include "gfx.h"
#include "command.h"
#include "vehicle.h"


static void CcTerraform(bool success, uint tile, uint32 p1, uint32 p2)
{
	if (success) {
		SndPlayTileFx(0x1D, tile);
	} else {
		SetRedErrorSquare(_terraform_err_tile);
	}
}

static void GenericRaiseLowerLand(uint tile, int mode)
{
	if (mode) {
		DoCommandP(tile, 8, (uint32)mode, CcTerraform, CMD_TERRAFORM_LAND | CMD_AUTO | CMD_MSG(STR_0808_CAN_T_RAISE_LAND_HERE)); 
	} else {
		DoCommandP(tile, 8, (uint32)mode, CcTerraform, CMD_TERRAFORM_LAND | CMD_AUTO | CMD_MSG(STR_0809_CAN_T_LOWER_LAND_HERE)); 
	}
}


typedef void OnButtonClick(Window *w);

static const uint16 _terraform_keycodes[] = {
	'1',
	'2',
	'3',
	'4',
	'5',
	'6',
	'7',
};

void PlaceProc_DemolishArea(uint tile)
{
	VpStartPlaceSizing(tile, VPM_X_AND_Y);
}

void PlaceProc_RaiseLand(uint tile)
{
	GenericRaiseLowerLand(tile, 1);
}

void PlaceProc_LowerLand(uint tile)
{
	GenericRaiseLowerLand(tile, 0);
}

void PlaceProc_LevelLand(uint tile)
{
	VpStartPlaceSizing(tile, VPM_X_AND_Y | (2<<4));
}

void PlaceProc_PlantTree(uint tile)
{
}




static void TerraformClick_Dynamite(Window *w)
{
	HandlePlacePushButton(w, 2, ANIMCURSOR_DEMOLISH, 1, PlaceProc_DemolishArea);
}

static void TerraformClick_Lower(Window *w)
{
	HandlePlacePushButton(w, 3, ANIMCURSOR_LOWERLAND, 2, PlaceProc_LowerLand);
}

static void TerraformClick_Raise(Window *w)
{
	HandlePlacePushButton(w, 4, ANIMCURSOR_RAISELAND, 2, PlaceProc_RaiseLand);
}

static void TerraformClick_Level(Window *w)
{
	HandlePlacePushButton(w, 5, SPR_OPENTTD_BASE+69, 2, PlaceProc_LevelLand);
}

static void TerraformClick_BuyLand(Window *w)
{
	HandlePlacePushButton(w, 6, 4792, 1, PlaceProc_BuyLand);
}

static void TerraformClick_Trees(Window *w)
{
	if (HandlePlacePushButton(w, 8, 0, 1, PlaceProc_PlantTree)) ShowBuildTreesToolbar();	
}

static void TerraformClick_PlaceSign(Window *w)
{
	HandlePlacePushButton(w, 9, 722, 1, PlaceProc_Sign);
}


static OnButtonClick * const _terraform_button_proc[] = {
	TerraformClick_Dynamite,
	TerraformClick_Lower,
	TerraformClick_Raise,
	TerraformClick_Level,
	TerraformClick_BuyLand,
	0,
	TerraformClick_Trees,
	TerraformClick_PlaceSign,
};



static void TerraformToolbWndProc(Window *w, WindowEvent *e)
{
	switch(e->event) {
	case WE_PAINT:
		DrawWindowWidgets(w);
		break;
	case WE_CLICK:
		if (e->click.widget >= 2 && e->click.widget != 7) {
			_terraform_button_proc[e->click.widget - 2](w);
		}
		break;

	case WE_KEYPRESS:
	{
		int i;
		for(i=0; i!=lengthof(_terraform_keycodes); i++)
			if (e->keypress.keycode == _terraform_keycodes[i]) {
				e->keypress.cont = false;
				_terraform_button_proc[i](w);
				break;
			}
	}
		break;

	case WE_PLACE_OBJ:
		_place_proc(e->place.tile);
		return;

	case WE_PLACE_DRAG:
		VpSelectTilesWithMethod(e->place.pt.x, e->place.pt.y, e->place.userdata & 0xF);
		break;

	case WE_PLACE_MOUSEUP:
		if (e->click.pt.x != -1) {
			uint start_tile = e->place.starttile;
			uint end_tile = e->place.tile;

			if (e->place.userdata == VPM_X_AND_Y) {
				DoCommandP(end_tile, start_tile, 0, CcPlaySound10, CMD_CLEAR_AREA | CMD_MSG(STR_00B5_CAN_T_CLEAR_THIS_AREA));
			} else if (e->place.userdata == (VPM_X_AND_Y | (2<<4))) {
				DoCommandP(end_tile, start_tile, 0, CcPlaySound10, CMD_LEVEL_LAND | CMD_AUTO);
			} else if (e->place.userdata == VPM_X_AND_Y_LIMITED) {
//				if (e->click.pt.x != -1) {
//					DoCommandP(e->place.tile, _tree_to_plant, e->place.starttile, NULL, 
//						CMD_PLANT_TREE | CMD_AUTO | CMD_MSG(STR_2805_CAN_T_PLANT_TREE_HERE));
			} else {
				assert(true);
			}
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
	} break;
	}
}

static const Widget _terraform_widgets[] = {
{ WWT_CLOSEBOX,     7,   0,  10,   0,  13, STR_00C5, STR_018B_CLOSE_WINDOW},
{  WWT_CAPTION,     7,  11, 157,   0,  13, STR_LANDSCAPING_TOOLBAR, STR_018C_WINDOW_TITLE_DRAG_THIS},


{    WWT_PANEL,     7,   0,  21,  14,  35,  703, STR_018D_DEMOLISH_BUILDINGS_ETC},
{    WWT_PANEL,     7,  22,  43,  14,  35,  695, STR_018E_LOWER_A_CORNER_OF_LAND},
{    WWT_PANEL,     7,  44,  65,  14,  35,  694, STR_018F_RAISE_A_CORNER_OF_LAND},
{    WWT_PANEL,     7,  66,  87,  14,  35,  SPR_OPENTTD_BASE+68, STR_LEVEL_LAND_TOOLTIP},
{    WWT_PANEL,     7,  88, 109,  14,  35, 4791, STR_0329_PURCHASE_LAND_FOR_FUTURE},
{    WWT_PANEL,     7, 110, 113,  14,  35,    0, 0x0},
{    WWT_PANEL,     7, 114, 135,  14,  35,  742, STR_0185_PLANT_TREES_PLACE_SIGNS},
{    WWT_PANEL,     7, 136, 157,  14,  35, SPR_OPENTTD_BASE+70, STR_0289_PLACE_SIGN},

{      WWT_LAST},
};

static const WindowDesc _terraform_desc = {
	640-158, 22+36, 158, 36,
	WC_SCEN_LAND_GEN,0,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET,
	_terraform_widgets,
	TerraformToolbWndProc
};



void ShowTerraformToolbar()
{
	AllocateWindowDescFront(&_terraform_desc, 0);
}
