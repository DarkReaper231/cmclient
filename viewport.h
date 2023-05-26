#ifndef VIEWPORT_H
#define VIEWPORT_H

struct ViewPort {
	int left,top;												// screen coordinates for the viewport
	int width, height;									// screen width/height for the viewport

	int virtual_left, virtual_top;			// virtual coordinates
	int virtual_width, virtual_height;	// these are just width << zoom, height << zoom

	byte zoom;	
};

/* viewport.c */
Point MapXYZToViewport(ViewPort *vp, uint x, uint y, uint z);
void AssignWindowViewport(Window *w, int x, int y, 
	int width, int height, uint32 follow_flags, byte zoom);
void SetViewportPosition(Window *w, int x, int y);
ViewPort *IsPtInWindowViewport(Window *w, int x, int y);
Point GetTileBelowCursor();
Point GetTileZoomCenter(bool in);
void UpdateViewportPosition(Window *w);

void OffsetGroundSprite(int x, int y);

void DrawGroundSprite(uint32 image);
void DrawGroundSpriteAt(uint32 image, int x, int y, byte z);
void AddSortableSpriteToDraw(uint32 image, int x, int y, int w, int h, byte dz, byte z);
void *AddStringToDraw(int x, int y, StringID string, uint32 params_1, uint32 params_2);
void AddChildSpriteScreen(uint32 image, int x, int y);


void StartSpriteCombine();
void EndSpriteCombine();

void HandleViewportClicked(ViewPort *vp, int x, int y);
void PlaceObject();
void SetRedErrorSquare(TileIndex tile);
void SetTileSelectSize(int w, int h);
void SetTileSelectBigSize(int ox, int oy, int sx, int sy);

void VpStartPlaceSizing(uint tile, int user);
void VpStartPreSizing();
void VpSetPresizeRange(uint from, uint to);
void VpSetPlaceSizingLimit(int limit);

Vehicle *CheckMouseOverVehicle();

enum {
	VPM_X_OR_Y = 0,
	VPM_FIX_X = 1,
	VPM_FIX_Y = 2,
	VPM_RAILDIRS = 3,
	VPM_X_AND_Y = 4,
	VPM_X_AND_Y_LIMITED = 5,
};

void VpSelectTilesWithMethod(int x, int y, int method);

enum {
	HT_NONE = 0,
	HT_RECT = 0x80,
	HT_POINT = 0x40,
	HT_LINE = 0x20,
};

typedef struct TileHighlightData {
	Point size;
	Point outersize;
	Point pos;
	Point offs;

	Point new_pos;
	Point new_size;
	Point new_outersize;

	Point selend, selstart;

	byte dirty;
	byte sizelimit;

	byte drawstyle;
	byte new_drawstyle;
	byte next_drawstyle;

	byte place_mode;
	byte window_class;
	bool make_square_red;
	uint16 window_number;

	int userdata;
	TileIndex redsq;
} TileHighlightData;


enum AnimCursors {
	ANIMCURSOR_DEMOLISH = -1,
	ANIMCURSOR_LOWERLAND = -2,
	ANIMCURSOR_RAISELAND = -3,
	ANIMCURSOR_PICKSTATION = -4,
	ANIMCURSOR_BUILDSIGNALS = -5,
};

// common button handler
bool HandlePlacePushButton(Window *w, int widget, uint32 cursor, int mode, PlaceProc *placeproc);

/* viewport.c */
VARDEF ViewPort _viewports[18];
VARDEF TileHighlightData _thd;
VARDEF uint32 _active_viewports;

VARDEF Point _tile_fract_coords;

extern TileHighlightData * const _thd_ptr;


#endif
