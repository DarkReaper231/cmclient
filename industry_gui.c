#include "stdafx.h"
#include "ttd.h"
//#include "gui.h"
#include "window.h"
#include "gfx.h"
#include "command.h"
#include "viewport.h"
#include "industry.h"
#include "town.h"

static const byte _build_industry_types[4][12] = {
	{ 1,  2, 4,  6, 8, 0, 3, 5,  9, 11, 18 },
	{ 1, 14, 4, 13, 7, 0, 3, 9, 11, 15 },
	{ 25, 13, 4, 23, 22, 11, 17, 10, 24, 19, 20, 21 },
	{ 27, 30, 31, 33, 26, 28, 29, 32, 34, 35, 36 },
};

extern const byte _industry_type_costs[37];

static void BuildIndustryWndProc(Window *w, WindowEvent *e)
{
	switch(e->event) {
	case WE_PAINT:
		DrawWindowWidgets(w);
		if (_thd.place_mode == 1 && _thd.window_class == WC_BUILD_INDUSTRY) {
			int ind_type = _build_industry_types[_opt.landscape][WP(w,def_d).data_1];

			SET_DPARAM32(0, (_price.build_industry >> 5) * _industry_type_costs[ind_type]);
			DrawStringCentered(85, w->height - 21, STR_482F_COST, 0);
		}
		break;

	case WE_CLICK: {
		int wid = e->click.widget;
		if (wid >= 3) {
			if (HandlePlacePushButton(w, wid, 0xFF1, 1, NULL))
				WP(w,def_d).data_1 = wid - 3;
		}
	} break;

	case WE_PLACE_OBJ:
		if (DoCommandP(e->place.tile, _build_industry_types[_opt.landscape][WP(w,def_d).data_1], 0, NULL, CMD_BUILD_INDUSTRY | CMD_MSG(STR_4830_CAN_T_CONSTRUCT_THIS_INDUSTRY)))
			ResetObjectToPlace();
		break;
		
	case WE_ABORT_PLACE_OBJ:
		w->click_state = 0;
		SetWindowDirty(w);
		break;
	}
}

static const Widget _build_industry_land0_widgets[] = {
{   WWT_CLOSEBOX,     7,     0,    10,     0,    13, STR_00C5,STR_018B_CLOSE_WINDOW},
{    WWT_CAPTION,     7,    11,   169,     0,    13, STR_0314_FUND_NEW_INDUSTRY,STR_018C_WINDOW_TITLE_DRAG_THIS},
{      WWT_PANEL,     7,     0,   169,    14,   115, 0x0,0},
{   WWT_CLOSEBOX,    14,     2,   167,    16,    27, STR_0241_POWER_STATION,STR_0263_CONSTRUCT_POWER_STATION},
{   WWT_CLOSEBOX,    14,     2,   167,    29,    40, STR_0242_SAWMILL,STR_0264_CONSTRUCT_SAWMILL},
{   WWT_CLOSEBOX,    14,     2,   167,    42,    53, STR_0244_OIL_REFINERY,STR_0266_CONSTRUCT_OIL_REFINERY},
{   WWT_CLOSEBOX,    14,     2,   167,    55,    66, STR_0246_FACTORY,STR_0268_CONSTRUCT_FACTORY},
{   WWT_CLOSEBOX,    14,     2,   167,    68,    79, STR_0247_STEEL_MILL,STR_0269_CONSTRUCT_STEEL_MILL},
{      WWT_LAST},
};

static const Widget _build_industry_land1_widgets[] = {
{   WWT_CLOSEBOX,     7,     0,    10,     0,    13, STR_00C5,STR_018B_CLOSE_WINDOW},
{    WWT_CAPTION,     7,    11,   169,     0,    13, STR_0314_FUND_NEW_INDUSTRY,STR_018C_WINDOW_TITLE_DRAG_THIS},
{      WWT_PANEL,     7,     0,   169,    14,   115, 0x0,0},
{   WWT_CLOSEBOX,    14,     2,   167,    16,    27, STR_0241_POWER_STATION,STR_0263_CONSTRUCT_POWER_STATION},
{   WWT_CLOSEBOX,    14,     2,   167,    29,    40, STR_024C_PAPER_MILL, STR_026E_CONSTRUCT_PAPER_MILL},
{   WWT_CLOSEBOX,    14,     2,   167,    42,    53, STR_0244_OIL_REFINERY,STR_0266_CONSTRUCT_OIL_REFINERY},
{   WWT_CLOSEBOX,    14,     2,   167,    55,    66, STR_024D_FOOD_PROCESSING_PLANT,STR_026F_CONSTRUCT_FOOD_PROCESSING},
{   WWT_CLOSEBOX,    14,     2,   167,    68,    79, STR_024E_PRINTING_WORKS,STR_0270_CONSTRUCT_PRINTING_WORKS},
{      WWT_LAST},
};

static const Widget _build_industry_land2_widgets[] = {
{   WWT_CLOSEBOX,     7,     0,    10,     0,    13, STR_00C5,STR_018B_CLOSE_WINDOW},
{    WWT_CAPTION,     7,    11,   169,     0,    13, STR_0314_FUND_NEW_INDUSTRY,STR_018C_WINDOW_TITLE_DRAG_THIS},
{      WWT_PANEL,     7,     0,   169,    14,   115, 0x0,0},
{   WWT_CLOSEBOX,    14,     2,   167,    16,    27, STR_0250_LUMBER_MILL,STR_0273_CONSTRUCT_LUMBER_MILL_TO},
{   WWT_CLOSEBOX,    14,     2,   167,    29,    40, STR_024D_FOOD_PROCESSING_PLANT,STR_026F_CONSTRUCT_FOOD_PROCESSING},
{   WWT_CLOSEBOX,    14,     2,   167,    42,    53, STR_0244_OIL_REFINERY,STR_0266_CONSTRUCT_OIL_REFINERY},
{   WWT_CLOSEBOX,    14,     2,   167,    55,    66, STR_0246_FACTORY,STR_0268_CONSTRUCT_FACTORY},
{   WWT_CLOSEBOX,    14,     2,   167,    68,    79, STR_0254_WATER_TOWER,STR_0277_CONSTRUCT_WATER_TOWER_CAN},
{      WWT_LAST},
};

static const Widget _build_industry_land3_widgets[] = {
{   WWT_CLOSEBOX,     7,     0,    10,     0,    13, STR_00C5,STR_018B_CLOSE_WINDOW},
{    WWT_CAPTION,     7,    11,   169,     0,    13, STR_0314_FUND_NEW_INDUSTRY,STR_018C_WINDOW_TITLE_DRAG_THIS},
{      WWT_PANEL,     7,     0,   169,    14,   115, 0x0,0},
{   WWT_CLOSEBOX,    14,     2,   167,    16,    27, STR_0258_CANDY_FACTORY,STR_027B_CONSTRUCT_CANDY_FACTORY},
{   WWT_CLOSEBOX,    14,     2,   167,    29,    40, STR_025B_TOY_SHOP,STR_027E_CONSTRUCT_TOY_SHOP},
{   WWT_CLOSEBOX,    14,     2,   167,    42,    53, STR_025C_TOY_FACTORY,STR_027F_CONSTRUCT_TOY_FACTORY},
{   WWT_CLOSEBOX,    14,     2,   167,    55,    66, STR_025E_FIZZY_DRINK_FACTORY,STR_0281_CONSTRUCT_FIZZY_DRINK_FACTORY},
{      WWT_LAST},
};

static const Widget _build_industry_land0_widgets_extra[] = {
{   WWT_CLOSEBOX,     7,     0,    10,     0,    13, STR_00C5,STR_018B_CLOSE_WINDOW},
{    WWT_CAPTION,     7,    11,   169,     0,    13, STR_0314_FUND_NEW_INDUSTRY,STR_018C_WINDOW_TITLE_DRAG_THIS},
{      WWT_PANEL,     7,     0,   169,    14,   187, 0x0,0},

{   WWT_CLOSEBOX,    14,     2,   167,    16,    27, STR_0241_POWER_STATION,STR_0263_CONSTRUCT_POWER_STATION},
{   WWT_CLOSEBOX,    14,     2,   167,    29,    40, STR_0242_SAWMILL,STR_0264_CONSTRUCT_SAWMILL},
{   WWT_CLOSEBOX,    14,     2,   167,    42,    53, STR_0244_OIL_REFINERY,STR_0266_CONSTRUCT_OIL_REFINERY},
{   WWT_CLOSEBOX,    14,     2,   167,    55,    66, STR_0246_FACTORY,STR_0268_CONSTRUCT_FACTORY},
{   WWT_CLOSEBOX,    14,     2,   167,    68,    79, STR_0247_STEEL_MILL,STR_0269_CONSTRUCT_STEEL_MILL},

{   WWT_CLOSEBOX,    14,     2,   167,    84,    95, STR_0240_COAL_MINE, STR_CONSTRUCT_COAL_MINE_TIP},
{   WWT_CLOSEBOX,    14,     2,   167,    97,    108, STR_0243_FOREST, STR_CONSTRUCT_FOREST_TIP},
{   WWT_CLOSEBOX,    14,     2,   167,    110,    121, STR_0245_OIL_RIG, STR_CONSTRUCT_OIL_RIG_TIP},
{   WWT_CLOSEBOX,    14,     2,   167,    123,    134, STR_0248_FARM, STR_CONSTRUCT_FARM_TIP},
{   WWT_CLOSEBOX,    14,     2,   167,    136,    147, STR_024A_OIL_WELLS, STR_CONSTRUCT_OIL_WELLS_TIP},
{   WWT_CLOSEBOX,    14,     2,   167,    149,    160, STR_0249_IRON_ORE_MINE, STR_CONSTRUCT_IRON_ORE_MINE_TIP},

{      WWT_LAST},
};

static const Widget _build_industry_land1_widgets_extra[] = {
{   WWT_CLOSEBOX,     7,     0,    10,     0,    13, STR_00C5,STR_018B_CLOSE_WINDOW},
{    WWT_CAPTION,     7,    11,   169,     0,    13, STR_0314_FUND_NEW_INDUSTRY,STR_018C_WINDOW_TITLE_DRAG_THIS},
{      WWT_PANEL,     7,     0,   169,    14,   174, 0x0,0},

{   WWT_CLOSEBOX,    14,     2,   167,    16,    27, STR_0241_POWER_STATION,STR_0263_CONSTRUCT_POWER_STATION},
{   WWT_CLOSEBOX,    14,     2,   167,    29,    40, STR_024C_PAPER_MILL, STR_026E_CONSTRUCT_PAPER_MILL},
{   WWT_CLOSEBOX,    14,     2,   167,    42,    53, STR_0244_OIL_REFINERY,STR_0266_CONSTRUCT_OIL_REFINERY},
{   WWT_CLOSEBOX,    14,     2,   167,    55,    66, STR_024D_FOOD_PROCESSING_PLANT,STR_026F_CONSTRUCT_FOOD_PROCESSING},
{   WWT_CLOSEBOX,    14,     2,   167,    68,    79, STR_024E_PRINTING_WORKS,STR_0270_CONSTRUCT_PRINTING_WORKS},

{   WWT_CLOSEBOX,    14,     2,   167,    81+3,    92+3, STR_0240_COAL_MINE,STR_CONSTRUCT_COAL_MINE_TIP},
{   WWT_CLOSEBOX,    14,     2,   167,    94+3,   105+3, STR_0243_FOREST,STR_CONSTRUCT_FOREST_TIP},
{   WWT_CLOSEBOX,    14,     2,   167,    107+3,  118+3, STR_0248_FARM,STR_CONSTRUCT_FARM_TIP},
{   WWT_CLOSEBOX,    14,     2,   167,    120+3,  131+3, STR_024A_OIL_WELLS,STR_CONSTRUCT_OIL_WELLS_TIP},
{   WWT_CLOSEBOX,    14,     2,   167,    133+3,  144+3, STR_024F_GOLD_MINE,STR_CONSTRUCT_GOLD_MINE_TIP},
{      WWT_LAST},
};

static const Widget _build_industry_land2_widgets_extra[] = {
{   WWT_CLOSEBOX,     7,     0,    10,     0,    13, STR_00C5,STR_018B_CLOSE_WINDOW},
{    WWT_CAPTION,     7,    11,   169,     0,    13, STR_0314_FUND_NEW_INDUSTRY,STR_018C_WINDOW_TITLE_DRAG_THIS},
{      WWT_PANEL,     7,     0,   169,    14,   200, 0x0,0},

{   WWT_CLOSEBOX,    14,     2,   167,    16,    27, STR_0250_LUMBER_MILL,STR_0273_CONSTRUCT_LUMBER_MILL_TO},
{   WWT_CLOSEBOX,    14,     2,   167,    29,    40, STR_024D_FOOD_PROCESSING_PLANT,STR_026F_CONSTRUCT_FOOD_PROCESSING},
{   WWT_CLOSEBOX,    14,     2,   167,    42,    53, STR_0244_OIL_REFINERY,STR_0266_CONSTRUCT_OIL_REFINERY},
{   WWT_CLOSEBOX,    14,     2,   167,    55,    66, STR_0246_FACTORY,STR_0268_CONSTRUCT_FACTORY},
{   WWT_CLOSEBOX,    14,     2,   167,    68,    79, STR_0254_WATER_TOWER,STR_0277_CONSTRUCT_WATER_TOWER_CAN},

{   WWT_CLOSEBOX,    14,     2,   167,    81+3,    92+3, STR_024A_OIL_WELLS,STR_CONSTRUCT_OIL_WELLS_TIP},
{   WWT_CLOSEBOX,    14,     2,   167,    94+3,    105+3, STR_0255_DIAMOND_MINE,STR_CONSTRUCT_DIAMOND_MINE_TIP},
{   WWT_CLOSEBOX,    14,     2,   167,    107+3,    118+3, STR_0256_COPPER_ORE_MINE,STR_CONSTRUCT_COPPER_ORE_MINE_TIP},
{   WWT_CLOSEBOX,    14,     2,   167,    120+3,    131+3, STR_0248_FARM,STR_CONSTRUCT_FARM_TIP},
{   WWT_CLOSEBOX,    14,     2,   167,    133+3,    144+3, STR_0251_FRUIT_PLANTATION,STR_CONSTRUCT_FRUIT_PLANTATION_TIP},
{   WWT_CLOSEBOX,    14,     2,   167,    146+3,    157+3, STR_0252_RUBBER_PLANTATION,STR_CONSTRUCT_RUBBER_PLANTATION_TIP},
{   WWT_CLOSEBOX,    14,     2,   167,    159+3,    170+3, STR_0253_WATER_SUPPLY,STR_CONSTRUCT_WATER_SUPPLY_TIP},
{      WWT_LAST},
};

static const Widget _build_industry_land3_widgets_extra[] = {
{   WWT_CLOSEBOX,     7,     0,    10,     0,    13, STR_00C5,STR_018B_CLOSE_WINDOW},
{    WWT_CAPTION,     7,    11,   169,     0,    13, STR_0314_FUND_NEW_INDUSTRY,STR_018C_WINDOW_TITLE_DRAG_THIS},
{      WWT_PANEL,     7,     0,   169,    14,   187, 0x0,0},

{   WWT_CLOSEBOX,    14,     2,   167,    16,    27, STR_0258_CANDY_FACTORY,STR_027B_CONSTRUCT_CANDY_FACTORY},
{   WWT_CLOSEBOX,    14,     2,   167,    29,    40, STR_025B_TOY_SHOP,STR_027E_CONSTRUCT_TOY_SHOP},
{   WWT_CLOSEBOX,    14,     2,   167,    42,    53, STR_025C_TOY_FACTORY,STR_027F_CONSTRUCT_TOY_FACTORY},
{   WWT_CLOSEBOX,    14,     2,   167,    55,    66, STR_025E_FIZZY_DRINK_FACTORY,STR_0281_CONSTRUCT_FIZZY_DRINK_FACTORY},

{   WWT_CLOSEBOX,    14,     2,   167,    68+3,    79+3, STR_0257_COTTON_CANDY_FOREST,STR_CONSTRUCT_COTTON_CANDY_TIP},
{   WWT_CLOSEBOX,    14,     2,   167,    81+3,    92+3, STR_0259_BATTERY_FARM,STR_CONSTRUCT_BATTERY_FARM_TIP},
{   WWT_CLOSEBOX,    14,     2,   167,    94+3,    105+3, STR_025A_COLA_WELLS,STR_CONSTRUCT_COLA_WELLS_TIP},
{   WWT_CLOSEBOX,    14,     2,   167,    107+3,    118+3, STR_025D_PLASTIC_FOUNTAINS,STR_CONSTRUCT_PLASTIC_FOUNTAINS_TIP},
{   WWT_CLOSEBOX,    14,     2,   167,    120+3,    131+3, STR_025F_BUBBLE_GENERATOR,STR_CONSTRUCT_BUBBLE_GENERATOR_TIP},
{   WWT_CLOSEBOX,    14,     2,   167,    133+3,    144+3, STR_0260_TOFFEE_QUARRY,STR_CONSTRUCT_TOFFEE_QUARRY_TIP},
{   WWT_CLOSEBOX,    14,     2,   167,    146+3,    157+3, STR_0261_SUGAR_MINE,STR_CONSTRUCT_SUGAR_MINE_TIP},
{      WWT_LAST},
};


static const WindowDesc _build_industry_land0_desc = {
	-1, -1, 170, 116,
	WC_BUILD_INDUSTRY,0,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET,
	_build_industry_land0_widgets,
	BuildIndustryWndProc
};

static const WindowDesc _build_industry_land1_desc = {
	-1, -1, 170, 116,
	WC_BUILD_INDUSTRY,0,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET,
	_build_industry_land1_widgets,
	BuildIndustryWndProc
};

static const WindowDesc _build_industry_land2_desc = {
	-1, -1, 170, 116,
	WC_BUILD_INDUSTRY,0,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET,
	_build_industry_land2_widgets,
	BuildIndustryWndProc
};

static const WindowDesc _build_industry_land3_desc = {
	-1, -1, 170, 116,
	WC_BUILD_INDUSTRY,0,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET,
	_build_industry_land3_widgets,
	BuildIndustryWndProc
};

static const WindowDesc _build_industry_land0_desc_extra = {
	-1, -1, 170, 188,
	WC_BUILD_INDUSTRY,0,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET,
	_build_industry_land0_widgets_extra,
	BuildIndustryWndProc
};

static const WindowDesc _build_industry_land1_desc_extra = {
	-1, -1, 170, 175,
	WC_BUILD_INDUSTRY,0,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET,
	_build_industry_land1_widgets_extra,
	BuildIndustryWndProc
};

static const WindowDesc _build_industry_land2_desc_extra = {
	-1, -1, 170, 201,
	WC_BUILD_INDUSTRY,0,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET,
	_build_industry_land2_widgets_extra,
	BuildIndustryWndProc
};

static const WindowDesc _build_industry_land3_desc_extra = {
	-1, -1, 170, 188,
	WC_BUILD_INDUSTRY,0,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET,
	_build_industry_land3_widgets_extra,
	BuildIndustryWndProc
};

static const WindowDesc * const _industry_window_desc[2][4] = {
	{
	&_build_industry_land0_desc,
	&_build_industry_land1_desc,
	&_build_industry_land2_desc,
	&_build_industry_land3_desc,
	},
	{
	&_build_industry_land0_desc_extra,
	&_build_industry_land1_desc_extra,
	&_build_industry_land2_desc_extra,
	&_build_industry_land3_desc_extra,
	},
};

void ShowBuildIndustryWindow()
{	
	AllocateWindowDescFront(_industry_window_desc[_patches.build_rawmaterial_ind][_opt.landscape],0);
}

static void IndustryViewWndProc(Window *w, WindowEvent *e)
{
	Industry *i;
	StringID str;

	switch(e->event) {
	case WE_PAINT:
		// in editor, use bulldoze to destroy industry
		// Destroy Industry button costing money removed per request of dominik
		//w->disabled_state = (_patches.extra_dynamite && !_networking && _game_mode != GM_EDITOR) ? 0 : (1 << 6);
		i = DEREF_INDUSTRY(w->window_number);
		SET_DPARAM16(0, i->town->index);
		SET_DPARAM16(1, i->type + STR_4802_COAL_MINE);
		DrawWindowWidgets(w);

		if (i->accepts_cargo[0] != 0xFF) {
			SET_DPARAM16(0, _cargoc.names_s[i->accepts_cargo[0]]);
			str = STR_4827_REQUIRES;
			if (i->accepts_cargo[1] != 0xFF) {
				SET_DPARAM16(1, _cargoc.names_s[i->accepts_cargo[1]]);
				str++;
				if (i->accepts_cargo[2] != 0xFF) {
					SET_DPARAM16(2, _cargoc.names_s[i->accepts_cargo[2]]);
					str++;
				}
			}
			DrawString(2, 107, str, 0);
		}
		
		if (i->produced_cargo[0] != 0xFF) {
			DrawString(2, 117, STR_482A_PRODUCTION_LAST_MONTH, 0);

			SET_DPARAM16(1, i->total_production[0]);
			SET_DPARAM16(0, _cargoc.names_long_s[i->produced_cargo[0]] + ((i->total_production[0]!=1)<<5)); 
			SET_DPARAM16(2, i->pct_transported[0] * 100 >> 8);
			DrawString(4, 127, STR_482B_TRANSPORTED, 0);

			if (i->produced_cargo[1] != 0xFF) {
				SET_DPARAM16(1, i->total_production[1]);
				SET_DPARAM16(0, _cargoc.names_long_s[i->produced_cargo[1]] + ((i->total_production[1]!=1)<<5)); 
				SET_DPARAM16(2, i->pct_transported[1] * 100 >> 8);
				DrawString(4, 137, STR_482B_TRANSPORTED, 0);				
			}
		}

		DrawWindowViewport(w);
		break;

	case WE_CLICK:
		switch(e->click.widget) {
		case 5:
			i = DEREF_INDUSTRY(w->window_number);
			ScrollMainWindowToTile(i->xy + TILE_XY(1,1));
			break;
		case 6:
			// Destroy Industry button costing money removed per request of dominik
			//i = DEREF_INDUSTRY(w->window_number);
			/*	passing only i->xy is not safe if industry has a weird shape like:
					_ X X
					X X X
					_ <--- grass, no industry, but i->xy points there (first top-left tile)!, 
					so passing i->xy to destroy industry will fail in called procedure
			*/
			//DoCommandP(i->xy, w->window_number, 0, CcPlaySound10,  CMD_DESTROY_INDUSTRY | CMD_MSG(STR_00B5_CAN_T_CLEAR_THIS_AREA));
			break;
		}
		break;
	}
}

static const Widget _industry_view_widgets[] = {
{    WWT_TEXTBTN,     9,     0,    10,     0,    13, STR_00C5, STR_018B_CLOSE_WINDOW},
{    WWT_CAPTION,     9,    11,   259,     0,    13, STR_4801, STR_018C_WINDOW_TITLE_DRAG_THIS},
{     WWT_IMGBTN,     9,     0,   259,    14,   105, 0x0},
{          WWT_6,     9,     2,   257,    16,   103, 0x0},
{     WWT_IMGBTN,     9,     0,   259,   106,   147, 0x0},
{ WWT_PUSHTXTBTN,     9,     0,   129,   148,   159, STR_00E4_LOCATION, STR_482C_CENTER_THE_MAIN_VIEW_ON},
{     WWT_IMGBTN,     9,   130,   259,   148,   159, 0x0, 0},
// Destroy Industry button costing money removed per request of dominik
//{ WWT_PUSHTXTBTN,     9,   130,   259,   148,   159, STR_INDUSTRYDIR_DESTROY, STR_482C_DESTROY_INDUSTRY},
{      WWT_LAST},
};

static const WindowDesc _industry_view_desc = {
	-1, -1, 260, 160,
	WC_INDUSTRY_VIEW,0,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET | WDF_UNCLICK_BUTTONS,
	_industry_view_widgets,
	IndustryViewWndProc
};

void ShowIndustryViewWindow(int industry)
{
	Window *w;
	Industry *i;

	w = AllocateWindowDescFront(&_industry_view_desc, industry);
	if (w) {
		w->flags4 |= WF_DISABLE_VP_SCROLL;
		i = DEREF_INDUSTRY(w->window_number);
		AssignWindowViewport(w, 3, 17, 0xFE, 0x56, i->xy + TILE_XY(1,1), 1);
	}
}

static const Widget _industry_directory_widgets[] = {
{    WWT_TEXTBTN,    13,     0,    10,     0,    13, STR_00C5, STR_018B_CLOSE_WINDOW},
{    WWT_CAPTION,    13,    11,   507,     0,    13, STR_INDUSTRYDIR_CAPTION, STR_018C_WINDOW_TITLE_DRAG_THIS},
{ WWT_PUSHTXTBTN,    13,     0,   100,    14,    25, STR_SORT_BY_NAME, STR_SORT_TIP},
{ WWT_PUSHTXTBTN,    13,   101,   200,    14,    25, STR_SORT_BY_TYPE, STR_SORT_TIP},
{ WWT_PUSHTXTBTN,    13,   201,   300,    14,    25, STR_SORT_BY_PRODUCTION, STR_SORT_TIP},
{ WWT_PUSHTXTBTN,    13,   301,   400,    14,    25, STR_SORT_BY_TRANSPORTED, STR_SORT_TIP},
{      WWT_PANEL,    13,   401,   496,    14,    25, 0x0, 0},
{     WWT_IMGBTN,    13,     0,   496,    26,   189, 0x0, STR_200A_TOWN_NAMES_CLICK_ON_NAME},
{  WWT_SCROLLBAR,    13,   497,   507,    14,   189, 0x0, STR_0190_SCROLL_BAR_SCROLLS_LIST},
{      WWT_LAST},
};

static byte _industry_sort[lengthof(_industries)];
static uint _num_industry_sort;

static char _bufcache[96];
static byte _last_industry_idx;

static byte _industry_sort_order;

static int CDECL IndustrySorter(const void *a, const void *b)
{
	char buf1[96];
	Industry *i, *j;
	byte val;
	int r = 0;

	i = DEREF_INDUSTRY(*(byte*)a);
	j = DEREF_INDUSTRY(*(byte*)b);

	switch (_industry_sort_order >> 1) {
	case 0: 
		r = 0; 
		break;
	case 1: /* Case 1, sort by type */
		r = i->type - j->type; 
		break;
	case 2: /* Case 2, sort by production */
		if (i->produced_cargo[0] != 0xFF && j->produced_cargo[0] != 0xFF) { //producing any cargo?
				if (i->produced_cargo[1] == 0xFF) //producing one or two things?
					r = j->total_production[0] - i->total_production[0];
				else	
					r = (j->total_production[0] + j->total_production[1]) / 2 - (i->total_production[0] + i->total_production[1]) / 2;
		} else if (i->produced_cargo[0] == 0xFF && j->produced_cargo[0] == 0xFF) //None of them producing anything, let them go to the name-sorting
			r = 0;
		else if (i->produced_cargo[0] == 0xFF) //Non-producers, end up last/first in list
			r = 1;
		else
			r = -1;
		break;
	case 3: /* Case 3, sort by transportation */
		if (i->produced_cargo[0] != 0xFF && j->produced_cargo[0] != 0xFF) { //producing any cargo?
				if (i->produced_cargo[1] == 0xFF) //producing one or two things?
					r = (j->pct_transported[0] * 100 >> 8) - (i->pct_transported[0] * 100 >> 8);
				else 
					r = ((j->pct_transported[0] * 100 >> 8) + (j->pct_transported[1] * 100 >> 8)) / 2 - ((i->pct_transported[0] * 100 >> 8) + (i->pct_transported[1] * 100 >> 8)) / 2;
		} else if (i->produced_cargo[0] == 0xFF && j->produced_cargo[0] == 0xFF) //None of them producing anything, let them go to the name-sorting
			r = 0;
		else if (i->produced_cargo[0] == 0xFF) //Non-producers, end up last/first in list
			r = 1;
		else
			r = -1;
		break;
	}

	// default to string sorting if they are otherwise equal
	if (r == 0) {
		SET_DPARAM32(0, i->town->townnameparts);
		GetString(buf1, i->town->townnametype);

		if ( (val=*(byte*)b) != _last_industry_idx) {
			_last_industry_idx = val;
			SET_DPARAM32(0, j->town->townnameparts);
			GetString(_bufcache, j->town->townnametype);
		}
		r = strcmp(buf1, _bufcache);
	}
	
	if (_industry_sort_order & 1)
		r = -r;
	
	return r;
}

static void MakeSortedIndustryList()
{
	Industry *i;
	int n = 0, index = 0;

	for(i=_industries; i != endof(_industries); i++) {
		if(i->xy) _industry_sort[n++] = index;
		index++;
	}
	_num_industry_sort = n;
	_last_industry_idx = 255; // used for "cache"

	qsort(_industry_sort, n, 1, IndustrySorter);
}


static void IndustryDirectoryWndProc(Window *w, WindowEvent *e)
{
	switch(e->event) {
	case WE_PAINT: {
		int n;
		uint p;
		Industry *i;
		static const uint16 _indicator_positions[4] = {88, 187, 284, 387};

		if (_industry_sort_dirty) {
			_industry_sort_dirty = false;
			MakeSortedIndustryList();
		}

		w->vscroll.count = _num_industry_sort;

		DrawWindowWidgets(w);
		DoDrawString(_industry_sort_order & 1 ? "\xAA" : "\xA0", _indicator_positions[_industry_sort_order>>1], 15, 0x10);

		p = w->vscroll.pos;
		n = 0;

		while (p < _num_industry_sort) {
			i = DEREF_INDUSTRY(_industry_sort[p]);
			SET_DPARAM16(0, i->town->index);
			SET_DPARAM16(1, i->type + STR_4802_COAL_MINE);
			if (i->produced_cargo[0] != 0xFF) {
				SET_DPARAM16(3, i->total_production[0] * 100 >> 8);
				SET_DPARAM16(2, _cargoc.names_long_s[i->produced_cargo[0]] + ((i->total_production[0]!=1)<<5));
				
				if (i->produced_cargo[1] != 0xFF) {
					SET_DPARAM16(5, i->total_production[1] * 100 >> 8);
					SET_DPARAM16(4, _cargoc.names_long_s[i->produced_cargo[1]] + ((i->total_production[1]!=1)<<5));
					SET_DPARAM16(6, i->pct_transported[0] * 100 >> 8);
					SET_DPARAM16(7, i->pct_transported[1] * 100 >> 8);
					DrawString(4, 28+n*10, STR_INDUSTRYDIR_ITEM_TWO, 0);
				} else {
					SET_DPARAM16(4, i->pct_transported[0] * 100 >> 8);
					DrawString(4, 28+n*10, STR_INDUSTRYDIR_ITEM, 0);
				}
			} else {
				DrawString(4, 28+n*10, STR_INDUSTRYDIR_ITEM_NOPROD, 0);
			}
			p++;
			if (++n == 16)
				break;
		}
	} break;

	case WE_CLICK:
		switch(e->click.widget) {
		case 2: {
			_industry_sort_order = _industry_sort_order==0 ? 1 : 0;
			_industry_sort_dirty = true;
			SetWindowDirty(w);
		} break;

		case 3: {
			_industry_sort_order = _industry_sort_order==2 ? 3 : 2;
			_industry_sort_dirty = true;
			SetWindowDirty(w);
		} break;

		case 4: {
			_industry_sort_order = _industry_sort_order==4 ? 5 : 4;
			_industry_sort_dirty = true;
			SetWindowDirty(w);
		} break;

		case 5: {
			_industry_sort_order = _industry_sort_order==6 ? 7 : 6;
			_industry_sort_dirty = true;
			SetWindowDirty(w);
		} break;
		
		case 7: {
			int y = (e->click.pt.y - 28) / 10;
			byte p;
			Industry *c;

			if (!IS_INT_INSIDE(y, 0, 16))
				return;
			p = y + w->vscroll.pos;
			if (p < _num_industry_sort) {
				c = DEREF_INDUSTRY(_industry_sort[p]);
				ScrollMainWindowToTile(c->xy);
			}
		} break;
		}
		break;

	case WE_4:
		SetWindowDirty(w);
		break;
	}  
}


/* Industry List */
static const WindowDesc _industry_directory_desc = {
	-1, -1, 508, 190,
	WC_INDUSTRY_DIRECTORY,0,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET | WDF_UNCLICK_BUTTONS,
	_industry_directory_widgets,
	IndustryDirectoryWndProc
};



void ShowIndustryDirectory()
{
	/* Industry List */
	Window *w;

	w = AllocateWindowDescFront(&_industry_directory_desc, 0);
	if (w) {
		w->vscroll.cap = 16;
		SetWindowDirty(w);
	}
}
