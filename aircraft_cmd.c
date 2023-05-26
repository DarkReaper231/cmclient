#include "stdafx.h"
#include "ttd.h"
#include "vehicle.h"
#include "engine.h"
#include "command.h"
#include "station.h"
#include "news.h"
#include "gfx.h"
#include "player.h"
#include "airport.h"

static bool AirportMove(Vehicle *v, const AirportFTAClass *Airport);
static bool AirportSetBlocks(Vehicle *v, AirportFTA *current_pos, const AirportFTAClass *Airport);
static bool AirportHasBlock(Vehicle *v, AirportFTA *current_pos, const AirportFTAClass *Airport);
static bool AirportFindFreeTerminal(Vehicle *v, const AirportFTAClass *Airport);
static bool AirportFindFreeHelipad(Vehicle *v, const AirportFTAClass *Airport);
static void AirportGoToNextPosition(Vehicle *v, const AirportFTAClass *Airport);

static void AircraftNextAirportPos_and_Order(Vehicle *v);
static byte GetAircraftFlyingAltitude(const Vehicle *v);

static const SpriteID _aircraft_sprite[] = {
	0x0EB5, 0x0EBD, 0x0EC5, 0x0ECD,
	0x0ED5, 0x0EDD, 0x0E9D, 0x0EA5,
	0x0EAD, 0x0EE5, 0x0F05, 0x0F0D,
	0x0F15, 0x0F1D, 0x0F25, 0x0F2D,
	0x0EED, 0x0EF5, 0x0EFD, 0x0F35,
	0x0E9D, 0x0EA5, 0x0EAD, 0x0EB5,
	0x0EBD, 0x0EC5
};

int GetAircraftImage(Vehicle *v, byte direction)
{
	int spritenum = v->spritenum;

#ifdef AIRCRAFT_CUSTOM_SPRITES // TODO --pasky
	if (is_custom_sprite(spritenum)) {
		int sprite = GetCustomVehicleSprite(v, direction);

		if (sprite) return sprite;
		spritenum = _engine_original_sprites[v->engine_type];
	}
#endif
	return direction + _aircraft_sprite[spritenum];
}

const byte _aircraft_cost_table[NUM_AIRCRAFT_ENGINES] = {
	14, 15, 16, 75, 15, 18, 17, 18,
	19, 20, 16, 18, 17, 30, 18, 19,
	27, 25, 20, 19, 18, 26, 16, 17,
	16, 16, 17, 18, 20, 21, 19, 24,
	80, 13, 18, 25, 32, 80, 15, 17,
	15
};

const byte _aircraft_speed[NUM_AIRCRAFT_ENGINES] = {
	37, 37, 74, 181, 37, 74, 74, 74,
	74, 74, 74, 74, 74, 74, 74, 74,
	74, 74, 74, 74, 74, 74, 74, 74,
	74, 74, 74, 74, 74, 74, 181, 74,
	181, 37, 37, 74, 74, 181, 25, 40,
	25
};

const byte _aircraft_running_cost[NUM_AIRCRAFT_ENGINES] = {
	85, 100, 130, 250, 98, 240, 150, 245,
	192, 190, 135, 240, 155, 253, 210, 220,
	230, 225, 235, 220, 170, 210, 125, 145,
	130, 149, 170, 210, 230, 220, 160, 248,
	251, 85, 100, 140, 220, 255, 81, 77,
	80
};

const byte _aircraft_num_mail[NUM_AIRCRAFT_ENGINES] = {
	 4,  8, 10, 20,  6, 30, 15, 30,
	40, 25, 10, 35, 15, 50, 25, 25,
	40, 35, 30, 25, 20, 20, 10, 10,
	10, 10, 18, 25, 60, 65, 45, 80,
	45,  5,  9, 12, 40, 30, 15, 20,
	10
};

const uint16 _aircraft_num_pass[NUM_AIRCRAFT_ENGINES] = {
	 25, 65, 90,100, 30,200,100,150,
	220,230, 95,170,110,300,200,240,
	260,240,260,210,160,220, 80, 85,
	 75, 85, 65,110,180,150, 85,400,
	130, 25, 60, 90,200,100, 40, 55,
	 40
};

const byte _aircraft_sounds[NUM_AIRCRAFT_ENGINES] = {
	 6,  6,  7, 59,  6,  7,  7,  7,
	 7,  7,  7,  7,  7, 61,  7,  7,
	 7,  7,  7,  7,  7,  7,  7,  7,
	 7,  7,  7,  7,  7,  7,  7, 61,
	59, 69, 70,  7, 61, 59,  7,  7,
	 7
};

const byte _aircraft_table_3[NUM_AIRCRAFT_ENGINES] = {
	 1,  0,  2,  8,  5,  6,  2,  2,
	 3,  3,  2,  2,  4,  7,  4,  4,
	 4,  3,  4,  4,  4,  4,  6,  2,
	11, 10, 15, 12, 13, 14, 16, 17,
	18, 20, 21, 22, 23, 24,  9, 19,
	25
};

// &1 = regular aircraft
// &2 = crashes easily on small airports
const byte _aircraft_subtype[NUM_AIRCRAFT_ENGINES] = {
	1,  1,  3,  3,  1,  3,  1,  3,
	3,  3,  3,  3,  3,  3,  3,  3,
	3,  3,  3,  3,  3,  3,  1,  1,
	3,  3,  3,  3,  3,  3,  3,  3,
	3,  1,  1,  1,  3,  3,  0,  0,
	0
};

const byte _aircraft_acceleration[NUM_AIRCRAFT_ENGINES] = {
	18, 20, 35, 50, 20, 40, 35, 40,
	40, 40, 35, 40, 40, 40, 40, 40,
	40, 40, 40, 40, 40, 40, 50, 40,
	40, 40, 40, 40, 40, 40, 40, 40,
	50, 18, 20, 40, 40, 50, 20, 20,
	20,
};

void DrawAircraftEngine(int x, int y, int engine, uint32 image_ormod)
{
	DrawSprite((6 + _aircraft_sprite[_aircraft_table_3[engine - AIRCRAFT_ENGINES_INDEX]]) | image_ormod, x, y);

	if ((_aircraft_subtype[engine - AIRCRAFT_ENGINES_INDEX]&1) == 0)
		DrawSprite(0xF3D, x, y-5);
}

void DrawAircraftEngineInfo(int engine, int x, int y, int maxw)
{
	engine -= AIRCRAFT_ENGINES_INDEX;

	SET_DPARAM32(0, ((_price.aircraft_base >> 3) * _aircraft_cost_table[engine]) >> 5);
	SET_DPARAM16(1, _aircraft_speed[engine] << 3);
	SET_DPARAM16(2, _aircraft_num_pass[engine]);
	SET_DPARAM16(3, _aircraft_num_mail[engine]);
	SET_DPARAM32(4, _aircraft_running_cost[engine] * _price.aircraft_running >> 8);

	DrawStringMultiCenter(x, y, STR_A02E_COST_MAX_SPEED_CAPACITY, maxw);
}

/* Allocate many vehicles */
bool AllocateVehicles(Vehicle **vl, int num)
{
	int i;
	Vehicle *v;
	bool success = true;

	for(i=0; i!=num; i++) {
		vl[i] = v = AllocateVehicle();
		if (v == NULL) {
			success = false;
			break;
		}
		v->type = 1;
	}

	while (--i >= 0) {
		vl[i]->type = 0;
	}

	return success;
}

static int32 EstimateAircraftCost(uint16 engine_type)
{
	return _aircraft_cost_table[engine_type - AIRCRAFT_ENGINES_INDEX] * (_price.aircraft_base>>3)>>5;
}


/* p1 = engine */
int32 CmdBuildAircraft(int x, int y, uint32 flags, uint32 p1, uint32 p2)
{
	int32 value;
	Vehicle *vl[3], *v, *u, *w;
	byte unit_num;
	uint tile = TILE_FROM_XY(x,y);
	Engine *e;

	SET_EXPENSES_TYPE(EXPENSES_NEW_VEHICLES);

	value = EstimateAircraftCost(p1);

	if (flags & DC_QUERY_COST)
		return value;

	// allocate 2 or 3 vehicle structs, depending on type
	if (!AllocateVehicles(vl, (_aircraft_subtype[p1 - AIRCRAFT_ENGINES_INDEX]&1) == 0 ? 3 : 2) ||
			_ptr_to_next_order >= endof(_order_array))
					return_cmd_error(STR_00E1_TOO_MANY_VEHICLES_IN_GAME);

	unit_num = GetFreeUnitNumber(VEH_Aircraft);
	if (unit_num > _patches.max_aircraft)
		return_cmd_error(STR_00E1_TOO_MANY_VEHICLES_IN_GAME);

	if (flags & DC_EXEC) {
		v = vl[0];
		u = vl[1];

		v->unitnumber = unit_num;
		v->type = u->type = VEH_Aircraft;
		v->direction = 3;

		v->owner = u->owner = _current_player;

		v->tile = tile;
//		u->tile = 0;

		x = GET_TILE_X(tile)*16 + 5;
		y = GET_TILE_Y(tile)*16 + 3;

		v->x_pos = u->x_pos = x;
		v->y_pos = u->y_pos = y;

		u->z_pos = GetSlopeZ(x, y);
		v->z_pos = u->z_pos + 1;

		v->x_offs = v->y_offs = -1;
//		u->delta_x = u->delta_y = 0;

		v->sprite_width = v->sprite_height = 2;
		v->z_height = 5;

		u->sprite_width = u->sprite_height = 2;
		u->z_height = 1;

		v->vehstatus = VS_HIDDEN | VS_STOPPED | VS_DEFPAL;
		u->vehstatus = VS_HIDDEN | VS_UNCLICKABLE | VS_DISASTER;

		v->spritenum = _aircraft_table_3[p1 - AIRCRAFT_ENGINES_INDEX];
//		v->cargo_count = u->number_of_pieces = 0;

		v->cargo_cap = _aircraft_num_pass[p1 - AIRCRAFT_ENGINES_INDEX];
		u->cargo_cap = _aircraft_num_mail[p1 - AIRCRAFT_ENGINES_INDEX];

		v->cargo_type = CT_PASSENGERS;
		u->cargo_type = CT_MAIL;

		v->string_id = STR_SV_AIRCRAFT_NAME;
//		v->next_order_param = v->next_order = 0;

//		v->load_unload_time_rem = 0;
//		v->progress = 0;
		v->last_station_visited = 0xFF;
//		v->destination_coords = 0;

		v->max_speed = _aircraft_speed[p1 - AIRCRAFT_ENGINES_INDEX];
		v->acceleration = _aircraft_acceleration[p1 - AIRCRAFT_ENGINES_INDEX];
		v->engine_type = (byte)p1;

		v->subtype = (_aircraft_subtype[p1 - AIRCRAFT_ENGINES_INDEX]&1) == 0 ? 0 : 2;
		v->value = value;

		u->subtype = 4;

		e = &_engines[p1];
		v->reliability = e->reliability;
		v->reliability_spd_dec = e->reliability_spd_dec;
		v->max_age = e->lifelength * 366;

		_new_aircraft_id = v->index;

		*(v->schedule_ptr = _ptr_to_next_order++) = 0;
		// the AI doesn't click on a tile to build airplanes, so the below code will
		// never work. Therefore just assume the AI's planes always come from Hangar0
		v->u.air.pos = (_is_ai_player) ? 0:MAX_ELEMENTS;

		/* When we click on hangar we know the tile (it is in var 'tile')it is on. By that we know 
			its position in the array of depots the airport has.....we can search 
			->layout for #th position of depot. Since layout must start with depots, it is simple
		*/
		{
			Station *st;
			const AirportFTAClass *Airport;
			const uint16 *cur_depot;
			byte i = 0;
			st = DEREF_STATION(_map2[tile]);
			Airport = GetAirport(st->airport_type);
			// first element of depot array contains #of depots on the airport
			for (cur_depot=&Airport->airport_depots[1]; i != Airport->airport_depots[0]; cur_depot++) {
				if ((uint)(st->airport_tile + *cur_depot) == tile) {
					assert(Airport->layout[i].heading == HANGAR);
					v->u.air.pos = Airport->layout[i].position;
					break;
				}
				i++;
			}
			// to ensure v->u.air.pos has been given a value
			assert(v->u.air.pos != MAX_ELEMENTS);
		}

		v->u.air.state = HANGAR;
		v->u.air.previous_pos = v->u.air.pos;
		v->u.air.targetairport = _map2[tile];
		v->next = u;

		v->service_interval = _patches.servint_aircraft;

		v->date_of_last_service = _date;
		v->build_year = _cur_year;

		v->cur_image = u->cur_image = 0xEA0;

		VehiclePositionChanged(v);
		VehiclePositionChanged(u);

		// Aircraft with 3 vehicles?
		if (v->subtype == 0) {
			w = vl[2];

			u->next = w;

			w->type = VEH_Aircraft;
			w->direction = 0;
			w->owner = _current_player;
			w->x_pos = v->x_pos;
			w->y_pos = v->y_pos;
			w->z_pos = v->z_pos + 5;
			w->x_offs = w->y_offs = -1;
			w->sprite_width = w->sprite_height = 2;
			w->z_height = 1;
			w->vehstatus = VS_HIDDEN | VS_UNCLICKABLE;
			w->subtype = 6;
			w->cur_image = 0xF3D;
			VehiclePositionChanged(w);
		}

		InvalidateWindow(WC_VEHICLE_DEPOT, v->tile);
		InvalidateWindow(WC_AIRCRAFT_LIST, v->owner);
		InvalidateWindow(WC_COMPANY, v->owner);
	}

	return value;
}

bool IsAircraftHangarTile(TileIndex tile)
{
	// 0x56 - hangar facing other way international airport (86)
	// 0x20 - hangar large airport (32)
	// 0x41 - hangar small airport (65)
	return IS_TILETYPE(tile, MP_STATION) &&
				(_map5[tile] == 32 || _map5[tile] == 65 || _map5[tile] == 86);
}

static bool CheckStoppedInHangar(Vehicle *v)
{
	if (!(v->vehstatus&VS_STOPPED) ||
			!IsAircraftHangarTile(v->tile)) {
		_error_message = STR_A01B_AIRCRAFT_MUST_BE_STOPPED;
		return false;
	}

	return true;
}


void DoDeleteAircraft(Vehicle *v)
{
	DeleteWindowById(WC_VEHICLE_VIEW, v->index);
	InvalidateWindow(WC_AIRCRAFT_LIST, v->owner);
	InvalidateWindow(WC_COMPANY, v->owner);
	DeleteVehicleChain(v);
}

// p1 = vehicle
int32 CmdSellAircraft(int x, int y, uint32 flags, uint32 p1, uint32 p2)
{
	Vehicle *v;

	SET_EXPENSES_TYPE(EXPENSES_NEW_VEHICLES);

	v = &_vehicles[p1];

	if (!CheckOwnership(v->owner) || !CheckStoppedInHangar(v))
		return CMD_ERROR;

	if (flags & DC_EXEC) {
		// Invalidate depot
		InvalidateWindow(WC_VEHICLE_DEPOT, v->tile);

		DoDeleteAircraft(v);

	}

	return -(int32)v->value;
}

// p1 = vehicle
int32 CmdStartStopAircraft(int x, int y, uint32 flags, uint32 p1, uint32 p2)
{
	Vehicle *v;

	v = &_vehicles[p1];

	if (!CheckOwnership(v->owner))
		return CMD_ERROR;

	// cannot stop airplane when in flight, or when taking off / landing
	if (v->u.air.state >= STARTTAKEOFF) {
		return_cmd_error(STR_A017_AIRCRAFT_IS_IN_FLIGHT);
	}

	if (flags & DC_EXEC) {
		v->vehstatus ^= VS_STOPPED;
		InvalidateWindowWidget(WC_VEHICLE_VIEW, v->index, 4);
		InvalidateWindow(WC_VEHICLE_DEPOT, v->tile);
	}

	return 0;
}

// p1 = vehicle
int32 CmdSendAircraftToHangar(int x, int y, uint32 flags, uint32 p1, uint32 p2)
{
	Vehicle *v;
	Station *st;

	v = &_vehicles[p1];

	if (!CheckOwnership(v->owner))
		return CMD_ERROR;

	if ((v->next_order&OT_MASK) == OT_GOTO_DEPOT) {
		if (flags & DC_EXEC) {
 			if (v->next_order&OF_UNLOAD)
 				{ v->cur_order_index++; }
			v->next_order = OT_DUMMY;
			InvalidateWindowWidget(WC_VEHICLE_VIEW, v->index, 4);
		}
	} else {
		st = DEREF_STATION(v->u.air.targetairport);
		// If an airport doesn't have terminals (so no landing space for airports),
		// it surely doesn't have any hangars
		if (st->xy == 0 || st->airport_tile == 0 || GetAirport(st->airport_type)->nofterminals == 0)
					return CMD_ERROR;

		if (flags & DC_EXEC) {
			v->next_order = OF_NON_STOP | OF_FULL_LOAD | OT_GOTO_DEPOT;
			v->next_order_param = v->u.air.targetairport;
			InvalidateWindowWidget(WC_VEHICLE_VIEW, v->index, 4);
		}
	}

	return 0;
}

// p1 = vehicle
// p2 = new service int
int32 CmdChangeAircraftServiceInt(int x, int y, uint32 flags, uint32 p1, uint32 p2)
{
	Vehicle *v;

	v = &_vehicles[p1];

	if (!CheckOwnership(v->owner))
		return CMD_ERROR;

	if (flags & DC_EXEC) {
		v->service_interval = (uint16)p2;
		InvalidateWindowWidget(WC_VEHICLE_DETAILS, v->index, 7);
	}

	return 0;
}

// p1 = vehicle
// p2 = new cargo type
int32 CmdRefitAircraft(int x, int y, uint32 flags, uint32 p1, uint32 p2)
{
	Vehicle *v,*u;
	int pass, mail;
	int32 cost;

	SET_EXPENSES_TYPE(EXPENSES_AIRCRAFT_RUN);

	v = &_vehicles[p1];
	if (!CheckOwnership(v->owner) || !CheckStoppedInHangar(v))
		return CMD_ERROR;

	pass = _aircraft_num_pass[v->engine_type - AIRCRAFT_ENGINES_INDEX];
	if (p2 != 0) {
		pass >>= 1;
		if (p2 != 5)
			pass >>= 1;
	}
	_aircraft_refit_capacity = pass;

	cost = 0;
	if (IS_HUMAN_PLAYER(v->owner) && (byte)p2 != v->cargo_type) {
		cost = _price.aircraft_base >> 7;
	}

	if (flags & DC_EXEC) {
		v->cargo_cap = pass;

		u = v->next;
		mail = _aircraft_num_mail[v->engine_type - AIRCRAFT_ENGINES_INDEX];
		if (p2 != 0) {
			mail = 0;
		}
		u->cargo_cap = mail;
		v->cargo_count = u->cargo_count = 0;
		v->cargo_type = (byte)p2;
		InvalidateWindow(WC_VEHICLE_DETAILS, v->index);
	}

	return cost;
}

void HandleClickOnAircraft(Vehicle *v)
{
	ShowAircraftViewWindow(v);
}

static void CheckIfAircraftNeedsService(Vehicle *v)
{
	Station *st;

	if (v->date_of_last_service + v->service_interval > _date)
		return;

	if (v->vehstatus & VS_STOPPED)
		return;

	if ((v->next_order & (OT_MASK | OF_FULL_LOAD)) == (OT_GOTO_DEPOT | OF_FULL_LOAD))
		return;

 	if (_patches.gotodepot && IS_HUMAN_PLAYER(v->owner) && ScheduleHasDepotOrders(v->schedule_ptr))
 		return;
 
	st = DEREF_STATION(v->next_order_param);
	// only goto depot if the target airport has terminals (eg. it is airport)
	if (st->xy != 0 && st->airport_tile != 0 && GetAirport(st->airport_type)->nofterminals != 0) {
//		printf("targetairport = %d, st->index = %d\n", v->u.air.targetairport, st->index);
//		v->u.air.targetairport = st->index;
		v->next_order = OF_NON_STOP | OT_GOTO_DEPOT;
		InvalidateWindowWidget(WC_VEHICLE_VIEW, v->index, 4);
	} else if ((v->next_order & OT_MASK) == OT_GOTO_DEPOT) {
		v->next_order = OT_DUMMY;
		InvalidateWindowWidget(WC_VEHICLE_VIEW, v->index, 4);
	}
}

void OnNewDay_Aircraft(Vehicle *v)
{
	int32 cost;

	if (v->subtype > 2)
		return;

	if ((++v->day_counter & 7) == 0)
		DecreaseVehicleValue(v);

	CheckVehicleBreakdown(v);
	AgeVehicle(v);
	CheckIfAircraftNeedsService(v);

	if (v->vehstatus & VS_STOPPED)
		return;

	cost = _aircraft_running_cost[v->engine_type - AIRCRAFT_ENGINES_INDEX] * _price.aircraft_running / 364;

	v->profit_this_year -= cost >> 8;

	SET_EXPENSES_TYPE(EXPENSES_AIRCRAFT_RUN);
	SubtractMoneyFromPlayerFract(v->owner, cost);

	InvalidateWindow(WC_VEHICLE_DETAILS, v->index);
	InvalidateWindow(WC_AIRCRAFT_LIST, v->owner);
}

void AircraftYearlyLoop()
{
	Vehicle *v;

	FOR_ALL_VEHICLES(v) {
		if (v->type == VEH_Aircraft && v->subtype <= 2) {
			v->profit_last_year = v->profit_this_year;
			v->profit_this_year = 0;
			InvalidateWindow(WC_VEHICLE_DETAILS, v->index);
		}
	}
}

static void AgeAircraftCargo(Vehicle *v)
{
	if (_age_cargo_skip_counter != 0)
		return;

	do {
		if (v->cargo_days != 0xFF)
			v->cargo_days++;
	} while ( (v=v->next) != NULL );
}

static void HelicopterTickHandler(Vehicle *v)
{
	Vehicle *u;
	int tick,spd;
	uint16 img;

	u = v->next->next;

	if (u->vehstatus & VS_HIDDEN)
		return;

	// if true, helicopter rotors do not rotate. This should only be the case if a helicopter is 
	// loading/unloading at a terminal or stopped
	if ((v->next_order&OT_MASK) == OT_LOADING || (v->vehstatus&VS_STOPPED)) {
		if (u->cur_speed != 0) {
			u->cur_speed++;
			if (u->cur_speed >= 0x80 && u->cur_image == 0xF40) {
				u->cur_speed = 0;
			}
		}
	} else {
		if (u->cur_speed == 0)
			u->cur_speed = 0x70;

		if (u->cur_speed >= 0x50)
			u->cur_speed--;
	}

	tick = ++u->tick_counter;
	spd = u->cur_speed >> 4;

	if (spd == 0) {
		img = 0xF3D;
		if (u->cur_image == img)
			return;
	} else if (tick >= spd) {
		u->tick_counter = 0;
		img = u->cur_image + 1;
		if (img > 0xF40)
			img = 0xF3E;
	} else
		return;

	u->cur_image=img;

	BeginVehicleMove(u);
	VehiclePositionChanged(u);
	EndVehicleMove(u);
}

static void SetAircraftPosition(Vehicle *v, int x, int y, int z)
{
	Vehicle *u;
	int yt;

	v->x_pos = x;
	v->y_pos = y;
	v->z_pos = z;

	v->cur_image = GetAircraftImage(v, v->direction);

	BeginVehicleMove(v);
	VehiclePositionChanged(v);
	EndVehicleMove(v);

	u = v->next;

	yt = y - ((v->z_pos-GetSlopeZ(x, y-1)) >> 3);
	u->x_pos = x;
	u->y_pos = yt;
	u->z_pos = GetSlopeZ(x,yt);
	u->cur_image = v->cur_image;

	BeginVehicleMove(u);
	VehiclePositionChanged(u);
	EndVehicleMove(u);

	if ((u=u->next) != NULL) {
		u->x_pos = x;
		u->y_pos = y;
		u->z_pos = z + 5;

		BeginVehicleMove(u);
		VehiclePositionChanged(u);
		EndVehicleMove(u);
	}
}

static void ServiceAircraft(Vehicle *v)
{
	Vehicle *u;

	v->cur_speed = 0;
	v->subspeed = 0;
	v->progress = 0;
	v->vehstatus |= VS_HIDDEN;

	u = v->next;
	u->vehstatus |= VS_HIDDEN;
	if ((u=u->next) != NULL) {
		u->vehstatus |= VS_HIDDEN;
		u->cur_speed = 0;
	}

	SetAircraftPosition(v, v->x_pos, v->y_pos, v->z_pos);
	InvalidateWindow(WC_VEHICLE_DEPOT, v->tile);

	v->date_of_last_service = _date;
	v->breakdowns_since_last_service = 0;
	v->reliability = _engines[v->engine_type].reliability;
	InvalidateWindow(WC_VEHICLE_DETAILS, v->index);
}

static void PlayAircraftSound(Vehicle *v)
{
	SndPlayVehicleFx(_aircraft_sounds[v->engine_type - AIRCRAFT_ENGINES_INDEX], v);
}

static bool UpdateAircraftSpeed(Vehicle *v)
{
	uint spd = v->acceleration * 2;
	byte t;

	v->subspeed = (t=v->subspeed) + (byte)spd;
	spd = min( v->cur_speed + (spd >> 8) + (v->subspeed < t), v->max_speed);
		   
	// adjust speed for broken vehicles
	if(v->vehstatus&VS_AIRCRAFT_BROKEN) spd = min(spd, 27);

	//updates statusbar only if speed have changed to save CPU time
	if (spd != v->cur_speed) {
		v->cur_speed = spd;
		if (_patches.vehicle_speed)
			InvalidateWindowWidget(WC_VEHICLE_VIEW, v->index, 4);
	}

	if (!(v->direction & 1)) {
		spd = spd * 3 >> 2;
	}

	if (spd == 0)
		return false;

	if ((byte)++spd == 0)
		return true;

	v->progress = (t = v->progress) - (byte)spd;

	return (t < v->progress);
}

// get Aircraft running altitude
static byte GetAircraftFlyingAltitude(const Vehicle *v)
{
	byte maxz = 162;
	if (v->max_speed != 37) {
		maxz = 171;
		if (v->max_speed != 74) {maxz = 180;}
	}
	return maxz;
}

static bool Aircraft_5(Vehicle *v)
{
	Station *st;
	const AirportMovingData *amd;
	Vehicle *u;
	byte z,dirdiff,newdir,maxz,curz;
	GetNewVehiclePosResult gp;
	uint dist;
	int x,y;

	st = DEREF_STATION(v->u.air.targetairport);

	// prevent going to 0,0 if airport is deleted.
	{
		uint tile = st->airport_tile;
		if (tile == 0) tile = st->xy;
		// xy of destination
		x = GET_TILE_X(tile)*16;
		y = GET_TILE_Y(tile)*16;
	}

	// get airport moving data
	assert(v->u.air.pos < GetAirport(st->airport_type)->nofelements);
	amd = &_airport_moving_datas[st->airport_type][v->u.air.pos];

	// Helicopter raise
	if (amd->flag & AMED_HELI_RAISE) {
		u = v->next->next;

		// Make sure the rotors don't rotate too fast
		if (u->cur_speed > 32) {
			v->cur_speed = 0;
			if (--u->cur_speed == 32) {
				SndPlayVehicleFx(0x16, v);
			}
		} else {
			u->cur_speed = 32;
			if (UpdateAircraftSpeed(v)) {
				v->tile = 0;

				// Reached altitude?
				if (v->z_pos >= 184) {
					v->cur_speed = 0;
					return true;
				}
				SetAircraftPosition(v, v->x_pos, v->y_pos, v->z_pos+1);
			}
		}
		return false;
	}

	// Helicopter landing.
	if (amd->flag & AMED_HELI_LOWER) {
		if (UpdateAircraftSpeed(v)) {
			if (st->airport_tile == 0) {
				// FIXME - Aircraft_5 -> if station no longer exists, do not land
				// helicopter will circle until sign disappears, then go to next order
				// * what to do when it is the only order left, right now it just stays in 1 place
				v->u.air.state = FLYING;
				AircraftNextAirportPos_and_Order(v);
				return false;
			}

			// Vehicle is now at the airport.
			v->tile = st->airport_tile;

			// Find altitude of landing position.
			z = GetSlopeZ(x, y) + 1;
			if (st->airport_type == AT_OILRIG) z += 54;
			if (st->airport_type == AT_HELIPORT) z += 60;

			if (z == v->z_pos) {
				u = v->next->next;

				// Increase speed of rotors. When speed is 80, we've landed.
				if (u->cur_speed >= 80)
					return true;
				u->cur_speed+=4;
			} else if (v->z_pos > z) {
				SetAircraftPosition(v, v->x_pos, v->y_pos, v->z_pos-1);
			} else {
				SetAircraftPosition(v, v->x_pos, v->y_pos, v->z_pos+1);
			}
		}
		return false;
	}

	// Get distance from destination pos to current pos.
	dist = myabs(x + amd->x - v->x_pos) +  myabs(y + amd->y - v->y_pos);

	// Need exact position?
	if (!(amd->flag & AMED_EXACTPOS) && dist <= (uint)((amd->flag&AMED_SLOWTURN)?8:4))
		return true;

	// At final pos?
	if (dist == 0) {

		// Clamp speed to 12.
		if (v->cur_speed > 12)
			v->cur_speed = 12;

		// Change direction smoothly to final direction.
		dirdiff = amd->direction - v->direction;
		// if distance is 0, and plane points in right direction, no point in calling 
		// UpdateAircraftSpeed(). So do it only afterwards
		if (dirdiff == 0) {
			v->cur_speed = 0;
			return true;
		}

		if (!UpdateAircraftSpeed(v))
			return false;

		v->direction = (v->direction+((dirdiff&7)<5?1:-1)) & 7;
		v->cur_speed >>= 1;

		SetAircraftPosition(v, v->x_pos, v->y_pos, v->z_pos);
		return false;
	}

	// Clamp speed?
	if (!(amd->flag & AMED_NOSPDCLAMP) && v->cur_speed > 12)
		v->cur_speed = 12;

	if (!UpdateAircraftSpeed(v))
		return false;

	// Decrease animation counter.
	if (v->load_unload_time_rem != 0)
		v->load_unload_time_rem--;

	// Turn. Do it slowly if in the air.
	newdir = GetDirectionTowards(v, x + amd->x, y + amd->y);
	if (newdir != v->direction) {
		if (amd->flag & AMED_SLOWTURN) {
			if (v->load_unload_time_rem == 0) {
				v->load_unload_time_rem = 8;
				v->direction = newdir;
			}
		} else {
			v->cur_speed >>= 1;
			v->direction = newdir;
		}
	}

	// Move vehicle.
	GetNewVehiclePos(v, &gp);
	v->tile = gp.new_tile;

	// If vehicle is in the air, use tile coordinate 0.
	if (amd->flag & (AMED_TAKEOFF | AMED_SLOWTURN | AMED_LAND)) {
		v->tile = 0;
	}

	// Adjust Z for land or takeoff?
	z = v->z_pos;

	if (amd->flag & AMED_TAKEOFF) {
		z+=2;
		// Determine running altitude
		maxz = GetAircraftFlyingAltitude(v);
		if (z > maxz)
			z = maxz;
	}

	if (amd->flag & AMED_LAND) {
		if (st->airport_tile == 0) {
			//FIXME -- FIXED - Aircraft_5 -> if station no longer exists, do not land
			// * what to do when it is the only order left, right now it just stays in 1 place?
			v->u.air.state = FLYING;
			AircraftNextAirportPos_and_Order(v);
			// get aircraft back on running altitude
			SetAircraftPosition(v, gp.x, gp.y, GetAircraftFlyingAltitude(v));
			return false;
		}

		curz = GetSlopeZ(x, y) + 1;

		if (curz > z) {
			z++;
		} else {
			int t = max(1, dist-4);

			z -= ((z - curz) + t - 1) / t;
			if (z < curz) z = curz;
		}
	}

	// We've landed. Decrase speed when we're reaching end of runway.
	if (amd->flag & AMED_BRAKE) {
		curz = GetSlopeZ(x, y) + 1;

		if (z > curz) z--;
		else if (z < curz) z++;

		if (dist < 64 && v->cur_speed > 12)
			v->cur_speed -= 4;
	}

	SetAircraftPosition(v, gp.x, gp.y, z);
	return false;
}

static const int8 _crashed_aircraft_moddir[4] = {
	-1,0,0,1
};

static void HandleCrashedAircraft(Vehicle *v)
{
	uint32 r;
	Station *st;

	v->u.air.crashed_counter++;

	if (v->u.air.crashed_counter < 650) {
		if (CHANCE16R(1,32,r)) {
			v->direction = (v->direction+_crashed_aircraft_moddir[(r >> 16)&3]) & 7;
			SetAircraftPosition(v, v->x_pos, v->y_pos, v->z_pos);
			r = Random();
			CreateEffectVehicleRel(v,
				4 + (r&0xF),
				4 + ((r>>4)&0xF),
				((r>>8)&0xF),
				EV_DEMOLISH);
		}
	} else if (v->u.air.crashed_counter >= 10000) {
		st = DEREF_STATION(v->u.air.targetairport);
		// remove rubble of crashed airplane

		// clear runway-in on all airports, set by crashing plane
		// small airports use AIRPORT_BUSY, city airports use RUNWAY_IN_OUT_block, etc.
		// but they all share the same number
		CLRBITS(st->airport_flags, RUNWAY_IN_block);

		BeginVehicleMove(v);
		EndVehicleMove(v);

		DoDeleteAircraft(v);
	}
}

static void HandleBrokenAircraft(Vehicle *v)
{
	if (v->breakdown_ctr != 1) {
		v->breakdown_ctr = 1;
		v->vehstatus |= VS_AIRCRAFT_BROKEN;

		if (v->breakdowns_since_last_service != 255)
			v->breakdowns_since_last_service++;
		InvalidateWindow(WC_VEHICLE_VIEW, v->index);
		InvalidateWindow(WC_VEHICLE_DETAILS, v->index);
	}
}

static const int8 _aircraft_smoke_xy[16] = {
	5,6,5,0,-5,-6,-5,0, /* x coordinates */
	5,0,-5,-6,-5,0,5,6, /* y coordinate */
};

static void HandleAircraftSmoke(Vehicle *v)
{
	if (!(v->vehstatus&VS_AIRCRAFT_BROKEN))
		return;

	if (v->cur_speed < 10) {
		v->vehstatus &= ~VS_AIRCRAFT_BROKEN;
		v->breakdown_ctr = 0;
		return;
	}

	if ((v->tick_counter & 0x1F) == 0) {
		CreateEffectVehicleRel(v,
			_aircraft_smoke_xy[v->direction],
			_aircraft_smoke_xy[v->direction + 8],
			2,
			EV_SMOKE_3
		);
	}
}

static void ProcessAircraftOrder(Vehicle *v)
{
	uint order;

	// OT_GOTO_DEPOT, OT_LOADING
 	if ((v->next_order & OT_MASK) >= OT_GOTO_DEPOT && (v->next_order & OT_MASK) <= OT_LOADING) {
 		if ((v->next_order & (OT_MASK|OF_UNLOAD)) != (OT_GOTO_DEPOT|OF_UNLOAD))
 			return;
 	}
 
 	if ((v->next_order & (OT_MASK|OF_UNLOAD|OF_FULL_LOAD)) == (OT_GOTO_DEPOT|OF_UNLOAD|OF_FULL_LOAD) &&
 		v->date_of_last_service+v->service_interval > _date) {
 		v->cur_order_index++;
 	}

	if (v->cur_order_index >= v->num_orders)
		v->cur_order_index = 0;

	order = v->schedule_ptr[v->cur_order_index];

	if (order == 0) {
		v->next_order = OT_NOTHING;
		return;
	}

	if (order == (uint)((v->next_order | (v->next_order_param<<8))))
		return;

	v->next_order = (byte)order;
	v->next_order_param = (byte)(order >> 8);

	// orders are changed in flight, ensure going to the right station
	if ((order & OT_MASK) == OT_GOTO_STATION && v->u.air.state == FLYING) {
		AircraftNextAirportPos_and_Order(v);
		v->u.air.targetairport = order >> 8;
	}

	InvalidateVehicleOrderWidget(v);
}

static void HandleAircraftLoading(Vehicle *v, int mode)
{
	if (v->next_order == OT_NOTHING)
		return;

	if (v->next_order != OT_DUMMY) {
		if ((v->next_order&OT_MASK) != OT_LOADING)
			return;

		if (mode != 0)
			return;

		if (--v->load_unload_time_rem)
			return;

		if (v->next_order&OF_FULL_LOAD && CanFillVehicle(v)) {
			SET_EXPENSES_TYPE(EXPENSES_AIRCRAFT_INC);
			LoadUnloadVehicle(v);
			return;
		}

		{
			byte b = v->next_order;
			v->next_order = OT_NOTHING;
			if (!(b & OF_NON_STOP))
				return;
		}
	}
	v->cur_order_index++;
	InvalidateVehicleOrderWidget(v);
}

static void MaybeCrashAirplane(Vehicle *v)
{
	Station *st;
	uint16 prob;
	int i;
	uint16 amt;

	st = DEREF_STATION(v->u.air.targetairport);

	//FIXME -- MaybeCrashAirplane -> increase crashing chances of very modern airplanes on smaller than AT_METROPOLITAN airports
	prob = 0x10000 / 1500;
	if (st->airport_type == AT_SMALL && (_aircraft_subtype[v->engine_type - AIRCRAFT_ENGINES_INDEX]&2)) {
		prob = 0x10000 / 20;
	}

	if ((uint16)Random() > prob)
		return;

	// Crash the airplane. Remove all goods stored at the station.
	for(i=0; i!=NUM_CARGO; i++) {
		st->goods[i].rating = 1;
		st->goods[i].waiting_acceptance &= ~0xFFF;
	}

	v->vehstatus |= VS_CRASHED;
	v->u.air.crashed_counter = 0;

	CreateEffectVehicleRel(v, 4, 4, 8, EV_CRASHED_SMOKE);

	InvalidateWindow(WC_VEHICLE_VIEW, v->index);

	amt = 2;
	if (v->cargo_type == CT_PASSENGERS) amt += v->cargo_count;
	SET_DPARAM16(0, amt);

	v->cargo_count = 0;
	v->next->cargo_count = 0,

	SET_DPARAM16(1, st->index);
	AddNewsItem(STR_A034_PLANE_CRASH_DIE_IN_FIREBALL,
		NEWS_FLAGS(NM_THIN, NF_VIEWPORT|NF_VEHICLE, NT_ACCIDENT, 0),
		v->index,
		0);

	ModifyStationRatingAround(TILE_FROM_XY(v->x_pos, v->y_pos), v->owner, -160, 30);
	SndPlayVehicleFx(16, v);
}

// we've landed and just arrived at a terminal
static void AircraftEntersTerminal(Vehicle *v)
{
	Station *st;
	byte old_order;

	if ((v->next_order & OT_MASK) == OT_GOTO_DEPOT)
		return;

	st = DEREF_STATION(v->u.air.targetairport);
	v->last_station_visited = v->u.air.targetairport;

	/* Check if station was ever visited before */
	if (!(st->had_vehicle_of_type & HVOT_AIRCRAFT)) {
		uint32 flags;

		st->had_vehicle_of_type |= HVOT_AIRCRAFT;
		SET_DPARAM16(0, st->index);
		// show newsitem of celebrating citizens
		flags = (v->owner == _local_player) ? NEWS_FLAGS(NM_THIN, NF_VIEWPORT|NF_VEHICLE, NT_ARRIVAL_PLAYER, 0) : NEWS_FLAGS(NM_THIN, NF_VIEWPORT|NF_VEHICLE, NT_ARRIVAL_OTHER, 0);
		AddNewsItem(
			STR_A033_CITIZENS_CELEBRATE_FIRST,
			flags,
			v->index,
			0);
	}

	old_order = v->next_order;
	v->next_order = OT_LOADING;

	if ((old_order & OT_MASK) == OT_GOTO_STATION &&
			v->next_order_param == v->last_station_visited) {
		v->next_order = OT_LOADING | (old_order & (OF_UNLOAD|OF_FULL_LOAD)) | OF_NON_STOP;
	}

	SET_EXPENSES_TYPE(EXPENSES_AIRCRAFT_INC);
	LoadUnloadVehicle(v);
	InvalidateWindowWidget(WC_VEHICLE_VIEW, v->index, 4);
}

static void AircraftEnterHangar(Vehicle *v)
{
	byte old_order;

	ServiceAircraft(v);
	
	MaybeRenewVehicle(v, EstimateAircraftCost(v->engine_type));

	if ((v->next_order & OT_MASK) == OT_GOTO_DEPOT) {
		InvalidateWindow(WC_VEHICLE_VIEW, v->index);

		old_order = v->next_order;
		v->next_order = OT_NOTHING;

 			if (old_order & OF_UNLOAD) { v->cur_order_index++; }
 
 			else if (old_order & OF_FULL_LOAD) { // force depot visit
			v->vehstatus |= VS_STOPPED;

			if (v->owner == _local_player) {
				SET_DPARAM16(0, v->unitnumber);
				AddNewsItem(
					STR_A014_AIRCRAFT_IS_WAITING_IN,
					NEWS_FLAGS(NM_SMALL, NF_VIEWPORT|NF_VEHICLE, NT_ADVICE, 0),
					v->index,
					0);
			}
		}
	}
}

static void AircraftLand(Vehicle *v)
{
	v->sprite_width = v->sprite_height = 2;
}

static void AircraftLandAirplane(Vehicle *v)
{
	AircraftLand(v);
	SndPlayVehicleFx(0x15, v);
	MaybeCrashAirplane(v);
}

// set the right pos when heading to other airports after takeoff
static void AircraftNextAirportPos_and_Order(Vehicle *v)
{
	Station *st;
	const AirportFTAClass *Airport;

	if ((v->next_order&OT_MASK) == OT_GOTO_STATION ||
			(v->next_order&OT_MASK) == OT_GOTO_DEPOT)
			v->u.air.targetairport = v->next_order_param;

	st = DEREF_STATION(v->u.air.targetairport);
	Airport = GetAirport(st->airport_type);
	v->u.air.pos = v->u.air.previous_pos = Airport->entry_point;
}

static void AircraftLeaveHangar(Vehicle *v)
{
	v->cur_speed = 0;
	v->subspeed = 0;
	v->progress = 0;
	v->direction = 3;
	v->vehstatus &= ~VS_HIDDEN;
	{
		Vehicle *u = v->next;
		u->vehstatus &= ~VS_HIDDEN;

		// Rotor blades
		if ((u=u->next) != NULL) {
			u->vehstatus &= ~VS_HIDDEN;
			u->cur_speed = 80;
		}
	}

	SetAircraftPosition(v, v->x_pos, v->y_pos, v->z_pos);
	InvalidateWindow(WC_VEHICLE_DEPOT, v->tile);
}


////////////////////////////////////////////////////////////////////////////////
///////////////////   AIRCRAFT MOVEMENT SCHEME  ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static void AircraftEventHandler_EnterTerminal(Vehicle *v, const AirportFTAClass *Airport)
{
	AircraftEntersTerminal(v);
	v->u.air.state = Airport->layout[v->u.air.pos].heading;
}

static void AircraftEventHandler_EnterHangar(Vehicle *v, const AirportFTAClass *Airport)
{
	AircraftEnterHangar(v);
	v->u.air.state = Airport->layout[v->u.air.pos].heading;
}

// In an Airport Hangar
static void AircraftEventHandler_InHangar(Vehicle *v, const AirportFTAClass *Airport)
{
	// if we just arrived, execute EnterHangar first
	if (v->u.air.previous_pos != v->u.air.pos) {
		AircraftEventHandler_EnterHangar(v, Airport);
		return;
	}

	if ((v->next_order&OT_MASK) == OT_GOTO_DEPOT && (v->vehstatus&VS_STOPPED)) { // if we were sent to the depot, stay there
		v->next_order = OT_NOTHING;
		return;
	}

	if ((v->next_order&OT_MASK) != OT_GOTO_STATION && (v->next_order&OT_MASK) != OT_GOTO_DEPOT)
		return;

	// if the block of the next position is busy, stay put
	if (AirportHasBlock(v, &Airport->layout[v->u.air.pos], Airport)) {return;}

	// We are already at the target airport, we need to find a terminal
	if (v->next_order_param == v->u.air.targetairport) {
		// FindFreeTerminal:
		// 1. Find a free terminal, 2. Occupy it, 3. Set the vehicle's state to that terminal
		if (v->subtype != 0) {if(!AirportFindFreeTerminal(v, Airport)) {return;}} // airplane
		else {if(!AirportFindFreeHelipad(v, Airport)) {return;}} // helicopter
	}
	else { // Else prepare for launch.
		// airplane goto state takeoff, helicopter to helitakeoff
		v->u.air.state = (v->subtype != 0) ? TAKEOFF : HELITAKEOFF;
	}
	AircraftLeaveHangar(v);
	AirportMove(v, Airport);
}

// At one of the Airport's Terminals
static void AircraftEventHandler_AtTerminal(Vehicle *v, const AirportFTAClass *Airport)
{
	// if we just arrived, execute EnterTerminal first
	if (v->u.air.previous_pos != v->u.air.pos) {
		AircraftEventHandler_EnterTerminal(v, Airport);
		// on an airport with helipads, a helicopter will always land there
		// and get serviced at the same time - patch setting
		if (_patches.serviceathelipad) {
			if (v->subtype == 0 && Airport->nofhelipads > 0) {
				// an exerpt of ServiceAircraft, without the invisibility stuff
				v->date_of_last_service = _date;
				v->breakdowns_since_last_service = 0;
				v->reliability = _engines[v->engine_type].reliability;
				InvalidateWindow(WC_VEHICLE_DETAILS, v->index);
			}
		}
		return;
	}

	// removed &0x1F
	if (v->next_order == OT_NOTHING) {return;}

	// if the block of the next position is busy, stay put
	if (AirportHasBlock(v, &Airport->layout[v->u.air.pos], Airport)) {
		return;
	}

	// airport-road is free. We either have to go to another airport, or to the hangar
	// ---> start moving

	switch (v->next_order&OT_MASK) {
		case OT_GOTO_STATION: // ready to fly to another airport
			// airplane goto state takeoff, helicopter to helitakeoff
			v->u.air.state = (v->subtype != 0) ? TAKEOFF : HELITAKEOFF;
			break;
		case OT_GOTO_DEPOT:   // visit hangar for serivicing, sale, etc.
			if (v->next_order_param == v->u.air.targetairport)
				v->u.air.state = HANGAR;
			else
				v->u.air.state = (v->subtype != 0) ? TAKEOFF : HELITAKEOFF;
			break;
		default:  // orders have been deleted (no orders), goto depot and don't bother us
			v->next_order  = OT_NOTHING;
			v->u.air.state = HANGAR;
	}
	AirportMove(v, Airport);
}

static void AircraftEventHandler_General(Vehicle *v, const AirportFTAClass *Airport)
{
	printf("OK, you shouldn't be here, check your Airport Scheme!\n");
	assert(1 == v->u.air.state); // when here state is 0, so this always fails
}

static void AircraftEventHandler_TakeOff(Vehicle *v, const AirportFTAClass *Airport) {
	PlayAircraftSound(v); // play takeoffsound for airplanes
	v->u.air.state = STARTTAKEOFF;
}

static void AircraftEventHandler_StartTakeOff(Vehicle *v, const AirportFTAClass *Airport)
{
	v->sprite_width = v->sprite_height = 24; // ??? no idea what this is
	v->u.air.state = ENDTAKEOFF;
}

static void AircraftEventHandler_EndTakeOff(Vehicle *v, const AirportFTAClass *Airport)
{
	v->u.air.state = FLYING;
	// get the next position to go to, differs per airport
	AircraftNextAirportPos_and_Order(v);
}

static void AircraftEventHandler_HeliTakeOff(Vehicle *v, const AirportFTAClass *Airport)
{
	v->sprite_width = v->sprite_height = 24; // ??? no idea what this is
	v->u.air.state = FLYING;
	// get the next position to go to, differs per airport
	AircraftNextAirportPos_and_Order(v);
}

static void AircraftEventHandler_Flying(Vehicle *v, const AirportFTAClass *Airport)
{
	Station *st;
	byte landingtype;
	AirportFTA *current;
	uint16 tcur_speed, tsubspeed;

	st = DEREF_STATION(v->u.air.targetairport);
	// flying device is accepted at this station
	// small airport --> no helicopters (AIRCRAFT_ONLY)
	// all other airports --> all types of flying devices (ALL)
	// heliport/oilrig, etc --> no airplanes (HELICOPTERS_ONLY)
	// runway busy or not allowed to use this airstation, circle
	if (! (v->subtype == Airport->acc_planes ||
			st->airport_tile == 0 || (st->owner != OWNER_NONE && st->owner != v->owner) )) {

		// {32,FLYING,NOTHING_block,37}, {32,LANDING,N,33}, {32,HELILANDING,N,41},
		// if it is an airplane, look for LANDING, for helicopter HELILANDING
		// it is possible to choose from multiple landing runways, so loop until a free one is found
		landingtype = (v->subtype != 0) ? LANDING : HELILANDING;
		current = Airport->layout[v->u.air.pos].next_in_chain;
		while (current != NULL) {
			if (current->heading == landingtype) {
				// save speed before, since if AirportHasBlock is false, it resets them to 0
				// we don't want that for plane in air
				// hack for speed thingie
				tcur_speed = v->cur_speed;
				tsubspeed = v->subspeed;
				if (!AirportHasBlock(v, current, Airport)) {
					v->u.air.state = landingtype; // LANDING / HELILANDING
					// it's a bit dirty, but I need to set position to next position, otherwise
					// if there are multiple runways, plane won't know which one it took (because
					// they all have heading LANDING). And also occupy that block!
					v->u.air.pos = current->next_position;
					SETBITS(st->airport_flags, Airport->layout[v->u.air.pos].block);
					return;
				}
				v->cur_speed = tcur_speed;
				v->subspeed = tsubspeed;
			}
			current = current->next_in_chain;
		}
	}
	v->u.air.state = FLYING;
	v->u.air.pos = Airport->layout[v->u.air.pos].next_position;
}

static void AircraftEventHandler_Landing(Vehicle *v, const AirportFTAClass *Airport)
{
	AircraftLandAirplane(v);  // maybe crash airplane
	v->u.air.state = ENDLANDING;
}

static void AircraftEventHandler_HeliLanding(Vehicle *v, const AirportFTAClass *Airport)
{
	AircraftLand(v); // helicopters don't crash
	v->u.air.state = HELIENDLANDING;
}

static void AircraftEventHandler_EndLanding(Vehicle *v, const AirportFTAClass *Airport)
{
	// next block busy, don't do a thing, just wait
	if(AirportHasBlock(v, &Airport->layout[v->u.air.pos], Airport)) {return;}

	// if going to terminal (OT_GOTO_STATION) choose one
	// 1. in case all terminals are busy AirportFindFreeTerminal() returns false or
	// 2. not going for terminal (but depot, no order),
	// --> get out of the way to the hangar.
	if ((v->next_order&OT_MASK) == OT_GOTO_STATION) {
		if (AirportFindFreeTerminal(v, Airport)) {return;}
	}
	v->u.air.state = HANGAR;

}

static void AircraftEventHandler_HeliEndLanding(Vehicle *v, const AirportFTAClass *Airport)
{
	// next block busy, don't do a thing, just wait
	if(AirportHasBlock(v, &Airport->layout[v->u.air.pos], Airport)) {return;}

	// if going to helipad (OT_GOTO_STATION) choose one. If airport doesn't have helipads, choose terminal
	// 1. in case all terminals/helipads are busy (AirportFindFreeHelipad() returns false) or
	// 2. not going for terminal (but depot, no order),
	// --> get out of the way to the hangar IF there are terminals on the airport.
	// --> else TAKEOFF
	// the reason behind this is that if an airport has a terminal, it also has a hangar. Airplanes
	// must go to a hangar.
	if ((v->next_order&OT_MASK) == OT_GOTO_STATION) {
		if (AirportFindFreeHelipad(v, Airport)) {return;}
	}
	v->u.air.state = (Airport->nofterminals != 0) ? HANGAR : HELITAKEOFF;
}

typedef void AircraftStateHandler(Vehicle *v, const AirportFTAClass *Airport);
static AircraftStateHandler * const _aircraft_state_handlers[] = {
	AircraftEventHandler_General,				// TO_ALL         =  0
	AircraftEventHandler_InHangar,			// HANGAR         =  1
	AircraftEventHandler_AtTerminal,		// TERM1          =  2
	AircraftEventHandler_AtTerminal,		// TERM2          =  3
	AircraftEventHandler_AtTerminal,		// TERM3          =  4
	AircraftEventHandler_AtTerminal,		// TERM4          =  5
	AircraftEventHandler_AtTerminal,		// TERM5          =  6
	AircraftEventHandler_AtTerminal,		// TERM6          =  7
	AircraftEventHandler_AtTerminal,		// HELIPAD1       =  8
	AircraftEventHandler_AtTerminal,		// HELIPAD2       =  9
	AircraftEventHandler_TakeOff,				// TAKEOFF        = 10
	AircraftEventHandler_StartTakeOff,	// STARTTAKEOFF   = 11
	AircraftEventHandler_EndTakeOff,		// ENDTAKEOFF     = 12
	AircraftEventHandler_HeliTakeOff,		// HELITAKEOFF    = 13
	AircraftEventHandler_Flying,				// FLYING         = 14
	AircraftEventHandler_Landing,				// LANDING        = 15
	AircraftEventHandler_EndLanding,		// ENDLANDING     = 16
	AircraftEventHandler_HeliLanding,		// HELILANDING    = 17
	AircraftEventHandler_HeliEndLanding,// HELIENDLANDING = 18
};

static void AirportClearBlock(Vehicle *v, const AirportFTAClass *Airport)
{
	Station *st;
	// we have left the previous block, and entered the new one. Free the previous block
	if (Airport->layout[v->u.air.previous_pos].block != Airport->layout[v->u.air.pos].block) {
		st = DEREF_STATION(v->u.air.targetairport);
		CLRBITS(st->airport_flags, Airport->layout[v->u.air.previous_pos].block);
	}
}

static void AirportGoToNextPosition(Vehicle *v, const AirportFTAClass *Airport)
{
	// if aircraft is not in position, wait until it is
	if (!Aircraft_5(v)) {return;}

	AirportClearBlock(v, Airport);
	AirportMove(v, Airport); // move aircraft to next position
}

// gets pos from vehicle and next orders
static bool AirportMove(Vehicle *v, const AirportFTAClass *Airport)
{
	AirportFTA *current;
	byte prev_pos;
	bool retval = false;

	// error handling
	if (v->u.air.pos >= Airport->nofelements) {
		printf("position %d is not valid for current airport. Max position is %d\n", v->u.air.pos, Airport->nofelements-1);
		assert(v->u.air.pos < Airport->nofelements);
	}

	current = &Airport->layout[v->u.air.pos];
	// we have arrived in an important state (eg terminal, hangar, etc.)
	if (current->heading == v->u.air.state) {
		prev_pos = v->u.air.pos; // location could be changed in state, so save it before-hand
		_aircraft_state_handlers[v->u.air.state](v, Airport);
		if (v->u.air.state != FLYING) {v->u.air.previous_pos = prev_pos;}
		return true;
	}

	v->u.air.previous_pos = v->u.air.pos; // save previous location

	// there is only one choice to move to
	if (current->next_in_chain == NULL) {
		if (AirportSetBlocks(v, current, Airport)) {
			v->u.air.pos = current->next_position;
		} // move to next position
		return retval;
	}

	// there are more choices to choose from, choose the one that
	// matches our heading
	do {
		if (v->u.air.state == current->heading || current->heading == TO_ALL) {
					if (AirportSetBlocks(v, current, Airport)) {
						v->u.air.pos = current->next_position;
					} // move to next position
					return retval;
		}
		current = current->next_in_chain;
	} while (current != NULL);

	printf("Cannot move further on Airport...! pos:%d state:%d\n", v->u.air.pos, v->u.air.state);
	printf("Airport entry point: %d, Vehicle: %d\n", Airport->entry_point, v->index);
	assert(0);
	return false;
}

// returns true if the road ahead is busy, eg. you must wait before proceeding
static bool AirportHasBlock(Vehicle *v, AirportFTA *current_pos, const AirportFTAClass *Airport)
{
	Station *st;
	uint32 airport_flags;
	AirportFTA *next, *reference;
	reference = &Airport->layout[v->u.air.pos];
	next = &Airport->layout[current_pos->next_position];

	// same block, then of course we can move
	if (Airport->layout[current_pos->position].block != next->block) {
		airport_flags = next->block;
		st = DEREF_STATION(v->u.air.targetairport);
		// check additional possible extra blocks
		if (current_pos != reference && current_pos->block != NOTHING_block) {
			airport_flags |= current_pos->block;
		}

		if (HASBITS(st->airport_flags, airport_flags)) {
			v->cur_speed = 0;
			v->subspeed = 0;
			return true;
		}
	}
	return false;
}

// returns true on success. Eg, next block was free and we have occupied it
static bool AirportSetBlocks(Vehicle *v, AirportFTA *current_pos, const AirportFTAClass *Airport)
{
	Station *st;
	uint32 airport_flags;
	AirportFTA *current, *reference, *next;
	next = &Airport->layout[current_pos->next_position];
	reference = &Airport->layout[v->u.air.pos];

	// if the next position is in another block, check it and wait until it is free
	if (Airport->layout[current_pos->position].block != next->block) {
		airport_flags = next->block;
		st = DEREF_STATION(v->u.air.targetairport);
		//search for all all elements in the list with the same state, and blocks != N
		// this means more blocks should be checked/set
		current = current_pos;
		if (current == reference) { current = current->next_in_chain;}
		while (current != NULL) {
			if (current->heading == current_pos->heading && current->block != 0) {
				airport_flags |= current->block;
				break;
			}
			current = current->next_in_chain;
		};

		// if the block to be checked is in the next position, then exclude that from
		// checking, because it has been set by the airplane before
		if (current_pos->block == next->block) {airport_flags ^= next->block;}

		if (HASBITS(st->airport_flags, airport_flags)) {
			v->cur_speed = 0;
			v->subspeed = 0;
			return false;
		}

		if (next->block != NOTHING_block) {
			SETBITS(st->airport_flags, airport_flags); // occupy next block
		}
	}
	return true;
}

static bool FreeTerminal(Vehicle *v, byte i, byte last_terminal)
{
	Station *st = DEREF_STATION(v->u.air.targetairport);
	for (; i < last_terminal; i++) {
		if (!HASBIT(st->airport_flags, i)) {
			// TERMINAL# HELIPAD#
			v->u.air.state = i + TERM1; // start moving to that terminal/helipad
			SETBIT(st->airport_flags, i); // occupy terminal/helipad
			return true;
		}
	}
	return false;
}

static bool AirportFindFreeTerminal(Vehicle *v, const AirportFTAClass *Airport) 
{
	byte nofterminalspergroup, i;
	AirportFTA *temp;
	Station *st;

	/* example of more terminalgroups
		{0,HANGAR,NOTHING_block,1}, {0,255,TERM_GROUP1_block,0}, {0,255,TERM_GROUP2_ENTER_block,1}, {0,0,N,1},
		Heading 255 denotes a group. We see 2 groups here:
		1. group 0 -- TERM_GROUP1_block (check block)
		2. group 1 -- TERM_GROUP2_ENTER_block (check block)
		First in line is checked first, group 0. If the block (TERM_GROUP1_block) is free, it 
		looks	at the corresponding terminals of that group. If no free ones are found, other 
		possible groups are checked	(in this case group 1, since that is after group 0). If that 
		fails, then attempt fails and plane waits
	*/
	if (Airport->nofterminalgroups > 1) {
		st = DEREF_STATION(v->u.air.targetairport);
		nofterminalspergroup = Airport->nofterminals / Airport->nofterminalgroups;
		temp = Airport->layout[v->u.air.pos].next_in_chain;
		while (temp != NULL) {
			if (temp->heading == 255) {
				if (!HASBITS(st->airport_flags, temp->block)) {
					i = temp->next_position * nofterminalspergroup; // next_position denotes the group to check
					// only that group will be checked (eg 6 terms, 2 groups)
					// with i = 0 terms 1 - 3 and
					// with i = 1 terms 4 - 6
					if (FreeTerminal(v, i, i + nofterminalspergroup)) {return true;}
				}
			}
			else {return false;} // once the heading isn't 255, we've exhausted the possible blocks. So we cannot move
			temp = temp->next_in_chain;
		}
	}

	// if there is only 1 terminalgroup, all terminals are checked (starting from 0 to max)
	return FreeTerminal(v, 0, Airport->nofterminals);
}

static bool AirportFindFreeHelipad(Vehicle *v, const AirportFTAClass *Airport) 
{
  Station *st;
  byte nofhelipadspergroup,  i;
  AirportFTA *temp;

	// if an airport doesn't have helipads, use terminals
	if (Airport->nofhelipads == 0) {return AirportFindFreeTerminal(v, Airport);}

	// if there are more helicoptergroups, pick one, just as in AirportFindFreeTerminal()
	if (Airport->nofhelipadgroups > 1) {
		st = DEREF_STATION(v->u.air.targetairport);
		nofhelipadspergroup = Airport->nofhelipads / Airport->nofhelipadgroups;
		temp = Airport->layout[v->u.air.pos].next_in_chain;
		while (temp != NULL) {
			if (temp->heading == 255) {
				if (!HASBITS(st->airport_flags, temp->block)) {
					i = temp->next_position * nofhelipadspergroup; // next position is the group to check
					// heliports start from after TERMINALS, so MAX_TERMINALS needs to be added
					if (FreeTerminal(v, i + MAX_TERMINALS, i + MAX_TERMINALS + nofhelipadspergroup)) {return true;}
				}
			}
			else {return false;} // once the heading isn't 255, we've exhausted the possible blocks. So we cannot move
			temp = temp->next_in_chain;
		}
	}
	// only 1 helicoptergroup, check all helipads
	// The blocks for helipads start after the last terminal (MAX_TERMINALS)
	else {return FreeTerminal(v, MAX_TERMINALS, Airport->nofhelipads + MAX_TERMINALS);}
	return false;	// it shouldn't get here anytime, but just to be sure
}

static void AircraftEventHandler(Vehicle *v, int loop)
{
	v->tick_counter++;

	if (v->vehstatus & VS_CRASHED) {
		HandleCrashedAircraft(v);
		return;
	}

	/* exit if aircraft is stopped */
	if (v->vehstatus & VS_STOPPED)
		return;

	/* aircraft is broken down? */
	if (v->breakdown_ctr != 0) {
		if (v->breakdown_ctr <= 2) {
			HandleBrokenAircraft(v);
		} else {
			v->breakdown_ctr--;
		}
	}

	HandleAircraftSmoke(v);
	ProcessAircraftOrder(v);
	HandleAircraftLoading(v, loop);

	if ((v->next_order&OT_MASK) >= OT_LOADING)
		return;

	// pass the right airport structure to the functions
	// DEREF_STATION gets target airport (Station *st), its type is passed to GetAirport
	// that returns the correct layout depending on type
	AirportGoToNextPosition(v, GetAirport(DEREF_STATION(v->u.air.targetairport)->airport_type));
}

void Aircraft_Tick(Vehicle *v)
{
	int i;

	if (v->subtype > 2)
		return;

	if (v->subtype == 0)
		HelicopterTickHandler(v);

	AgeAircraftCargo(v);

	for(i=0; i!=6; i++) {
		AircraftEventHandler(v, i);
		if (v->type != VEH_Aircraft) // In case it was deleted
			break;
	}
}

// need to be called to load aircraft from old version
void UpdateOldAircraft()
{
	Station *st;
	Vehicle *v_oldstyle;
	GetNewVehiclePosResult gp;

	// set airport_flags to 0 for all airports just to be sure
	FOR_ALL_STATIONS(st) {
		st->airport_flags = 0; // reset airport
		// type of oilrig has been moved, update it (3-5)
		if (st->airport_type == 3) {st->airport_type = AT_OILRIG;}
	}

	FOR_ALL_VEHICLES(v_oldstyle) {
	// airplane has another vehicle with subtype 4 (shadow), helicopter also has 3 (rotor)
	// skip those
		if (v_oldstyle->type == VEH_Aircraft && v_oldstyle->subtype <= 2) {
			// airplane in terminal stopped doesn't hurt anyone, so goto next
			if ((v_oldstyle->vehstatus & VS_STOPPED) && (v_oldstyle->u.air.state == 0)) {
				v_oldstyle->u.air.state = HANGAR;
				continue;
			}

			AircraftLeaveHangar(v_oldstyle); // make airplane visible if it was in a depot for example
			v_oldstyle->vehstatus &= ~VS_STOPPED; // make airplane moving
			v_oldstyle->u.air.state = FLYING;
			AircraftNextAirportPos_and_Order(v_oldstyle); // move it to the entry point of the airport
			GetNewVehiclePos(v_oldstyle, &gp); // get the position of the plane (to be used for setting)
			v_oldstyle->tile = 0; // aircraft in air is tile=0
			
			// correct speed of helicopter-rotors
			if (v_oldstyle->subtype == 0) {v_oldstyle->next->next->cur_speed = 32;}

			// set new position x,y,z
			SetAircraftPosition(v_oldstyle, gp.x, gp.y, GetAircraftFlyingAltitude(v_oldstyle));
		}
	}
}

void UpdateAirplanesOnNewStation(Station *st) 
{
	GetNewVehiclePosResult gp;
	Vehicle *v;
	byte takeofftype;
	uint16 cnt;
	// only 1 station is updated per function call, so it is enough to get entry_point once
	const AirportFTAClass *ap = GetAirport(st->airport_type);
	FOR_ALL_VEHICLES(v) {
		if (v->type == VEH_Aircraft && v->subtype <= 2) {
			if (v->u.air.targetairport == st->index) {	// if heading to this airport
				/*	update position of airplane. If plane is not flying, landing, or taking off
						you cannot delete airport, so it doesn't matter
				*/
				if (v->u.air.state >= FLYING) {	// circle around
					v->u.air.pos = v->u.air.previous_pos = ap->entry_point;
					v->u.air.state = FLYING;
					// landing plane needs to be reset to flying height (only if in pause mode upgrade,
					// in normal mode, plane is reset in Aircraft_5. It doesn't hurt for FLYING
					GetNewVehiclePos(v, &gp);
					// set new position x,y,z
					SetAircraftPosition(v, gp.x, gp.y, GetAircraftFlyingAltitude(v));
				}
				else {
					assert(v->u.air.state == ENDTAKEOFF || v->u.air.state == HELITAKEOFF);
					takeofftype = (v->subtype == 0) ? HELITAKEOFF : ENDTAKEOFF;
					// search in airportdata for that heading
					// easiest to do, since this doesn't happen a lot
					for (cnt = 0; cnt < ap->nofelements; cnt++) {
						if (ap->layout[cnt].heading == takeofftype) {
							v->u.air.pos = ap->layout[cnt].position;
							break;
						}
					}
				}
			}
		}
	}
}
