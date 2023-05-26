#include "stdafx.h"
#include "ttd.h"

#include "window.h"
#include "gui.h"
#include "viewport.h"
#include "gfx.h"
#include "player.h"
#include "command.h"
#include "news.h"
#include "town.h"
#include "vehicle.h"

#include "table/animcursors.h"


extern void DoTestSave();
extern void DoTestLoad();

extern bool disable_computer;

static int _rename_id;
static int _rename_what;

static byte _terraform_size = 1;
static byte _last_built_railtype;
extern void GenerateWorld(int mode);

extern void GenerateIndustries();
extern void GenerateTowns();

static void HandleOnEditText(WindowEvent *e) {
	byte *b = e->edittext.str;
	int id;
	memcpy(_decode_parameters, b, 32);

	id = _rename_id;
	
	switch(_rename_what) {
	case 0:
		// for empty string send "remove sign" parameter
		DoCommandP(0, id, (*b==0)?1:0, NULL, CMD_RENAME_SIGN | CMD_MSG(STR_280C_CAN_T_CHANGE_SIGN_NAME));
		break;
	case 1:
		if(*b == 0)
			return;
		DoCommandP(0, id, 0, NULL, CMD_RENAME_CHECKPOINT | CMD_MSG(STR_CANT_CHANGE_CHECKPOINT_NAME));
		break;
	}
}

// this code is shared for the majority of the pushbuttons
bool HandlePlacePushButton(Window *w, int widget, uint32 cursor, int mode, PlaceProc *placeproc)
{
	uint32 mask = 1 << widget;

	if (w->disabled_state & mask)
		return false;

	if (!_no_button_sound) SndPlayFx(0x13);
	SetWindowDirty(w);

	if (w->click_state & mask) {
		ResetObjectToPlace();
		return false;
	}

	SetObjectToPlace(cursor, mode, w->window_class, w->window_number);
	w->click_state |= mask;
	_place_proc = placeproc;
	return true;
}


void CcPlaySound10(bool success, uint tile, uint32 p1, uint32 p2)
{
	if (success) { SndPlayTileFx(0x10, tile); }
}


typedef void ToolbarButtonProc(Window *w);

static void ToolbarPauseClick(Window *w)
{
	if (DoCommandP(0, _pause?0:1, 0, NULL, CMD_PAUSE))
		SndPlayFx(0x13);
}

static void ToolbarFastForwardClick(Window *w)
{
	_fast_forward ^= true;
	SndPlayFx(0x13);
}


typedef void MenuClickedProc(int index);


void MenuClickSettings(int index)
{
	switch(index) {
	case 0: ShowGameOptions(); return;
	case 1: ShowGameDifficulty(); return;
	case 2: ShowPatchesSelection(); return;
	case 4: _display_opt ^= DO_SHOW_TOWN_NAMES; MarkWholeScreenDirty(); return;
	case 5: _display_opt ^= DO_SHOW_STATION_NAMES; MarkWholeScreenDirty(); return;
	case 6: _display_opt ^= DO_SHOW_SIGNS; MarkWholeScreenDirty(); return;
	case 7: _display_opt ^= DO_CHECKPOINTS; MarkWholeScreenDirty(); return;
	case 8: _display_opt ^= DO_FULL_ANIMATION; MarkWholeScreenDirty(); return;
	case 9: _display_opt ^= DO_FULL_DETAIL; MarkWholeScreenDirty(); return;
	case 10: _display_opt ^= DO_TRANS_BUILDINGS; MarkWholeScreenDirty(); return;
	}
}

void MenuClickSaveLoad(int index)
{
	if (_game_mode == GM_EDITOR) {
		switch(index) {
		case 0:
			ShowSaveLoadDialog(SLD_SAVE_SCENARIO);
			break;
		case 1:
			ShowSaveLoadDialog(SLD_LOAD_SCENARIO);
			break;
		case 2:
			AskExitToGameMenu();
			break;
		case 4:
			AskExitGame();
			break;
		}
	} else {
		switch(index) {
		case 0:
			ShowSaveLoadDialog(SLD_SAVE_GAME);
			break;
		case 1:
			ShowSaveLoadDialog(SLD_LOAD_GAME);
			break;
		case 2:
			AskExitToGameMenu();
			break;
		case 3:
			AskExitGame();
			break;
		}
	}
}

void MenuClickMap(int index)
{
	switch(index) {
	case 0: ShowSmallMap(); break;
	}
}

void MenuClickTown(int index)
{
	ShowTownDirectory();
}

void MenuClickScenMap(int index)
{
	switch(index) {
	case 0: ShowSmallMap(); break;
	case 1: ShowTownDirectory(); break;
	}
}

void MenuClickSubsidies(int index)
{
	ShowSubsidiesList();
}

void MenuClickStations(int index)
{
	ShowPlayerStations(index);
}

void MenuClickFinances(int index)
{
	ShowPlayerFinances(index);
}

void MenuClickCompany(int index)
{
	ShowPlayerCompany(index);
}
	

void MenuClickGraphs(int index)
{
	switch(index) {
	case 0: ShowOperatingProfitGraph(); return;
	case 1: ShowIncomeGraph(); return;
	case 2: ShowDeliveredCargoGraph(); return;
	case 3: ShowPerformanceHistoryGraph(); return;
	case 4: ShowCompanyValueGraph(); return;
	case 5: ShowCargoPaymentRates(); return;
	}
}

void MenuClickLeague(int index)
{
	ShowCompanyLeagueTable();
}

void MenuClickIndustry(int index)
{
	switch(index) {
	case 0: ShowBuildIndustryWindow(); break;
	case 1: ShowIndustryDirectory(); break;
	}
}

void MenuClickShowTrains(int index)
{
	ShowPlayerTrains(index);
}

void MenuClickShowRoad(int index)
{
	ShowPlayerRoadVehicles(index);
}

void MenuClickShowShips(int index)
{
	ShowPlayerShips(index);
}

void MenuClickShowAir(int index)
{
	ShowPlayerAircraft(index);
}

void MenuClickBuildRail(int index)
{
	Player *p = DEREF_PLAYER(_local_player);
	_last_built_railtype = min(index, p->max_railtype-1);
	ShowBuildRailToolbar(_last_built_railtype, -1);
}

void MenuClickBuildRoad(int index)
{
	ShowBuildRoadToolbar();
}

void MenuClickBuildWater(int index)
{
	ShowBuildDocksToolbar();
}

void MenuClickBuildAir(int index)
{
	ShowBuildAirToolbar();
}

void ShowRenameSignWindow(SignStruct *ss)
{
	_rename_id = ss - _sign_list;
	_rename_what = 0;
	ShowQueryString(ss->str, STR_280B_EDIT_SIGN_TEXT, 30, 180, 1, 0);
}

void ShowRenameCheckpointWindow(Checkpoint *cp)
{
	int id = cp - _checkpoints;
	_rename_id = id;
	_rename_what = 1;
	SET_DPARAM16(0, id);
	ShowQueryString(STR_CHECKPOINT_RAW, STR_EDIT_CHECKPOINT_NAME, 30, 180, 1, 0);
}

static void CcPlaceSign(bool success, uint tile, uint32 p1, uint32 p2)
{
	if (success) {
		ShowRenameSignWindow(_new_sign_struct);
		ResetObjectToPlace();
	}
}

void PlaceProc_Sign(uint tile)
{
	DoCommandP(tile, 0, 0, CcPlaceSign, CMD_PLACE_SIGN | CMD_MSG(STR_2809_CAN_T_PLACE_SIGN_HERE));
}

static void SelectSignTool()
{
	if (_cursor.sprite == 0x2D2)
		ResetObjectToPlace();
	else {
		SetObjectToPlace(0x2D2, 1, 1, 0);
		_place_proc = PlaceProc_Sign;
	}
}

void MenuClickForest(int index)
{
	switch(index) {
	case 0: ShowTerraformToolbar(); break;
	case 1: ShowBuildTreesToolbar(); break;
	case 2: SelectSignTool(); break;
	}
}

void MenuClickMusicWindow(int index)
{
	ShowMusicWindow();
}

void MenuClickNewspaper(int index)
{
	switch(index) {
	case 0: ShowLastNewsMessage(); break;
	case 1: ShowMessageOptions(); break;
	case 2: ShowMessageHistory(); break;
	case 3: ; /* XXX: chat not done */
	}
}

void MenuClickHelp(int index)
{
	switch(index) {
	case 0: PlaceLandBlockInfo(); break;
	case 2: _make_screenshot = 1; break;
	case 3: _make_screenshot = 2; break;
	case 4: ShowAboutWindow(); break;
	}
}

static MenuClickedProc * const _menu_clicked_procs[] = {
	NULL, /* 0 */
	NULL, /* 1 */
	MenuClickSettings, /* 2 */
	MenuClickSaveLoad, /* 3 */
	MenuClickMap, /* 4 */
	MenuClickTown, /* 5 */
	MenuClickSubsidies, /* 6 */
	MenuClickStations, /* 7 */
	MenuClickFinances, /* 8 */
	MenuClickCompany, /* 9 */
	MenuClickGraphs, /* 10 */
	MenuClickLeague, /* 11 */
	MenuClickIndustry, /* 12 */
	MenuClickShowTrains, /* 13 */
	MenuClickShowRoad, /* 14 */
	MenuClickShowShips, /* 15 */
	MenuClickShowAir, /* 16 */
	MenuClickScenMap,  /* 17 */
	NULL, /* 18 */
	MenuClickBuildRail, /* 19 */
	MenuClickBuildRoad, /* 20 */
	MenuClickBuildWater, /* 21 */
	MenuClickBuildAir, /* 22 */
	MenuClickForest, /* 23 */
	MenuClickMusicWindow, /* 24 */
	MenuClickNewspaper, /* 25 */
	MenuClickHelp, /* 26 */
};

static void MenuWndProc(Window *w, WindowEvent *e)
{
	switch(e->event) {
	case WE_PAINT: {
		int count,sel;
		int x,y;
		uint16 chk;
		StringID string;
		int eo;
		int inc;

		DrawWindowWidgets(w);

		count = WP(w,menu_d).item_count;
		sel = WP(w,menu_d).sel_index;
		chk = WP(w,menu_d).checked_items;
		string = WP(w,menu_d).string_id;

		x = 1;
		y = 1;

		eo = 157;

		inc = (chk != 0) ? 2 : 1;

		do {
			if (sel== 0) GfxFillRect(x, y, x + eo, y+9, 0);
			DrawString(x + 2, y, (StringID)(string + (chk&1)), (byte)(sel==0?(byte)0xC:(byte)0x10));
			y += 10;
			string += inc;
			chk >>= 1;
		} while (--sel,--count);
	} break;

	case WE_DESTROY: {
			Window *v = FindWindowById(WC_MAIN_TOOLBAR, 0);
			v->click_state &= ~(1 << WP(w,menu_d).main_button);
			SetWindowDirty(v);
			return;
		}
		
	case WE_POPUPMENU_SELECT: {
		int index = GetMenuItemIndex(w, e->popupmenu.pt.x, e->popupmenu.pt.y);
		int action_id;
		

		if (index < 0) {
			Window *w2 = FindWindowById(WC_MAIN_TOOLBAR,0);
			if (GetWidgetFromPos(w2, e->popupmenu.pt.x - w2->left, e->popupmenu.pt.y - w2->top) == WP(w,menu_d).main_button)
				index = WP(w,menu_d).sel_index;
		}

		action_id = WP(w,menu_d).action_id;
		DeleteWindow(w);
		
		if (index >= 0)
			_menu_clicked_procs[action_id](index);
	
		break;
		}
	case WE_POPUPMENU_OVER: {
		int index = GetMenuItemIndex(w, e->popupmenu.pt.x, e->popupmenu.pt.y);

		if (index == -1 || index == WP(w,menu_d).sel_index)
			return;

		WP(w,menu_d).sel_index = index;
		SetWindowDirty(w);
		return;
		}
	}
}

static Widget _menu_widgets[] = {
{      WWT_PANEL,    14,     0,   159,     0, 65535,     0},
{      WWT_LAST},
};


static Widget _player_menu_widgets[] = {
{      WWT_PANEL,    14,     0,   240,     0,    81,     0},
{      WWT_LAST},
};


static int GetPlayerIndexFromMenu(int index)
{
	Player *p;

	if (index >= 0) {
		FOR_ALL_PLAYERS(p) {
			if (p->is_active) {
				if (--index < 0)
					return p->index;
			}
		} 
	}
	return -1;
}

static void UpdatePlayerMenuHeight(Window *w)
{
	int num = 0;
	Player *p;

	FOR_ALL_PLAYERS(p) {
		if (p->is_active)
			num++;
	}
	
	if (WP(w,menu_d).item_count != num) {
		WP(w,menu_d).item_count = num;
		SetWindowDirty(w);
		num = num * 10 + 2;
		w->height = num;
		_player_menu_widgets[0].bottom = _player_menu_widgets[0].top + num - 1;
		SetWindowDirty(w);
	}
}

static void PlayerMenuWndProc(Window *w, WindowEvent *e)
{
	switch(e->event) {
	case WE_PAINT: {
		int x,y;
		byte sel, color;
		Player *p;
		uint16 chk;

		UpdatePlayerMenuHeight(w);
		DrawWindowWidgets(w);

		x = 1;
		y = 1;
		sel = WP(w,menu_d).sel_index;
		chk = WP(w,menu_d).checked_items; // let this mean gray items.
		
		FOR_ALL_PLAYERS(p) {
			if (p->is_active) {
				if (p->index == sel) {
					GfxFillRect(x, y, x + 0xEE, y + 9, 0);
				}
				DrawSprite( ((p->player_color + 0x307)<<16)+0x82EB, x+2, y+1);
			
				SET_DPARAM16(0, p->name_1);
				SET_DPARAM32(1, p->name_2);
				SET_DPARAM16(2, GetPlayerNameString(p->index));
							
				color = (byte)((p->index==sel) ? 0xC : 0x10);
				if (chk&1) color = 14;
				DrawString(x+0x13, y, STR_7021, color);
				
				y += 10;
			}	
			chk >>= 1;		
		}
		break;
		}

	case WE_DESTROY: {
		Window *v = FindWindowById(WC_MAIN_TOOLBAR, 0);
		v->click_state &= ~(1 << WP(w,menu_d).main_button);
		SetWindowDirty(v);
		return;
		}
		
	case WE_POPUPMENU_SELECT: {
		int index = GetPlayerIndexFromMenu(GetMenuItemIndex(w, e->popupmenu.pt.x, e->popupmenu.pt.y));
		int action_id = WP(w,menu_d).action_id;

		if (index < 0) {
			Window *w2 = FindWindowById(WC_MAIN_TOOLBAR,0);
			if (GetWidgetFromPos(w2, e->popupmenu.pt.x - w2->left, e->popupmenu.pt.y - w2->top) == WP(w,menu_d).main_button)
				index = WP(w,menu_d).sel_index;
		}

		DeleteWindow(w);
		
		if (index >= 0) {
			assert(index >= 0 && index < 30);
			_menu_clicked_procs[action_id](index);
		}
		break;
		}
	case WE_POPUPMENU_OVER: {
		int index;
		UpdatePlayerMenuHeight(w);
		index = GetPlayerIndexFromMenu(GetMenuItemIndex(w, e->popupmenu.pt.x, e->popupmenu.pt.y));

		if (index == -1 || index == WP(w,menu_d).sel_index)
			return;

		WP(w,menu_d).sel_index = index;
		SetWindowDirty(w);
		return;
		}
	}
}

static Window *PopupMainToolbMenu(Window *w, int x, int main_button, StringID base_string, int item_count)
{
	int h;

	x += w->left;

	SETBIT(w->click_state, (byte)main_button);
	InvalidateWidget(w, (byte)main_button);

	DeleteWindowById(WC_TOOLBAR_MENU, 0);

	_menu_widgets[0].bottom = h = item_count * 10 + 1;
	w = AllocateWindow(x, 0x16, 0xA0, h+1, MenuWndProc, WC_TOOLBAR_MENU, _menu_widgets);
	w->flags4 &= ~WF_WHITE_BORDER_MASK;
	
	WP(w,menu_d).item_count = item_count;
	WP(w,menu_d).sel_index = 0;
	WP(w,menu_d).main_button = main_button;
	WP(w,menu_d).action_id = (main_button >> 8) ? (main_button >> 8) : main_button;
	WP(w,menu_d).string_id = base_string;
	WP(w,menu_d).checked_items = 0;

	_popup_menu_active = true;
	
	SndPlayFx(0x13);

	return w;
}

static Window *PopupMainPlayerToolbMenu(Window *w, int x, int main_button, int gray)
{
	x += w->left;
	
	SETBIT(w->click_state, main_button);
	InvalidateWidget(w, main_button);

	DeleteWindowById(WC_TOOLBAR_MENU, 0);
	w = AllocateWindow(x, 0x16, 0xF1, 0x52, PlayerMenuWndProc, WC_TOOLBAR_MENU, _player_menu_widgets);
	w->flags4 &= ~WF_WHITE_BORDER_MASK;
	WP(w,menu_d).item_count = 0;
	WP(w,menu_d).sel_index = _local_player != 0xff ? _local_player : 0;
	WP(w,menu_d).action_id = main_button;
	WP(w,menu_d).main_button = main_button;
	WP(w,menu_d).checked_items = gray;
	_popup_menu_active = true;
	SndPlayFx(0x13);
	return w;
}

static void ToolbarSaveClick(Window *w)
{
	PopupMainToolbMenu(w, 66, 3, STR_015C_SAVE_GAME, 4);
}

static void ToolbarMapClick(Window *w)
{
	PopupMainToolbMenu(w, 96, 4, STR_02DE_MAP_OF_WORLD, 1);
}

static void ToolbarTownClick(Window *w)
{
	PopupMainToolbMenu(w, 118, 5, STR_02BB_TOWN_DIRECTORY, 1);
}

static void ToolbarSubsidiesClick(Window *w)
{
	PopupMainToolbMenu(w, 140, 6, STR_02DD_SUBSIDIES, 1);
}

static void ToolbarStationsClick(Window *w)
{
	PopupMainPlayerToolbMenu(w, 162, 7, 0);
}

static void ToolbarMoneyClick(Window *w)
{
	PopupMainPlayerToolbMenu(w, 191, 8, 0);
}

static void ToolbarPlayersClick(Window *w)
{
	PopupMainPlayerToolbMenu(w, 213, 9, 0);
}

static void ToolbarGraphsClick(Window *w)
{
	PopupMainToolbMenu(w, 236, 10, STR_0154_OPERATING_PROFIT_GRAPH, 6);
}

static void ToolbarLeagueClick(Window *w)
{
	PopupMainToolbMenu(w, 258, 11, STR_015A_COMPANY_LEAGUE_TABLE, 1);
}

static void ToolbarIndustryClick(Window *w)
{
	PopupMainToolbMenu(w, 280, 12, STR_0313_FUND_NEW_INDUSTRY, 2);
}

static void ToolbarTrainClick(Window *w)
{
	Vehicle *v;
	int dis = -1;
	FOR_ALL_VEHICLES(v)
		if (v->type == VEH_Train && v->subtype == 0) CLRBIT(dis, v->owner);
	PopupMainPlayerToolbMenu(w, 310, 13, dis);
}

static void ToolbarRoadClick(Window *w)
{
	Vehicle *v;
	int dis = -1;
	FOR_ALL_VEHICLES(v)
		if (v->type == VEH_Road) CLRBIT(dis, v->owner);
	PopupMainPlayerToolbMenu(w, 332, 14, dis);
}

static void ToolbarShipClick(Window *w)
{
	Vehicle *v;
	int dis = -1;
	FOR_ALL_VEHICLES(v)
		if (v->type == VEH_Ship) CLRBIT(dis, v->owner);
	PopupMainPlayerToolbMenu(w, 354, 15, dis);
}

static void ToolbarAirClick(Window *w)
{
	Vehicle *v;
	int dis = -1;
	FOR_ALL_VEHICLES(v)
		if (v->type == VEH_Aircraft) CLRBIT(dis, v->owner);
	PopupMainPlayerToolbMenu(w, 376, 16, dis);
}

bool DoZoomInOut(int how)
{
	ViewPort *vp;
	Window *w, *wt;
	int button;
	
	switch(_game_mode) {
	case GM_EDITOR: button = 9; break;
	case GM_NORMAL: button = 17; break;
	default: return false;
	}

	w = FindWindowById(WC_MAIN_WINDOW, 0);
	assert(w);
	vp = w->viewport;

	wt = FindWindowById(WC_MAIN_TOOLBAR, 0);
	assert(wt);

	if (how == ZOOM_IN) {
		if (vp->zoom == 0) return false;
		vp->zoom--;
		vp->virtual_width >>= 1;
		vp->virtual_height >>= 1;

		WP(w,vp_d).scrollpos_x += vp->virtual_width >> 1;
		WP(w,vp_d).scrollpos_y += vp->virtual_height >> 1;

		SetWindowDirty(w);
	} else if (how == ZOOM_OUT) {
		if (vp->zoom == 2) return false;
		vp->zoom++;

		WP(w,vp_d).scrollpos_x -= vp->virtual_width >> 1;
		WP(w,vp_d).scrollpos_y -= vp->virtual_height >> 1;

		vp->virtual_width <<= 1;
		vp->virtual_height <<= 1;

		SetWindowDirty(w);
	}

	// update the toolbar button too
	CLRBIT(wt->disabled_state, button);
	CLRBIT(wt->disabled_state, button + 1);
	if (vp->zoom == 0) SETBIT(wt->disabled_state, button);
	else if (vp->zoom == 2) SETBIT(wt->disabled_state, button + 1);
	SetWindowDirty(wt);

	return true;
}

static void MaxZoomIn()
{
	while (DoZoomInOut(ZOOM_IN)) {}
}

static void ToolbarZoomInClick(Window *w)
{
	if (DoZoomInOut(ZOOM_IN)) {
		HandleButtonClick(w, 17);
		SndPlayFx(0x13);
	}
}

static void ToolbarZoomOutClick(Window *w)
{
	if (DoZoomInOut(ZOOM_OUT)) {
		HandleButtonClick(w, 18);
		SndPlayFx(0x13);
	}
}

static void ToolbarBuildRailClick(Window *w)
{
	Player *p = DEREF_PLAYER(_local_player);
	Window *w2;
	w2 = PopupMainToolbMenu(w, 457, 19, STR_1015_RAILROAD_CONSTRUCTION, p->max_railtype);
	WP(w2,menu_d).sel_index = _last_built_railtype;
}

static void ToolbarBuildRoadClick(Window *w)
{
	PopupMainToolbMenu(w, 479, 20, STR_180A_ROAD_CONSTRUCTION, 1);
}

static void ToolbarBuildWaterClick(Window *w)
{
	PopupMainToolbMenu(w, 501, 21, STR_9800_DOCK_CONSTRUCTION, 1);
}

static void ToolbarBuildAirClick(Window *w)
{
	PopupMainToolbMenu(w, 0x1E0, 22, STR_A01D_AIRPORT_CONSTRUCTION, 1);
}

static void ToolbarForestClick(Window *w)
{
	PopupMainToolbMenu(w, 0x1E0, 23, STR_LANDSCAPING, 3);
}

static void ToolbarMusicClick(Window *w)
{
	PopupMainToolbMenu(w, 0x1E0, 24, STR_01D3_SOUND_MUSIC, 1);
}

static void ToolbarNewspaperClick(Window *w)
{
	PopupMainToolbMenu(w, 0x1E0, 25, STR_0200_LAST_MESSAGE_NEWS_REPORT, _newspaper_flag != 2 ? 3 : 4);
}

static void ToolbarHelpClick(Window *w)
{
	PopupMainToolbMenu(w, 0x1E0, 26, STR_02D5_LAND_BLOCK_INFO, 5);
}

static void ToolbarOptionsClick(Window *w)
{
	uint16 x;

	w = PopupMainToolbMenu(w,  43, 2, STR_02C3_GAME_OPTIONS, 11);

	x = (uint16)-1;
	if (_display_opt & DO_SHOW_TOWN_NAMES) x &= ~(1<<4);
	if (_display_opt & DO_SHOW_STATION_NAMES) x &= ~(1<<5);
	if (_display_opt & DO_SHOW_SIGNS) x &= ~(1<<6);
	if (_display_opt & DO_CHECKPOINTS) x &= ~(1<<7);
	if (_display_opt & DO_FULL_ANIMATION) x &= ~(1<<8);
	if (_display_opt & DO_FULL_DETAIL) x &= ~(1<<9);
	if (!(_display_opt & DO_TRANS_BUILDINGS)) x &= ~(1<<10);
	WP(w,menu_d).checked_items = x;
}


static void ToolbarScenSaveOrLoad(Window *w)
{
	PopupMainToolbMenu(w, 0x2C, 3, STR_0292_SAVE_SCENARIO, 5);
}

static void ToolbarScenDateBackward(Window *w)
{
	// don't allow too fast scrolling
	if ((w->flags4 & WF_TIMEOUT_MASK) <= 2 << WF_TIMEOUT_SHL) {
		HandleButtonClick(w, 6);
		InvalidateWidget(w, 5);
		
		if (_date > 0x2ACE) {
			SetDate(ConvertYMDToDay(_cur_year - 1, 0, 1));
		}
	}
	_left_button_clicked = false;
}

static void ToolbarScenDateForward(Window *w)
{
	// don't allow too fast scrolling
	if ((w->flags4 & WF_TIMEOUT_MASK) <= 2 << WF_TIMEOUT_SHL) {
		HandleButtonClick(w, 7);
		InvalidateWidget(w, 5);

		if (_date < 0x4E79) {
			SetDate(ConvertYMDToDay(_cur_year + 1, 0, 1));
		}
	}
	_left_button_clicked = false;
}

static void ToolbarScenMapTownDir(Window *w)
{
	PopupMainToolbMenu(w, 0x16A, 8 | (17<<8), STR_02DE_MAP_OF_WORLD, 2);
}

static void ToolbarScenZoomIn(Window *w)
{
	if (DoZoomInOut(ZOOM_IN)) {
		HandleButtonClick(w, 9);
		SndPlayFx(0x13);
	}
}

static void ToolbarScenZoomOut(Window *w)
{
	if (DoZoomInOut(ZOOM_OUT)) {
		HandleButtonClick(w, 10);
		SndPlayFx(0x13);
	}
}

void ZoomInOrOutToCursor(bool in)
{
	Point pt;
	Window* w;
	ViewPort* vp;

	w = FindWindowById(WC_MAIN_WINDOW, 0);
	assert(w != 0);

	vp = w->viewport;

	if (_game_mode != GM_MENU) {
		if ((in && vp->zoom == 0) || (!in && vp->zoom == 2))
			return;

		pt = GetTileZoomCenter(in);
		if (pt.x != -1) {
			ScrollMainWindowTo(pt.x, pt.y);

			DoZoomInOut(in ? ZOOM_IN : ZOOM_OUT);
		}
	}
}

void ResetLandscape()
{
	_random_seed_1 = InteractiveRandom();
	_random_seed_2 = InteractiveRandom();
	
	GenerateWorld(1);
	MarkWholeScreenDirty();
} 

static const Widget _ask_reset_landscape_widgets[] = {
{    WWT_TEXTBTN,     4,     0,    10,     0,    13, STR_00C5},
{    WWT_CAPTION,     4,    11,   179,     0,    13, STR_022C_RESET_LANDSCAPE},
{     WWT_IMGBTN,     4,     0,   179,    14,    91, 0x0},
{    WWT_TEXTBTN,    12,    25,    84,    72,    83, STR_00C9_NO},
{    WWT_TEXTBTN,    12,    95,   154,    72,    83, STR_00C8_YES},
{      WWT_LAST},
};

// Ask first to reset landscape or to make a random landscape
static void AskResetLandscapeWndProc(Window *w, WindowEvent *e)
{
	uint mode = w->window_number;

	switch(e->event) {
	case WE_PAINT:
		DrawWindowWidgets(w);
		DrawStringMultiCenter(90, 38, mode?STR_022D_ARE_YOU_SURE_YOU_WANT_TO:STR_GENERATE_RANDOM_LANDSCAPE , 168);
	case WE_CLICK:
		switch(e->click.widget) {
		case 3:
			DeleteWindow(w);
			break;
		case 4:
			DeleteWindow(w);
			if(mode) { // reset landscape
				ResetLandscape();
			}
			else { // make random landscape
				SndPlayFx(0x13);
				_switch_mode = SM_GENRANDLAND;
			}
			
			break;
		}
	}
}

static const WindowDesc _ask_reset_landscape_desc = {
	230,205, 180, 92,
	WC_ASK_RESET_LANDSCAPE,0,
	WDF_STD_BTN | WDF_DEF_WIDGET,
	_ask_reset_landscape_widgets,
	AskResetLandscapeWndProc,
};

static void AskResetLandscape(uint mode)
{
	AllocateWindowDescFront(&_ask_reset_landscape_desc, mode);
}

static void CcTerraform(bool success, uint tile, uint32 p1, uint32 p2)
{
	if (success) {
		SndPlayTileFx(0x1D, tile);
	} else {
		SetRedErrorSquare(_terraform_err_tile);
	}
}

static void CommonRaiseLowerBigLand(uint tile, int mode)
{
	int size;
	uint h;

	_error_message_2 = mode ? STR_0808_CAN_T_RAISE_LAND_HERE : STR_0809_CAN_T_LOWER_LAND_HERE;

	_generating_world = true;

//	tile = TILE_FROM_XY(GET_TILE_X(tile)*16+_tile_fract_coords.x + 8,GET_TILE_Y(tile)*16+_tile_fract_coords.y + 8);

	if (_terraform_size == 1) {
		DoCommandP(tile, 8, (uint32)mode, CcTerraform, CMD_TERRAFORM_LAND | CMD_AUTO);
	} else {
		SndPlayTileFx(0x1D, tile);

		size = _terraform_size;
		assert(size != 0);
		if (mode != 0) {
			/* Raise land */
			h = 15;
			BEGIN_TILE_LOOP(tile2, size, size, tile)
				h = min(h, _map_type_and_height[tile2]&0xF);
			END_TILE_LOOP(tile2, size, size, tile)
		} else {
			/* Lower land */
			h = 0;
			BEGIN_TILE_LOOP(tile2, size, size, tile)
				h = max(h, _map_type_and_height[tile2]&0xF);
			END_TILE_LOOP(tile2, size, size, tile)
		}

		BEGIN_TILE_LOOP(tile2, size, size, tile)
			if ((uint)(_map_type_and_height[tile2]&0xF) == h) {
				DoCommandP(tile2, 8, (uint32)mode, NULL, CMD_TERRAFORM_LAND | CMD_AUTO);
			}
		END_TILE_LOOP(tile2, size, size, tile)
	}

	_generating_world = false;
}

void PlaceProc_RaiseBigLand(uint tile)
{
	CommonRaiseLowerBigLand(tile, 1);
}

void PlaceProc_LowerBigLand(uint tile)
{
	CommonRaiseLowerBigLand(tile, 0);
}

//static void CcDemolish(bool success, uint tile, uint32 p1, uint32 p2)
//{
//	if (success) {
		//SndPlayTileFx(0x10, tile);
		//CreateEffectVehicleAbove(GET_TILE_X(tile)*16 + 8,GET_TILE_Y(tile)*16 + 8, 2, EV_DEMOLISH);
//	}
//}

//void PlaceProc_Demolish(uint tile)
//{
//	DoCommandP(tile, 0, 0, CcDemolish, CMD_LANDSCAPE_CLEAR | CMD_MSG(STR_00B5_CAN_T_CLEAR_THIS_AREA));
//}


void PlaceProc_RockyArea(uint tile)
{
	if (!IS_TILETYPE(tile, MP_CLEAR))
		return;

	_map5[tile] = (_map5[tile] & ~0x1C) | 8;
	MarkTileDirtyByTile(tile);
	SndPlayTileFx(0x1D, tile);
}

void PlaceProc_LightHouse(uint tile)
{
	TileInfo ti;

	FindLandscapeHeightByTile(&ti, tile);
	if (ti.type != MP_CLEAR || (ti.tileh & 0x10))
		return;

	ModifyTile(tile, MP_SETTYPE(MP_UNMOVABLE) | MP_MAP5, 1);
	SndPlayTileFx(0x1D, tile);
}

void PlaceProc_Transmitter(uint tile)
{
	TileInfo ti;

	FindLandscapeHeightByTile(&ti, tile);
	if (ti.type != MP_CLEAR || (ti.tileh & 0x10))
		return;

	ModifyTile(tile, MP_SETTYPE(MP_UNMOVABLE) | MP_MAP5, 0);
	SndPlayTileFx(0x1D, tile);
}

void PlaceProc_Desert(uint tile)
{
	SetMapExtraBits(tile, GetMapExtraBits(tile) == 1 ? 0 : 1);
}

static const Widget _scen_edit_land_gen_widgets[] = {
{    WWT_TEXTBTN,     7,     0,    10,     0,    13, STR_00C5, STR_018B_CLOSE_WINDOW},
{    WWT_CAPTION,     7,    11,   153,     0,    13, STR_0223_LAND_GENERATION, STR_018C_WINDOW_TITLE_DRAG_THIS},
{     WWT_IMGBTN,     7,     0,   153,    14,    99, 0x0, 0},
{     WWT_IMGBTN,    14,    22,    43,    14,    35, 0x2B6, STR_018F_RAISE_A_CORNER_OF_LAND},
{     WWT_IMGBTN,    14,    44,    65,    14,    35, 0x2B7, STR_018E_LOWER_A_CORNER_OF_LAND},
{     WWT_IMGBTN,    14,     0,    21,    14,    35, 0x2BF, STR_018D_DEMOLISH_BUILDINGS_ETC},
{    WWT_TEXTBTN,    14,   125,   135,    43,    54, STR_0224, STR_0228_INCREASE_SIZE_OF_LAND_AREA},
{    WWT_TEXTBTN,    14,   125,   135,    56,    67, STR_0225, STR_0229_DECREASE_SIZE_OF_LAND_AREA},
{    WWT_TEXTBTN,    14,    20,   135,    75,    86, STR_0226_RANDOM_LAND, STR_022A_GENERATE_RANDOM_LAND},
{    WWT_TEXTBTN,    14,    20,   135,    88,    99, STR_0227_RESET_LAND, STR_022B_RESET_LANDSCAPE},

{     WWT_IMGBTN,    14,    88,   109,    14,    35, 0xFF4, STR_028C_PLACE_ROCKY_AREAS_ON_LANDSCAPE},
{     WWT_IMGBTN,    14,   110,   131,    14,    35, 0xFF5, STR_028D_PLACE_LIGHTHOUSE},
{     WWT_IMGBTN,    14,   132,   153,    14,    35, 0xFF6, STR_028E_PLACE_TRANSMITTER},
{     WWT_IMGBTN,    14,    66,    87,    14,    35, SPR_OPENTTD_BASE+68, STR_LEVEL_LAND_TOOLTIP},
{      WWT_LAST},
};

static const int8 _multi_terraform_coords[][2] = {
	{  0, -2},
	{  4,  0},{ -4,  0},{  0,  2},
	{ -8,  2},{ -4,  4},{  0,  6},{  4,  4},{  8,  2},
	{-12,  0},{ -8, -2},{ -4, -4},{  0, -6},{  4, -4},{  8, -2},{ 12,  0},
	{-16,  2},{-12,  4},{ -8,  6},{ -4,  8},{  0, 10},{  4,  8},{  8,  6},{ 12,  4},{ 16,  2},
	{-20,  0},{-16, -2},{-12, -4},{ -8, -6},{ -4, -8},{  0,-10},{  4, -8},{  8, -6},{ 12, -4},{ 16, -2},{ 20,  0},
	{-24,  2},{-20,  4},{-16,  6},{-12,  8},{ -8, 10},{ -4, 12},{  0, 14},{  4, 12},{  8, 10},{ 12,  8},{ 16,  6},{ 20,  4},{ 24,  2},
	{-28,  0},{-24, -2},{-20, -4},{-16, -6},{-12, -8},{ -8,-10},{ -4,-12},{  0,-14},{  4,-12},{  8,-10},{ 12, -8},{ 16, -6},{ 20, -4},{ 24, -2},{ 28,  0},
};

static void ScenEditLandGenWndProc(Window *w, WindowEvent *e)
{
	// XXX: show different tooltips in desert mode
	switch(e->event) {
	case WE_PAINT:
		// XXX: only show reset button when nothing was built
		DrawWindowWidgets(w);
		
		{
			int n = _terraform_size * _terraform_size;
			const int8 *coords = &_multi_terraform_coords[0][0];

			assert(n != 0);
			do {
				DrawSprite(0xFEF, 77 + coords[0], 55 + coords[1]);
				coords += 2;
			} while (--n);
		}

		if (_thd.window_class == WC_SCEN_LAND_GEN && (w->click_state&(1<<3|1<<4))) {
			SetTileSelectSize(_terraform_size, _terraform_size);
		}
		break;
	case WE_CLICK:
		switch(e->click.widget) {
		case 3: /* raise corner */
			HandlePlacePushButton(w, 3, ANIMCURSOR_RAISELAND, 2, PlaceProc_RaiseBigLand);
			break;
		case 4: /* lower corner */
			HandlePlacePushButton(w, 4, ANIMCURSOR_LOWERLAND, 2, PlaceProc_LowerBigLand);
			break;
		case 5: /* demolish */
			HandlePlacePushButton(w, 5, ANIMCURSOR_DEMOLISH, 1, PlaceProc_DemolishArea);
			break;
		{
			int size;
		case 6: /* increase terraform size */
			HandleButtonClick(w, 6);
			size = 1;
			goto terraform_size_common;
		case 7: /* decrease terraform size */
			HandleButtonClick(w, 7);
			size = -1;
terraform_size_common:;
			size += _terraform_size;
			if (!IS_INT_INSIDE(size, 1, 8+1))
				return;
			_terraform_size = size;
			SndPlayFx(0x13);
			SetWindowDirty(w);
			break;
		}

		case 8: /* gen random land */
			HandleButtonClick(w, 8);
			AskResetLandscape(0);
			break;

		case 9: /* reset landscape */
			HandleButtonClick(w,9);
			AskResetLandscape(1);
			break;

		case 10: /* place rocky areas */
			HandlePlacePushButton(w, 10, 0xFF7, 1, PlaceProc_RockyArea);
			break;
			
		case 11: /* place lighthouse */
			HandlePlacePushButton(w, 11, 0xFF8, 1, _opt.landscape == LT_DESERT ? PlaceProc_Desert : PlaceProc_LightHouse);
			break;

		case 12: /* place transmitter */
			HandlePlacePushButton(w, 12, 0xFF9, 1, PlaceProc_Transmitter);
			break;

		case 13: /* level Land */
			HandlePlacePushButton(w, 13, SPR_OPENTTD_BASE+69, 2, PlaceProc_LevelLand);
			break;
		}
		break;
	case WE_TIMEOUT:
		UnclickSomeWindowButtons(w, ~(1<<3 | 1<<4 | 1<<5 | 1<<10|1<<11|1<<12));
		break;
	case WE_PLACE_OBJ:
		_place_proc(e->place.tile);
		break;
	case WE_PLACE_DRAG: {
		VpSelectTilesWithMethod(e->place.pt.x, e->place.pt.y, e->place.userdata & 0xF);
		return;
	}
	case WE_PLACE_MOUSEUP:
		if (e->click.pt.x != -1) {
			uint start_tile = e->place.starttile;
			uint end_tile = e->place.tile;
			if (e->place.userdata == VPM_X_AND_Y) {
				DoCommandP(end_tile, start_tile, 0, CcPlaySound10, CMD_CLEAR_AREA | CMD_MSG(STR_00B5_CAN_T_CLEAR_THIS_AREA));
			} else if (e->place.userdata == (VPM_X_AND_Y | (2<<4))) {
				DoCommandP(end_tile, start_tile, 0, CcPlaySound10, CMD_LEVEL_LAND | CMD_AUTO);
			}
		}
	break;

	case WE_ABORT_PLACE_OBJ:
		w->click_state = 0;
		SetWindowDirty(w);
		break;
	}
}

static const WindowDesc _scen_edit_land_gen_desc = {
	-1,-1, 154, 100,
	WC_SCEN_LAND_GEN,0,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET,
	_scen_edit_land_gen_widgets,
	ScenEditLandGenWndProc,
};

static void ToolbarScenGenLand(Window *w)
{
	HandleButtonClick(w, 11);
	SndPlayFx(0x13);

	AllocateWindowDescFront(&_scen_edit_land_gen_desc, 0);
}

static void CcBuildTown(bool success, uint tile, uint32 p1, uint32 p2)
{
	if (success) {
		SndPlayTileFx(0x1D, tile);
		ResetObjectToPlace();
	}
}

void PlaceProc_Town(uint tile)
{
	DoCommandP(tile, 0, 0, CcBuildTown, CMD_BUILD_TOWN | CMD_MSG(STR_0236_CAN_T_BUILD_TOWN_HERE));
}


static const Widget _scen_edit_town_gen_widgets[] = {
{    WWT_TEXTBTN,     7,     0,    10,     0,    13, STR_00C5, STR_018B_CLOSE_WINDOW},
{    WWT_CAPTION,     7,    11,   159,     0,    13, STR_0233_TOWN_GENERATION, STR_018C_WINDOW_TITLE_DRAG_THIS},
{     WWT_IMGBTN,     7,     0,   159,    14,    81, 0x0},
{    WWT_TEXTBTN,    14,     2,   157,    16,    27, STR_0234_NEW_TOWN, STR_0235_CONSTRUCT_NEW_TOWN},
{    WWT_TEXTBTN,    14,     2,   157,    29,    40, STR_023D_RANDOM_TOWN, STR_023E_BUILD_TOWN_IN_RANDOM_LOCATION},
{    WWT_TEXTBTN,    14,     2,   157,    42,    53, STR_MANY_RANDOM_TOWNS, STR_RANDOM_TOWNS_TIP},
{    WWT_TEXTBTN,    14,     2,    53,    68,    79, STR_02A1_SMALL, STR_02A4_SELECT_TOWN_SIZE},
{    WWT_TEXTBTN,    14,    54,   105,    68,    79, STR_02A2_MEDIUM, STR_02A4_SELECT_TOWN_SIZE},
{    WWT_TEXTBTN,    14,   106,   157,    68,    79, STR_02A3_LARGE, STR_02A4_SELECT_TOWN_SIZE},
{      WWT_LAST},
};

static void ScenEditTownGenWndProc(Window *w, WindowEvent *e)
{
	switch(e->event) {
	case WE_PAINT:
		w->click_state = (w->click_state & ~(1<<6 | 1<<7 | 1<<8) ) | (1 << (_new_town_size + 6));
		DrawWindowWidgets(w);
		DrawStringCentered(80, 56, STR_02A5_TOWN_SIZE, 0);
		break;

	case WE_CLICK:
		switch(e->click.widget) {
		case 3: /* new town */
			HandlePlacePushButton(w, 3, 0xFF0, 1, PlaceProc_Town);
			break;
		case 4: {/* random town */
			Town *t;

			HandleButtonClick(w, 4);
			_generating_world = true;
			t = CreateRandomTown();
			_generating_world = false;
			if (t != NULL)
				ScrollMainWindowToTile(t->xy);
			break;
		}
		case 5: {/* many random towns */
			HandleButtonClick(w, 5);
			_generating_world = true;
			_game_mode = GM_NORMAL; // little hack to avoid towns of the same size
			GenerateTowns();
			_generating_world = false;
			_game_mode = GM_EDITOR;
			break;
		}

		case 6: case 7: case 8:
			_new_town_size = e->click.widget - 6;
			SetWindowDirty(w);
			break;
		}
		break;

	case WE_TIMEOUT:
		UnclickSomeWindowButtons(w, 1<<4 | 1<<5);
		break;
	case WE_PLACE_OBJ:
		_place_proc(e->place.tile);
		break;
	case WE_ABORT_PLACE_OBJ:
		w->click_state = 0;
		SetWindowDirty(w);
		break;
	}
}

static const WindowDesc _scen_edit_town_gen_desc = {
	-1,-1, 160, 82,
	WC_SCEN_TOWN_GEN,0,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET,
	_scen_edit_town_gen_widgets,
	ScenEditTownGenWndProc,
};

static void ToolbarScenGenTown(Window *w)
{
	HandleButtonClick(w, 12);
	SndPlayFx(0x13);

	AllocateWindowDescFront(&_scen_edit_town_gen_desc, 0);
}


static const Widget _scenedit_industry_normal_widgets[] = {
{    WWT_TEXTBTN,     7,     0,    10,     0,    13, STR_00C5},
{    WWT_CAPTION,     7,    11,   169,     0,    13, STR_023F_INDUSTRY_GENERATION},
{     WWT_IMGBTN,     7,     0,   169,    14,   224, 0x0},

{    WWT_TEXTBTN,    14,     2,   167,    16,    27, STR_MANY_RANDOM_INDUSTRIES, STR_RANDOM_INDUSTRIES_TIP},

{    WWT_TEXTBTN,    14,     2,   167,    42,    53, STR_0240_COAL_MINE, STR_0262_CONSTRUCT_COAL_MINE},
{    WWT_TEXTBTN,    14,     2,   167,    55,    66, STR_0241_POWER_STATION, STR_0263_CONSTRUCT_POWER_STATION},
{    WWT_TEXTBTN,    14,     2,   167,    68,    79, STR_0242_SAWMILL, STR_0264_CONSTRUCT_SAWMILL},
{    WWT_TEXTBTN,    14,     2,   167,    81,    92, STR_0243_FOREST, STR_0265_PLANT_FOREST},
{    WWT_TEXTBTN,    14,     2,   167,    94,   105, STR_0244_OIL_REFINERY, STR_0266_CONSTRUCT_OIL_REFINERY},
{    WWT_TEXTBTN,    14,     2,   167,   107,   118, STR_0245_OIL_RIG, STR_0267_CONSTRUCT_OIL_RIG_CAN_ONLY},
{    WWT_TEXTBTN,    14,     2,   167,   120,   131, STR_0246_FACTORY, STR_0268_CONSTRUCT_FACTORY},
{    WWT_TEXTBTN,    14,     2,   167,   133,   144, STR_0247_STEEL_MILL, STR_0269_CONSTRUCT_STEEL_MILL},
{    WWT_TEXTBTN,    14,     2,   167,   146,   157, STR_0248_FARM, STR_026A_CONSTRUCT_FARM},
{    WWT_TEXTBTN,    14,     2,   167,   159,   170, STR_0249_IRON_ORE_MINE, STR_026B_CONSTRUCT_IRON_ORE_MINE},
{    WWT_TEXTBTN,    14,     2,   167,   172,   183, STR_024A_OIL_WELLS, STR_026C_CONSTRUCT_OIL_WELLS},
{    WWT_TEXTBTN,    14,     2,   167,   185,   196, STR_024B_BANK, STR_026D_CONSTRUCT_BANK_CAN_ONLY},
{      WWT_LAST},
};


static const Widget _scenedit_industry_hilly_widgets[] = {
{    WWT_TEXTBTN,     7,     0,    10,     0,    13, STR_00C5},
{    WWT_CAPTION,     7,    11,   169,     0,    13, STR_023F_INDUSTRY_GENERATION},
{     WWT_IMGBTN,     7,     0,   169,    14,   224, 0x0},

{    WWT_TEXTBTN,    14,     2,   167,    16,    27, STR_MANY_RANDOM_INDUSTRIES, STR_RANDOM_INDUSTRIES_TIP},

{    WWT_TEXTBTN,    14,     2,   167,    42,    53, STR_0240_COAL_MINE, STR_0262_CONSTRUCT_COAL_MINE},
{    WWT_TEXTBTN,    14,     2,   167,    55,    66, STR_0241_POWER_STATION, STR_0263_CONSTRUCT_POWER_STATION},
{    WWT_TEXTBTN,    14,     2,   167,    68,    79, STR_024C_PAPER_MILL, STR_026E_CONSTRUCT_PAPER_MILL},
{    WWT_TEXTBTN,    14,     2,   167,    81,    92, STR_0243_FOREST, STR_0265_PLANT_FOREST},
{    WWT_TEXTBTN,    14,     2,   167,    94,   105, STR_0244_OIL_REFINERY, STR_0266_CONSTRUCT_OIL_REFINERY},
{    WWT_TEXTBTN,    14,     2,   167,   107,   118, STR_024D_FOOD_PROCESSING_PLANT, STR_026F_CONSTRUCT_FOOD_PROCESSING},
{    WWT_TEXTBTN,    14,     2,   167,   120,   131, STR_024E_PRINTING_WORKS, STR_0270_CONSTRUCT_PRINTING_WORKS},
{    WWT_TEXTBTN,    14,     2,   167,   133,   144, STR_024F_GOLD_MINE, STR_0271_CONSTRUCT_GOLD_MINE},
{    WWT_TEXTBTN,    14,     2,   167,   146,   157, STR_0248_FARM, STR_026A_CONSTRUCT_FARM},
{    WWT_TEXTBTN,    14,     2,   167,   159,   170, STR_024B_BANK, STR_0272_CONSTRUCT_BANK_CAN_ONLY},
{    WWT_TEXTBTN,    14,     2,   167,   172,   183, STR_024A_OIL_WELLS, STR_026C_CONSTRUCT_OIL_WELLS},
{      WWT_LAST},
};

static const Widget _scenedit_industry_desert_widgets[] = {
{    WWT_TEXTBTN,     7,     0,    10,     0,    13, STR_00C5},
{    WWT_CAPTION,     7,    11,   169,     0,    13, STR_023F_INDUSTRY_GENERATION},
{     WWT_IMGBTN,     7,     0,   169,    14,   224, 0x0},

{    WWT_TEXTBTN,    14,     2,   167,    16,    27, STR_MANY_RANDOM_INDUSTRIES, STR_RANDOM_INDUSTRIES_TIP},

{    WWT_TEXTBTN,    14,     2,   167,    42,    53, STR_0250_LUMBER_MILL, STR_0273_CONSTRUCT_LUMBER_MILL_TO},
{    WWT_TEXTBTN,    14,     2,   167,    55,    66, STR_0251_FRUIT_PLANTATION, STR_0274_PLANT_FRUIT_PLANTATION},
{    WWT_TEXTBTN,    14,     2,   167,    68,    79, STR_0252_RUBBER_PLANTATION, STR_0275_PLANT_RUBBER_PLANTATION},
{    WWT_TEXTBTN,    14,     2,   167,    81,    92, STR_0244_OIL_REFINERY, STR_0266_CONSTRUCT_OIL_REFINERY},
{    WWT_TEXTBTN,    14,     2,   167,    94,   105, STR_024D_FOOD_PROCESSING_PLANT, STR_026F_CONSTRUCT_FOOD_PROCESSING},
{    WWT_TEXTBTN,    14,     2,   167,   107,   118, STR_0246_FACTORY, STR_0268_CONSTRUCT_FACTORY},
{    WWT_TEXTBTN,    14,     2,   167,   120,   131, STR_0253_WATER_SUPPLY, STR_0276_CONSTRUCT_WATER_SUPPLY},
{    WWT_TEXTBTN,    14,     2,   167,   133,   144, STR_0248_FARM, STR_026A_CONSTRUCT_FARM},
{    WWT_TEXTBTN,    14,     2,   167,   146,   157, STR_0254_WATER_TOWER, STR_0277_CONSTRUCT_WATER_TOWER_CAN},
{    WWT_TEXTBTN,    14,     2,   167,   159,   170, STR_024A_OIL_WELLS, STR_026C_CONSTRUCT_OIL_WELLS},
{    WWT_TEXTBTN,    14,     2,   167,   172,   183, STR_024B_BANK, STR_0272_CONSTRUCT_BANK_CAN_ONLY},
{    WWT_TEXTBTN,    14,     2,   167,   185,   196, STR_0255_DIAMOND_MINE, STR_0278_CONSTRUCT_DIAMOND_MINE},
{    WWT_TEXTBTN,    14,     2,   167,   198,   209, STR_0256_COPPER_ORE_MINE, STR_0279_CONSTRUCT_COPPER_ORE_MINE},
{      WWT_LAST},
};

static const Widget _scenedit_industry_candy_widgets[] = {
{    WWT_TEXTBTN,     7,     0,    10,     0,    13, STR_00C5},
{    WWT_CAPTION,     7,    11,   169,     0,    13, STR_023F_INDUSTRY_GENERATION},
{     WWT_IMGBTN,     7,     0,   169,    14,   224, 0x0},

{    WWT_TEXTBTN,    14,     2,   167,    16,    27, STR_MANY_RANDOM_INDUSTRIES, STR_RANDOM_INDUSTRIES_TIP},

{    WWT_TEXTBTN,    14,     2,   167,    42,    53, STR_0257_COTTON_CANDY_FOREST, STR_027A_PLANT_COTTON_CANDY_FOREST},
{    WWT_TEXTBTN,    14,     2,   167,    55,    66, STR_0258_CANDY_FACTORY, STR_027B_CONSTRUCT_CANDY_FACTORY},
{    WWT_TEXTBTN,    14,     2,   167,    68,    79, STR_0259_BATTERY_FARM, STR_027C_CONSTRUCT_BATTERY_FARM},
{    WWT_TEXTBTN,    14,     2,   167,    81,    92, STR_025A_COLA_WELLS, STR_027D_CONSTRUCT_COLA_WELLS},
{    WWT_TEXTBTN,    14,     2,   167,    94,   105, STR_025B_TOY_SHOP, STR_027E_CONSTRUCT_TOY_SHOP},
{    WWT_TEXTBTN,    14,     2,   167,   107,   118, STR_025C_TOY_FACTORY, STR_027F_CONSTRUCT_TOY_FACTORY},
{    WWT_TEXTBTN,    14,     2,   167,   120,   131, STR_025D_PLASTIC_FOUNTAINS, STR_0280_CONSTRUCT_PLASTIC_FOUNTAINS},
{    WWT_TEXTBTN,    14,     2,   167,   133,   144, STR_025E_FIZZY_DRINK_FACTORY, STR_0281_CONSTRUCT_FIZZY_DRINK_FACTORY},
{    WWT_TEXTBTN,    14,     2,   167,   146,   157, STR_025F_BUBBLE_GENERATOR, STR_0282_CONSTRUCT_BUBBLE_GENERATOR},
{    WWT_TEXTBTN,    14,     2,   167,   159,   170, STR_0260_TOFFEE_QUARRY, STR_0283_CONSTRUCT_TOFFEE_QUARRY},
{    WWT_TEXTBTN,    14,     2,   167,   172,   183, STR_0261_SUGAR_MINE, STR_0284_CONSTRUCT_SUGAR_MINE},
{      WWT_LAST},
};

int _industry_type_to_place;

static bool AnyTownExists() {
	Town *t;
	FOR_ALL_TOWNS(t) {
		if (t->xy)
			return true;
	}
	return false;
}

extern Industry *CreateNewIndustry(uint tile, int type);

static bool TryBuildIndustry(TileIndex tile, int type)
{
	int n;

	n = 100;
	do {
		if (CreateNewIndustry(AdjustTileCoordRandomly(tile, 1), type)) return true;
	} while (--n);

	n = 200;
	do {
		if (CreateNewIndustry(AdjustTileCoordRandomly(tile, 2), type)) return true;
	} while (--n);

	n = 700;
	do {
		if (CreateNewIndustry(AdjustTileCoordRandomly(tile, 4), type)) return true;
	} while (--n);

	return false;
}


static const byte _industry_type_list[4][16] = {
	{0, 1, 2, 3, 4, 5, 6, 8, 9, 18, 11, 12},
	{0, 1, 14, 3, 4, 13, 7, 15, 9, 16, 11, 12},
	{25, 19, 20, 4, 13, 23, 21, 24, 22, 11, 16, 17, 10},
	{26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36},
};

static void ScenEditIndustryWndProc(Window *w, WindowEvent *e)
{
	int button;

	switch(e->event) {
	case WE_PAINT:
		DrawWindowWidgets(w);
		break;

	case WE_CLICK:
		if ((button=e->click.widget) == 3) { 
			HandleButtonClick(w, 3);

		if (!AnyTownExists()) {
			ShowErrorMessage(STR_0286_MUST_BUILD_TOWN_FIRST, STR_CAN_T_GENERATE_INDUSTRIES, 0, 0);
			return;
		}

			_generating_world = true;
			GenerateIndustries();
			_generating_world = false;
		}
	
		if ((button=e->click.widget) >= 4) {
			if (HandlePlacePushButton(w, button, 0xFF1, 1, NULL))
				_industry_type_to_place = _industry_type_list[_opt.landscape][button - 4];
		}
		break;
	case WE_PLACE_OBJ: {
		int type;

		// Show error if no town exists at all
		type = _industry_type_to_place;
		if (!AnyTownExists()) {
			SET_DPARAM16(0, type + STR_4802_COAL_MINE);
			ShowErrorMessage(STR_0286_MUST_BUILD_TOWN_FIRST,STR_0285_CAN_T_BUILD_HERE,e->place.pt.x, e->place.pt.y);
			return;
		}

		_current_player = OWNER_NONE;
		_generating_world = true;
		if (!TryBuildIndustry(e->place.tile,type)) {
			SET_DPARAM16(0, type + STR_4802_COAL_MINE);
			ShowErrorMessage(_error_message, STR_0285_CAN_T_BUILD_HERE,e->place.pt.x, e->place.pt.y);
		}
		_generating_world = false;
		break;
	}
	case WE_ABORT_PLACE_OBJ:
		w->click_state = 0;
		SetWindowDirty(w);
		break;
	case WE_TIMEOUT:
		UnclickSomeWindowButtons(w, 1<<3);
		break;
	}
}

static const WindowDesc _scenedit_industry_normal_desc = {
	-1,-1, 170, 225,
	WC_SCEN_INDUSTRY,0,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET,
	_scenedit_industry_normal_widgets,
	ScenEditIndustryWndProc,
};

static const WindowDesc _scenedit_industry_hilly_desc = {
	-1,-1, 170, 225,
	WC_SCEN_INDUSTRY,0,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET,
	_scenedit_industry_hilly_widgets,
	ScenEditIndustryWndProc,
};

static const WindowDesc _scenedit_industry_desert_desc = {
	-1,-1, 170, 225,
	WC_SCEN_INDUSTRY,0,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET,
	_scenedit_industry_desert_widgets,
	ScenEditIndustryWndProc,
};

static const WindowDesc _scenedit_industry_candy_desc = {
	-1,-1, 170, 225,
	WC_SCEN_INDUSTRY,0,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET,
	_scenedit_industry_candy_widgets,
	ScenEditIndustryWndProc,
};

static const WindowDesc * const _scenedit_industry_descs[] = {
	&_scenedit_industry_normal_desc,
	&_scenedit_industry_hilly_desc,
	&_scenedit_industry_desert_desc,
	&_scenedit_industry_candy_desc,
};


static void ToolbarScenGenIndustry(Window *w)
{
	HandleButtonClick(w, 13);
	SndPlayFx(0x13);
	AllocateWindowDescFront(_scenedit_industry_descs[_opt.landscape],0);
}

static void ToolbarScenBuildRoad(Window *w)
{
	HandleButtonClick(w, 14);
	SndPlayFx(0x13);
	ShowBuildRoadScenToolbar();
}

static void ToolbarScenPlantTrees(Window *w)
{
	HandleButtonClick(w, 15);
	SndPlayFx(0x13);
	ShowBuildTreesScenToolbar();
}

static void ToolbarScenPlaceSign(Window *w)
{
	HandleButtonClick(w, 16);
	SndPlayFx(0x13);
	SelectSignTool();
}

static void ToolbarBtn_NULL(Window *w)
{
}

static ToolbarButtonProc* const _toolbar_button_procs[] = {
	ToolbarPauseClick,
	ToolbarFastForwardClick,
	ToolbarOptionsClick,
	ToolbarSaveClick,
	ToolbarMapClick,
	ToolbarTownClick,
	ToolbarSubsidiesClick,
	ToolbarStationsClick,
	ToolbarMoneyClick,
	ToolbarPlayersClick,
	ToolbarGraphsClick,
	ToolbarLeagueClick,
	ToolbarIndustryClick,
	ToolbarTrainClick,
	ToolbarRoadClick,
	ToolbarShipClick,
	ToolbarAirClick,
	ToolbarZoomInClick,
	ToolbarZoomOutClick,
	ToolbarBuildRailClick,
	ToolbarBuildRoadClick,
	ToolbarBuildWaterClick,
	ToolbarBuildAirClick,
	ToolbarForestClick,
	ToolbarMusicClick,
	ToolbarNewspaperClick,
	ToolbarHelpClick,
};

static void MainToolbarWndProc(Window *w, WindowEvent *e)
{
	switch(e->event) {
	case WE_PAINT: {

		// Draw brown-red toolbar bg.
		GfxFillRect(0, 0, w->width-1, w->height-1, 0xB2);
		GfxFillRect(0, 0, w->width-1, w->height-1, 0x80B4);

		// if spectator, disable things
		if (_current_player == 0xff){
			w->disabled_state |= (1 << 0) | (1 << 19) | (1<<20) | (1<<21) | (1<<22) | (1<<23);
		} else {
			w->disabled_state &= ~((1 << 0) | (1 << 19) | (1<<20) | (1<<21) | (1<<22) | (1<<23));
		}

		DrawWindowWidgets(w);
		break;
	}

	case WE_CLICK: {
		if (_game_mode != GM_MENU && !HASBIT(w->disabled_state, e->click.widget))
			_toolbar_button_procs[e->click.widget](w);
	} break;

	case WE_KEYPRESS: {
		int local = _local_player;
		if (local == 0xff) local = 0; // spectator

		switch(e->keypress.keycode) {
		case WKC_F1: ToolbarPauseClick(w); break;
		case WKC_F2: ShowGameOptions(); break;
		case WKC_F3: MenuClickSaveLoad(0); break;
		case WKC_F4: ShowSmallMap(); break;
		case WKC_F5: ShowTownDirectory(); break;
		case WKC_F6: ShowSubsidiesList(); break;
		case WKC_F7: ShowPlayerStations(local); break;
		case WKC_F8: ShowPlayerFinances(local); break;
		case WKC_F9: ShowPlayerCompany(local); break;
		case WKC_F10:ShowOperatingProfitGraph(); break;
		case WKC_F11: ShowCompanyLeagueTable(); break;
		case WKC_F12: ShowBuildIndustryWindow(); break;
		case WKC_SHIFT | WKC_F1: ShowPlayerTrains(local); break;
		case WKC_SHIFT | WKC_F2: ShowPlayerRoadVehicles(local); break;
		case WKC_SHIFT | WKC_F3: ShowPlayerShips(local); break;
		case WKC_SHIFT | WKC_F4: ShowPlayerAircraft(local); break;
		case WKC_SHIFT | WKC_F5: ToolbarZoomInClick(w); break;
		case WKC_SHIFT | WKC_F6: ToolbarZoomOutClick(w); break;
		case WKC_SHIFT | WKC_F7: ShowBuildRailToolbar(_last_built_railtype,-1); break;
		case WKC_SHIFT | WKC_F8: ShowBuildRoadToolbar(); break;
		case WKC_SHIFT | WKC_F9: ShowBuildDocksToolbar(); break;
		case WKC_SHIFT | WKC_F10:ShowBuildAirToolbar(); break;
		case WKC_SHIFT | WKC_F11: ShowBuildTreesToolbar(); break;
		case WKC_SHIFT | WKC_F12: ShowMusicWindow(); break;
		case WKC_CTRL  | 'S': _make_screenshot = 1; break;
		case WKC_CTRL  | 'G': _make_screenshot = 2; break;
		case WKC_CTRL | WKC_ALT | 'C': if(!_networking) ShowCheatWindow(); break;
		}
	} break;
	
	case WE_PLACE_OBJ: {
		_place_proc(e->place.tile);
	} break;

	case WE_ABORT_PLACE_OBJ: {
		w->click_state &= ~(1<<25);
		SetWindowDirty(w);
	} break;

	case WE_ON_EDIT_TEXT: HandleOnEditText(e); break;

	case WE_MOUSELOOP:
		
		if (((w->click_state) & 1) != (uint)!!_pause) {
			w->click_state ^= (1 << 0);
			SetWindowDirty(w);
		}
		
		if (((w->click_state >> 1) & 1) != (uint)!!_fast_forward) {
			w->click_state ^= (1 << 1);
			SetWindowDirty(w);
		}
		break;

	case WE_TIMEOUT:
		UnclickSomeWindowButtons(w, ~(1<<0 | 1<<1));
		break;
	}
}

static const Widget _toolb_normal_widgets[] = {
{      WWT_PANEL,    14,     0,    21,     0,    21, 0x2D6, STR_0171_PAUSE_GAME},
{      WWT_PANEL,    14,    22,    43,     0,    21, SPR_OPENTTD_BASE + 57, STR_FAST_FORWARD},
{      WWT_PANEL,    14,    44,    65,     0,    21, 0x2EF, STR_0187_OPTIONS},
{    WWT_PANEL_2,    14,    66,    87,     0,    21, 0x2D4, STR_0172_SAVE_GAME_ABANDON_GAME},

{      WWT_PANEL,    14,    96,   117,     0,    21, 0x2C4, STR_0174_DISPLAY_MAP},
{      WWT_PANEL,    14,   118,   139,     0,    21, 0xFED, STR_0176_DISPLAY_TOWN_DIRECTORY},
{      WWT_PANEL,    14,   140,   161,     0,    21, 0x2A7, STR_02DC_DISPLAY_SUBSIDIES},
{      WWT_PANEL,    14,   162,   183,     0,    21, 0x513, STR_0173_DISPLAY_LIST_OF_COMPANY},

{      WWT_PANEL,    14,   191,   212,     0,    21, 0x2E1, STR_0177_DISPLAY_COMPANY_FINANCES},
{      WWT_PANEL,    14,   213,   235,     0,    21, 0x2E7, STR_0178_DISPLAY_COMPANY_GENERAL},
{      WWT_PANEL,    14,   236,   257,     0,    21, 0x2E9, STR_0179_DISPLAY_GRAPHS},
{      WWT_PANEL,    14,   258,   279,     0,    21, 0x2AC, STR_017A_DISPLAY_COMPANY_LEAGUE},
{      WWT_PANEL,    14,   280,   301,     0,    21, 0x2E5, STR_0312_FUND_CONSTRUCTION_OF_NEW},

{      WWT_PANEL,    14,   310,   331,     0,    21, 0x2DB, STR_017B_DISPLAY_LIST_OF_COMPANY},
{      WWT_PANEL,    14,   332,   353,     0,    21, 0x2DC, STR_017C_DISPLAY_LIST_OF_COMPANY},
{      WWT_PANEL,    14,   354,   375,     0,    21, 0x2DD, STR_017D_DISPLAY_LIST_OF_COMPANY},
{      WWT_PANEL,    14,   376,   397,     0,    21, 0x2DE, STR_017E_DISPLAY_LIST_OF_COMPANY},

{      WWT_PANEL,    14,   406,   427,     0,    21, 0x2DF, STR_017F_ZOOM_THE_VIEW_IN},
{      WWT_PANEL,    14,   428,   449,     0,    21, 0x2E0, STR_0180_ZOOM_THE_VIEW_OUT},

{      WWT_PANEL,    14,   457,   478,     0,    21, 0x2D7, STR_0181_BUILD_RAILROAD_TRACK},
{      WWT_PANEL,    14,   479,   500,     0,    21, 0x2D8, STR_0182_BUILD_ROADS},
{      WWT_PANEL,    14,   501,   522,     0,    21, 0x2D9, STR_0183_BUILD_SHIP_DOCKS},
{      WWT_PANEL,    14,   523,   544,     0,    21, 0x2DA, STR_0184_BUILD_AIRPORTS},
{      WWT_PANEL,    14,   545,   566,     0,    21, 0xFF3, STR_0185_PLANT_TREES_PLACE_SIGNS}, // tree icon is 0x2E6

{      WWT_PANEL,    14,   574,   595,     0,    21, 0x2C9, STR_01D4_SHOW_SOUND_MUSIC_WINDOW},
{      WWT_PANEL,    14,   596,   617,     0,    21, 0x2A8, STR_0203_SHOW_LAST_MESSAGE_NEWS},
{      WWT_PANEL,    14,   618,   639,     0,    21, 0x2D3, STR_0186_LAND_BLOCK_INFORMATION},
{      WWT_LAST},
};

static const WindowDesc _toolb_normal_desc = {
	0, 0, 640, 22,
	WC_MAIN_TOOLBAR,0,
	WDF_STD_TOOLTIPS | WDF_DEF_WIDGET,
	_toolb_normal_widgets,
	MainToolbarWndProc
};

static const WindowDesc _toolb_intro_desc = {
	0, -22, 640, 22,
	WC_MAIN_TOOLBAR,0,
	WDF_STD_TOOLTIPS | WDF_DEF_WIDGET,
	_toolb_normal_widgets,
	MainToolbarWndProc
};


static const Widget _toolb_scen_widgets[] = {
{      WWT_PANEL,    14,     0,    21,     0,    21, 0x2D6, STR_0171_PAUSE_GAME},
{      WWT_PANEL,    14,    22,    43,     0,    21, SPR_OPENTTD_BASE + 57, STR_FAST_FORWARD},
{      WWT_PANEL,    14,    44,    65,     0,    21, 0x2EF, STR_0187_OPTIONS},
{    WWT_PANEL_2,    14,    66,    87,     0,    21, 0x2D4, STR_0297_SAVE_SCENARIO_LOAD_SCENARIO	},

{      WWT_PANEL,    14,    96,   225,     0,    21, 0x0},

{      WWT_PANEL,    14,   233,   362,     0,    21, 0x0},
{   WWT_CLOSEBOX,    14,   236,   246,     5,    16, STR_0225, STR_029E_MOVE_THE_STARTING_DATE},
{   WWT_CLOSEBOX,    14,   347,   357,     5,    16, STR_0224, STR_029F_MOVE_THE_STARTING_DATE},

{      WWT_PANEL,    14,   371,   392,     0,    21, 0x2C4, STR_0175_DISPLAY_MAP_TOWN_DIRECTORY},

{      WWT_PANEL,    14,   400,   421,     0,    21, 0x2DF, STR_017F_ZOOM_THE_VIEW_IN},
{      WWT_PANEL,    14,   422,   443,     0,    21, 0x2E0, STR_0180_ZOOM_THE_VIEW_OUT},

{      WWT_PANEL,    14,   452,   473,     0,    21, 0xFF3, STR_022E_LANDSCAPE_GENERATION},
{      WWT_PANEL,    14,   474,   495,     0,    21, 0xFED, STR_022F_TOWN_GENERATION},
{      WWT_PANEL,    14,   496,   517,     0,    21, 0x2E5, STR_0230_INDUSTRY_GENERATION},
{      WWT_PANEL,    14,   518,   539,     0,    21, 0x2D8, STR_0231_ROAD_CONSTRUCTION},
{      WWT_PANEL,    14,   540,   561,     0,    21, 0x2E6, STR_0288_PLANT_TREES},
{      WWT_PANEL,    14,   562,   583,     0,    21, 0xFF2, STR_0289_PLACE_SIGN},

{      WWT_EMPTY,     0,     0,     0,     0,     0, 0x0},
{      WWT_EMPTY,     0,     0,     0,     0,     0, 0x0},
{      WWT_EMPTY,     0,     0,     0,     0,     0, 0x0},
{      WWT_EMPTY,     0,     0,     0,     0,     0, 0x0},
{      WWT_EMPTY,     0,     0,     0,     0,     0, 0x0},
{      WWT_EMPTY,     0,     0,     0,     0,     0, 0x0},
{      WWT_EMPTY,     0,     0,     0,     0,     0, 0x0},
{      WWT_PANEL,    14,   596,   617,     0,    21, 0x2C9,STR_01D4_SHOW_SOUND_MUSIC_WINDOW},
{      WWT_EMPTY,     0,     0,     0,     0,     0, 0x0},
{      WWT_PANEL,    14,   618,   639,     0,    21, 0x2D3, STR_0186_LAND_BLOCK_INFORMATION},
{      WWT_LAST},
};

static ToolbarButtonProc* const _scen_toolbar_button_procs[] = {
	ToolbarPauseClick,
	ToolbarFastForwardClick,
	ToolbarOptionsClick,
	ToolbarScenSaveOrLoad,
	ToolbarBtn_NULL,
	ToolbarBtn_NULL,
	ToolbarScenDateBackward,
	ToolbarScenDateForward,
	ToolbarScenMapTownDir,
	ToolbarScenZoomIn,
	ToolbarScenZoomOut,
	ToolbarScenGenLand,
	ToolbarScenGenTown,
	ToolbarScenGenIndustry,
	ToolbarScenBuildRoad,
	ToolbarScenPlantTrees,
	ToolbarScenPlaceSign,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	ToolbarMusicClick,
	NULL,
	ToolbarHelpClick,
};

static void ScenEditToolbarWndProc(Window *w, WindowEvent *e)
{
	switch(e->event) {
	case WE_PAINT:
		// Draw brown-red toolbar bg.
		GfxFillRect(0, 0, w->width-1, w->height-1, 0xB2);
		GfxFillRect(0, 0, w->width-1, w->height-1, 0x80B4);

		DrawWindowWidgets(w);

		SET_DPARAM16(0, _date);
		DrawStringCentered(298, 6, STR_00AF, 0);

		SET_DPARAM16(0, _date);
		DrawStringCentered(161, 1, STR_0221_TRANSPORT_TYCOON, 0);
		DrawStringCentered(161, 11,STR_0222_SCENARIO_EDITOR, 0);

		break;

	case WE_CLICK: {
		if (_game_mode == GM_MENU)
			return;
		_scen_toolbar_button_procs[e->click.widget](w);
	} break;

	case WE_KEYPRESS: {
		switch(e->keypress.keycode) {
		case WKC_F1: ToolbarPauseClick(w); break;
		case WKC_F2: ShowGameOptions(); break;
		case WKC_F3: MenuClickSaveLoad(0); break;
		case WKC_F4: ToolbarScenGenLand(w); break;
		case WKC_F5: ToolbarScenGenTown(w); break;
		case WKC_F6: ToolbarScenGenIndustry(w); break;
		case WKC_F7: ToolbarScenBuildRoad(w); break;
		case WKC_F8: ToolbarScenPlantTrees(w); break;
		case WKC_F9: ToolbarScenPlaceSign(w); break;
		case WKC_F10: ShowMusicWindow(); break;
		case WKC_F11: PlaceLandBlockInfo(); break;
		case WKC_CTRL  | 'S': _make_screenshot = 1; break;
		case WKC_CTRL  | 'G': _make_screenshot = 2; break;
		}
		break;
	}
	
	case WE_PLACE_OBJ: {
		_place_proc(e->place.tile);
	} break;

	case WE_ABORT_PLACE_OBJ: {
		w->click_state &= ~(1<<25);
		SetWindowDirty(w);
	} break;

	case WE_ON_EDIT_TEXT: HandleOnEditText(e); break;

	case WE_MOUSELOOP:
		if (((w->click_state >> 1) & 1) != (uint)!!_fast_forward) {
			w->click_state ^= (1 << 1);
			SetWindowDirty(w);
		}
		break;

	}
}

static const WindowDesc _toolb_scen_desc = {
	0, 0, 640, 22,
	WC_MAIN_TOOLBAR,0,
	WDF_STD_TOOLTIPS | WDF_DEF_WIDGET | WDF_UNCLICK_BUTTONS,
	_toolb_scen_widgets,
	ScenEditToolbarWndProc
};

extern GetNewsStringCallbackProc * const _get_news_string_callback[];


static bool DrawScrollingStatusText(NewsItem *ni, int pos)
{
	StringID str;
	byte *s, *d;
	DrawPixelInfo tmp_dpi, *old_dpi;
	int x;
	byte buffer[256];

	if (ni->display_mode == 3) {
		str = _get_news_string_callback[ni->callback](ni);
	} else {
		COPY_IN_DPARAM(0, ni->params, lengthof(ni->params));
		str = ni->string_id;	
	}

	GetString(str_buffr, str);
	assert(strlen(str_buffr) < sizeof(str_buffr) - 1);

	s = str_buffr;
	d = buffer;

	for(;;s++) {
		if (*s == 0) {
			*d = 0;
			break;
		} else if (*s == 0x0D) {
			d[0] = d[1] = d[2] = d[3] = ' ';
			d+=4;
		} else if (*s >= ' ' && (*s < 0x88 || *s >= 0x99)) {
			*d++ = *s;
		}
	}

	if (!FillDrawPixelInfo(&tmp_dpi, NULL, 141, 1, 358, 11))
		return true;

	old_dpi = _cur_dpi;
	_cur_dpi = &tmp_dpi;

	x = DoDrawString(buffer, pos, 0, 13);
	_cur_dpi = old_dpi;

	return x > 0;
}

void StatusBarWndProc(Window *w, WindowEvent *e)
{
	Player *p;

	switch(e->event) {
	case WE_PAINT:
		DrawWindowWidgets(w);
		SET_DPARAM16(0, _date);
		DrawStringCentered(70, 1, ((_pause||_patches.status_long_date)?STR_00AF:STR_00AE), 0);
		
		p = _local_player == 0xff ? NULL : DEREF_PLAYER(_local_player);

		if (p) {
			// Draw player money
			SET_DPARAM64(0, p->money64);
			DrawStringCentered(570, 1, p->player_money >= 0 ? STR_0004 : STR_0005, 0);
		}

		// Draw status bar
		if (_do_autosave) {
			DrawStringCentered(320, 1,	STR_032F_AUTOSAVE, 0);
		} else if (_pause) {
			DrawStringCentered(320, 1,	STR_0319_PAUSED, 0);
		} else if (WP(w,def_d).data_1 > -1280 && FindWindowById(WC_NEWS_WINDOW,0) == NULL && _statusbar_news_item.string_id != 0) {
			// Draw the scrolling news text
			if (!DrawScrollingStatusText(&_statusbar_news_item, WP(w,def_d).data_1))
				WP(w,def_d).data_1 = -1280;
		} else {
			if (p) {
				// This is the default text
				SET_DPARAM16(0, p->name_1);
				SET_DPARAM32(1, p->name_2);
				DrawStringCentered(320, 1,	STR_02BA, 0);
			}
		}
		break;

	case WE_CLICK:
		if (e->click.widget == 1) {
			ShowLastNewsMessage();
		} else if (e->click.widget == 2) {
			if (_local_player != 0xff) ShowPlayerFinances(_local_player);
		} else {
			ResetObjectToPlace();
		}
		break;

	case WE_TICK: {
		if (_pause || WP(w,def_d).data_1 <= -1280)
			return;
		WP(w,def_d).data_1 -= 2;
		InvalidateWidget(w, 1);
		break;
	}
	}
}

static void ScrollMainViewport(int x, int y)
{
	if (_game_mode != GM_MENU) {
		Window *w = FindWindowById(WC_MAIN_WINDOW, 0);
		assert(w);

		WP(w,vp_d).scrollpos_x += x << w->viewport->zoom;
		WP(w,vp_d).scrollpos_y += y << w->viewport->zoom;
	}
}


static const Widget _main_status_widgets[] = {
{     WWT_IMGBTN,    14,     0,   139,     0,    11, 0x0, 0},
{ WWT_PUSHIMGBTN,    14,   140,   499,     0,    11, 0x0, STR_02B7_SHOW_LAST_MESSAGE_OR_NEWS},
{ WWT_PUSHIMGBTN,    14,   500,   639,     0,    11, 0x0, 0},
{      WWT_LAST},
};

static WindowDesc _main_status_desc = {
	WDP_CENTER, 0, 640, 12,
	WC_STATUS_BAR,0,
	WDF_STD_TOOLTIPS | WDF_DEF_WIDGET | WDF_UNCLICK_BUTTONS,
	_main_status_widgets,
	StatusBarWndProc
};

extern void DebugProc(int i);

static void MainWindowWndProc(Window *w, WindowEvent *e) {
	int off_x;

	switch(e->event) {
	case WE_PAINT:
		DrawWindowViewport(w);
		if (_game_mode == GM_MENU) {
			off_x = ((_screen.width - 400) >> 1) - 70;

			DrawSprite(0x12EA, off_x+0-40, 50); // o
			DrawSprite(0x12E9, off_x+28-40,50); // p
			DrawSprite(SPR_OPENTTD_BASE+16, off_x+55-40,50); // e
			DrawSprite(0x12E7, off_x+84-40,50); // n

			DrawSprite(0x12E4, off_x+90, 50); // t
			DrawSprite(0x12E5, off_x+119, 50); // r
			DrawSprite(0x12E6, off_x+148,50); // a
			DrawSprite(0x12E7, off_x+181,50); // n
			DrawSprite(0x12E8, off_x+215,50); // s
			DrawSprite(0x12E9, off_x+246,50); // p
			DrawSprite(0x12EA, off_x+275,50); // o
			DrawSprite(0x12E5, off_x+307,50); // r
			DrawSprite(0x12E4, off_x+337,50); // t
			
			DrawSprite(0x12E4, off_x+390,50); // t
			DrawSprite(0x12EB, off_x+417,50); // y
			DrawSprite(0x12EC, off_x+447,50); // c
			DrawSprite(0x12EA, off_x+478,50); // o
			DrawSprite(0x12EA, off_x+509,50); // o
			DrawSprite(0x12E7, off_x+541,50); // n
		}
		break;

	case WE_KEYPRESS:
		if (_game_mode == GM_MENU)
			break;

		// this is a hack, but this needs to be called at a constant interval and i found
		// no other window event that was suited for that purpose.
		{
			static const int8 scrollamt[16][2] = {
				{0,0},
				{-1,0},   // 1:left
				{0,-1},   // 2:up
				{-1,-1}, // 3:left + up
				{1,0},    // 4:right
				{0,0},     // 5:left + right
				{1,-1},  // 6:right + up
				{0,0},     // 7:impossible
				{0,1},    // 8:down
				{-1,1},  // 9:down+left
				{0,0},     // 10:impossible
				{0,0},     // 11:impossible
				{1,1},   // 12:down+right
				{0,0},     // 13:impossible
				{0,0},     // 14:impossible
				{0,0},     // 15:impossible
			};

			if (_dirkeys) {
				int factor = _shift_pressed ? 50 : 10;
				ScrollMainViewport(scrollamt[_dirkeys][0] * factor, scrollamt[_dirkeys][1] * factor);
			}
		}

		switch(e->keypress.keycode) {
		case 'C':
		case 'Z': {
			Point pt;
			pt = GetTileBelowCursor();
			if (pt.x != -1) {
				ScrollMainWindowTo(pt.x, pt.y);
				if (e->keypress.keycode == 'Z')
					MaxZoomIn();
			}
			break;
		}
		
		case WKC_ESC: ResetObjectToPlace(); break;
		case WKC_DELETE: DeleteNonVitalWindows(); break;
		case 'Q' | WKC_CTRL: AskExitGame(); break;
		case 'Q' | WKC_META: AskExitGame(); break; // this enables command + Q on mac
		case 'R' | WKC_CTRL: MarkWholeScreenDirty(); break;
		case '0' | WKC_ALT:
		case '1' | WKC_ALT:
		case '2' | WKC_ALT:
		case '3' | WKC_ALT:
		case '4' | WKC_ALT:
#if defined(_DEBUG)
			DebugProc(e->keypress.keycode - ('0' | WKC_ALT));
#endif
			break;
		case 'A' | WKC_CTRL:
			disable_computer^=1;
			ShowInfoF("Disable Computer: %s", disable_computer?"ON":"OFF");
			break;
		case 'B' | WKC_CTRL:
			_local_player^=1;
			ShowInfoF("Play as computer: %s", _local_player?"ON":"OFF");
			break;

		// invoke autorail
		case 'A':
			ShowBuildRailToolbar(_last_built_railtype, 0);
			break;

		case 'X':
			_display_opt ^= DO_TRANS_BUILDINGS;
			MarkWholeScreenDirty();
			break;

		default:
			return;
		}
		e->keypress.cont = false;
		break;
				
	}
}


void ShowSelectGameWindow();

void SetupColorsAndInitialWindow()
{
	int i;
	byte *b;
	Window *w;
	int width,height;

	for(i=0; i!=16; i++) {
		b = GetSpritePtr(0x307 + i);
		assert(b);
		_color_list[i] = *(ColorList*)(b + 0xC6);
	}

	width = _screen.width;
	height = _screen.height;

	// XXX: these are not done
	switch(_game_mode) {
	case GM_MENU:
		w = AllocateWindow(0, 0, width, height, MainWindowWndProc, 0, NULL);
		AssignWindowViewport(w, 0, 0, width, height, 0x8080, 0);
//		w = AllocateWindowDesc(&_toolb_intro_desc);
//		w->flags4 &= ~WF_WHITE_BORDER_MASK;
		ShowSelectGameWindow();
		break;
	case GM_NORMAL:
		w = AllocateWindow(0, 0, width, height, MainWindowWndProc, 0, NULL);
		AssignWindowViewport(w, 0, 0, width, height, 0x8080, 0);

		w = AllocateWindowDesc(&_toolb_normal_desc);
		w->disabled_state = 1 << 17;
		w->flags4 &= ~WF_WHITE_BORDER_MASK;

		_main_status_desc.top = height - 12;
		w = AllocateWindowDesc(&_main_status_desc);
		w->flags4 &= ~WF_WHITE_BORDER_MASK;

		WP(w,def_d).data_1 = -1280;

		break;
	case GM_EDITOR:
		w = AllocateWindow(0, 0, width, height, MainWindowWndProc, 0, NULL);
		AssignWindowViewport(w, 0, 0, width, height, 0, 0);

		w = AllocateWindowDesc(&_toolb_scen_desc);
		w->disabled_state = 1 << 9;
		w->flags4 &= ~WF_WHITE_BORDER_MASK;
		break;
	default:
		NOT_REACHED();
	}
}

void GameSizeChanged()
{
	RelocateAllWindows(_screen.width, _screen.height);
	ScreenSizeChanged();
	MarkWholeScreenDirty();
}
