#include "stdafx.h"
#include "ttd.h"
#include "vehicle.h"
#include "viewport.h"
#include "command.h"
#include "pathfind.h"
#include "town.h"

void ShowTrainDepotWindow(uint tile);

enum {
	RAIL_TYPE_NORMAL = 0,
	RAIL_TYPE_SIGNALS = 0x40,
	RAIL_TYPE_SPECIAL = 0x80, // If this bit is set, then it's not a regular track.
	RAIL_TYPE_DEPOT = 0xC0,
	RAIL_TYPE_MASK = 0xC0,

	RAIL_BIT_DIAG1 = 1,  // 0
	RAIL_BIT_DIAG2 = 2,  // 1
	RAIL_BIT_UPPER = 4,  // 2
	RAIL_BIT_LOWER = 8,  // 3
	RAIL_BIT_LEFT  = 16, // 4
	RAIL_BIT_RIGHT = 32, // 5
	RAIL_BIT_MASK = 0x3F,

	RAIL_DEPOT_TRACK_MASK = 1,
	RAIL_DEPOT_DIR = 3,
	RAIL_DEPOT_UNUSED_BITS = 0x3C,

	RAIL_TYPE_CHECKPOINT = 0xC4,
	RAIL_CHECKPOINT_TRACK_MASK = 1,
	RAIL_CHECKPOINT_UNUSED_BITS = 0x3E,
};

#define IS_RAIL_DEPOT(x) (((x) & (RAIL_TYPE_DEPOT|RAIL_DEPOT_UNUSED_BITS)) == RAIL_TYPE_DEPOT)
#define IS_RAIL_CHECKPOINT(x) (((x) & (RAIL_TYPE_CHECKPOINT|RAIL_CHECKPOINT_UNUSED_BITS)) == RAIL_TYPE_CHECKPOINT)

/* Format of rail map5 byte.
 * 00 abcdef  => Normal rail
 * 01 abcdef  => Rail with signals
 * 10 ??????  => Unused
 * 11 ????dd  => Depot
 */

/*         4
 *     ---------
 *    |\       /|
 *    | \    1/ |
 *    |  \   /  |
 *    |   \ /   |
 *  16|    \    |32
 *    |   / \2  | 
 *    |  /   \  |
 *    | /     \ |
 *    |/       \|
 *     ---------
 *         8
 */


// Constants for lower part of Map2 byte.
enum RailMap2Lower4 { 
	RAIL_MAP2LO_GROUND_MASK = 0xF,
	RAIL_GROUND_BROWN = 0,
	RAIL_GROUND_GREEN = 1,
	RAIL_GROUND_FENCE_NW = 2,
	RAIL_GROUND_FENCE_SE = 3,
	RAIL_GROUND_FENCE_SENW = 4,
	RAIL_GROUND_FENCE_NE = 5,
	RAIL_GROUND_FENCE_SW = 6,
	RAIL_GROUND_FENCE_NESW = 7,
	RAIL_GROUND_FENCE_VERT1 = 8,
	RAIL_GROUND_FENCE_VERT2 = 9,
	RAIL_GROUND_FENCE_HORIZ1 = 10,
	RAIL_GROUND_FENCE_HORIZ2 = 11,
	RAIL_GROUND_ICE_DESERT = 12,
};


/* MAP2 byte:    abcd???? => Signal On?
 * MAP3LO byte:  abcd???? => Signal Exists?
 * MAP2 byte:    ????abcd => Type of ground.
 * MAP3LO byte:  ????abcd => Type of rail.
 * MAP5:         00abcdef => rail
 *               01abcdef => rail w/ signals
 *               10uuuuuu => unused
 *               11uuuudd => rail depot   
 */

static bool CheckTrackCombination(byte map5, byte trackbits, byte flags)
{
	_error_message = STR_1001_IMPOSSIBLE_TRACK_COMBINATION;

	if ((map5&RAIL_TYPE_MASK) == RAIL_TYPE_SIGNALS) {
		
		if (map5 & trackbits) {
			_error_message = STR_1007_ALREADY_BUILT;
			return false;
		}

		map5 |= trackbits;
		return (map5 == (RAIL_TYPE_SIGNALS|RAIL_BIT_UPPER|RAIL_BIT_LOWER) || map5 == (RAIL_TYPE_SIGNALS|RAIL_BIT_LEFT|RAIL_BIT_RIGHT));

	} else if ((map5&RAIL_TYPE_MASK) == RAIL_TYPE_NORMAL) {
		_error_message = STR_1007_ALREADY_BUILT;
		if (map5 & trackbits)
			return false;
		
		// Computer players are not allowed to intersect pieces of rail.
		if (!(flags&DC_NO_RAIL_OVERLAP))
			return true;
		
		map5 |= trackbits;
		return (map5 == (RAIL_BIT_UPPER|RAIL_BIT_LOWER) || map5 == (RAIL_BIT_LEFT|RAIL_BIT_RIGHT));
	} else {
		return false;
	}
}


static const byte _valid_tileh_slopes[3][15] = {

// set of normal ones
{
	RAIL_BIT_DIAG1|RAIL_BIT_DIAG2|RAIL_BIT_UPPER|RAIL_BIT_LOWER|RAIL_BIT_LEFT|RAIL_BIT_RIGHT,
	RAIL_BIT_RIGHT,
	RAIL_BIT_UPPER,
	RAIL_BIT_DIAG1,

	RAIL_BIT_LEFT,
	0,
	RAIL_BIT_DIAG2,
	RAIL_BIT_LOWER,
	
	RAIL_BIT_LOWER,
	RAIL_BIT_DIAG2,
	0,
	RAIL_BIT_LEFT,
	
	RAIL_BIT_DIAG1,
	RAIL_BIT_UPPER,
	RAIL_BIT_RIGHT,
},

// allowed rail for an evenly raised platform
{
	0,
	RAIL_BIT_LEFT,
	RAIL_BIT_LOWER,
	RAIL_BIT_DIAG2 | RAIL_BIT_LOWER | RAIL_BIT_LEFT,

	RAIL_BIT_RIGHT,
	RAIL_BIT_DIAG1|RAIL_BIT_DIAG2|RAIL_BIT_UPPER|RAIL_BIT_LOWER|RAIL_BIT_LEFT|RAIL_BIT_RIGHT,
	RAIL_BIT_DIAG1 | RAIL_BIT_LOWER | RAIL_BIT_RIGHT,
	RAIL_BIT_DIAG1|RAIL_BIT_DIAG2|RAIL_BIT_UPPER|RAIL_BIT_LOWER|RAIL_BIT_LEFT|RAIL_BIT_RIGHT,

	RAIL_BIT_UPPER,
	RAIL_BIT_DIAG1 | RAIL_BIT_UPPER | RAIL_BIT_LEFT,
	RAIL_BIT_DIAG1|RAIL_BIT_DIAG2|RAIL_BIT_UPPER|RAIL_BIT_LOWER|RAIL_BIT_LEFT|RAIL_BIT_RIGHT,
	RAIL_BIT_DIAG1|RAIL_BIT_DIAG2|RAIL_BIT_UPPER|RAIL_BIT_LOWER|RAIL_BIT_LEFT|RAIL_BIT_RIGHT,

	RAIL_BIT_DIAG2 | RAIL_BIT_UPPER | RAIL_BIT_RIGHT,
	RAIL_BIT_DIAG1|RAIL_BIT_DIAG2|RAIL_BIT_UPPER|RAIL_BIT_LOWER|RAIL_BIT_LEFT|RAIL_BIT_RIGHT,
	RAIL_BIT_DIAG1|RAIL_BIT_DIAG2|RAIL_BIT_UPPER|RAIL_BIT_LOWER|RAIL_BIT_LEFT|RAIL_BIT_RIGHT,
},

// allowed rail on coast tile
{
	0,
	RAIL_BIT_LEFT,
	RAIL_BIT_LOWER,
	RAIL_BIT_DIAG2|RAIL_BIT_LEFT|RAIL_BIT_LOWER,
 
	RAIL_BIT_RIGHT,
	RAIL_BIT_DIAG1|RAIL_BIT_DIAG2|RAIL_BIT_UPPER|RAIL_BIT_LOWER|RAIL_BIT_LEFT|RAIL_BIT_RIGHT,
	RAIL_BIT_DIAG1|RAIL_BIT_RIGHT|RAIL_BIT_LOWER,
	RAIL_BIT_DIAG1|RAIL_BIT_DIAG2|RAIL_BIT_UPPER|RAIL_BIT_LOWER|RAIL_BIT_LEFT|RAIL_BIT_RIGHT,

	RAIL_BIT_UPPER,
	RAIL_BIT_DIAG1|RAIL_BIT_LEFT|RAIL_BIT_UPPER,
	RAIL_BIT_DIAG1|RAIL_BIT_DIAG2|RAIL_BIT_UPPER|RAIL_BIT_LOWER|RAIL_BIT_LEFT|RAIL_BIT_RIGHT,
	RAIL_BIT_DIAG1|RAIL_BIT_DIAG2|RAIL_BIT_UPPER|RAIL_BIT_LOWER|RAIL_BIT_LEFT|RAIL_BIT_RIGHT,

	RAIL_BIT_DIAG2|RAIL_BIT_RIGHT|RAIL_BIT_UPPER,
	RAIL_BIT_DIAG1|RAIL_BIT_DIAG2|RAIL_BIT_UPPER|RAIL_BIT_LOWER|RAIL_BIT_LEFT|RAIL_BIT_RIGHT,
	RAIL_BIT_DIAG1|RAIL_BIT_DIAG2|RAIL_BIT_UPPER|RAIL_BIT_LOWER|RAIL_BIT_LEFT|RAIL_BIT_RIGHT,
	}
};

static uint GetRailFoundation(uint tileh, uint bits)
{
	int i;

	if ((~_valid_tileh_slopes[0][tileh] & bits) == 0)
		return 0;

	if ((~_valid_tileh_slopes[1][tileh] & bits) == 0)
		return tileh;

	if ( ((i=0, tileh == 1) || (i+=2, tileh == 2) || (i+=2, tileh == 4) || (i+=2, tileh == 8)) && (bits == RAIL_BIT_DIAG1 || (i++, bits == RAIL_BIT_DIAG2))) 
		return i + 15;

	return 0;
}

//
static uint32 CheckRailSlope(int tileh, uint rail_bits, uint existing, uint tile)
{
	// never allow building on top of steep tiles
	if (!(tileh & 0x10)) {
		rail_bits |= existing;

		// don't allow building on the lower side of a coast
		if (IS_TILETYPE(tile, MP_WATER) && ~_valid_tileh_slopes[2][tileh] & rail_bits) {
			return_cmd_error(STR_3807_CAN_T_BUILD_ON_WATER);
		}

		// no special foundation
		if ((~_valid_tileh_slopes[0][tileh] & rail_bits) == 0)
			return 0;
	
		if (((~_valid_tileh_slopes[1][tileh] & rail_bits) == 0) || // whole tile is leveled up
			((rail_bits == RAIL_BIT_DIAG1 || rail_bits == RAIL_BIT_DIAG2) && (tileh == 1 || tileh == 2 || tileh == 4 || tileh == 8))) { // partly up
			return existing ? 0 : _price.terraform;	
		}
	}
	return_cmd_error(STR_1000_LAND_SLOPED_IN_WRONG_DIRECTION);
}

/* Build a single track.
 * p1 - railroad type normal/maglev
 * p2 - tile direction
 */

int32 CmdBuildSingleRail(int x, int y, uint32 flags, uint32 p1, uint32 p2)
{
	TileInfo ti;
	int32 ret, cost = 0;
	byte rail_bit = 1 << p2;
	byte rail_type = (byte)(p1 & 0xF);
	uint tile;
	byte existing = 0;
	bool need_clear = false;

	SET_EXPENSES_TYPE(EXPENSES_CONSTRUCTION);

	_error_message = STR_1007_ALREADY_BUILT;

	FindLandscapeHeight(&ti, x, y);
	tile = ti.tile;

	// allow building rail under bridge
	if (ti.type != MP_TUNNELBRIDGE && !EnsureNoVehicle(tile))	
		return CMD_ERROR;

	if (ti.type == MP_TUNNELBRIDGE) {
/* BUILD ON BRIDGE CODE */
		if (!EnsureNoVehicleZ(tile, GET_TILEHEIGHT(tile)))
			return CMD_ERROR;

		if ((ti.map5 & 0xF8) == 0xC0) {
			if (ti.tileh & 0x10 || rail_bit != (byte)((ti.map5 & 1) ? 1 : 2)) goto need_clear;

			if (!(flags & DC_EXEC))
				return _price.build_rail;

			_map5[tile] = (ti.map5 & 0xC7) | 0x20;
			goto set_ownership;
		} else if ((ti.map5 & 0xF8) == 0xE0) {
			if ((_map3_lo[tile] & 0xF) != (int)p1) goto need_clear;
			if (rail_bit != (byte)((ti.map5 & 1) ? 1 : 2)) goto need_clear;
			return CMD_ERROR;
		} else
			goto need_clear;
	} else if (ti.type == MP_STREET) {
		byte m5;
/* BUILD ON STREET CODE */
		if (ti.tileh != 0) goto need_clear;
		
		if (!(ti.map5 & 0xF0)) {
			if ((ti.map5 & 0x0F) == 0xA) {
				if (rail_bit != 2) goto need_clear;
				m5 = 0x10;
			} else if ((ti.map5 & 0x0F) == 0x5) {
				if (rail_bit != 1) goto need_clear;
				m5 = 0x18;
			} else
				goto need_clear;

			if (!(flags & DC_EXEC))
				return _price.build_rail;

			ModifyTile(tile,
				MP_SETTYPE(MP_STREET) | 
				MP_MAP3LO | MP_MAP3HI | MP_MAPOWNER_CURRENT | MP_MAP5,
				_map_owner[tile], /* map3_lo */
				p1, /* map3_hi */
				m5  /* map5 */
			);
			goto fix_signals;
		} else if (!(ti.map5 & 0xE0)) {
			if (rail_bit != (byte)((ti.map5 & 8) ? 1 : 2)) goto need_clear;
			return CMD_ERROR;
		} else
			goto need_clear;
	} else if (ti.type == MP_RAILWAY) {

/* BUILD ON RAILWAY CODE */
		if (_map_owner[tile] != _current_player || (byte)(_map3_lo[tile]&0xF) != rail_type) 
			goto need_clear;
		if (!CheckTrackCombination(ti.map5, rail_bit, (byte)flags))
			return CMD_ERROR;

		existing = ti.map5 & 0x3F;
	} else {

/* DEFAULT BUILD ON CODE */
need_clear:;
		/* isnot_railway */
		if (!(flags & DC_EXEC)) {
			ret = DoCommandByTile(tile, 0, 0, flags, CMD_LANDSCAPE_CLEAR);
			if (ret == CMD_ERROR) return CMD_ERROR;
			cost += ret;
		}
		need_clear = true;
	}

	ret = CheckRailSlope(ti.tileh, rail_bit, existing, tile);
	if (ret & 0x80000000)
		return ret;
	cost += ret;

	// the AI is not allowed to used foundationed tiles.
	if (ret && (_is_ai_player || !_patches.build_on_slopes))
		return_cmd_error(STR_1000_LAND_SLOPED_IN_WRONG_DIRECTION);

	if (flags & DC_EXEC && need_clear) {
		ret = DoCommandByTile(tile, 0, 0, flags, CMD_LANDSCAPE_CLEAR);
		if (ret == CMD_ERROR) return CMD_ERROR;
		cost += ret;
	}

	if (flags & DC_EXEC) {
		_map_type_and_height[tile] &= 0xF;
		_map_type_and_height[tile] |= MP_RAILWAY << 4;
		_map5[tile] |= rail_bit;
		_map2[tile] &= ~RAIL_MAP2LO_GROUND_MASK;

		// In case it's a tile without signals, clear the signal bits. Why?
		if ((_map5[tile] & RAIL_TYPE_MASK) != RAIL_TYPE_SIGNALS)
			_map2[tile] &= ~0xF0;

set_ownership:
		_map_owner[tile] = _current_player;

		_map3_lo[tile] &= ~0xF;
		_map3_lo[tile] |= rail_type;

		MarkTileDirtyByTile(tile);

fix_signals:
		SetSignalsOnBothDir(tile, (byte)p2);
	}

	return cost + _price.build_rail;
}

static const byte _signals_table[] = {
	0x40, 0x40, 0x40, 0x10, 0x80, 0x20, 0, 0, // direction 1
	0x80, 0x80, 0x80, 0x20, 0x40, 0x10, 0, 0  // direction 2
};

static const byte _signals_table_other[] = {
	0x80, 0x80, 0x80, 0x20, 0x40, 0x10, 0, 0, // direction 1
	0x40, 0x40, 0x40, 0x10, 0x80, 0x20, 0, 0  // direction 2
};

static const byte _signals_table_both[] = {
	0xC0, 0xC0, 0xC0, 0x30, 0xC0, 0x30, 0, 0,	// both directions combined 
	0xC0, 0xC0, 0xC0, 0x30, 0xC0, 0x30, 0, 0
};


/* Remove a single track.
 * p1 - unused
 * p2 - tile direction
 */

int32 CmdRemoveSingleRail(int x, int y, uint32 flags, uint32 p1, uint32 p2)
{
	TileInfo ti;
	byte rail_bit = 1 << p2;
	byte m5;
	uint tile;

	SET_EXPENSES_TYPE(EXPENSES_CONSTRUCTION);

	FindLandscapeHeight(&ti, x, y);

	tile = ti.tile;

	if (!((1<<ti.type) & ((1<<MP_TUNNELBRIDGE)|(1<<MP_STREET)|(1<<MP_RAILWAY))))
		return CMD_ERROR;

	if (_current_player != OWNER_WATER && !CheckTileOwnership(tile))
		return CMD_ERROR;

	// allow building rail under bridge
	if (ti.type != MP_TUNNELBRIDGE && !EnsureNoVehicle(tile))	
		return CMD_ERROR;

	if (ti.type == MP_TUNNELBRIDGE) {
		if (!EnsureNoVehicleZ(tile, GET_TILEHEIGHT(tile)))
			return CMD_ERROR;

		if ((ti.map5 & 0xF8) != 0xE0)
			return CMD_ERROR;

		if ( ((ti.map5 & 1) ? 1 : 2) != rail_bit )
			return CMD_ERROR;

		if (!(flags & DC_EXEC))
			return _price.remove_rail;
		
		_map_owner[tile] = OWNER_NONE;
		_map5[tile] = ti.map5 & 0xC7;
	} else if (ti.type == MP_STREET) {
		if (!(ti.map5 & 0xF0))
			return CMD_ERROR;

		if (ti.map5 & 0xE0)
			return CMD_ERROR;

		if (ti.map5 & 8) {
			m5 = 5;
			if (rail_bit != 1)
				return CMD_ERROR;
		} else {
			m5 = 10;
			if (rail_bit != 2)
				return CMD_ERROR;
		}

		if (!(flags & DC_EXEC))
			return _price.remove_rail;

		_map5[tile] = m5;
		_map_owner[tile] = _map3_lo[tile];
		_map2[tile] = 0;
	} else {
		assert(ti.type == MP_RAILWAY);

		if (ti.map5 & RAIL_TYPE_SPECIAL)
			return CMD_ERROR;

		if (!(ti.map5 & rail_bit))
			return CMD_ERROR;

		// don't allow remove if there are signals on the track
		if (ti.map5 & RAIL_TYPE_SIGNALS) {
			if (_map3_lo[tile] & _signals_table_both[p2])
				return CMD_ERROR;
		}

		if (!(flags & DC_EXEC))
			return _price.remove_rail;

		if ( (_map5[tile] ^= rail_bit) == 0) {
			DoClearSquare(tile);
			goto skip_mark_dirty;
		}
	}

	/* mark_dirty */
	MarkTileDirtyByTile(tile);

skip_mark_dirty:;

	SetSignalsOnBothDir(tile, (byte)p2);
	
	return _price.remove_rail;
}

static const struct {
	int8 xinc[16];
	int8 yinc[16];
	byte initial[16];
} _railbit = {{
//  0   1   2    3   4   5 
	 16,  0,-16,   0, 16,  0,    0,  0,
	-16,  0,  0,  16,  0,-16,    0,  0,
},{
	0,   16,  0,  16,  0, 16,    0,  0,
	0,  -16, -16,  0,-16,  0,    0,  0,
},{
	5,  1, 0,4, // normal
	2,  1, 8,3, // x > sx
	10, 9, 0,11, // y > sy
	13, 9, 8,12, // x > sx && y > sy
}};


/* Build either NE or NW sequence of tracks.
 * p1 0:15  - start pt X
 * p1 16:31 - start pt y
 *
 * p2 0:3   - rail type
 * p2 4:7   - rail direction
 */


int32 CmdBuildRailroadTrack(int x, int y, uint32 flags, uint32 p1, uint32 p2)
{
	int sx, sy;
	int32 ret, total_cost = 0;
	int railbit;

	if (flags & DC_EXEC)
		SndPlayTileFx(0x1E, TILE_FROM_XY(x,y));
	
	/* unpack start point */
	sx = (p1 & 0xFFFF) & ~0xF;
	sy = (p1 >> 16) & ~0xF;

	railbit = _railbit.initial[(p2 >> 4) + (x > sx ? 4 : 0) + (y > sy ? 8 : 0)];

	for(;;) {
		ret = DoCommand(x, y, p2&0xF, railbit&7, flags, CMD_BUILD_SINGLE_RAIL);

		if (ret == CMD_ERROR) {
			if (_error_message != STR_1007_ALREADY_BUILT)
				break;
		} else
			total_cost += ret;

		if (x == sx && y == sy)
			break;

		x += _railbit.xinc[railbit];
		y += _railbit.yinc[railbit];
		
		// toggle railbit for the diagonal tiles
		if (railbit & 0x6) railbit ^= 1;
	}			

	if (total_cost == 0)
		return CMD_ERROR;

	return total_cost;
}


/* Remove either NE or NW sequence of tracks.
 * p1 0:15  - start pt X
 * p1 16:31 - start pt y
 *
 * p2 0:3   - rail type
 * p2 4:7   - rail direction
 */


int32 CmdRemoveRailroadTrack(int x, int y, uint32 flags, uint32 p1, uint32 p2)
{
	int sx, sy;
	int32 ret, total_cost = 0;
	int railbit;

	if (flags & DC_EXEC)
		SndPlayTileFx(0x1E, TILE_FROM_XY(x,y));
	
	/* unpack start point */
	sx = (p1 & 0xFFFF) & ~0xF;
	sy = (p1 >> 16) & ~0xF;

	railbit = _railbit.initial[(p2 >> 4) + (x > sx ? 4 : 0) + (y > sy ? 8 : 0)];

	for(;;) {
		ret = DoCommand(x, y, p2&0xF, railbit&7, flags, CMD_REMOVE_SINGLE_RAIL);
		if (ret != CMD_ERROR) total_cost += ret;
		if (x == sx && y == sy)
			break;
		x += _railbit.xinc[railbit];
		y += _railbit.yinc[railbit];
		// toggle railbit for the diagonal tiles
		if (railbit & 0x6) railbit ^= 1;
	}			

	if (total_cost == 0)
		return CMD_ERROR;

	return total_cost;
}

/* Build a train depot
 * p1 = rail type
 * p2 = depot direction
 */
int32 CmdBuildTrainDepot(int x, int y, uint32 flags, uint32 p1, uint32 p2)
{
	uint tile = TILE_FROM_XY(x,y);
	int32 cost, ret;
	Depot *dep;
	uint tileh;

	SET_EXPENSES_TYPE(EXPENSES_CONSTRUCTION);

	if (!EnsureNoVehicle(tile)) return CMD_ERROR;

	tileh = GetTileSlope(tile, NULL);
	if (tileh != 0) {
		if (_is_ai_player || !_patches.build_on_slopes || (tileh & 0x10 || !((0x4C >> p2) & tileh) ))
			return_cmd_error(STR_0007_FLAT_LAND_REQUIRED);
	}

	ret = DoCommandByTile(tile, 0, 0, flags, CMD_LANDSCAPE_CLEAR);
	if (ret == CMD_ERROR) return CMD_ERROR;
	cost = ret;

	dep = AllocateDepot();
	if (dep == NULL)
		return CMD_ERROR;

	if (flags & DC_EXEC) {
		if (_current_player == _local_player)
			_last_built_train_depot_tile = (TileIndex)tile;
		
		ModifyTile(tile, 
			MP_SETTYPE(MP_RAILWAY) |
			MP_MAP3LO | MP_MAPOWNER_CURRENT | MP_MAP5,
			p1, /* map3_lo */
			p2 | RAIL_TYPE_DEPOT /* map5 */
		);
		
		dep->xy = tile;
		dep->town_index = ClosestTownFromTile(tile, (uint)-1)->index;

		SetSignalsOnBothDir(tile, (p2&1) ? 2 : 1);

	}

	return cost + _price.build_train_depot;
}

static void MakeDefaultCheckpointName(Checkpoint *cp)
{
	int townidx = ClosestTownFromTile(cp->xy, (uint)-1)->index;
	Checkpoint *cc;
	bool used_checkpoint[64];
	int i;

	memset(used_checkpoint, 0, sizeof(used_checkpoint));

	// find an unused checkpoint number belonging to this town
	for(cc = _checkpoints; cc != endof(_checkpoints); cc++) {
		if (cc->xy && cc->town_or_string & 0xC000 && (cc->town_or_string & 0xFF) == townidx)
			used_checkpoint[(cc->town_or_string >> 8) & 0x3F] = true;
	}

	for(i=0; used_checkpoint[i] && i!=lengthof(used_checkpoint)-1; i++) {}
	cp->town_or_string = 0xC000 + (i << 8) + townidx;
}

// find a deleted checkpoint close to a tile.
static Checkpoint *FindDeletedCheckpointCloseTo(uint tile)
{
	Checkpoint *cp,*best = NULL;
	uint thres = 8, cur_dist;

	for(cp = _checkpoints; cp != endof(_checkpoints); cp++) {
		if (cp->deleted && cp->xy) {
			cur_dist = GetTileDist(tile, cp->xy);	
			if (cur_dist < thres) {
				thres = cur_dist;
				best = cp;
			}
		}
	}
	return best;
}

/* Convert existing rail to checkpoint */

int32 CmdBuildTrainCheckpoint(int x, int y, uint32 flags, uint32 p1, uint32 p2)
{
	uint tile = TILE_FROM_XY(x,y);
	Checkpoint *cp;
	uint tileh;
	uint dir;

	SET_EXPENSES_TYPE(EXPENSES_CONSTRUCTION);

	if (!IS_TILETYPE(tile, MP_RAILWAY) || ((dir=0, _map5[tile] != 1) && (dir=1, _map5[tile] != 2)))
		return_cmd_error(STR_1005_NO_SUITABLE_RAILROAD_TRACK);

	if (!CheckTileOwnership(tile))
		return CMD_ERROR;
	
	if (!EnsureNoVehicle(tile)) return CMD_ERROR;

	tileh = GetTileSlope(tile, NULL);
	if (tileh != 0) {
		if (!_patches.build_on_slopes || tileh & 0x10 || !(tileh & (0x3 << dir)) || !(tileh & ~(0x3 << dir)))
			return_cmd_error(STR_0007_FLAT_LAND_REQUIRED);
	}

	// check if there is an already existing, deleted, checkpoint close to us that we can reuse.
	cp = FindDeletedCheckpointCloseTo(tile);
	if (cp == NULL) {
		cp = AllocateCheckpoint();
		if (cp == NULL) return CMD_ERROR;
		cp->town_or_string = 0;
	}

	if (flags & DC_EXEC) {
		ModifyTile(tile, MP_MAP5, RAIL_TYPE_CHECKPOINT | dir);
		
		cp->deleted = 0;
		cp->xy = tile;
		
		if (cp->town_or_string == 0) MakeDefaultCheckpointName(cp); else RedrawCheckpointSign(cp);
		UpdateCheckpointSign(cp);
		RedrawCheckpointSign(cp);
		SetSignalsOnBothDir(tile, dir ? 2 : 1);
	}

	return _price.build_train_depot;
}

static void DoDeleteCheckpoint(Checkpoint *cp)
{
	cp->xy = 0;
	DeleteCommandFromVehicleSchedule(((cp-_checkpoints) << 8) + OT_GOTO_CHECKPOINT);
	if (~cp->town_or_string & 0xC000) DeleteName(cp->town_or_string);
	RedrawCheckpointSign(cp);
}

// delete checkpoints after a while
void CheckpointsDailyLoop()
{
	Checkpoint *cp;
	for(cp = _checkpoints; cp != endof(_checkpoints); cp++) {
		if (cp->deleted && !--cp->deleted) {
			DoDeleteCheckpoint(cp);
		}
	}
}

static int32 RemoveTrainCheckpoint(uint tile, uint32 flags, bool justremove)
{
	Checkpoint *cp;

	// make sure it's a checkpoint
	if (!IS_TILETYPE(tile, MP_RAILWAY) || !IS_RAIL_CHECKPOINT(_map5[tile]))
		return CMD_ERROR;

	if (!CheckTileOwnership(tile) && !(_current_player==17))
		return CMD_ERROR;

	if (!EnsureNoVehicle(tile))
		return CMD_ERROR;

	if (flags & DC_EXEC) {
		int direction = _map5[tile] & RAIL_CHECKPOINT_TRACK_MASK;

		// mark the checkpoint deleted
		for(cp=_checkpoints; cp->xy != (TileIndex)tile; cp++) {}
		cp->deleted = 30; // let it live for this many days before we do the actual deletion.
		RedrawCheckpointSign(cp);

		if (justremove) {
			ModifyTile(tile, MP_MAP5, 1<<direction);
		} else {
			DoClearSquare(tile);
			SetSignalsOnBothDir(tile, direction);
		}
	}

	return _price.remove_train_depot;
}

int32 CmdRemoveTrainCheckpoint(int x, int y, uint32 flags, uint32 p1, uint32 p2)
{
	uint tile = TILE_FROM_XY(x,y);
	return RemoveTrainCheckpoint(tile, flags, true);
}


// p1 = id of checkpoint
int32 CmdRenameCheckpoint(int x, int y, uint32 flags, uint32 p1, uint32 p2)
{
	Checkpoint *cp;
	StringID str;

	if (_decode_parameters[0] != 0) {
		str = AllocateName((byte*)_decode_parameters, 0);
		if (str == 0) return CMD_ERROR;

		if (flags & DC_EXEC) {
			cp = &_checkpoints[p1];
			if (~cp->town_or_string & 0xC000) DeleteName(cp->town_or_string);
			cp->town_or_string = str;
			UpdateCheckpointSign(cp);
			MarkWholeScreenDirty();
		} else {
			DeleteName(str);
		}
	}	else {
		if (flags & DC_EXEC) {
			cp = &_checkpoints[p1];
			if (~cp->town_or_string & 0xC000) DeleteName(cp->town_or_string);
			MakeDefaultCheckpointName(cp);
			UpdateCheckpointSign(cp);
			MarkWholeScreenDirty();
		}
	}
	return 0;
}


/* build signals, p1 = track */
int32 CmdBuildSignals(int x, int y, uint32 flags, uint32 p1, uint32 p2)
{
	uint tile;
	byte m5;
	int32 cost;
	int track = p1 & 0x7;
	
	assert(track >= 0 && track < 6);
	assert(p2 == 0);
	
	SET_EXPENSES_TYPE(EXPENSES_CONSTRUCTION);

	tile = TILE_FROM_XY(x,y);
	
	if (!EnsureNoVehicle(tile))
		return CMD_ERROR;

	_error_message = STR_1005_NO_SUITABLE_RAILROAD_TRACK;
	
	// must be railway, and not a depot, and it must have a track in the suggested position.
	if (!IS_TILETYPE(tile, MP_RAILWAY) || (m5=_map5[tile], m5&0x80) || !HASBIT(m5, track))
		return CMD_ERROR;

	// check rail combination
	{
		byte m = m5 & RAIL_BIT_MASK;
		if (m != RAIL_BIT_DIAG1 && m != RAIL_BIT_DIAG2 && m != RAIL_BIT_UPPER && m != RAIL_BIT_LOWER && 
				m != RAIL_BIT_LEFT && m != RAIL_BIT_RIGHT && m != (RAIL_BIT_UPPER|RAIL_BIT_LOWER) && m != (RAIL_BIT_LEFT|RAIL_BIT_RIGHT))
			return CMD_ERROR;
	}

	// check ownership
	if (!CheckTileOwnership(tile))
		return CMD_ERROR;

	// If it had signals previously it's no cost to build.
	cost = 0;
	if (!(m5 & RAIL_TYPE_SIGNALS)) {
		cost = _price.build_signals;
	}

	if (flags & DC_EXEC) {
		

		if (!(m5 & RAIL_TYPE_SIGNALS)) {
			_map5[tile] |= RAIL_TYPE_SIGNALS; // change into signals
			_map2[tile] |= 0xF0;              // all signals are on
			_map3_lo[tile] &= ~0xF0;          // no signals built by default
			_map3_hi[tile] = (p1 & 8) ? 4 : 0;// initial presignal state, semaphores depend on ctrl key
			goto ignore_presig;
		}

		if (!(p1 & 8)) {
			byte a,b,c,d;
ignore_presig:
			a = _signals_table[track];      // signal for this track in one direction
			b = _signals_table[track + 8];  // signal for this track in the other direction
			c = a | b;
			d = _map3_lo[tile] & c;					// mask of built signals. it only affects &0xF0

			// Alternate between a|b, b, a
			if ( d != 0 && d != a) {
				c = (d==c)?b:a;
			}

			_map3_lo[tile] = (_map3_lo[tile]&~(a|b)) | c;
		} else {
			// toggle between the signal types. Using low 2 bits of map3_hi.
			_map3_hi[tile] = (_map3_hi[tile] & ~3) | ((_map3_hi[tile] + 1) & 3);
		}
		
		MarkTileDirtyByTile(tile);
		SetSignalsOnBothDir(tile, track);
	}

	return cost;
}

/* Remove signals
 * p1 = unused
 * p2 = unused
 */

int32 CmdRemoveSignals(int x, int y, uint32 flags, uint32 p1, uint32 p2)
{
	TileInfo ti;
	uint tile;

	SET_EXPENSES_TYPE(EXPENSES_CONSTRUCTION);

	FindLandscapeHeight(&ti, x, y);
	tile = ti.tile;
	
	/* No vehicle behind. */
	if (!EnsureNoVehicle(tile))
		return CMD_ERROR;

	/* Signals can only exist on railways */
	if (ti.type != MP_RAILWAY)
		return CMD_ERROR;

	/* Map5 mode is 0x40 when there's signals */
	if ((ti.map5 & RAIL_TYPE_MASK) != RAIL_TYPE_SIGNALS)
		return CMD_ERROR;

	/* Who owns the tile? */
	if (_current_player != OWNER_WATER && !CheckTileOwnership(tile))
		return CMD_ERROR;

	/* Do it? */
	if (flags & DC_EXEC) {
		byte bits = _map5[tile];
		_map5[tile] &= ~RAIL_TYPE_SIGNALS;
		_map2[tile] &= ~0xF0;
		
		/* TTDBUG: this code contains a bug, if a tile contains 2 signals
		 * on separate tracks, it won't work properly for the 2nd track */		
		SetSignalsOnBothDir(tile, FIND_FIRST_BIT(bits & RAIL_BIT_MASK)); 

		MarkTileDirtyByTile(tile);
	}
	
	return _price.remove_signals;
}

typedef int32 DoConvertRailProc(uint tile, uint totype, bool exec); 

static int32 DoConvertRail(uint tile, uint totype, bool exec)
{
	if (!CheckTileOwnership(tile) || !EnsureNoVehicle(tile))
		return CMD_ERROR;

	// tile is already of requested type?
	if ( (uint)(_map3_lo[tile] & 0xF) == totype)
		return CMD_ERROR;

	// change type.
	if (exec) {
		_map3_lo[tile] = (_map3_lo[tile] & 0xF0) + totype;
		MarkTileDirtyByTile(tile);
	}

	return _price.build_rail >> 1;
}

extern int32 DoConvertStationRail(uint tile, uint totype, bool exec);
extern int32 DoConvertStreetRail(uint tile, uint totype, bool exec);
extern int32 DoConvertTunnelBridgeRail(uint tile, uint totype, bool exec);

// p1 = start tile
// p2 = new railtype
int32 CmdConvertRail(int ex, int ey, uint32 flags, uint32 p1, uint32 p2)
{
	int32 ret, cost, money;
	int sx,sy,x,y;

	// make sure sx,sy are smaller than ex,ey
	sx = GET_TILE_X(p1)*16;
	sy = GET_TILE_Y(p1)*16;
	if (ex < sx) intswap(ex, sx);
	if (ey < sy) intswap(ey, sy);
	
	money = GetAvailableMoneyForCommand();
	ret = false;
	cost = 0;
	for(x=sx; x<=ex; x+=16) {
		for(y=sy; y<=ey; y+=16) {
			uint tile = TILE_FROM_XY(x,y);
			DoConvertRailProc *p;
			
			if (IS_TILETYPE(tile, MP_RAILWAY)) p = DoConvertRail;
			else if (IS_TILETYPE(tile, MP_STATION)) p = DoConvertStationRail;
			else if (IS_TILETYPE(tile, MP_STREET)) p = DoConvertStreetRail;
			else if (IS_TILETYPE(tile, MP_TUNNELBRIDGE)) p = DoConvertTunnelBridgeRail;
			else continue;
			
			ret = p(tile, p2, false);
			if (ret == CMD_ERROR) continue;
			cost += ret;

			if (flags & DC_EXEC) {
				if ( (money -= ret) < 0) { _additional_cash_required = ret; return cost - ret; }
				p(tile, p2, true);
			}
		}
	}
	if (cost == 0) cost = CMD_ERROR;
	return cost;
}

static int32 RemoveTrainDepot(uint tile, uint32 flags)
{
	if (!CheckTileOwnership(tile) && !(_current_player==17))
		return CMD_ERROR;

	if (!EnsureNoVehicle(tile))
		return CMD_ERROR;

	if (flags & DC_EXEC) {
		int track = _map5[tile] & RAIL_DEPOT_TRACK_MASK;

		DoDeleteDepot(tile);
		SetSignalsOnBothDir(tile, track);
	}

	return _price.remove_train_depot;
}

static int32 ClearTile_Track(uint tile, byte flags) {
	int32 cost,ret;
	int i;
	byte m5;

	m5 = _map5[tile];

	if (flags & DC_AUTO) {
		if (m5 & RAIL_TYPE_SPECIAL)
			return_cmd_error(STR_2004_BUILDING_MUST_BE_DEMOLISHED);

		if (_map_owner[tile] != _current_player)
			return_cmd_error(STR_1024_AREA_IS_OWNED_BY_ANOTHER);

		return_cmd_error(STR_1008_MUST_REMOVE_RAILROAD_TRACK);
	}

	cost = 0;
	

	if ( (m5 & RAIL_TYPE_MASK) == RAIL_TYPE_NORMAL) {
regular_track:;
		for(i=0; m5; i++, m5>>=1) {
			if (m5 & 1) {
				ret = DoCommandByTile(tile, 0, i, flags, CMD_REMOVE_SINGLE_RAIL);
				if (ret == CMD_ERROR)
					return CMD_ERROR;
				cost += ret;
			}
		}
		return cost;

	} else if ((m5 & RAIL_TYPE_MASK)==RAIL_TYPE_SIGNALS) {
		cost = DoCommandByTile(tile, 0, 0, flags, CMD_REMOVE_SIGNALS);
		if (cost == CMD_ERROR)
			return CMD_ERROR;
		m5 &= RAIL_BIT_MASK;
		if (flags & DC_EXEC)
			goto regular_track;
		return cost + _price.remove_rail;
	} else if ( (m5 & (RAIL_TYPE_MASK|RAIL_DEPOT_UNUSED_BITS)) == RAIL_TYPE_DEPOT) {
		return RemoveTrainDepot(tile, flags);
	} else if ( (m5 & (RAIL_TYPE_MASK|RAIL_CHECKPOINT_UNUSED_BITS)) == RAIL_TYPE_CHECKPOINT) {
		return RemoveTrainCheckpoint(tile, flags, false);
	} else
		return CMD_ERROR;
}



typedef struct DrawTrackSeqStruct {
	uint16 image;
	byte subcoord_x;
	byte subcoord_y;
	byte width;
	byte height;
} DrawTrackSeqStruct;

#include "table/track_land.h"

// used for presignals
static const SpriteID _signal_base_sprites[16] = {
	0x4FB,
	0x1323,
	0x1333,
	0x1343,

	// use semaphores instead of signals?
	0x1353,
	0x1363,
	0x1373,
	0x1383,

	// mirrored versions
	0x4FB,
	0x1323,
	0x1333,
	0x1343,
	
	0x13C6,
	0x13D6,
	0x13E6,
	0x13F6,
};

// used to determine the side of the road for the signal
static const byte _signal_position[24] = {
	/* original: left side position */
	0x58,0x1E,0xE1,0xB9,0x01,0xA3,0x4B,0xEE,0x3B,0xD4,0x43,0xBD,
	/* patch: ride side position */
	0x1E,0xAC,0x64,0xE1,0x4A,0x10,0xEE,0xC5,0xDB,0x34,0x4D,0xB3
};

static void DrawSignalHelper(TileInfo *ti, byte condition, uint32 image_and_pos)
{
	bool otherside = _opt.road_side & _patches.signal_side;

	uint v = _signal_position[(image_and_pos & 0xF) + (otherside ? 12 : 0)];
	uint x = ti->x | (v&0xF);
	uint y = ti->y | (v>>4);
	uint sprite = _signal_base_sprites[(_map3_hi[ti->tile] & 7) + (otherside ? 8 : 0)] + (image_and_pos>>4) + ((condition != 0) ? 1 : 0);
	AddSortableSpriteToDraw(sprite, x, y, 1, 1, 10, GetSlopeZ(x,y));
}

static uint32 _drawtile_track_palette;


static void DrawTrackFence_NW(TileInfo *ti)
{
	uint32 image = 0x515;
	if (ti->tileh != 0) {
		image = 0x519;
		if (!(ti->tileh & 2)) {
			image = 0x51B;
		}
	}
	AddSortableSpriteToDraw(image | _drawtile_track_palette, 
		ti->x, ti->y+1, 16, 1, 4, ti->z);
}

static void DrawTrackFence_SE(TileInfo *ti)
{
	uint32 image = 0x515;
	if (ti->tileh != 0) {
		image = 0x519;
		if (!(ti->tileh & 2)) {
			image = 0x51B;
		}
	}
	AddSortableSpriteToDraw(image | _drawtile_track_palette, 
		ti->x, ti->y+15, 16, 1, 4, ti->z);
}

static void DrawTrackFence_NW_SE(TileInfo *ti)
{
	DrawTrackFence_NW(ti);
	DrawTrackFence_SE(ti);
}

static void DrawTrackFence_NE(TileInfo *ti)
{
	uint32 image = 0x516;
	if (ti->tileh != 0) {
		image = 0x51A;
		if (!(ti->tileh & 2)) {
			image = 0x51C;
		}
	}
	AddSortableSpriteToDraw(image | _drawtile_track_palette, 
		ti->x+1, ti->y, 1, 16, 4, ti->z);
}

static void DrawTrackFence_SW(TileInfo *ti)
{
	uint32 image = 0x516;
	if (ti->tileh != 0) {
		image = 0x51A;
		if (!(ti->tileh & 2)) {
			image = 0x51C;
		}
	}
	AddSortableSpriteToDraw(image | _drawtile_track_palette, 
		ti->x+15, ti->y, 1, 16, 4, ti->z);
}

static void DrawTrackFence_NE_SW(TileInfo *ti)
{
	DrawTrackFence_NE(ti);
	DrawTrackFence_SW(ti);
}

static void DrawTrackFence_NS_1(TileInfo *ti)
{
	int z = ti->z;
	if (ti->tileh & 1)
		z += 8;
	AddSortableSpriteToDraw(0x517 | _drawtile_track_palette,
		ti->x + 8, ti->y + 8, 1, 1, 4, z);
}

static void DrawTrackFence_NS_2(TileInfo *ti)
{
	int z = ti->z;
	if (ti->tileh & 4)
		z += 8;
	AddSortableSpriteToDraw(0x517 | _drawtile_track_palette,
		ti->x + 8, ti->y + 8, 1, 1, 4, z);
}

static void DrawTrackFence_WE_1(TileInfo *ti)
{
	int z = ti->z;
	if (ti->tileh & 8)
		z += 8;
	AddSortableSpriteToDraw(0x518 | _drawtile_track_palette,
		ti->x + 8, ti->y + 8, 1, 1, 4, z);
}

static void DrawTrackFence_WE_2(TileInfo *ti)
{
	int z = ti->z;
	if (ti->tileh & 2)
		z += 8;
	AddSortableSpriteToDraw(0x518 | _drawtile_track_palette,
		ti->x + 8, ti->y + 8, 1, 1, 4, z);
}

static void DetTrackDrawProc_Null(TileInfo *ti)
{
	/* nothing should be here */
}

typedef void DetailedTrackProc(TileInfo *ti);
DetailedTrackProc * const _detailed_track_proc[16] = {
	DetTrackDrawProc_Null,
	DetTrackDrawProc_Null,
	
	DrawTrackFence_NW,
	DrawTrackFence_SE,
	DrawTrackFence_NW_SE,
	
	DrawTrackFence_NE,
	DrawTrackFence_SW,
	DrawTrackFence_NE_SW,
	
	DrawTrackFence_NS_1,
	DrawTrackFence_NS_2,
	
	DrawTrackFence_WE_1,
	DrawTrackFence_WE_2,
	
	DetTrackDrawProc_Null,
	DetTrackDrawProc_Null,
	DetTrackDrawProc_Null,
	DetTrackDrawProc_Null,
};

static void DrawTile_Track(TileInfo *ti)
{
	uint32 tracktype_offs, image;
	byte m5;

	_drawtile_track_palette = SPRITE_PALETTE(PLAYER_SPRITE_COLOR(_map_owner[ti->tile]));

	tracktype_offs = (_map3_lo[ti->tile] & 0xF) * TRACKTYPE_SPRITE_PITCH;

	m5 = (byte)ti->map5;
	if (!(m5 & RAIL_TYPE_SPECIAL)) {
		bool special;

		m5 &= RAIL_BIT_MASK;
		
		special = false;

		// select the sprite to use based on the map5 byte.
		(image = 0x3F3, m5 == RAIL_BIT_DIAG2) ||
		(image++,				m5 == RAIL_BIT_DIAG1) ||
		(image++,				m5 == RAIL_BIT_UPPER) ||
		(image++,				m5 == RAIL_BIT_LOWER) ||
		(image++,				m5 == RAIL_BIT_RIGHT) ||
		(image++,				m5 == RAIL_BIT_LEFT) ||
		(image++,				m5 == (RAIL_BIT_DIAG1|RAIL_BIT_DIAG2)) ||
		
		(image = 0x40B, m5 == (RAIL_BIT_UPPER|RAIL_BIT_LOWER)) ||
		(image++,				m5 == (RAIL_BIT_LEFT|RAIL_BIT_RIGHT)) ||
		
		(special=true, false) ||

		(image = 0x3FA, !(m5 & (RAIL_BIT_RIGHT|RAIL_BIT_UPPER|RAIL_BIT_DIAG1))) ||
		(image++,				!(m5 & (RAIL_BIT_LEFT|RAIL_BIT_LOWER|RAIL_BIT_DIAG1))) ||
		(image++,				!(m5 & (RAIL_BIT_LEFT|RAIL_BIT_UPPER|RAIL_BIT_DIAG2))) ||
		(image++,				!(m5 & (RAIL_BIT_RIGHT|RAIL_BIT_LOWER|RAIL_BIT_DIAG2))) ||
		(image++, true);

		if (ti->tileh != 0) {
			int f = GetRailFoundation(ti->tileh, ti->map5 & 0x3F);
			if (f) DrawFoundation(ti, f);
			
			// default sloped sprites..
			if (ti->tileh != 0) image = _track_sloped_sprites[ti->tileh - 1] + 0x3F3;
		}

		if ((_map2[ti->tile] & RAIL_MAP2LO_GROUND_MASK)==RAIL_GROUND_BROWN)
			image = (image & 0xFFFF) | 0x3178000; // use a brown palette
		else if ((_map2[ti->tile] & RAIL_MAP2LO_GROUND_MASK)==RAIL_GROUND_ICE_DESERT)
			image += 26;

		DrawGroundSprite(image + tracktype_offs);

		if (special) {
			if (m5 & RAIL_BIT_DIAG1) DrawGroundSprite(0x3ED + tracktype_offs);
			if (m5 & RAIL_BIT_DIAG2) DrawGroundSprite(0x3EE + tracktype_offs);
			if (m5 & RAIL_BIT_UPPER) DrawGroundSprite(0x3EF + tracktype_offs);
			if (m5 & RAIL_BIT_LOWER) DrawGroundSprite(0x3F0 + tracktype_offs);
			if (m5 & RAIL_BIT_LEFT)  DrawGroundSprite(0x3F2 + tracktype_offs);
			if (m5 & RAIL_BIT_RIGHT) DrawGroundSprite(0x3F1 + tracktype_offs);
		}

		if (_display_opt & DO_FULL_DETAIL) {
			_detailed_track_proc[_map2[ti->tile] & RAIL_MAP2LO_GROUND_MASK](ti);
		}

		/* draw signals also? */
		if (!(ti->map5 & RAIL_TYPE_SIGNALS))
			return;

		{
			byte m23;

			m23 = (_map3_lo[ti->tile] >> 4) | (_map2[ti->tile] & 0xF0);

#define HAS_SIGNAL(x) (m23 & (byte)(0x1 << (x)))
#define ISON_SIGNAL(x) (m23 & (byte)(0x10 << (x)))
#define MAYBE_DRAW_SIGNAL(x,y,z) if (HAS_SIGNAL(x)) DrawSignalHelper(ti, ISON_SIGNAL(x), ((y-0x4FB) << 4)|(z))

		if (!(m5 & RAIL_BIT_DIAG2)) {
			if (!(m5 & RAIL_BIT_DIAG1)) {
				if (m5 & RAIL_BIT_LEFT) {
					MAYBE_DRAW_SIGNAL(2, 0x509, 0);
					MAYBE_DRAW_SIGNAL(3, 0x507, 1);
				}
				if (m5 & RAIL_BIT_RIGHT) {
					MAYBE_DRAW_SIGNAL(0, 0x509, 2);
					MAYBE_DRAW_SIGNAL(1, 0x507, 3);
				}
				if (m5 & RAIL_BIT_UPPER) {
					MAYBE_DRAW_SIGNAL(3, 0x505, 4);
					MAYBE_DRAW_SIGNAL(2, 0x503, 5);
				}
				if (m5 & RAIL_BIT_LOWER) {
					MAYBE_DRAW_SIGNAL(1, 0x505, 6);
					MAYBE_DRAW_SIGNAL(0, 0x503, 7);
				}
			} else {
				MAYBE_DRAW_SIGNAL(3, 0x4FB, 8);
				MAYBE_DRAW_SIGNAL(2, 0x4FD, 9);
			}
		} else {
			MAYBE_DRAW_SIGNAL(3, 0x4FF, 10);
			MAYBE_DRAW_SIGNAL(2, 0x501, 11);
		}
		}
	} else {
		const byte *s;
		const DrawTrackSeqStruct *drss;

		if (!(m5 & (RAIL_TYPE_MASK&~RAIL_TYPE_SPECIAL)))
			return;
		
		if (ti->tileh != 0) { DrawFoundation(ti, ti->tileh); }

		s = _track_depot_layout_table[m5 & 0x3F];

		image = *(uint16*)s;
		if (image & 0x8000) image = (image & 0x7FFF) + tracktype_offs;

		// adjust ground tile for desert
		// (don't adjust for arctic, because snow in depots looks weird)
		if ((_map2[ti->tile] & RAIL_MAP2LO_GROUND_MASK)==RAIL_GROUND_ICE_DESERT && _opt.landscape == LT_DESERT)
		{
			if(image!=3981)
				image += 26; // tile with tracks
			else
				image = 4550; // flat ground
		}

		DrawGroundSprite(image);

		drss = (const DrawTrackSeqStruct*)(s + sizeof(uint16));

		while ((image=drss->image) != 0) {
			if (image & 0x8000)
				image |= _drawtile_track_palette;
			image += tracktype_offs;
			if (!(_display_opt & DO_TRANS_BUILDINGS)) 
				image = (image & 0x3FFF) | 0x3224000;
			AddSortableSpriteToDraw(image, ti->x | drss->subcoord_x, 
				ti->y | drss->subcoord_y, drss->width, drss->height, 0x17, ti->z);
			drss++;
		}
	}
}

void DrawTrainDepotSprite(int x, int y, int image, int railtype)
{
	uint32 ormod, img;
	const DrawTrackSeqStruct *dtss;
	const byte *t;

	/* baseimage */
	railtype *= TRACKTYPE_SPRITE_PITCH;

	ormod = PLAYER_SPRITE_COLOR(_local_player);

	t = _track_depot_layout_table[image];

	x+=33;
	y+=17;

	img = *(uint16*)t;
	if (img & 0x8000) img = (img & 0x7FFF) + railtype; 
	DrawSprite(img, x, y);

	for(dtss = (DrawTrackSeqStruct *)(t + sizeof(uint16)); dtss->image != 0; dtss++) {
		Point pt = RemapCoords(dtss->subcoord_x, dtss->subcoord_y, 0);
		image = dtss->image;
		if (image & 0x8000) image |= ormod;
		DrawSprite(image + railtype, x + pt.x, y + pt.y);
	}
}

#define NUM_SSD_ENTRY 256
#define NUM_SSD_STACK 32

typedef struct SetSignalsData {
	int cur;
	int cur_stack;
	bool stop;
	bool has_presignal;
	
	// presignal info
	int presignal_exits;
	int presignal_exits_free;

	// these are used to keep track of the signals that change.
	byte bit[NUM_SSD_ENTRY];
	TileIndex tile[NUM_SSD_ENTRY];

	// these are used to keep track of the stack that modifies presignals recursively
	TileIndex next_tile[NUM_SSD_STACK];
	byte next_dir[NUM_SSD_STACK];

} SetSignalsData;

static bool SetSignalsEnumProc(uint tile, SetSignalsData *ssd, int track, uint length, byte *state)
{
	// the tile has signals?
	if (IS_TILETYPE(tile, MP_RAILWAY)) {
		if ((_map5[tile]&RAIL_TYPE_MASK) == RAIL_TYPE_SIGNALS) {
			if ((_map3_lo[tile] & _signals_table_both[track]) != 0) {
				
				// is the signal pointing in to the segment existing?
				if ((_map3_lo[tile] & _signals_table[track]) != 0) {
					// yes, add the signal to the list of signals
					if (ssd->cur != NUM_SSD_ENTRY) {
						ssd->tile[ssd->cur] = tile; // remember the tile index
						ssd->bit[ssd->cur] = track; // and the controlling bit number
						ssd->cur++;
					}

					// remember if this block has a presignal.
					ssd->has_presignal |= (_map3_hi[tile]&1);
				}
				
				// is this an exit signal that points out from the segment?
				if ((_map3_hi[tile]&2) && _map3_lo[tile]&_signals_table_other[track]) {
					ssd->presignal_exits++;
					if ((_map2[tile]&_signals_table_other[track]) != 0)
						ssd->presignal_exits_free++;
				}

				return true;
			}
		} else if (IS_RAIL_DEPOT(_map5[tile]))
			return true; // don't look further if the tile is a depot
	}
	return false;
}

static void SetSignalsAfterProc(TrackPathFinder *tpf)
{
	SetSignalsData *ssd = tpf->userdata;
	Vehicle *v;
	uint tile, hash, val, offs;
	TrackPathFinderLink *link;

	ssd->stop = false;
	
	// for each train, check if it is in the segment.
	// then we know that the signal should be red.
	FOR_ALL_VEHICLES(v) {
		if (v->type != VEH_Train)
			continue;

		tile = v->tile;
		if (v->u.rail.track == 0x40) { tile = GetVehicleOutOfTunnelTile(v); }
		
		hash = PATHFIND_HASH_TILE(tile);
	
		val = tpf->hash_head[hash];
		if (val == 0)
			continue;

		if (!(val & 0x8000)) {
			if ((TileIndex)tile == tpf->hash_tile[hash])
				goto check_val;
		} else {
			offs = tpf->hash_tile[hash];
			do {
				link = PATHFIND_GET_LINK_PTR(tpf, offs);
				if ( (TileIndex)tile == link->tile) {
					val = link->flags;
check_val:;
					// the train is on the track, in either direction?
					if (val & (v->u.rail.track + (v->u.rail.track<<8))) {
						ssd->stop = true;
						return;
					}
					break;
				}
			} while ((offs=link->next) != 0xFFFF);
		}
	}
}

static const byte _dir_from_track[14] = {
	0,1,0,1,2,1, 0,0,
	2,3,3,2,3,0,
};


static void ChangeSignalStates(SetSignalsData *ssd)
{
	int i;

	// thinking about presignals...
	// the presignal is green if,
	//   if no train is in the segment AND
	//   there is at least one green exit signal OR
	//   there are no exit signals in the segment

	// then mark the signals in the segment accordingly
	for(i=0; i!=ssd->cur; i++) {
		uint tile = ssd->tile[i];
		byte bit = _signals_table[ssd->bit[i]];
		byte m2 = _map2[tile];

		// presignals don't turn green if there is at least one presignal exit and none are free
		if (_map3_hi[tile] & 1) {
			int ex = ssd->presignal_exits, exfree = ssd->presignal_exits_free;

			// subtract for dual combo signals so they don't count themselves
			if (_map3_hi[tile]&2 && _map3_lo[tile]&_signals_table_other[ssd->bit[i]]) {
				ex--;
				if ((_map2[tile]&_signals_table_other[ssd->bit[i]]) != 0) exfree--;
			}

			// if we have exits and none are free, make red.
			if (ex && !exfree) goto make_red;
		}

		// check if the signal is unaffected.
		if (ssd->stop) {
make_red:
			// turn red
			if ( (bit&m2) == 0 )
				continue;	
		} else {
			// turn green
			if ( (bit&m2) != 0 )
				continue;
		}

		// Update signals on the other side of this exit signal, it changed.
		// If this segment has presignals, then we treat exit signals going into the segment as normal signals.
		if (_map3_hi[tile]&2 && (_map3_hi[tile]&1 || !ssd->has_presignal)) {
			if (ssd->cur_stack != NUM_SSD_STACK) {
				ssd->next_tile[ssd->cur_stack] = tile;
				ssd->next_dir[ssd->cur_stack] = _dir_from_track[ssd->bit[i]];
				ssd->cur_stack++;
			} else {
				printf("NUM_SSD_STACK too small\n");
			}
		}
		
		// it changed, so toggle it
		_map2[tile] = m2 ^ bit;
		MarkTileDirtyByTile(tile);
	}
}


bool UpdateSignalsOnSegment(uint tile, byte direction)
{
	SetSignalsData ssd;
	int result = -1;

	ssd.cur_stack = 0;
	direction>>=1;

	for(;;) {
		// go through one segment and update all signals pointing into that segment.
		ssd.cur = ssd.presignal_exits = ssd.presignal_exits_free = 0;
		ssd.has_presignal = false;

		FollowTrack(tile, 0xC000, direction, (TPFEnumProc*)SetSignalsEnumProc, SetSignalsAfterProc, &ssd);
		ChangeSignalStates(&ssd);

		// remember the result only for the first iteration.
		if (result < 0) result = ssd.stop;
		
		// if any exit signals were changed, we need to keep going to modify the stuff behind those.
		if(!ssd.cur_stack)
			break;

		// one or more exit signals were changed, so we need to update another segment too.
		tile = ssd.next_tile[--ssd.cur_stack];
		direction = ssd.next_dir[ssd.cur_stack];
	}
	
	return (bool)result;
}

void SetSignalsOnBothDir(uint tile, byte track)
{
	static const byte _search_dir_1[6] = {1, 3, 1, 3, 5, 3};
	static const byte _search_dir_2[6] = {5, 7, 7, 5, 7, 1};

	UpdateSignalsOnSegment(tile, _search_dir_1[track]);
	UpdateSignalsOnSegment(tile, _search_dir_2[track]);
}

uint GetSlopeZ_Track(TileInfo *ti) 
{
	uint z = ti->z;
	int th = ti->tileh;
	
	// check if it's a foundation
	if (ti->tileh != 0) {
		if ((ti->map5 & 0x80) == 0) {
			uint f = GetRailFoundation(ti->tileh, ti->map5 & 0x3F);
			if (f != 0) {
				if (f < 15) {
					// leveled foundation
					return z + 8;
				}
				// inclined foundation
				th = _inclined_tileh[f - 15];
			}
		} else if ((ti->map5 & 0xC0) == 0xC0) {
			// depot or checkpoint
			return z + 8;
		}
		return GetPartialZ(ti->x&0xF, ti->y&0xF, th) + z;
	}
	return z;
}

static void GetAcceptedCargo_Track(uint tile, AcceptedCargo *ac)
{
	/* not used */
}

static void AnimateTile_Track(uint tile)
{
	/* not used */
}

static void TileLoop_Track(uint tile)
{
	byte a2;
	byte rail;
	byte m2;
	byte owner;

	m2 = _map2[tile] & 0xF;

	/* special code for alps landscape */
	if (_opt.landscape == LT_HILLY) {
		/* convert into snow? */
		if (GetTileZ(tile) > _opt.snow_line) {
			a2 = RAIL_GROUND_ICE_DESERT;
			goto modify_me;
		}

	/* special code for desert landscape */
	} else if (_opt.landscape == LT_DESERT) {
		/* convert into desert? */
		if (GetMapExtraBits(tile) == 1) {
			a2 = RAIL_GROUND_ICE_DESERT;
			goto modify_me;
		}
	}

	// Don't continue tile loop for depots
	if (_map5[tile] & RAIL_TYPE_SPECIAL)
		return;

	a2 = RAIL_GROUND_GREEN;

	if (m2 != RAIL_GROUND_BROWN) { /* wait until bottom is green */
		/* determine direction of fence */
		rail = _map5[tile] & RAIL_BIT_MASK;
		
		if (rail == RAIL_BIT_UPPER) {
			a2 = RAIL_GROUND_FENCE_HORIZ1;
		} else if (rail == RAIL_BIT_LOWER) {
			a2 = RAIL_GROUND_FENCE_HORIZ2;
		} else if (rail == RAIL_BIT_LEFT) {
			a2 = RAIL_GROUND_FENCE_VERT1;
		} else if (rail == RAIL_BIT_RIGHT) {
			a2 = RAIL_GROUND_FENCE_VERT2;
		} else {
			owner = _map_owner[tile];

			if ( (!(rail&(RAIL_BIT_DIAG2|RAIL_BIT_UPPER|RAIL_BIT_LEFT)) && (rail&RAIL_BIT_DIAG1)) || rail==(RAIL_BIT_LOWER|RAIL_BIT_RIGHT)) {
				if (!IS_TILETYPE(tile + TILE_XY(0,-1), MP_RAILWAY) ||
						owner != _map_owner[tile + TILE_XY(0,-1)] ||
						(_map5[tile + TILE_XY(0,-1)]==RAIL_BIT_UPPER || _map5[tile + TILE_XY(0,-1)]==RAIL_BIT_LEFT))
							a2 = RAIL_GROUND_FENCE_NW;					
			}

			if ( (!(rail&(RAIL_BIT_DIAG2|RAIL_BIT_LOWER|RAIL_BIT_RIGHT)) && (rail&RAIL_BIT_DIAG1)) || rail==(RAIL_BIT_UPPER|RAIL_BIT_LEFT)) {
				if (!IS_TILETYPE(tile + TILE_XY(0,1), MP_RAILWAY) ||
						owner != _map_owner[tile + TILE_XY(0,1)] ||
						(_map5[tile + TILE_XY(0,1)]==RAIL_BIT_LOWER || _map5[tile + TILE_XY(0,1)]==RAIL_BIT_RIGHT))
							a2 = (a2 == RAIL_GROUND_FENCE_NW) ? RAIL_GROUND_FENCE_SENW : RAIL_GROUND_FENCE_SE;
			}

			if ( (!(rail&(RAIL_BIT_DIAG1|RAIL_BIT_UPPER|RAIL_BIT_RIGHT)) && (rail&RAIL_BIT_DIAG2)) || rail==(RAIL_BIT_LOWER|RAIL_BIT_LEFT)) {
				if (!IS_TILETYPE(tile + TILE_XY(-1,0), MP_RAILWAY) ||
						owner != _map_owner[tile + TILE_XY(-1,0)] ||
						(_map5[tile + TILE_XY(-1,0)]==RAIL_BIT_UPPER || _map5[tile + TILE_XY(-1,0)]==RAIL_BIT_RIGHT))
							a2 = RAIL_GROUND_FENCE_NE;
			}

			if ( (!(rail&(RAIL_BIT_DIAG1|RAIL_BIT_LOWER|RAIL_BIT_LEFT)) && (rail&RAIL_BIT_DIAG2)) || rail==(RAIL_BIT_UPPER|RAIL_BIT_RIGHT)) {
				if (!IS_TILETYPE(tile + TILE_XY(1,0), MP_RAILWAY) ||
						owner != _map_owner[tile + TILE_XY(1,0)] ||
						(_map5[tile + TILE_XY(1,0)]==RAIL_BIT_LOWER || _map5[tile + TILE_XY(1,0)]==RAIL_BIT_LEFT))
							a2 = (a2 == RAIL_GROUND_FENCE_NE) ? RAIL_GROUND_FENCE_NESW : RAIL_GROUND_FENCE_SW;
			}
		}
	}

modify_me:;
	/* tile changed? */
	if ( m2 != a2) {
		_map2[tile] = (_map2[tile] & ~RAIL_MAP2LO_GROUND_MASK) | a2;
		MarkTileDirtyByTile(tile);
	}
}


static uint32 GetTileTrackStatus_Track(uint tile, int mode) {
	byte m5, a, b;
	uint32 ret;

	if (mode != 0)
		return 0;

	m5 = _map5[tile];

	if (!(m5 & RAIL_TYPE_SPECIAL)) {
		ret = (m5 | (m5 << 8)) & 0x3F3F;
		if (!(m5 & RAIL_TYPE_SIGNALS)) {
			if ( (ret & 0xFF) == 3)
				ret |= 0x40;
		} else {
			/* has_signals */

			a = _map3_lo[tile];
			b = _map2[tile];

			b &= a;

			if ((a & 0xC0) == 0) { b |= 0xC0; }
			if ((a & 0x30) == 0) { b |= 0x30; }

			if ( (b & 0x80) == 0)	ret |= 0x10070000;
			if ( (b & 0x40) == 0)	ret |= 0x7100000;
			if ( (b & 0x20) == 0)	ret |= 0x20080000;
			if ( (b & 0x10) == 0)	ret |= 0x8200000;
		}
	} else if (m5 & 0x40) {
		static const byte _train_spec_tracks[6] = {1,2,1,2,1,2};
		m5 = _train_spec_tracks[m5 & 0x3F];
		ret = (m5 << 8) + m5;
	} else
		return 0;

	return ret;
}

static void ClickTile_Track(uint tile)
{
	if (IS_RAIL_DEPOT(_map5[tile]))
		ShowTrainDepotWindow(tile);
	else if (IS_RAIL_CHECKPOINT(_map5[tile]))
		ShowRenameCheckpointWindow(&_checkpoints[GetCheckpointByTile(tile)]);
		
}

static void GetTileDesc_Track(uint tile, TileDesc *td)
{
	byte m5 = _map5[tile];
	if (!(m5 & 0x80)) {
		if(!(m5 & 0x40)) // no signals
			td->str = STR_1021_RAILROAD_TRACK;
		else
		{
			switch(_map3_hi[tile] & 0x03)
			{
				case 0: td->str = STR_RAILROAD_TRACK_WITH_NORMAL_SIGNALS;
					break;
				case 1: td->str = STR_RAILROAD_TRACK_WITH_PRESIGNALS;
					break;
				case 2: td->str = STR_RAILROAD_TRACK_WITH_EXITSIGNALS;
					break;
				case 3: td->str = STR_RAILROAD_TRACK_WITH_COMBOSIGNALS;
					break;
			}
		}
	} else {
		td->str = m5 < 0xC4 ? STR_1023_RAILROAD_TRAIN_DEPOT : STR_LANDINFO_CHECKPOINT;
	}
	td->owner = _map_owner[tile];
}

static void ChangeTileOwner_Track(uint tile, byte old_player, byte new_player)
{
	if (_map_owner[tile] != old_player)
		return;
	
	if (new_player != 255) {
		_map_owner[tile] = new_player;
	}	else {
		DoCommandByTile(tile, 0, 0, DC_EXEC, CMD_LANDSCAPE_CLEAR);
	}
}

static const byte _fractcoords_behind[4] = { 0x8F, 0x8, 0x80, 0xF8 };
static const byte _fractcoords_enter[4] = { 0x8A, 0x48, 0x84, 0xA8 };
static const byte _fractcoords_leave[4] = { 0x81, 0xD8, 0x8D, 0x18 };
static const byte _enter_directions[4] = {5, 7, 1, 3};
static const byte _leave_directions[4] = {1, 3, 5, 7};
static const byte _depot_track_mask[4] = {1, 2, 1, 2};

uint32 VehicleEnter_Track(Vehicle *v, uint tile, int x, int y)
{
	byte fract_coord;
	int dir;
	
	// this routine applies only to trains in depot tiles
	if (v->type != VEH_Train || !IS_RAIL_DEPOT(_map5[tile]))
		return 0;

	/* depot direction */
	dir = _map5[tile] & RAIL_DEPOT_DIR;
	
	fract_coord = (x & 0xF) + ((y & 0xF) << 4);
	if (_fractcoords_behind[dir] == fract_coord) {
		/* make sure a train is not entering the tile from behind */
		return 8;
	} else if (_fractcoords_enter[dir] == fract_coord) {
		if (_enter_directions[dir] == v->direction) {
			/* enter the depot */
			v->u.rail.track = 0x80,
			v->vehstatus |= VS_HIDDEN; /* hide it */
			v->direction ^= 4;
			if (v->next == NULL)
				TrainEnterDepot(v, tile);
			v->tile = tile;
			InvalidateWindow(WC_VEHICLE_DEPOT, tile);
			return 4;
		}
	} else if (_fractcoords_leave[dir] == fract_coord) {
		if (_leave_directions[dir] == v->direction) {
			/* leave the depot? */
			if ((v=v->next) != NULL) {
				v->vehstatus &= ~VS_HIDDEN;
				v->u.rail.track = _depot_track_mask[dir];
			}
		}
	}

	return 0;
}

void InitializeRail()
{
	_last_built_train_depot_tile = 0;
}

const TileTypeProcs _tile_type_rail_procs = {
	DrawTile_Track,						/* draw_tile_proc */
	GetSlopeZ_Track,					/* get_slope_z_proc */
	ClearTile_Track,					/* clear_tile_proc */
	GetAcceptedCargo_Track,		/* get_accepted_cargo_proc */
	GetTileDesc_Track,				/* get_tile_desc_proc */
	GetTileTrackStatus_Track,	/* get_tile_track_status_proc */
	ClickTile_Track,					/* click_tile_proc */
	AnimateTile_Track,				/* animate_tile_proc */
	TileLoop_Track,						/* tile_loop_clear */
	ChangeTileOwner_Track,		/* change_tile_owner_clear */
	NULL,											/* get_produced_cargo_proc */
	VehicleEnter_Track,				/* vehicle_enter_tile_proc */
	NULL,											/* vehicle_leave_tile_proc */
};
