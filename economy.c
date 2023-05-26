#include "stdafx.h"
#include "ttd.h"
#include "news.h"
#include "player.h"
#include "station.h"
#include "vehicle.h"
#include "window.h"
#include "gfx.h"
#include "command.h"
#include "saveload.h"
#include "economy.h"
#include "industry.h"
#include "town.h"

static void UpdatePlayerHouse(Player *p, uint score)
{
	byte val;
	uint tile = p->location_of_house;

	if (tile == 0)
		return;

	(val = 128, score < 170) ||
	(val+= 4, score < 350) ||
	(val+= 4, score < 520) ||
	(val+= 4, score < 720) ||
	(val+= 4, true);

/* house is already big enough */
	if (val <= _map5[tile])
		return;

	_map5[tile + TILE_XY(0,0)] = val;
	_map5[tile + TILE_XY(0,1)] = ++val;
	_map5[tile + TILE_XY(1,0)] = ++val;
	_map5[tile + TILE_XY(1,1)] = ++val;

	MarkTileDirtyByTile(tile + TILE_XY(0,0));
	MarkTileDirtyByTile(tile + TILE_XY(0,1));
	MarkTileDirtyByTile(tile + TILE_XY(1,0));
	MarkTileDirtyByTile(tile + TILE_XY(1,1));
}

uint32 CalculateCompanyValue(Player *p) {
	byte owner = p->index;
	uint32 value;
	
	{
		Station *st;
		uint num = 0;
		
		FOR_ALL_STATIONS(st) {
			if (st->xy != 0 && st->owner == owner) {
				uint facil = st->facilities;
				do num += (facil&1); while (facil >>= 1);
			}
		}

		value = num * _price.station_value * 25;
	}

	{
		Vehicle *v;

		FOR_ALL_VEHICLES(v) {
			if (v->owner != owner)
				continue;
			if (v->type == VEH_Train ||
					v->type == VEH_Road ||
					(v->type == VEH_Aircraft && v->subtype<=2) ||
					v->type == VEH_Ship) {
				value += v->value * 3 >> 1;
			}
		}
	}

	if (p->player_money > 0)
		value += p->player_money;

	return value;
}

static void UpdateCompanyRatingAndValue(Player *p)
{
	byte owner = p->index;
	uint score;

/* Count vehicles */
	{
		Vehicle *v;
		int32 min_profit = MAX_INT;
		uint num = 0;

		FOR_ALL_VEHICLES(v) {
			if (v->owner != owner)
				continue;
			if ((v->type == VEH_Train && v->subtype == 0) ||
					v->type == VEH_Road ||
					(v->type == VEH_Aircraft && v->subtype<=2) ||
					v->type == VEH_Ship) {
				num++;
				if (v->age > 730) {
					if (min_profit >= v->profit_last_year)
						min_profit = v->profit_last_year;
				}
			}
		}

		score = (num < 120) ? num * 100 / 120 : 100;

		if (min_profit != MAX_INT && min_profit > 0)
			score += (min_profit < 10000) ? min_profit/100 : 100;
	}

/* Count stations */
	{
		uint num = 0;
		Station *st;

		FOR_ALL_STATIONS(st) {
			if (st->xy != 0 && st->owner == owner) {
				int facil = st->facilities;
				do num += facil&1; while (facil>>=1);
			}
		}
		score += (num < 80) ? num * 100 / 80 : 100;
	}

/* Generate statistics depending on recent income statistics */
	{
		PlayerEconomyEntry *pee;
		int numec;
		int32 min_income;
		uint32 max_income;

		numec = min(p->num_valid_stat_ent, 12);
		if (numec != 0) {
			min_income = 0x7FFFFFFF;
			max_income = 0;
			pee = p->old_economy;
			do {
				min_income = min(min_income, pee->income + pee->expenses);
				max_income = max(max_income, pee->income + pee->expenses);
			} while (++pee,--numec);

			if (min_income > 0)
				score += min_income >= 50000 ? 50 : min_income / 1000;

			score += max_income >= 100000 ? 100 : max_income / 1000;
		}
	}

/* Generate score depending on amount of transported cargo */
	{
		PlayerEconomyEntry *pee;
		int numec;
		uint32 total_delivered;

		numec = min(p->num_valid_stat_ent, 4);
		if (numec != 0) {
			pee = p->old_economy;
			total_delivered = 0;
			do {
				total_delivered += pee->delivered_cargo;
			} while (++pee,--numec);

			score += total_delivered >= 40000 ? 400 : total_delivered / 100;
		}
	}
	
/* Generate score for variety of cargo */
	{
		uint cargo = p->cargo_types;
		uint num = 0;
		do num += cargo&1; while (cargo>>=1);
		score += num < 8 ? num * 50 / 8 : 50;
		p->cargo_types = 0;
	}

/* Generate score for player money */
	{
		int32 money = p->player_money;
		if (money > 0) {
			score += money < 10000000 ? money / (10000000/50) : 50;
		}
	}

/* Generate score for loan */
	{
		score += (250000 - p->current_loan) >= 0 ? (250000 - p->current_loan) / 5000 : 0;
	}

	p->old_economy[0].performance_history = score;
	UpdatePlayerHouse(p, score);
	p->old_economy[0].company_value = CalculateCompanyValue(p);
}

// use 255 as new_player to delete the player.
void ChangeOwnershipOfPlayerItems(byte old_player, byte new_player)
{
	byte old = _current_player;
	_current_player = old_player;

	if (new_player == 255) {
		Subsidy *s;
		
		for(s=_subsidies; s != endof(_subsidies); s++) {
			if (s->cargo_type != 0xff && s->age >= 12) {
				Station *st = DEREF_STATION(s->to);
				if (st->owner == old_player)
					s->cargo_type = 0xff;
			}
		}
	}

	// take care of rating in towns
	{
		Town *t;
		
		// if a player takes over, give the ratings to that player.
		if (new_player != 255) {
			FOR_ALL_TOWNS(t) if (t->xy && HASBIT(t->have_ratings, old_player)) {
				if (HASBIT(t->have_ratings, new_player)) {
					// use max of the two ratings.
					t->ratings[new_player] = max(t->ratings[new_player], t->ratings[old_player]);
				} else {
					SETBIT(t->have_ratings, new_player);
					t->ratings[new_player] = t->ratings[old_player];
				}
			}
		}

		// reset rating for the towns.
		FOR_ALL_TOWNS(t) if (t->xy) {
			t->ratings[old_player] = 500;
			CLRBIT(t->have_ratings, old_player);
		}
	}
	
	{
		int num_train = 0;
		int num_road = 0;
		int num_ship = 0;
		int num_aircraft = 0;
		Vehicle *v;

		// Determine Ids for the new vehicles
		FOR_ALL_VEHICLES(v) {
			if (v->owner == new_player) {
				if (v->type == VEH_Train && v->subtype == 0)
					num_train++;
				else if (v->type == VEH_Road)
					num_road++;
				else if (v->type == VEH_Ship)
					num_ship++;
				else if (v->type == VEH_Aircraft && v->subtype <= 2)
					num_aircraft++;
			}
		}

		FOR_ALL_VEHICLES(v) {
			if (v->owner == old_player && IS_BYTE_INSIDE(v->type, VEH_Train, VEH_Aircraft+1) ) {
				if (new_player == 255) {
					DeleteWindowById(WC_VEHICLE_VIEW, v->index);
					DeleteWindowById(WC_VEHICLE_DETAILS, v->index);
					DeleteWindowById(WC_VEHICLE_ORDERS, v->index);
					DeleteVehicle(v);
				} else {
					v->owner = new_player;
					if (v->type == VEH_Train && v->subtype == 0)
						v->unitnumber = ++num_train;
					else if (v->type == VEH_Road)
						v->unitnumber = ++num_road;
					else if (v->type == VEH_Ship)
						v->unitnumber = ++num_ship;
					else if (v->type == VEH_Aircraft && v->subtype <= 2)
						v->unitnumber = ++num_aircraft;
				}
			}
		}
	}

	// Change ownership of tiles
	{
		uint tile = 0;
		do {
			ChangeTileOwner(tile, old_player, new_player);
		} while (++tile != TILES_X * TILES_Y);
	}

	// Change color of existing windows
	if (new_player != 255) {
		Window *w;
		for(w=_windows; w != _last_window; w++) {
			if (w->caption_color == old_player)
				w->caption_color = new_player;
		}
	}

	_current_player = old;

	MarkWholeScreenDirty();
}

extern void DeletePlayerWindows(int pi);

static void PlayersCheckBankrupt(Player *p)
{
	int owner;
	uint32 val;

	if (p->player_money >= 0) {
		p->quarters_of_bankrupcy = 0;
		return;
	}

	p->quarters_of_bankrupcy++;

	owner = p->index;

	if (p->quarters_of_bankrupcy == 2) {
year_2:
		AddNewsItem( (StringID)(owner + 16), 
			NEWS_FLAGS(NM_CALLBACK, 0, NT_COMPANY_INFO, DNC_BANKRUPCY),0,0);

	} else if (p->quarters_of_bankrupcy == 3) {
		if (IS_HUMAN_PLAYER(owner))
			goto year_2;

		val = CalculateCompanyValue(p);
		if (val == 0) goto year_4;

		p->bankrupt_value = val;
		p->bankrupt_asked = 1 << owner;
		p->bankrupt_timeout = 0;
	} else if (p->quarters_of_bankrupcy == 4) {
year_4:
		DeletePlayerWindows(owner);

		if (IS_HUMAN_PLAYER(owner)) {
// what does this code do??
			InitNewsItemStructs();
			DeleteWindowById(WC_NEWS_WINDOW, 0);
		}

// Show bankrupt news
		SET_DPARAM16(0, p->name_1);
		SET_DPARAM32(1, p->name_2);
		AddNewsItem( (StringID)(owner + 16*3), NEWS_FLAGS(NM_CALLBACK, 0, NT_COMPANY_INFO, DNC_BANKRUPCY),0,0);

		if (IS_HUMAN_PLAYER(owner)) {
			p->bankrupt_asked = 255;
			p->bankrupt_timeout = 0x456;
		} else {
			p->money64 = p->player_money = 100000000;
			ChangeOwnershipOfPlayerItems(owner, 0xFF);
			p->is_active = false;
		}
	}
}

void DrawNewsBankrupcy(Window *w)
{
	Player *p;

	DrawNewsBorder(w);		

	p = DEREF_PLAYER(WP(w,news_d).ni->string_id & 15);
	DrawPlayerFace(p->face, p->player_color, 2, 23);
	GfxFillRect(3, 23, 3+91, 23+118, 0x4323);

	SET_DPARAM16(0, p->president_name_1);
	SET_DPARAM32(1, p->president_name_2);

	DrawStringMultiCenter(49, 148, STR_7058_PRESIDENT, 94);

	switch(WP(w,news_d).ni->string_id >> 4) {
	case 1:
		DrawStringCentered(w->width>>1, 1, STR_7056_TRANSPORT_COMPANY_IN_TROUBLE, 0);

		SET_DPARAM16(0, p->name_1);
		SET_DPARAM32(1, p->name_2);

		DrawStringMultiCenter(
			((w->width - 101) >> 1) + 98,
			90,
			STR_7057_WILL_BE_SOLD_OFF_OR_DECLARED,
			w->width - 101);
		break;

	case 2: {
		int32 price;

		DrawStringCentered(w->width>>1, 1, STR_7059_TRANSPORT_COMPANY_MERGER, 0);
		COPY_IN_DPARAM(0,WP(w,news_d).ni->params, 2);
		SET_DPARAM16(2, p->name_1);
		SET_DPARAM32(3, p->name_2);
		price = WP(w,news_d).ni->params[2];
		SET_DPARAM32(4, price);
		DrawStringMultiCenter(
			((w->width - 101) >> 1) + 98,
			90,
			price==0 ? STR_707F_HAS_BEEN_TAKEN_OVER_BY : STR_705A_HAS_BEEN_SOLD_TO_FOR,
			w->width - 101);
		break;
	}

	case 3:
		DrawStringCentered(w->width>>1, 1, STR_705C_BANKRUPT, 0);
		COPY_IN_DPARAM(0,WP(w,news_d).ni->params, 2);
		DrawStringMultiCenter(
			((w->width - 101) >> 1) + 98,
			90,
			STR_705D_HAS_BEEN_CLOSED_DOWN_BY,
			w->width - 101);
		break;		

	case 4:
		DrawStringCentered(w->width>>1, 1, STR_705E_NEW_TRANSPORT_COMPANY_LAUNCHED, 0);
		SET_DPARAM16(0, p->name_1);
		SET_DPARAM32(1, p->name_2);
		COPY_IN_DPARAM(2,WP(w,news_d).ni->params, 2);
		DrawStringMultiCenter(
			((w->width - 101) >> 1) + 98,
			90,
			STR_705F_STARTS_CONSTRUCTION_NEAR,
			w->width - 101);
		break;

	default:
		NOT_REACHED();
	}
}

StringID GetNewsStringBankrupcy(NewsItem *ni)
{
	Player *p = DEREF_PLAYER(ni->string_id & 0xF);

	switch(ni->string_id >> 4) {
	case 1:
		SET_DPARAM16(0, STR_7056_TRANSPORT_COMPANY_IN_TROUBLE);
		SET_DPARAM16(1, STR_7057_WILL_BE_SOLD_OFF_OR_DECLARED);
		SET_DPARAM16(2, p->name_1);
		SET_DPARAM32(3, p->name_2);
		return STR_02B6;
	case 2:
		SET_DPARAM16(0, STR_7059_TRANSPORT_COMPANY_MERGER);
		SET_DPARAM16(1, STR_705A_HAS_BEEN_SOLD_TO_FOR);
		COPY_IN_DPARAM(2,ni->params, 2);
		SET_DPARAM16(4, p->name_1);
		SET_DPARAM32(5, p->name_2);
		COPY_IN_DPARAM(6,ni->params + 2, 1);
		return STR_02B6;
	case 3:
		SET_DPARAM16(0, STR_705C_BANKRUPT);
		SET_DPARAM16(1, STR_705D_HAS_BEEN_CLOSED_DOWN_BY);
		COPY_IN_DPARAM(2,ni->params, 2);
		return STR_02B6;
	case 4:
		SET_DPARAM16(0, STR_705E_NEW_TRANSPORT_COMPANY_LAUNCHED);
		SET_DPARAM16(1, STR_705F_STARTS_CONSTRUCTION_NEAR);
		SET_DPARAM16(2, p->name_1);
		SET_DPARAM32(3, p->name_2);
		COPY_IN_DPARAM(4,ni->params, 2);
		return STR_02B6;
	default:
		NOT_REACHED();
	}
  
	/* useless, but avoids compiler warning this way */
	return 0;
}

static void PlayersGenStatistics()
{
	Station *st;
	Player *p;

	FOR_ALL_STATIONS(st) {
		if (st->xy != 0) {
			_current_player = st->owner;
			SET_EXPENSES_TYPE(EXPENSES_PROPERTY);
			SubtractMoneyFromPlayer(_price.station_value >> 1);
		}
	}

	if (!HASBIT(1<<0|1<<3|1<<6|1<<9, _cur_month))
		return;

	FOR_ALL_PLAYERS(p) {
		if (p->is_active) {
			memmove(&p->old_economy, &p->cur_economy, sizeof(p->old_economy));
			memset(&p->cur_economy, 0, sizeof(p->cur_economy));

			if (p->num_valid_stat_ent != 24)
				p->num_valid_stat_ent++;

			UpdateCompanyRatingAndValue(p);
			PlayersCheckBankrupt(p);

			if (p->block_preview != 0)
				p->block_preview--;
		}
	}

	InvalidateWindow(WC_INCOME_GRAPH, 0);
	InvalidateWindow(WC_OPERATING_PROFIT, 0);
	InvalidateWindow(WC_DELIVERED_CARGO, 0);
	InvalidateWindow(WC_PERFORMANCE_HISTORY, 0);
	InvalidateWindow(WC_COMPANY_VALUE, 0);
	InvalidateWindow(WC_COMPANY_LEAGUE, 0);
}

static void AddSingleInflation(int32 *value, uint16 *frac, int32 amt)
{
	int64 tmp;
	int32 low;
	tmp = BIGMULS(*value, amt);
	*frac = (uint16)(low = (uint16)tmp + *frac);
	*value += (int32)(tmp >> 16) + (low >> 16);
}

static void AddInflation()
{
	int i;
	int32 inf = _economy.infl_amount * 54;

	for(i=0; i!=NUM_PRICES; i++) {
		AddSingleInflation( (int32*)&_price + i, _price_frac + i, inf );
	}

	_economy.max_loan_unround += BIGMULUS(_economy.max_loan_unround, inf, 16);
	
	if (_economy.max_loan + 50000 <= _economy.max_loan_unround)
		_economy.max_loan += 50000;

	inf = _economy.infl_amount_pr * 54;
	for(i=0; i!=NUM_CARGO; i++) {
		AddSingleInflation( 
			_cargo_payment_rates + i,
			_cargo_payment_rates_frac + i,
			inf
		);
	}

	InvalidateWindowClasses(WC_BUILD_VEHICLE);
	InvalidateWindowClasses(WC_VEHICLE_DETAILS);
	InvalidateWindow(WC_PAYMENT_RATES, 0);
}

static void PlayersPayInterest()
{
	Player *p;
	int interest = _economy.interest_rate * 54;

	FOR_ALL_PLAYERS(p) {
		if (!p->is_active)
			continue;

		_current_player = p->index;
		SET_EXPENSES_TYPE(EXPENSES_LOAN_INT);
		
		SubtractMoneyFromPlayer(BIGMULUS(p->current_loan, interest, 16));

		SET_EXPENSES_TYPE(EXPENSES_OTHER);
		SubtractMoneyFromPlayer(_price.station_value >> 2);
	}
}

static void HandleEconomyFluctuations()
{
	if (_opt.diff.economy == 0)
		return;

	if (--_economy.fluct == 0) {
		_economy.fluct = - (int)(Random()&3);
		AddNewsItem(STR_7073_WORLD_RECESSION_FINANCIAL, NEWS_FLAGS(NM_NORMAL,0,NT_ECONOMY,0), 0, 0);
	} else if (_economy.fluct == -12) {
		_economy.fluct = (Random()&255) + 312;
		AddNewsItem(STR_7074_RECESSION_OVER_UPTURN_IN, NEWS_FLAGS(NM_NORMAL,0,NT_ECONOMY,0), 0, 0);
	}
}

static byte _price_category[NUM_PRICES] = {
	0, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 1, 1, 1, 1, 1, 1,
	2,
};

static const int32 _price_base[NUM_PRICES] = {
	100, 100, 95, 65, 275, 600, 500, 700,
	450, 200, 180, 600, 200, 200, 350, 400000,
	2000, 700000, 14000, 65000, 20, 250, 20, 40,
	200, 500, 20, -70, 10, 50, 80, 80,
	90, 30, 10000, 50, 30, 50, 50, 55,
	1600, 40, 5600, 5200, 4800, 9600, 1600, 5600,
	1000000,
};

void StartupEconomy()
{
	int i;

	assert(sizeof(_price) == NUM_PRICES * sizeof(int32));

	for(i=0; i!=NUM_PRICES; i++) {
		int32 price = _price_base[i];
		if (_price_category[i] != 0) {
			uint mod = _price_category[i] == 1 ? _opt.diff.vehicle_costs : _opt.diff.construction_cost;
			if (mod < 1) {
				price = price * 3 >> 2;
			} else if (mod > 1) {
				price = price * 9 >> 3;
			}
		}
		((int32*)&_price)[i] = price;
		_price_frac[i] = 0;
	}

	_economy.interest_rate = _opt.diff.initial_interest;
	_economy.infl_amount = _opt.diff.initial_interest;
	_economy.infl_amount_pr = max(0, _opt.diff.initial_interest - 1);
	_economy.max_loan_unround = _economy.max_loan = _opt.diff.max_loan * 1000;
	_economy.fluct = (byte)(Random()) + 168;
}

Pair SetupSubsidyDecodeParam(Subsidy *s, bool mode)
{
	int tile, tile2;
	Industry *i;
	Town *t;
	Station *st;
	Pair tp;

	/* if mode is false, convert into plural */
	SET_DPARAM16(0, _cargoc.names_s[s->cargo_type] + (mode?0:32));

	if (s->age < 12) {
		if (!(s->cargo_type == CT_PASSENGERS || s->cargo_type == CT_MAIL)) {
			SET_DPARAM16(1, STR_2029);
			i = DEREF_INDUSTRY(s->from);
			tile = i->xy;
			SET_DPARAM16(2, i->town->townnametype);
			SET_DPARAM32(3, i->town->townnameparts);

			SET_DPARAM16(4, i->type + STR_4802_COAL_MINE);

			if (s->cargo_type != CT_GOODS && s->cargo_type != CT_FOOD) {
				SET_DPARAM16(5, STR_2029);
				i = DEREF_INDUSTRY(s->to);
				tile2 = i->xy;
				SET_DPARAM16(8, i->type + STR_4802_COAL_MINE);
				SET_DPARAM16(6, i->town->townnametype);
				SET_DPARAM32(7, i->town->townnameparts);
			} else {
				t = DEREF_TOWN(s->to);
				tile2 = t->xy;
				SET_DPARAM16(5, t->townnametype);
				SET_DPARAM32(6, t->townnameparts);
			}
		} else {
			t = DEREF_TOWN(s->from);
			tile = t->xy;
			SET_DPARAM16(1, t->townnametype);
			SET_DPARAM32(2, t->townnameparts);

			t = DEREF_TOWN(s->to);
			tile2 = t->xy;
			SET_DPARAM16(3, t->townnametype);
			SET_DPARAM32(4, t->townnameparts);
		}
	} else {
		st = DEREF_STATION(s->from);
		tile = st->xy;
		SET_DPARAM16(1, st->index);

		st = DEREF_STATION(s->to);
		tile2 = st->xy;
		SET_DPARAM16(2, st->index);
	}

	tp.a = tile;
	tp.b = tile2;

	return tp;
}

void DeleteSubsidyWithIndustry(byte index)
{
	Subsidy *s;
	
	for(s=_subsidies; s != endof(_subsidies); s++) {
		if (s->cargo_type != 0xFF && s->age < 12 &&
				s->cargo_type != CT_PASSENGERS && s->cargo_type != CT_MAIL &&
				(index == s->from || (s->cargo_type!=CT_GOODS && s->cargo_type!=CT_FOOD && index==s->to))) {
			s->cargo_type = 0xFF;
		}
	}	
}

void DeleteSubsidyWithStation(byte index) 
{
	Subsidy *s;
	bool dirty = false;
	
	for(s=_subsidies; s != endof(_subsidies); s++) {
		if (s->cargo_type != 0xFF && s->age >= 12 &&
				(s->from == index || s->to == index)) {
			s->cargo_type = 0xFF;
			dirty = true;
		}
	}	

	if (dirty)
		InvalidateWindow(WC_SUBSIDIES_LIST, 0);
}

typedef struct FoundRoute {
	uint distance;
	byte cargo;
	void *from;
	void *to;
} FoundRoute;

static void FindSubsidyPassengerRoute(FoundRoute *fr)
{
	Town *from,*to;

	fr->distance = (uint)-1;

	fr->from = from = DEREF_TOWN(RandomRange(_total_towns));
	if (from->xy == 0 || from->population < 400)
		return;

	fr->to = to = DEREF_TOWN(RandomRange(_total_towns));
	if (from==to || to->xy == 0 || to->population < 400 || to->pct_pass_transported > 42)
		return;

	fr->distance = GetTileDist(from->xy, to->xy);
}

static void FindSubsidyCargoRoute(FoundRoute *fr)
{
	Industry *i;
	int trans, total;
	byte cargo;

	fr->distance = (uint)-1;

	fr->from = i = DEREF_INDUSTRY(RandomRange(_total_industries));
	if (i->xy == 0)
		return;

	// Randomize cargo type
	if (Random()&1 && i->produced_cargo[1] != 0xFF) {
		cargo = i->produced_cargo[1];
		trans = i->pct_transported[1];
		total = i->total_production[1];
	} else {
		cargo = i->produced_cargo[0];
		trans = i->pct_transported[0];
		total = i->total_production[0];
	}

	// Quit if no production in this industry
	//  or if the cargo type is passengers
	//  or if the pct transported is already large enough
	if (total == 0  || trans > 42 || cargo == 0xFF || cargo == CT_PASSENGERS)
		return;

	fr->cargo = cargo;

	if (cargo == CT_GOODS || cargo == CT_FOOD) {
		// The destination is a town
		Town *t = DEREF_TOWN(RandomRange(_total_towns));
		
		// Only want big towns
		if (t->xy == 0 || t->population < 900)
			return;
		fr->distance = GetTileDist(i->xy, t->xy);
		fr->to = t;
	} else {
		// The destination is an industry
		Industry *i2 = DEREF_INDUSTRY(RandomRange(_total_industries));
		
		// The industry must accept the cargo
		if (i == i2 || i2->xy == 0 || 
				(cargo != i2->accepts_cargo[0] &&
				cargo != i2->accepts_cargo[1] &&
				cargo != i2->accepts_cargo[2]))
			return;
		fr->distance = GetTileDist(i->xy, i2->xy);
		fr->to = i2;
	}
}

static bool CheckSubsidyDuplicate(Subsidy *s) 
{
	Subsidy *ss;

	for(ss=_subsidies; ss != endof(_subsidies); ss++) {
		if (s != ss && 
//				ss->age < 12 && 
				ss->from == s->from && 
				ss->to == s->to && 
				ss->cargo_type == s->cargo_type) {
			s->cargo_type = 0xFF;
			return true;
		}
	}
	return false;
}

static void SubsidyMonthlyHandler()
{
	Subsidy *s;
	Pair pair;
	Station *st;
	uint n;
	FoundRoute fr;
	bool modified = false;

	for(s=_subsidies; s != endof(_subsidies); s++) {
		if (s->cargo_type == 0xFF)
			continue;

		if (s->age == 12-1) {
			pair = SetupSubsidyDecodeParam(s, 1);
			AddNewsItem(STR_202E_OFFER_OF_SUBSIDY_EXPIRED, NEWS_FLAGS(NM_NORMAL, NF_TILE, NT_SUBSIDIES, 0), pair.a, pair.b);
			s->cargo_type = 0xFF;
			modified = true;
		} else if (s->age == 2*12-1) {
			st = DEREF_STATION(s->to);
			if (st->owner == _local_player) {
				pair = SetupSubsidyDecodeParam(s, 1);
				AddNewsItem(STR_202F_SUBSIDY_WITHDRAWN_SERVICE, NEWS_FLAGS(NM_NORMAL, NF_TILE, NT_SUBSIDIES, 0), pair.a, pair.b);
			}
			s->cargo_type = 0xFF;
			modified = true;
		} else {
			s->age++;
		}
	}

	// 25% chance to go on
	if (CHANCE16(1,4)) {
		// Find a free slot
		s = _subsidies;
		while (s->cargo_type != 0xFF) {
			if (++s == endof(_subsidies))
				goto no_add;
		}
		
		n = 1000;
		do {
			FindSubsidyPassengerRoute(&fr);
			if (fr.distance <= 70) {
				s->cargo_type = CT_PASSENGERS;
				s->from = ((Town*)fr.from)->index;
				s->to = ((Town*)fr.to)->index;
				goto add_subsidy;
			}
			FindSubsidyCargoRoute(&fr);
			if (fr.distance <= 70) {
				s->cargo_type = fr.cargo;
				s->from = (Industry*)fr.from - _industries;
				s->to = (fr.cargo == CT_GOODS || fr.cargo == CT_FOOD) ? ((Town*)fr.to)->index : (Industry*)fr.to - _industries;
	add_subsidy:
				if (!CheckSubsidyDuplicate(s)) {
					s->age = 0;
					pair = SetupSubsidyDecodeParam(s, 0);
					AddNewsItem(STR_2030_SERVICE_SUBSIDY_OFFERED, NEWS_FLAGS(NM_NORMAL, NF_TILE, NT_SUBSIDIES, 0), pair.a, pair.b);
					modified = true;
					break;
				}
			}
		} while (--n);
	}
no_add:;
	if (modified)
		InvalidateWindow(WC_SUBSIDIES_LIST, 0);
}

static const byte _subsidies_desc[] = {
	SLE_VAR(Subsidy,cargo_type,		SLE_UINT8),
	SLE_VAR(Subsidy,age,					SLE_UINT8),
	SLE_VAR(Subsidy,from,					SLE_UINT8),
	SLE_VAR(Subsidy,to,						SLE_UINT8),
	SLE_END()
};

static void Save_SUBS()
{
	int i;
	Subsidy *s;

	for(i=0; i!=lengthof(_subsidies); i++) {
		s = &_subsidies[i];
		if (s->cargo_type != 0xFF) {
			SlSetArrayIndex(i);
			SlObject(s, _subsidies_desc);
		}
	}
}

static void Load_SUBS()
{
	int index;
	while ((index = SlIterateArray()) != -1)
		SlObject(&_subsidies[index], _subsidies_desc);
}

int32 GetTransportedGoodsIncome(uint num_pieces, uint dist, byte transit_days, byte cargo_type)
{
	int cargo = cargo_type;
	byte f;

	/* zero the distance if it's the bank and very short transport. */
	if (_opt.landscape == LT_NORMAL && cargo == CT_VALUABLES && dist < 10)
		dist = 0;

	f = 255;
	if (transit_days > _cargoc.transit_days_1[cargo]) {
		transit_days -= _cargoc.transit_days_1[cargo];
		f -= transit_days;
		
		if (transit_days > _cargoc.transit_days_2[cargo]) {
			transit_days -= _cargoc.transit_days_2[cargo];

			if (f < transit_days)
				f = 0;
			else
				f -= transit_days;
		}
	}
	if (f < 31) f = 31;

	return BIGMULSS(dist * f * num_pieces, _cargo_payment_rates[cargo], 21);
}

static void DeliverGoodsToIndustry(TileIndex xy, byte cargo_type, int num_pieces)
{
	Industry *ind, *best;	
	int t, u;

	/* Check if there's an industry close to the station that accepts
	 * the cargo */
	best = NULL;
	u = 0x11;
	for(ind = _industries; ind != endof(_industries); ind++) {
		if (ind->xy != 0 && (cargo_type == ind->accepts_cargo[0] || cargo_type
				 == ind->accepts_cargo[1] || cargo_type == ind->accepts_cargo[2]) &&
				 ind->produced_cargo[0] != 0xFF &&
				 ind->produced_cargo[0] != cargo_type &&
				 (t=GetTileDist(ind->xy, xy)) < u) {			
			u = t;
			best = ind;
		}
	}

	/* Found one? */
	if (best != NULL) {
		best->was_cargo_delivered = true;
		best->cargo_waiting[0] = min(best->cargo_waiting[0] + num_pieces, 0xFFFF);
	}
}

static bool CheckSubsidised(Station *from, Station *to, byte cargo_type)
{
	Subsidy *s;
	TileIndex xy;
	Pair pair;
	Player *p;

	// check if there is an already existing subsidy that applies to us
	for(s=_subsidies; s != endof(_subsidies); s++) {
		if (s->cargo_type == cargo_type &&
				s->age >= 12 &&
				s->from == from->index &&
				s->to == to->index)
			return true;
	}

	/* check if there's a new subsidy that applies.. */
	for(s=_subsidies; s != endof(_subsidies); s++) {
		if (s->cargo_type == cargo_type && s->age < 12) {
		
			/* Check distance from source */
			if (cargo_type == CT_PASSENGERS || cargo_type == CT_MAIL) {
				xy = DEREF_TOWN(s->from)->xy;
			} else {
				xy = (DEREF_INDUSTRY(s->from))->xy;
			}
			if (GetTileDist1D(xy, from->xy) > 9)
				continue;
			
			/* Check distance from dest */
			if (cargo_type == CT_PASSENGERS || cargo_type == CT_MAIL || cargo_type == CT_GOODS || cargo_type == CT_FOOD) {
				xy = DEREF_TOWN(s->to)->xy;
			} else {
				xy = (DEREF_INDUSTRY(s->to))->xy;
			}

			if (GetTileDist1D(xy, to->xy) > 9)
				continue;

			/* Found a subsidy, change the values to indicate that it's in use */
			s->age = 12;
			s->from = from->index;
			s->to = to->index;

			/* Add a news item */
			pair = SetupSubsidyDecodeParam(s, 0);
			INJECT_DPARAM(2);

			p = DEREF_PLAYER(_current_player);
			SET_DPARAM16(0, p->name_1);
			SET_DPARAM32(1, p->name_2);
			AddNewsItem(
				STR_2031_SERVICE_SUBSIDY_AWARDED + _opt.diff.subsidy_multiplier, 
				NEWS_FLAGS(NM_NORMAL, NF_TILE, NT_SUBSIDIES, 0),
				pair.a, pair.b);

			InvalidateWindow(WC_SUBSIDIES_LIST, 0);
			return true;
		}
	}
	return false;
}

static int32 DeliverGoods(int num_pieces, byte cargo_type, byte source, byte dest, byte days_in_transit)
{
	bool subsidised;
	Station *s_from, *s_to;
	int32 profit;

 	assert(num_pieces > 0);

	// Update player statistics
	{
		Player *p = DEREF_PLAYER(_current_player);
		p->cur_economy.delivered_cargo += num_pieces;
		SETBIT(p->cargo_types, cargo_type);
	}

	// Get station pointers.
	s_from = DEREF_STATION(source);
	s_to = DEREF_STATION(dest);

	// Check if a subsidy applies.
	subsidised = CheckSubsidised(s_from, s_to, cargo_type);

	// Increase town's counter for some special goods types
	{
		Town *t = s_to->town;
		if (cargo_type == CT_FOOD) t->new_act_food += num_pieces;
		if (cargo_type == CT_STEEL)  t->new_act_paper += num_pieces;
	}

	// Give the goods to the industry.
	DeliverGoodsToIndustry(s_to->xy, cargo_type, num_pieces);
	
	// Determine profit
	{
		int t = GetTileDist(s_from->xy, s_to->xy);
		int r = num_pieces;
		profit = 0;
		do {
			int u = min(r, 255);
			r -= u;
			profit += GetTransportedGoodsIncome(u, t, days_in_transit, cargo_type);
		} while (r != 0);
	}

	// Modify profit if a subsidy is in effect
	if (subsidised) {
		if (_opt.diff.subsidy_multiplier < 1) {
			/* 1.5x */
			profit += profit >> 1;
		} else if (_opt.diff.subsidy_multiplier == 1) {
			/* 2x */
			profit *= 2;
		} else if (_opt.diff.subsidy_multiplier == 2) {
			/* 3x */
			profit *= 3;
		} else {
			/* 4x */
			profit *= 4;
		}
	}

	// Computers get 25% extra profit if they're intelligent.
	if (_opt.diff.competitor_intelligence>=1 && !IS_HUMAN_PLAYER(_current_player))
		profit += profit >> 2;

	return profit;
}

int LoadUnloadVehicle(Vehicle *v)
{
	int profit = 0;
	int unloading_time = 20;
	Vehicle *u = v;
	int result = 0;
	int last_visited;
	Station *st;
	GoodsEntry *ge;
	int t;
	uint count, cap;

	assert((v->next_order&0x1F) == OT_LOADING);

	v->cur_speed = 0;
	_current_player = v->owner;

	st = DEREF_STATION(last_visited = v->last_station_visited);

	for(;;) {
		if (v->cargo_cap == 0)
			goto next_vehicle;
		
		ge = &st->goods[v->cargo_type];

		/* unload? */
		if (v->cargo_count != 0) {
			if (v->cargo_source != (byte)last_visited && ge->waiting_acceptance & 0x8000) {
				// deliver goods to the station
				unloading_time += v->cargo_count; /* TTDBUG: bug in original TTD */
				profit += DeliverGoods(v->cargo_count, v->cargo_type, v->cargo_source, last_visited, v->cargo_days);
				result |= 1;
				v->cargo_count = 0;
			} else if (u->next_order & OF_UNLOAD) {
				/* unload goods and let it wait at the station */
				st->time_since_unload = 0;
				
				if ((t=ge->waiting_acceptance & 0xFFF) == 0) {
					// No goods waiting at station
					ge->enroute_time = v->cargo_days;
					ge->enroute_from = v->cargo_source;
				} else {
					// Goods already waiting at station. Set counters to the worst value.
					if (v->cargo_days >= ge->enroute_time)
						ge->enroute_time = v->cargo_days;
					if ((byte)last_visited != ge->enroute_from)
						ge->enroute_from = v->cargo_source;
				}
				// Update amount of waiting cargo
				ge->waiting_acceptance = (ge->waiting_acceptance &~0xFFF) | min(v->cargo_count + t, 0xFFF);
				result |= 2;
				v->cargo_count = 0;
			}
		}
	
		/* don't pick up goods that we unloaded */
		if (u->next_order & OF_UNLOAD)
			goto next_vehicle;

		/* update stats */
		ge->days_since_pickup = 0;
		t = u->max_speed;
		if (u->type == VEH_Road) t >>=1;
		
		// if last speed is 0, we treat that as if no vehicle has ever visited the station.
		ge->last_speed = t < 255 ? t : 255;
		ge->last_age = _cur_year - v->build_year;

		// If there's goods waiting at the station, and the vehicle
		//  has capacity for it, load it on the vehicle.
		if ((count=ge->waiting_acceptance & 0xFFF) != 0 &&
				(cap = v->cargo_cap - v->cargo_count) != 0) {
			if (cap > count) cap = count;
			v->cargo_count += cap;
			ge->waiting_acceptance -= cap;
			unloading_time += cap;
			st->time_since_load = 0;
			
			// And record the source of the cargo, and the days in travel.
			v->cargo_source = ge->enroute_from;
			v->cargo_days = ge->enroute_time;
			result |= 2;
			st->last_vehicle = v->index;
		}

next_vehicle:;
		if (!(v = v->next))
			break;
	}

	v = u;

	if (v->type == VEH_Train) {
		int num = - (int)GetStationPlatforms(st, v->tile) * 2;
		do num++; while ( (v=v->next) != NULL);
		if (num > 0) {
			unloading_time <<=1;
			unloading_time += num * unloading_time;
		}
		v = u;
	}

	v->load_unload_time_rem = unloading_time;

	if (result != 0) {
		InvalidateWindow(WC_VEHICLE_DETAILS, v->index);

		if (result & 2)
			InvalidateWindow(WC_STATION_VIEW, last_visited);

		if (profit != 0) {
			v->profit_this_year += profit;
			SubtractMoneyFromPlayer(-profit);

			if (_current_player == _local_player)
				SndPlayVehicleFx(0x12, v);

			ShowCostOrIncomeAnimation(v->x_pos, v->y_pos, v->z_pos, -profit);
		}
	}

	return result;
}

void PlayersMonthlyLoop()
{
	PlayersGenStatistics();
	if (_patches.inflation)
		AddInflation();
	PlayersPayInterest();
	HandleEconomyFluctuations();
	SubsidyMonthlyHandler();
}

static void DoAcquireCompany(Player *p)
{
	Player *owner;
	int i,pi;
	int32 value;

	SET_DPARAM16(0, p->name_1);
	SET_DPARAM32(1, p->name_2);
	SET_DPARAM32(2, p->bankrupt_value);
	AddNewsItem( (StringID)(_current_player + 16*2), NEWS_FLAGS(NM_CALLBACK, 0, NT_COMPANY_INFO, DNC_BANKRUPCY),0,0);

	// original code does this a little bit differently
	pi = p->index;
	ChangeOwnershipOfPlayerItems(pi, _current_player);

	if (p->bankrupt_value == 0) {
		owner = DEREF_PLAYER(_current_player);
		owner->current_loan += p->current_loan;
	}

	value = CalculateCompanyValue(p) >> 2;
	for(i=0; i!=4; i++) {
		if (p->share_owners[i] != 0xFF) {
			owner = DEREF_PLAYER(p->share_owners[i]);
			owner->money64 += value;
			owner->yearly_expenses[0][EXPENSES_OTHER] += value;
			UpdatePlayerMoney32(owner);
		}
	}

	p->is_active = false;

	DeletePlayerWindows(pi);
}

int32 CmdBuyShareInCompany(int x, int y, uint32 flags, uint32 p1, uint32 p2)
{
	Player *p;
	int32 cost;
	byte *b;
	int i;

	SET_EXPENSES_TYPE(EXPENSES_OTHER);
	p = DEREF_PLAYER(p1);

	cost = CalculateCompanyValue(p) >> 2;
	if (flags & DC_EXEC) {
		b = p->share_owners;
		while (*b != 0xFF) b++; /* share owners is guaranteed to contain at least one 0xFF */
		*b = _current_player;

		for(i=0;p->share_owners[i] == _current_player;) {
			if (++i == 4) {
				p->bankrupt_value = 0;
				DoAcquireCompany(p);
				break;
			}
		}
		InvalidateWindow(WC_COMPANY, (int)p1);
	}
	return cost;
}

int32 CmdSellShareInCompany(int x, int y, uint32 flags, uint32 p1, uint32 p2)
{
	Player *p;
	int32 cost;
	byte *b;

	SET_EXPENSES_TYPE(EXPENSES_OTHER);
	p = DEREF_PLAYER(p1);

	/* adjust it a little to make it less profitable to sell and buy */
	cost = CalculateCompanyValue(p) >> 2;
	cost = -(cost - (cost >> 7));

	if (flags & DC_EXEC) {
		b = p->share_owners;
		while (*b != _current_player) b++; /* share owners is guaranteed to contain player */
		*b = 0xFF;
		InvalidateWindow(WC_COMPANY, (int)p1);
	}
	return cost;
}

int32 CmdBuyCompany(int x, int y, uint32 flags, uint32 p1, uint32 p2)
{
	Player *p;
	SET_EXPENSES_TYPE(EXPENSES_OTHER);
	p = DEREF_PLAYER(p1);
	if (flags & 1) {
		DoAcquireCompany(p);
	}
	return p->bankrupt_value;
}

// Prices
static void SaveLoad_PRIC()
{
	SlArray(&_price, NUM_PRICES, SLE_INT32);
	SlArray(&_price_frac, NUM_PRICES, SLE_UINT16);
}

// Cargo payment rates
static void SaveLoad_CAPR()
{
	SlArray(&_cargo_payment_rates, NUM_CARGO, SLE_INT32);
	SlArray(&_cargo_payment_rates_frac, NUM_CARGO, SLE_UINT16);
}

static const byte _economy_desc[] = {
	SLE_VAR(Economy,max_loan,						SLE_INT32),
	SLE_VAR(Economy,max_loan_unround,		SLE_INT32),
	SLE_VAR(Economy,fluct,							SLE_FILE_I16 | SLE_VAR_I32),
	SLE_VAR(Economy,interest_rate,			SLE_UINT8),
	SLE_VAR(Economy,infl_amount,				SLE_UINT8),
	SLE_VAR(Economy,infl_amount_pr,			SLE_UINT8),
	SLE_END()
};

// Economy variables
static void SaveLoad_ECMY()
{
	SlObject(&_economy, &_economy_desc);
}

const ChunkHandler _economy_chunk_handlers[] = {
	{ 'PRIC', SaveLoad_PRIC, SaveLoad_PRIC, CH_RIFF | CH_AUTO_LENGTH},
	{ 'CAPR', SaveLoad_CAPR, SaveLoad_CAPR, CH_RIFF | CH_AUTO_LENGTH},
	{ 'SUBS', Save_SUBS,			Load_SUBS, CH_ARRAY},
	{ 'ECMY', SaveLoad_ECMY, SaveLoad_ECMY, CH_RIFF | CH_LAST},
};




