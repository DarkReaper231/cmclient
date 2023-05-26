#include "stdafx.h"
#include "ttd.h"
#include "window.h"
#include "gui.h"
#include "gfx.h"
#include "player.h"

static uint _legend_showbits;
static uint _legend_cargobits;

/************************/
/* GENERIC GRAPH DRAWER */
/************************/

typedef struct GraphDrawer {
	uint sel;
	byte num_dataset;
	byte num_on_x_axis;
	byte month;
	byte year;
	bool include_neg;
	byte num_vert_lines;
	uint16 unk61A;
	uint16 unk61C;
	int left, top;
	uint height;
	StringID format_str_y_axis;
	byte color_3, color_2, bg_line_color;
	byte colors[16];
	uint32 cost[16][24];
} GraphDrawer;

void DrawGraph(GraphDrawer *gw)
{
	
	int i,j,k;
	int x,y,old_x,old_y;
	int color;
	int right, bottom;
	int num_x, num_dataset;
	uint32 *row_ptr, *col_ptr;
	int32 mx;
	int adj_height;
	uint32 y_scaling, tmp;
	int32 value;
	int32 cur_val;
	uint sel;

	color = _color_list[gw->bg_line_color].window_color_1b;

	/* draw the vertical lines */
	i = gw->num_vert_lines; assert(i > 0);
	x = gw->left + 66;
	bottom = gw->top + gw->height - 1;
	do {
		GfxFillRect(x, gw->top, x, bottom, color);
		x += 22;
	} while (--i);
	
	/* draw the horizontal lines */
	i = 9;
	x = gw->left + 44;
	y = gw->height + gw->top;
	right = gw->left + 44 + gw->num_vert_lines*22-1;

	do {
		GfxFillRect(x, y, right, y, color);
		y -= gw->height >> 3;
	} while (--i);

	/* draw vertical edge line */
	GfxFillRect(x, gw->top, x, bottom, gw->color_2);

	adj_height = gw->height;
	if (gw->include_neg) adj_height >>= 1;

	/* draw horiz edge line */
	y = adj_height + gw->top;
	GfxFillRect(x, y, right, y, gw->color_2);

	/* find the max element */
	if (gw->num_on_x_axis == 0)
		return;

	num_dataset = gw->num_dataset;assert(num_dataset > 0);
	row_ptr = gw->cost[0];
	mx = 0;
	do {
		num_x = gw->num_on_x_axis;assert(num_x > 0);
		col_ptr = row_ptr;
		do {
			if (*col_ptr != 0x80000000) {
				mx = max(mx, myabs(*col_ptr));
			}
		} while (col_ptr++, --num_x);
	} while (row_ptr+=24, --num_dataset);
	
	/* setup scaling */
	y_scaling = 0x80000000;
	value = adj_height * 2;

	if (mx > value) {
		mx = (mx + 7) & ~7;
		y_scaling = (uint32) (((uint64) (value>>1) << 32) / mx);
		value = mx;
	}

	/* draw text strings on the y axis */
	tmp = value;
	if (gw->include_neg) tmp >>= 1;
	x = gw->left + 45;
	y = gw->top - 3;
	i = 9;
	do {
		SET_DPARAM16(0, gw->format_str_y_axis);
		SET_DPARAM32(1, tmp);
		tmp -= (value >> 3);
		DrawStringRightAligned(x, y, STR_0170, gw->color_3);
		y += gw->height >> 3;
	} while (--i);

	/* draw strings on the x axis */
	if (gw->month != 0xFF) {
		x = gw->left + 44;
		y = gw->top + gw->height + 1;
		j = gw->month;
		k = gw->year + 1920;
		i = gw->num_on_x_axis;assert(i>0);
		do {
			SET_DPARAM16(2, k);
			SET_DPARAM16(0, j + STR_0162_JAN);
			SET_DPARAM16(1, j + STR_0162_JAN + 2);
			DrawString(x, y, j == 0 ? STR_016F : STR_016E, gw->color_3);

			j += 3;
			if (j >= 12) {
				j = 0;
				k++;
			}
			x += 22;
		} while (--i);
	} else {
		x = gw->left + 52;
		y = gw->top + gw->height + 1;
		j = gw->unk61A;
		i = gw->num_on_x_axis;assert(i>0);
		do {
			SET_DPARAM16(0, j);
			DrawString(x, y, STR_01CB, gw->color_3);
			j += gw->unk61C;
			x += 22;
		} while (--i);
	}

	/* draw lines and dots */
	i = 0;
	row_ptr = gw->cost[0];
	sel = gw->sel;
	do {
		if (!(sel & 1)) {
			x = gw->left + 55;
			j = gw->num_on_x_axis;assert(j>0);
			col_ptr = row_ptr;
			color = gw->colors[i];
			old_y = old_x = 0x80000000;
			do {
				cur_val = *col_ptr++;
				if (cur_val != (int32)0x80000000) {
					y = adj_height - BIGMULSS(cur_val, y_scaling >> 1, 31) + gw->top;

					GfxFillRect(x-1, y-1, x+1, y+1, color);
					if (old_x != 0x80000000)
						GfxDrawLine(old_x, old_y, x, y, color);

					old_x = x;
					old_y = y;
				} else {
					old_x = 0x80000000;
				}
			} while (x+=22,--j);
		}
	} while (sel>>=1,row_ptr+=24, ++i < gw->num_dataset);
}

/****************/
/* GRAPH LEGEND */
/****************/

void DrawPlayerIcon(int p, int x, int y)
{
	DrawSprite(SPRITE_PALETTE(PLAYER_SPRITE_COLOR(p) + 0x2EB), x, y);
}

static void GraphLegendWndProc(Window *w, WindowEvent *e)
{
	Player *p;

	switch(e->event) {
	case WE_PAINT:
		FOR_ALL_PLAYERS(p) {
			if (!p->is_active)
				SETBIT(_legend_showbits, p->index);
		}
		w->click_state = ((~_legend_showbits) << 3);
		DrawWindowWidgets(w);

		FOR_ALL_PLAYERS(p) {
			if (!p->is_active)
				continue;

			DrawPlayerIcon(p->index, 4, 18+p->index*12);

			SET_DPARAM16(0, p->name_1);
			SET_DPARAM32(1, p->name_2);
			SET_DPARAM16(2, GetPlayerNameString(p->index));
			DrawString(21,17+p->index*12,STR_7021,HASBIT(_legend_showbits, p->index) ? 0x10 : 0xC);
		}
		break;

	case WE_CLICK:
		if (IS_INT_INSIDE(e->click.widget, 3, 11)) {
			_legend_showbits ^= (1 << (e->click.widget-3));
			SetWindowDirty(w);
			InvalidateWindow(WC_INCOME_GRAPH, 0);
			InvalidateWindow(WC_OPERATING_PROFIT, 0);
			InvalidateWindow(WC_DELIVERED_CARGO, 0);
			InvalidateWindow(WC_PERFORMANCE_HISTORY, 0);
		}
		break;
	}
}

static const Widget _graph_legend_widgets[] = {
{    WWT_TEXTBTN,    14,     0,    10,     0,    13, STR_00C5, STR_018B_CLOSE_WINDOW},
{    WWT_CAPTION,    14,    11,   249,     0,    13, STR_704E_KEY_TO_COMPANY_GRAPHS, STR_018C_WINDOW_TITLE_DRAG_THIS},
{     WWT_IMGBTN,    14,     0,   249,    14,   113, 0x0},
{     WWT_IMGBTN,    14,     2,   247,    16,    27, 0x0,STR_704F_CLICK_HERE_TO_TOGGLE_COMPANY},
{     WWT_IMGBTN,    14,     2,   247,    28,    39, 0x0,STR_704F_CLICK_HERE_TO_TOGGLE_COMPANY},
{     WWT_IMGBTN,    14,     2,   247,    40,    51, 0x0,STR_704F_CLICK_HERE_TO_TOGGLE_COMPANY},
{     WWT_IMGBTN,    14,     2,   247,    52,    63, 0x0,STR_704F_CLICK_HERE_TO_TOGGLE_COMPANY},
{     WWT_IMGBTN,    14,     2,   247,    64,    75, 0x0,STR_704F_CLICK_HERE_TO_TOGGLE_COMPANY},
{     WWT_IMGBTN,    14,     2,   247,    76,    87, 0x0,STR_704F_CLICK_HERE_TO_TOGGLE_COMPANY},
{     WWT_IMGBTN,    14,     2,   247,    88,    99, 0x0,STR_704F_CLICK_HERE_TO_TOGGLE_COMPANY},
{     WWT_IMGBTN,    14,     2,   247,   100,   111, 0x0,STR_704F_CLICK_HERE_TO_TOGGLE_COMPANY},
{      WWT_LAST},
};

static const WindowDesc _graph_legend_desc = {
	-1, -1, 250, 114,
	WC_GRAPH_LEGEND,0,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET,
	_graph_legend_widgets,
	GraphLegendWndProc
};

static void ShowGraphLegend()
{
	AllocateWindowDescFront(&_graph_legend_desc, 0);
}

/********************/
/* OPERATING PROFIT */
/********************/

static void SetupGraphDrawerForPlayers(GraphDrawer *gd)
{
	Player *p;
	uint showbits = _legend_showbits;
	int nums;
	int mo,yr;

	// Exclude the players which aren't valid	
	FOR_ALL_PLAYERS(p) {
		if (!p->is_active) CLRBIT(showbits,p->index);
	}	
	gd->sel = showbits;
	gd->num_vert_lines = 24;

	nums = 0;
	FOR_ALL_PLAYERS(p) {
		if (p->is_active) nums = max(nums,p->num_valid_stat_ent);
	}
	gd->num_on_x_axis = min(nums,24);

	mo = (_cur_month/3-nums)*3;
	yr = _cur_year;
	while (mo < 0) {
		yr--;
		mo += 12;
	}

	gd->year = yr;
	gd->month = mo;
}

static void OperatingProfitWndProc(Window *w, WindowEvent *e)
{
	switch(e->event) {
	case WE_PAINT: {
		GraphDrawer gd;
		Player *p;
		int i,j;
		int numd;

		DrawWindowWidgets(w);
		
		gd.left = 2;
		gd.top = 18;
		gd.height = 136;
		gd.include_neg = true;
		gd.format_str_y_axis = STR_7023;
		gd.color_3 = 0x10;
		gd.color_2 = 0xD7;
		gd.bg_line_color = 0xE;
		
		SetupGraphDrawerForPlayers(&gd);

		numd = 0;
		FOR_ALL_PLAYERS(p) {
			if (!p->is_active)
				continue;
			gd.colors[numd] = _color_list[p->player_color].window_color_bgb;
			for(j=gd.num_on_x_axis,i=0; --j >= 0;) {
				gd.cost[numd][i] = (j >= p->num_valid_stat_ent) ? 0x80000000 : 	(p->old_economy[j].income + p->old_economy[j].expenses);
				i++;
			}
			numd++;
		}
		gd.num_dataset=numd;

		DrawGraph(&gd);
		break;
	}

	case WE_CLICK:
		if (e->click.widget == 2)
			ShowGraphLegend();
		break;
	}
}

static const Widget _operating_profit_widgets[] = {
{    WWT_TEXTBTN,    14,     0,    10,     0,    13, STR_00C5,STR_018B_CLOSE_WINDOW},
{    WWT_CAPTION,    14,    11,   525,     0,    13, STR_7025_OPERATING_PROFIT_GRAPH, STR_018C_WINDOW_TITLE_DRAG_THIS},
{ WWT_PUSHTXTBTN,    14,   526,   575,     0,    13, STR_704C_KEY, STR_704D_SHOW_KEY_TO_GRAPHS},
{     WWT_IMGBTN,    14,     0,   575,    14,   173, 0x0},
{      WWT_LAST},
};

static const WindowDesc _operating_profit_desc = {
	-1, -1, 576, 174,
	WC_OPERATING_PROFIT,0,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET | WDF_UNCLICK_BUTTONS,
	_operating_profit_widgets,
	OperatingProfitWndProc
};


void ShowOperatingProfitGraph()
{
	if (AllocateWindowDescFront(&_operating_profit_desc, 0)) {
		InvalidateWindow(WC_GRAPH_LEGEND, 0);
		_legend_showbits = 0;
	}
}


/****************/
/* INCOME GRAPH */
/****************/

static void IncomeGraphWndProc(Window *w, WindowEvent *e)
{
	switch(e->event) {
	case WE_PAINT: {
		GraphDrawer gd;
		Player *p;
		int i,j;
		int numd;

		DrawWindowWidgets(w);

		gd.left = 2;
		gd.top = 18;
		gd.height = 104;
		gd.include_neg = false;
		gd.format_str_y_axis = STR_7023;
		gd.color_3 = 0x10;
		gd.color_2 = 0xD7;
		gd.bg_line_color = 0xE;
		SetupGraphDrawerForPlayers(&gd);

		numd = 0;
		FOR_ALL_PLAYERS(p) {
			if (!p->is_active)
				continue;
			gd.colors[numd] = _color_list[p->player_color].window_color_bgb;
			for(j=gd.num_on_x_axis,i=0; --j >= 0;) {
				gd.cost[numd][i] = (j >= p->num_valid_stat_ent) ? 0x80000000 : (p->old_economy[j].income);
				i++;
			}
			numd++;
		}

		gd.num_dataset = numd;

		DrawGraph(&gd);
		break;
	}

	case WE_CLICK:
		if (e->click.widget == 2)
			ShowGraphLegend();
		break;
	}
}

static const Widget _income_graph_widgets[] = {
{    WWT_TEXTBTN,    14,     0,    10,     0,    13, STR_00C5,STR_018B_CLOSE_WINDOW},
{    WWT_CAPTION,    14,    11,   525,     0,    13, STR_7022_INCOME_GRAPH, STR_018C_WINDOW_TITLE_DRAG_THIS},
{ WWT_PUSHTXTBTN,    14,   526,   575,     0,    13, STR_704C_KEY, STR_704D_SHOW_KEY_TO_GRAPHS},
{     WWT_IMGBTN,    14,     0,   575,    14,   141, 0x0},
{      WWT_LAST},
};

static const WindowDesc _income_graph_desc = {
	-1, -1, 576, 142,
	WC_INCOME_GRAPH,0,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET | WDF_UNCLICK_BUTTONS,
	_income_graph_widgets,
	IncomeGraphWndProc
};

void ShowIncomeGraph()
{
	if (AllocateWindowDescFront(&_income_graph_desc, 0)) {
		InvalidateWindow(WC_GRAPH_LEGEND, 0);
		_legend_showbits = 0;
	}
}

/*******************/
/* DELIVERED CARGO */
/*******************/

static void DeliveredCargoGraphWndProc(Window *w, WindowEvent *e)
{
	switch(e->event) {
	case WE_PAINT: {
		GraphDrawer gd;
		Player *p;
		int i,j;
		int numd;

		DrawWindowWidgets(w);

		gd.left = 2;
		gd.top = 18;
		gd.height = 104;
		gd.include_neg = false;
		gd.format_str_y_axis = STR_7024;
		gd.color_3 = 0x10;
		gd.color_2 = 0xD7;
		gd.bg_line_color = 0xE;
		SetupGraphDrawerForPlayers(&gd);

		numd = 0;
		FOR_ALL_PLAYERS(p) {
			if (!p->is_active)
				continue;
			gd.colors[numd] = _color_list[p->player_color].window_color_bgb;
			for(j=gd.num_on_x_axis,i=0; --j >= 0;) {
				gd.cost[numd][i] = (j >= p->num_valid_stat_ent) ? 0x80000000 : p->old_economy[j].delivered_cargo;
				i++;
			}
			numd++;
		}

		gd.num_dataset = numd;

		DrawGraph(&gd);
		break;
	}

	case WE_CLICK:
		if (e->click.widget == 2)
			ShowGraphLegend();
		break;
	}
}

static const Widget _delivered_cargo_graph_widgets[] = {
{    WWT_TEXTBTN,    14,     0,    10,     0,    13, STR_00C5, STR_018B_CLOSE_WINDOW},
{    WWT_CAPTION,    14,    11,   525,     0,    13, STR_7050_UNITS_OF_CARGO_DELIVERED, STR_018C_WINDOW_TITLE_DRAG_THIS},
{ WWT_PUSHTXTBTN,    14,   526,   575,     0,    13, STR_704C_KEY, STR_704D_SHOW_KEY_TO_GRAPHS},
{     WWT_IMGBTN,    14,     0,   575,    14,   141, 0x0},
{      WWT_LAST},
};

static const WindowDesc _delivered_cargo_graph_desc = {
	-1, -1, 576, 142,
	WC_DELIVERED_CARGO,0,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET | WDF_UNCLICK_BUTTONS,
	_delivered_cargo_graph_widgets,
	DeliveredCargoGraphWndProc
};

void ShowDeliveredCargoGraph()
{
	if (AllocateWindowDescFront(&_delivered_cargo_graph_desc, 0)) {
		InvalidateWindow(WC_GRAPH_LEGEND, 0);
		_legend_showbits = 0;
	}
}	

/***********************/
/* PERFORMANCE HISTORY */
/***********************/

static void PerformanceHistoryWndProc(Window *w, WindowEvent *e)
{
	switch(e->event) {
	case WE_PAINT: {
		GraphDrawer gd;
		Player *p;
		int i,j;
		int numd;

		DrawWindowWidgets(w);

		gd.left = 2;
		gd.top = 18;
		gd.height = 200;
		gd.include_neg = false;
		gd.format_str_y_axis = STR_7024;
		gd.color_3 = 0x10;
		gd.color_2 = 0xD7;
		gd.bg_line_color = 0xE;
		SetupGraphDrawerForPlayers(&gd);

		numd = 0;
		FOR_ALL_PLAYERS(p) {
			if (!p->is_active)
				continue;
			gd.colors[numd] = _color_list[p->player_color].window_color_bgb;
			for(j=gd.num_on_x_axis,i=0; --j >= 0;) {
				gd.cost[numd][i] = (j >= p->num_valid_stat_ent) ? 0x80000000 : p->old_economy[j].performance_history;
				i++;
			}
			numd++;
		}
		gd.num_dataset = numd;

		DrawGraph(&gd);
		break;
	}

	case WE_CLICK:
		if (e->click.widget == 2)
			ShowGraphLegend();
		break;
	}
}

static const Widget _performance_history_widgets[] = {
{    WWT_TEXTBTN,    14,     0,    10,     0,    13, STR_00C5, STR_018B_CLOSE_WINDOW},
{    WWT_CAPTION,    14,    11,   525,     0,    13, STR_7051_COMPANY_PERFORMANCE_RATINGS, STR_018C_WINDOW_TITLE_DRAG_THIS},
{ WWT_PUSHTXTBTN,    14,   526,   575,     0,    13, STR_704C_KEY, STR_704D_SHOW_KEY_TO_GRAPHS},
{     WWT_IMGBTN,    14,     0,   575,    14,   237, 0x0},
{      WWT_LAST},
};

static const WindowDesc _performance_history_desc = {
	-1, -1, 576, 238,
	WC_PERFORMANCE_HISTORY,0,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET | WDF_UNCLICK_BUTTONS,
	_performance_history_widgets,
	PerformanceHistoryWndProc
};

void ShowPerformanceHistoryGraph()
{
	if (AllocateWindowDescFront(&_performance_history_desc, 0)) {
		InvalidateWindow(WC_GRAPH_LEGEND, 0);
		_legend_showbits = 0;
	}
}

/*****************/
/* COMPANY VALUE */
/*****************/

static void CompanyValueGraphWndProc(Window *w, WindowEvent *e)
{
	switch(e->event) {
	case WE_PAINT: {
		GraphDrawer gd;
		Player *p;
		int i,j;
		int numd;

		DrawWindowWidgets(w);

		gd.left = 2;
		gd.top = 18;
		gd.height = 200;
		gd.include_neg = false;
		gd.format_str_y_axis = STR_7023;
		gd.color_3 = 0x10;
		gd.color_2 = 0xD7;
		gd.bg_line_color = 0xE;
		SetupGraphDrawerForPlayers(&gd);

		numd = 0;
		FOR_ALL_PLAYERS(p) {
			if (!p->is_active)
				continue;
			gd.colors[numd] = _color_list[p->player_color].window_color_bgb;
			for(j=gd.num_on_x_axis,i=0; --j >= 0;) {
				gd.cost[numd][i] = (j >= p->num_valid_stat_ent) ? 0x80000000 : (p->old_economy[j].company_value);
				i++;
			}
			numd++;
		}
		gd.num_dataset = numd;


		DrawGraph(&gd);
		break;
	}

	case WE_CLICK:
		if (e->click.widget == 2)
			ShowGraphLegend();
		break;
	}
}

static const Widget _company_value_graph_widgets[] = {
{    WWT_TEXTBTN,    14,     0,    10,     0,    13, STR_00C5, STR_018B_CLOSE_WINDOW},
{    WWT_CAPTION,    14,    11,   525,     0,    13, STR_7052_COMPANY_VALUES, STR_018C_WINDOW_TITLE_DRAG_THIS},
{ WWT_PUSHTXTBTN,    14,   526,   575,     0,    13, STR_704C_KEY, STR_704D_SHOW_KEY_TO_GRAPHS},
{     WWT_IMGBTN,    14,     0,   575,    14,   237, 0x0},
{      WWT_LAST},
};

static const WindowDesc _company_value_graph_desc = {
	-1, -1, 576, 238,
	WC_COMPANY_VALUE,0,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET | WDF_UNCLICK_BUTTONS,
	_company_value_graph_widgets,
	CompanyValueGraphWndProc
};

void ShowCompanyValueGraph()
{
	if (AllocateWindowDescFront(&_company_value_graph_desc, 0)) {
		InvalidateWindow(WC_GRAPH_LEGEND, 0);
		_legend_showbits = 0;
	}
}

/*****************/
/* PAYMENT RATES */
/*****************/

static const byte _cargo_legend_colors[12] = {152, 32, 15, 174, 208, 194, 191, 84, 184, 10, 202, 215};

static void CargoPaymentRatesWndProc(Window *w, WindowEvent *e)
{
	switch(e->event) {
	case WE_PAINT: {
		int i, j, x, y;
		GraphDrawer gd;

		gd.sel = _legend_cargobits;
		w->click_state = (~_legend_cargobits) << 3;
		DrawWindowWidgets(w);

		x = 495;
		y = 25;

		for(i=0; i!=NUM_CARGO; i++) {
			GfxFillRect(x, y, x+8, y+5, 0);
			GfxFillRect(x+1, y+1, x+7, y+4, _cargo_legend_colors[i]);
			SET_DPARAM16(0, _cargoc.names_s[i]);
			DrawString(x+14, y, STR_7065, 0);
			y += 8;
		}

		gd.left = 2;
		gd.top = 24;
		gd.height = 104;
		gd.include_neg = false;
		gd.format_str_y_axis = STR_7023;
		gd.color_3 = 16;
		gd.color_2 = 215;
		gd.bg_line_color = 14;
		gd.num_dataset = 12;
		gd.num_on_x_axis = 20;
		gd.num_vert_lines = 20;
		gd.month = 0xFF;
		gd.unk61A = 10;
		gd.unk61C = 10;
		
		for(i=0; i!=NUM_CARGO; i++) {
			gd.colors[i] = _cargo_legend_colors[i];
			for(j=0; j!=20; j++) {
				gd.cost[i][j] = GetTransportedGoodsIncome(10, 20, j*6+6,i);
			}
		}

		DrawGraph(&gd);

		DrawString(2 + 46, 24 + gd.height + 7, STR_7062_DAYS_IN_TRANSIT, 0);
		DrawString(2 + 84, 24 - 9, STR_7063_PAYMENT_FOR_DELIVERING, 0);
	} break;

	case WE_CLICK: {
		switch(e->click.widget) {
		case 3: case 4: case 5: case 6:
		case 7: case 8: case 9: case 10:
		case 11: case 12: case 13: case 14:
			_legend_cargobits ^= 1 << (e->click.widget - 3);
			SetWindowDirty(w);
			break;
		}
	} break;
	}
}

static const Widget _cargo_payment_rates_widgets[] = {
{   WWT_CLOSEBOX,    14,     0,    10,     0,    13, STR_00C5,											STR_018B_CLOSE_WINDOW},
{    WWT_CAPTION,    14,    11,   567,     0,    13, STR_7061_CARGO_PAYMENT_RATES,	STR_018C_WINDOW_TITLE_DRAG_THIS},
{      WWT_PANEL,    14,     0,   567,    14,   141, 0x0,														0},
{      WWT_PANEL,    12,   493,   562,    24,    31, 0x0,														STR_7064_TOGGLE_GRAPH_FOR_CARGO},
{      WWT_PANEL,    12,   493,   562,    32,    39, 0x0,														STR_7064_TOGGLE_GRAPH_FOR_CARGO},
{      WWT_PANEL,    12,   493,   562,    40,    47, 0x0,														STR_7064_TOGGLE_GRAPH_FOR_CARGO},
{      WWT_PANEL,    12,   493,   562,    48,    55, 0x0,														STR_7064_TOGGLE_GRAPH_FOR_CARGO},
{      WWT_PANEL,    12,   493,   562,    56,    63, 0x0,														STR_7064_TOGGLE_GRAPH_FOR_CARGO},
{      WWT_PANEL,    12,   493,   562,    64,    71, 0x0,														STR_7064_TOGGLE_GRAPH_FOR_CARGO},
{      WWT_PANEL,    12,   493,   562,    72,    79, 0x0,														STR_7064_TOGGLE_GRAPH_FOR_CARGO},
{      WWT_PANEL,    12,   493,   562,    80,    87, 0x0,														STR_7064_TOGGLE_GRAPH_FOR_CARGO},
{      WWT_PANEL,    12,   493,   562,    88,    95, 0x0,														STR_7064_TOGGLE_GRAPH_FOR_CARGO},
{      WWT_PANEL,    12,   493,   562,    96,   103, 0x0,														STR_7064_TOGGLE_GRAPH_FOR_CARGO},
{      WWT_PANEL,    12,   493,   562,   104,   111, 0x0,														STR_7064_TOGGLE_GRAPH_FOR_CARGO},
{      WWT_PANEL,    12,   493,   562,   112,   119, 0x0,														STR_7064_TOGGLE_GRAPH_FOR_CARGO},
{      WWT_LAST},
};

static const WindowDesc _cargo_payment_rates_desc = {
	-1, -1, 568, 142,
	WC_PAYMENT_RATES,0,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET,
	_cargo_payment_rates_widgets,
	CargoPaymentRatesWndProc
};


void ShowCargoPaymentRates()
{
	AllocateWindowDescFront(&_cargo_payment_rates_desc, 0);
}

/************************/
/* COMPANY LEAGUE TABLE */
/************************/

static const StringID _performance_titles[] = {
	STR_7066_ENGINEER,
	STR_7066_ENGINEER,
	STR_7067_TRAFFIC_MANAGER,
	STR_7067_TRAFFIC_MANAGER,
	STR_7068_TRANSPORT_COORDINATOR,
	STR_7068_TRANSPORT_COORDINATOR,
	STR_7069_ROUTE_SUPERVISOR,
	STR_7069_ROUTE_SUPERVISOR,
	STR_706A_DIRECTOR,
	STR_706A_DIRECTOR,
	STR_706B_CHIEF_EXECUTIVE,
	STR_706B_CHIEF_EXECUTIVE,
	STR_706C_CHAIRMAN,
	STR_706C_CHAIRMAN,
	STR_706D_PRESIDENT,
	STR_706E_TYCOON,
};

static StringID GetPerformanceTitleFromValue(uint v)
{
	return _performance_titles[minu(v, 1000) >> 6];
}

static int CDECL _perf_hist_comp(const void *elem1, const void *elem2 ) {
	Player *p1 = *(Player**)elem1;
	Player *p2 = *(Player**)elem2;
	int32 v = p2->old_economy[1].performance_history - p1->old_economy[1].performance_history;
	return (v!=0) | (v >> (sizeof(int32)*8-1));
}

static void CompanyLeagueWndProc(Window *w, WindowEvent *e)
{
	switch(e->event) {
	case WE_PAINT: {
		Player *p;
		Player *plist[MAX_PLAYERS];
		size_t pl_num, i;

		DrawWindowWidgets(w);
		
		pl_num=0;
		FOR_ALL_PLAYERS(p) {
			if (p->is_active)
				plist[pl_num++] = p;
		}
		assert(pl_num > 0);
		
		qsort(plist, pl_num, sizeof(Player*), _perf_hist_comp);

		i = 0;
		do {
			SET_DPARAM16(0, i + 1 + STR_01AB);
			p = plist[i];
			SET_DPARAM16(1, p->name_1);
			SET_DPARAM32(2, p->name_2);
			
			SET_DPARAM16(3, GetPlayerNameString(p->index));
			SET_DPARAM16(4, GetPerformanceTitleFromValue(p->old_economy[1].performance_history));

			DrawString(2, 15 + i * 10, i == 0 ? STR_7054 : STR_7055, 0);
			DrawPlayerIcon(p->index, 27, 16 + i * 10);
		} while (++i != pl_num);

		break;
	}
	}
}


static const Widget _company_league_widgets[] = {
{    WWT_TEXTBTN,    14,     0,    10,     0,    13, STR_00C5, STR_018B_CLOSE_WINDOW},
{    WWT_CAPTION,    14,    11,   399,     0,    13, STR_7053_COMPANY_LEAGUE_TABLE, STR_018C_WINDOW_TITLE_DRAG_THIS},
{     WWT_IMGBTN,    14,     0,   399,    14,    96, 0x0},
{      WWT_LAST},
};

static const WindowDesc _company_league_desc = {
	-1, -1, 400, 97,
	WC_COMPANY_LEAGUE,0,
	WDF_STD_TOOLTIPS | WDF_STD_BTN | WDF_DEF_WIDGET,
	_company_league_widgets,
	CompanyLeagueWndProc
};

void ShowCompanyLeagueTable()
{
	AllocateWindowDescFront(&_company_league_desc,0);
}
