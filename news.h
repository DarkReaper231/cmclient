#ifndef NEWS_H
#define NEWS_H

struct NewsItem {
	StringID string_id;
	uint16 duration;
	uint16 date;
	byte flags;
	byte display_mode;
	byte type;
	byte callback;

	TileIndex data_a;
	TileIndex data_b;

	uint32 params[10];
};


#define NEWS_FLAGS(mode,flag,type,cb) ((cb)<<24 | (type)<<16 | (flag)<<8 | (mode))
void AddNewsItem(StringID string, uint32 flags, uint data_a, uint data_b);
void NewsLoop();
void DrawNewsBorder(Window *w);
void InitNewsItemStructs();

VARDEF NewsItem _statusbar_news_item;

typedef void DrawNewsCallbackProc(Window *w);
typedef StringID GetNewsStringCallbackProc(NewsItem *ni);

enum {
	NT_ARRIVAL_PLAYER = 0,
	NT_ARRIVAL_OTHER = 1,
	NT_ACCIDENT = 2,
	NT_COMPANY_INFO = 3,
	NT_ECONOMY = 4,
	NT_ADVICE = 5,
	NT_NEW_VEHICLES = 6,
	NT_ACCEPTANCE = 7,
	NT_SUBSIDIES = 8,
	NT_GENERAL = 9,
};

enum NewsMode {
	NM_SMALL = 0,
	NM_NORMAL = 1,
	NM_THIN = 2,
	NM_CALLBACK = 3,
};

enum NewsFlags {
	NF_VIEWPORT = 1,
	NF_TILE = 4,
	NF_VEHICLE = 8,
	NF_FORCE_BIG = 0x10,
	NF_NOEXPIRE = 0x20,
	NF_INCOLOR = 0x40,
};

enum {
	DNC_TRAINAVAIL = 0,
	DNC_ROADAVAIL = 1,
	DNC_SHIPAVAIL = 2,
	DNC_AIRCRAFTAVAIL = 3,
	DNC_BANKRUPCY = 4,
};

#endif /* NEWS_H */
