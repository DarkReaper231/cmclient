#include "stdafx.h"
#include "ttd.h"
#include "player.h"
#include "town.h"
#include "vehicle.h"
#include "station.h"
#include "gfx.h"
#include "news.h"
#include "saveload.h"
#include "command.h"

extern void StartupEconomy();

static const SpriteID cheeks_table[4] = {
	0x325, 0x326, 
	0x390, 0x3B0,
};

static const SpriteID mouth_table[3] = {
	0x34C, 0x34D, 0x34F
};

void DrawPlayerFace(uint32 face, int color, int x, int y)
{
	byte flag = 0;

	if ( (int32)face < 0)
		flag |= 1;
	if ((((((face >> 7) ^ face) >> 7) ^ face) & 0x8080000) == 0x8000000)
		flag |= 2;

	/* draw the gradient */
	DrawSprite( (color<<16) + 0x0307836A, x, y);

	/* draw the cheeks */
	DrawSprite(cheeks_table[flag&3], x, y);

	/* draw the chin */
	/* FIXME: real code uses -2 in zoomlevel 1 */
	{
		uint val = (face >> 4) & 3;
		if (!(flag & 2)) {
			DrawSprite(0x327 + (flag&1?0:val), x, y);
		} else {
			DrawSprite((flag&1?0x3B1:0x391) + (val>>1), x, y);
		}
	}
	/* draw the eyes */
	{
		uint val1 = (face >> 6)&15;
		uint val2 = (face >> 20)&7;
		uint32 high = 0x314<<16;

		if (val2 >= 6) {
			high = 0x30F<<16;
			if (val2 != 6)
				high = 0x30D<<16;
		}

		if (!(flag & 2)) {
			if (!(flag & 1)) {
				DrawSprite(high+((val1 * 12 >> 4) + 0x832B), x, y); 			
			} else {
				DrawSprite(high+(val1 + 0x8337), x, y);
			}	
		} else {
			if (!(flag & 1)) {
				DrawSprite(high+((val1 * 11 >> 4) + 0x839A), x, y);
			} else {
				DrawSprite(high+(val1 + 0x83B8), x, y);
			}
		}
	}

	/* draw the mouth */
	{
		uint val = (face >> 10) & 63;
		uint val2;

		if (!(flag&1)) {
			val2 = ((val&0xF) * 15 >> 4);
			
			if (val2 < 3) {
				DrawSprite((flag&2 ? 0x397 : 0x367) + val2, x, y);
				/* skip the rest */
				goto skip_mouth;				
			}

			val2 -= 3;
			if (flag & 2) {
				if (val2 > 8) val2 = 0;
				val2 += 0x3A5 - 0x35B;
			}
			DrawSprite(val2 + 0x35B, x, y);
		} else if (!(flag&2)) {
			DrawSprite(((val&0xF) * 10 >> 4) + 0x351, x, y);
		} else {
			DrawSprite(((val&0xF) * 9 >> 4) + 0x3C8, x, y);
		}

		val >>= 3;

		if (!(flag&2)) {
			if (!(flag&1)) {
				DrawSprite(0x349 + val, x, y);
			} else {
				DrawSprite( mouth_table[(val*3>>3)], x, y);
			}
		} else {
			if (!(flag&1)) {
				DrawSprite(0x393 + (val&3), x, y);
			} else {
				DrawSprite(0x3B3 + (val*5>>3), x, y);
			}
		}

		skip_mouth:;
	}


	/* draw the hair */
	{
		uint val = (face >> 16) & 15;
		if (!(flag&2)) {
			if (!(flag&1)) {
				DrawSprite(0x382 + (val*9>>4), x, y);
			} else {
				DrawSprite(0x38B + (val*5>>4), x, y);
			}
		} else {
			if (!(flag&1)) {
				DrawSprite(0x3D4 + (val*5>>4), x, y);
			} else {
				DrawSprite(0x3D9 + (val*5>>4), x, y);
			}
		}
	}

	/* draw the tie */
	{
		uint val = (face >> 20) & 0xFF;

		if (!(flag&1)) {
			DrawSprite(0x36B + ((val&3)*3>>2), x, y);
			DrawSprite(0x36E + ((val>>2)&3), x, y);
			DrawSprite(0x372 + ((val>>4)*6>>4), x, y);
		} else {
			DrawSprite(0x378 + ((val&3)*3>>2), x, y);
			DrawSprite(0x37B + ((val>>2)&3), x, y);
			
			val >>= 4;
			if (val < 3) {
				DrawSprite((flag&2 ? 0x3D1 : 0x37F) + val, x, y);
			}
		}
	}

	/* draw the glasses */
	{
		uint val = (face >> 28) & 7;

		if (!(flag&2)) {
			if (val<=1) {
				DrawSprite(0x347 + val, x, y);
			}
		} else {
			if (val<=1) {
				DrawSprite(0x3AE + val, x, y);
			}
		}
	}
}

void InvalidatePlayerWindows(Player *p)
{
	uint pid = p->index;
	if ( (byte)pid == _local_player)
		InvalidateWindow(WC_STATUS_BAR, 0);

	InvalidateWindow(WC_FINANCES, pid);
}

bool CheckPlayerHasMoney(int32 cost)
{
	if (cost > 0) {
		uint pid = _current_player;
		if (pid < MAX_PLAYERS && cost > DEREF_PLAYER(pid)->player_money) {
			SET_DPARAM32(0, cost);
			_error_message = STR_0003_NOT_ENOUGH_CASH_REQUIRES;
			return false;
		}
	}
	return true;
}

static void SubtractMoneyFromAnyPlayer(Player *p, int32 cost)
{
	p->money64 -= cost;
	UpdatePlayerMoney32(p);
	
	p->yearly_expenses[0][_yearly_expenses_type] += cost;

	if ( ( 1 << _yearly_expenses_type ) & (1<<7|1<<8|1<<9|1<<10))
		p->cur_economy.income -= cost;
	else if (( 1 << _yearly_expenses_type ) & (1<<2|1<<3|1<<4|1<<5|1<<6|1<<11))
		p->cur_economy.expenses -= cost;
		
	InvalidatePlayerWindows(p);
}

void SubtractMoneyFromPlayer(int32 cost)
{
	uint pid = _current_player;
	if (pid < MAX_PLAYERS)
		SubtractMoneyFromAnyPlayer(DEREF_PLAYER(pid), cost);
}

void SubtractMoneyFromPlayerFract(byte player, int32 cost)
{
	Player *p = DEREF_PLAYER(player);
	byte m = p->player_money_fraction;
	p->player_money_fraction = m - (byte)cost;
	cost >>= 8;
	if (p->player_money_fraction > m)
		cost++;
	if (cost != 0)
		SubtractMoneyFromAnyPlayer(p, cost);
}

// the player_money field is kept as it is, but money64 contains the actual amount of money.
void UpdatePlayerMoney32(Player *p)
{
	if (p->money64 < -2000000000)
		p->player_money = -2000000000;
	else if (p->money64 > 2000000000)
		p->player_money = 2000000000;
	else
		p->player_money = (int32)p->money64; 
}

void GetNameOfOwner(byte owner, uint tile)
{
	SET_DPARAM8(2, owner);
		
	if (owner != OWNER_TOWN) {
		if (owner >= 8)
			SET_DPARAM16(0, STR_0150_SOMEONE);
		else {
			Player *p = DEREF_PLAYER(owner);
			SET_DPARAM16(0, p->name_1);
			SET_DPARAM32(1, p->name_2);
		}
	} else {
		Town *t = ClosestTownFromTile(tile, (uint)-1);
		SET_DPARAM16(0, t->townnametype);
		SET_DPARAM32(1, t->townnameparts);
	}
}


bool CheckOwnership(byte owner)
{
	assert(owner <= OWNER_WATER);

	if (owner == _current_player)
		return true;
	_error_message = STR_013B_OWNED_BY;
	GetNameOfOwner(owner, 0);
	return false;
}

bool CheckTileOwnership(uint tile)
{
	byte owner = _map_owner[tile];
	assert(owner <= OWNER_WATER);
	if (owner == _current_player) 
		return true;
	_error_message = STR_013B_OWNED_BY;
	
	// no need to get the name of the owner unless we're the local player (saves some time)
	if (_current_player == _local_player)
		GetNameOfOwner(owner, tile);
	return false;
}

static void GenerateCompanyName(Player *p)
{
	uint tile;
	Town *t;
	StringID str;
	Player *pp;
	uint32 strp;
	char buffer[100];

	if (p->name_1 != STR_SV_UNNAMED)
		return;

	tile = p->last_build_coordinate;
	if (tile == 0)
		return;

	t = ClosestTownFromTile(tile, (uint)-1);

	if (IS_INT_INSIDE(t->townnametype, SPECSTR_TOWNNAME_START, SPECSTR_TOWNNAME_LAST+1)) {
		str = t->townnametype - SPECSTR_TOWNNAME_START + SPECSTR_PLAYERNAME_START;
		strp = t->townnameparts;

verify_name:;
		// No player must have this name already
		FOR_ALL_PLAYERS(pp) {
			if (pp->name_1 == str && pp->name_2 == strp)
				goto bad_town_name;
		}

		GetString(buffer, str);
		if (strlen(buffer) >= 32 || GetStringWidth(buffer) >= 150)
			goto bad_town_name;

set_name:;
		p->name_1 = str;
		p->name_2 = strp;

		MarkWholeScreenDirty();

		if (!IS_HUMAN_PLAYER(p->index)) {
			SET_DPARAM16(0, t->index);
			AddNewsItem(p->index + (4 << 4), NEWS_FLAGS(NM_CALLBACK, NF_TILE, NT_COMPANY_INFO, DNC_BANKRUPCY), p->last_build_coordinate, 0);
		}
		return;
	}
bad_town_name:;
	
	if (p->president_name_1 == SPECSTR_PRESIDENT_NAME) {
		str = SPECSTR_ANDCO_NAME;
		strp = p->president_name_2;
		goto set_name;	
	} else {
		str = SPECSTR_ANDCO_NAME;
		strp = Random();
		goto verify_name;
	}
}

#define COLOR_SWAP(i,j) do { byte t=colors[i];colors[i]=colors[j];colors[j]=t; } while(0)

static const byte _color_sort[16] = {2, 2, 3, 2, 3, 2, 3, 2, 3, 2, 2, 2, 3, 1, 1, 1};
static const byte _color_similar_1[16] = {8, 6, 255, 12,  255, 0, 1, 1, 0, 13,  11,  10, 3,   9,  15, 14};
static const byte _color_similar_2[16] = {5, 7, 255, 255, 255, 8, 7, 6, 5, 12, 255, 255, 9, 255, 255, 255};

static byte GeneratePlayerColor()
{
	byte colors[16], pcolor, t2;
	int i,j,n;
	uint32 r;
	Player *p;

	// Initialize array
	for(i=0; i!=16; i++)
		colors[i] = i;

	// And randomize it
	n = 100;
	do {
		r = Random();
		COLOR_SWAP(r & 0xF, (r >> 4) & 0xF);
	} while (--n);

	// Bubble sort it according to the values in table 1
	i = 16;
	do {
		for(j=0; j!=15; j++) {
			if (_color_sort[colors[j]] < _color_sort[colors[j+1]]) {
				COLOR_SWAP(j,j+1);	
			}
		}
	} while (--i);

	// Move the colors that look similar to each player's color to the side
	FOR_ALL_PLAYERS(p) if (p->is_active) {
		pcolor = p->player_color;
		for(i=0; i!=16; i++) if (colors[i] == pcolor) {
			colors[i] = 0xFF;

			t2 = _color_similar_1[pcolor]; 
			if (t2 == 0xFF) break;
			for(i=0; i!=15; i++) {
				if (colors[i] == t2) {
					do COLOR_SWAP(i,i+1); while (++i != 15);
					break;
				}
			}

			t2 = _color_similar_2[pcolor]; 
			if (t2 == 0xFF) break;
			for(i=0; i!=15; i++) {
				if (colors[i] == t2) {
					do COLOR_SWAP(i,i+1); while (++i != 15);
					break;
				}
			}
			break;
		}
	}

	// Return the first available color
	i = 0;
	for(;;) {
		if (colors[i] != 0xFF)
			return colors[i];
		i++;
	}
}

static void GeneratePresidentName(Player *p)
{
	Player *pp;
	char buffer[100], buffer2[40];

	for(;;) {
restart:;

		p->president_name_2 = Random();
		p->president_name_1 = SPECSTR_PRESIDENT_NAME;
		
		SET_DPARAM32(0, p->president_name_2);
		GetString(buffer, p->president_name_1);
		if (strlen(buffer) >= 32 || GetStringWidth(buffer) >= 94)
			continue;

		FOR_ALL_PLAYERS(pp) {
			if (pp->is_active && p != pp) {
				SET_DPARAM32(0, pp->president_name_2);
				GetString(buffer2, pp->president_name_1);
				if (str_eq(buffer2, buffer))
					goto restart;
			}
		}
		return;
	}
}

extern int GetPlayerMaxRailtype(int p);

static Player *AllocatePlayer()
{
	Player *p;
	// Find a free slot
	FOR_ALL_PLAYERS(p) {
		if (!p->is_active) {
			int i = p->index;
			memset(p, 0, sizeof(Player));
			p->index = i;
			return p;
		}
	}
	return NULL;
}

Player *DoStartupNewPlayer(bool is_ai)
{
	Player *p;
	int index;

	p = AllocatePlayer();
	if (p == NULL) return NULL;
		
	index = p->index;

	// Make a color
	p->player_color = GeneratePlayerColor();
	_player_colors[index] = p->player_color;
	p->name_1 = STR_SV_UNNAMED;
	p->is_active = true;
	
	p->money64 = p->player_money = p->current_loan = 100000;

	p->is_ai = is_ai;
	p->ai.state = 5; /* AIS_WANT_NEW_ROUTE */
	p->share_owners[0] = p->share_owners[1] = p->share_owners[2] = p->share_owners[3] = 0xFF;

	p->max_railtype = GetPlayerMaxRailtype(index);
	p->inaugurated_year = _cur_year;
	p->face = Random();

	GeneratePresidentName(p);

	InvalidateWindow(WC_GRAPH_LEGEND, 0);
	InvalidateWindow(WC_TOOLBAR_MENU, 0);

	return p;
}

void StartupPlayers()
{
	uint32 r;
	
	r = Random();
	_next_competitor_start = (byte)r + _opt.diff.competitor_start_time * 6882 + 1;
}

static void MaybeStartNewPlayer()
{
	uint n;
	Player *p;
	
	// count number of competitors
	n = 0;
	for(p=_players; p!=endof(_players); p++)
		if (p->is_active && p->is_ai)
			n++;

	// when there's a lot of computers in game, the probability that a new one starts is lower
	if (n < (uint)_opt.diff.max_no_competitors && n < RandomRange(_opt.diff.max_no_competitors))
		DoStartupNewPlayer(true);
	
	_next_competitor_start = RandomRange(19980) + 11100;
}

void InitializePlayers()
{
	int i;
	memset(_players, 0, sizeof(_players));
	for(i = 0; i != MAX_PLAYERS; i++)
		_players[i].index=i;
	_cur_player_tick_index = 0;
}

void OnTick_Players()
{
	Player *p;

	if (_game_mode == GM_EDITOR)
		return;
	
	p = DEREF_PLAYER(_cur_player_tick_index);
	_cur_player_tick_index = (_cur_player_tick_index + 1) % MAX_PLAYERS;
	if (p->name_1 != 0) GenerateCompanyName(p);

	if (_game_mode != GM_MENU && !--_next_competitor_start) {
		MaybeStartNewPlayer();		
	}
}

void RunOtherPlayersLoop()
{
	Player *p;

	_is_ai_player = true;

	FOR_ALL_PLAYERS(p) {
		if (p->is_active) {
			_current_player = p->index;
			AiDoGameLoop(p);
		}
	}

	_is_ai_player = false;
// XXX: is this needed?
	_current_player = 0;
}

StringID GetPlayerNameString(byte player)
{
	if (IS_HUMAN_PLAYER(player) && player < 2) // temporarily fixes the names in the list.
		return STR_7002_PLAYER_1+player;
	return STR_EMPTY;
}

extern void ShowPlayerFinances(int player);

void PlayersYearlyLoop()
{
	Player *p;

	// Copy statistics
	FOR_ALL_PLAYERS(p) {
		if (p->is_active) {
			memmove(&p->yearly_expenses[1], &p->yearly_expenses[0], sizeof(p->yearly_expenses) - sizeof(p->yearly_expenses[0]));
			memset(&p->yearly_expenses[0], 0, sizeof(p->yearly_expenses[0]));
			InvalidateWindow(WC_FINANCES, p->index);
		}
	}

	if (_patches.show_finances && _local_player != 0xff) {
		ShowPlayerFinances(_local_player);
		p = DEREF_PLAYER(_local_player);
		if (p->num_valid_stat_ent > 5 && p->old_economy[0].performance_history < p->old_economy[4].performance_history) {
			SndPlayFx(0x28);
		} else {
			SndPlayFx(0x27);
		}
	}
}

void DeletePlayerWindows(int pi)
{
	DeleteWindowById(WC_COMPANY, pi);
	DeleteWindowById(WC_FINANCES, pi);
	DeleteWindowById(WC_STATION_LIST, pi);
	DeleteWindowById(WC_TRAINS_LIST, pi);
	DeleteWindowById(WC_ROADVEH_LIST, pi);
	DeleteWindowById(WC_SHIPS_LIST, pi);
	DeleteWindowById(WC_AIRCRAFT_LIST, pi);
	DeleteWindowById(WC_BUY_COMPANY, pi);
}

static void DeletePlayerStuff(int pi)
{
	Player *p;

	DeletePlayerWindows(pi);
	p = DEREF_PLAYER(pi);
	DeleteName(p->name_1);
	DeleteName(p->president_name_1);
	p->name_1 = 0;
	p->president_name_1 = 0;
}

// functionality.
// 0 - make new player
// 1 - make new AI player
// 2 - delete player (p1 >> 8) & 0xFF
// 3 - join player (p1 >> 8) & 0xFF with (p1 >> 16) & 0xFF
int32 CmdPlayerCtrl(int x, int y, uint32 flags, uint32 p1, uint32 p2)
{
	int pi;
	Player *p;

	if (!(flags & DC_EXEC))
		return 0;
	
	switch(p1 & 0xff) {
	case 0: // make new player
		p = DoStartupNewPlayer(false);
		if (_local_player == 0xff && p != NULL) {
			_local_player = p->index;
			MarkWholeScreenDirty();
		}
		break;
	case 1: // make new ai player
		DoStartupNewPlayer(true);
		break;
	case 2: // delete player
		pi = (byte)(p1 >> 8);
		ChangeOwnershipOfPlayerItems(pi, 255);
		DeletePlayerStuff(pi);
		break;

	case 3: // join player
		pi = (byte)(p1 >> 8);
		ChangeOwnershipOfPlayerItems(pi, (byte)(p1 >> 16));
		DeletePlayerStuff(pi);
		break;
	}


	return 0;
}

// Save/load of players
static const byte _player_desc[] = {
	SLE_VAR(Player,name_2,					SLE_UINT32),
	SLE_VAR(Player,name_1,					SLE_STRINGID),

	SLE_VAR(Player,president_name_1,SLE_UINT16),
	SLE_VAR(Player,president_name_2,SLE_UINT32),

	SLE_VAR(Player,face,						SLE_UINT32),
	
	// money was changed to a 64 bit field in savegame version 1.
	SLE_CONDVAR(Player,money64,			SLE_VAR_I64 | SLE_FILE_I32, 0, 0),
	SLE_CONDVAR(Player,money64,			SLE_INT64, 1, 255),

	SLE_VAR(Player,current_loan,		SLE_INT32),

	SLE_VAR(Player,player_color,		SLE_UINT8),
	SLE_VAR(Player,player_money_fraction,SLE_UINT8),
	SLE_VAR(Player,max_railtype,		SLE_UINT8),
	SLE_VAR(Player,block_preview,		SLE_UINT8),

	SLE_VAR(Player,cargo_types,			SLE_UINT16),
	SLE_VAR(Player,location_of_house,SLE_UINT16),
	SLE_VAR(Player,last_build_coordinate,SLE_UINT16),
	SLE_VAR(Player,inaugurated_year,SLE_UINT8),

	SLE_ARR(Player,share_owners,		SLE_UINT8, 4),

	SLE_VAR(Player,num_valid_stat_ent,SLE_UINT8),

	SLE_VAR(Player,quarters_of_bankrupcy,SLE_UINT8),
	SLE_VAR(Player,bankrupt_asked,	SLE_UINT8),
	SLE_VAR(Player,bankrupt_timeout,SLE_INT16),
	SLE_VAR(Player,bankrupt_value,	SLE_INT32),

	// yearly expenses was changed to 64-bit in savegame version 2.
	SLE_CONDARR(Player,yearly_expenses,	SLE_FILE_I32|SLE_VAR_I64, 3*13, 0, 1),
	SLE_CONDARR(Player,yearly_expenses,	SLE_INT64, 3*13, 2, 255),

	SLE_CONDVAR(Player,is_ai,			SLE_UINT8, 2, 255),
	SLE_CONDVAR(Player,is_active,	SLE_UINT8, 4, 255),

	// reserve extra space in savegame here. (currently 64 bytes)
	SLE_CONDARR(NullStruct,null,SLE_FILE_U64 | SLE_VAR_NULL, 8, 2, 255),

	SLE_END()
};

static const byte _player_economy_desc[] = {
	// these were changed to 64-bit in savegame format 2
	SLE_CONDVAR(PlayerEconomyEntry,income,							SLE_INT32, 0, 1),
	SLE_CONDVAR(PlayerEconomyEntry,expenses,						SLE_INT32, 0, 1),
	SLE_CONDVAR(PlayerEconomyEntry,company_value,				SLE_INT32, 0, 1),
	SLE_CONDVAR(PlayerEconomyEntry,income,	SLE_FILE_I64 | SLE_VAR_I32, 2, 255),
	SLE_CONDVAR(PlayerEconomyEntry,expenses,SLE_FILE_I64 | SLE_VAR_I32, 2, 255),
	SLE_CONDVAR(PlayerEconomyEntry,company_value,SLE_FILE_I64 | SLE_VAR_I32, 2, 255),

	SLE_VAR(PlayerEconomyEntry,delivered_cargo,			SLE_INT32),
	SLE_VAR(PlayerEconomyEntry,performance_history,	SLE_INT32),
	
	SLE_END()
};

static const byte _player_ai_desc[] = {
	SLE_VAR(PlayerAI,state,							SLE_UINT8),
	SLE_VAR(PlayerAI,tick,							SLE_UINT8),
	SLE_VAR(PlayerAI,state_counter,			SLE_UINT16),
	SLE_VAR(PlayerAI,timeout_counter,		SLE_UINT16),

	SLE_VAR(PlayerAI,state_mode,				SLE_UINT8),
	SLE_VAR(PlayerAI,banned_tile_count,	SLE_UINT8),
	SLE_VAR(PlayerAI,railtype_to_use,		SLE_UINT8),

	SLE_VAR(PlayerAI,cargo_type,				SLE_UINT8),
	SLE_VAR(PlayerAI,num_wagons,				SLE_UINT8),
	SLE_VAR(PlayerAI,build_kind,				SLE_UINT8),
	SLE_VAR(PlayerAI,num_build_rec,			SLE_UINT8),
	SLE_VAR(PlayerAI,num_loco_to_build,	SLE_UINT8),
	SLE_VAR(PlayerAI,num_want_fullload,	SLE_UINT8),

	SLE_VAR(PlayerAI,route_type_mask,		SLE_UINT8),

	SLE_VAR(PlayerAI,start_tile_a,			SLE_UINT16),
	SLE_VAR(PlayerAI,cur_tile_a,				SLE_UINT16),
	SLE_VAR(PlayerAI,start_dir_a,				SLE_UINT8),
	SLE_VAR(PlayerAI,cur_dir_a,					SLE_UINT8),

	SLE_VAR(PlayerAI,start_tile_b,			SLE_UINT16),
	SLE_VAR(PlayerAI,cur_tile_b,				SLE_UINT16),
	SLE_VAR(PlayerAI,start_dir_b,				SLE_UINT8),
	SLE_VAR(PlayerAI,cur_dir_b,					SLE_UINT8),

	SLE_REF(PlayerAI,cur_veh,						REF_VEHICLE),

	SLE_ARR(PlayerAI,wagon_list,				SLE_UINT16, 9),
	SLE_ARR(PlayerAI,order_list_blocks,	SLE_UINT8, 20),
	SLE_ARR(PlayerAI,banned_tiles,			SLE_UINT16, 16),

	SLE_CONDARR(NullStruct,null,SLE_FILE_U64 | SLE_VAR_NULL, 8, 2, 255),
	SLE_END()
};

static const byte _player_ai_build_rec_desc[] = {
	SLE_VAR(AiBuildRec,spec_tile,			SLE_UINT16),
	SLE_VAR(AiBuildRec,use_tile,			SLE_UINT16),
	SLE_VAR(AiBuildRec,rand_rng,			SLE_UINT8),
	SLE_VAR(AiBuildRec,cur_building_rule,SLE_UINT8),
	SLE_VAR(AiBuildRec,unk6,					SLE_UINT8),
	SLE_VAR(AiBuildRec,unk7,					SLE_UINT8),
	SLE_VAR(AiBuildRec,buildcmd_a,		SLE_UINT8),
	SLE_VAR(AiBuildRec,buildcmd_b,		SLE_UINT8),
	SLE_VAR(AiBuildRec,direction,			SLE_UINT8),
	SLE_VAR(AiBuildRec,cargo,					SLE_UINT8),
	SLE_END()
};

static void SaveLoad_PLYR(Player *p) {
	int i;

	SlObject(p, _player_desc);

	// Write AI?
	if (!IS_HUMAN_PLAYER(p->index)) {
		SlObject(&p->ai, _player_ai_desc);
		for(i=0; i!=p->ai.num_build_rec; i++)
			SlObject(&p->ai.src + i, _player_ai_build_rec_desc);
	}

	// Write economy
	SlObject(&p->cur_economy, _player_economy_desc);

	// Write old economy entries.
	{
		PlayerEconomyEntry *pe;
		for(i=p->num_valid_stat_ent,pe=p->old_economy; i!=0; i--,pe++)
			SlObject(pe, _player_economy_desc);
	}
}

static void Save_PLYR()
{
	Player *p;
	FOR_ALL_PLAYERS(p) {
		if (p->is_active) {
			SlSetArrayIndex(p->index);
			SlAutolength((AutolengthProc*)SaveLoad_PLYR, p);
		}
	}
}

static void Load_PLYR()
{
	int index;
	while ((index = SlIterateArray()) != -1) {
		Player *p = DEREF_PLAYER(index);		
		p->is_ai = (index != 0);
		SaveLoad_PLYR(p);
		_player_colors[index] = p->player_color;
		UpdatePlayerMoney32(p);
	}
}

const ChunkHandler _player_chunk_handlers[] = {
	{ 'PLYR', Save_PLYR, Load_PLYR, CH_ARRAY | CH_LAST},
};

