#include "stdafx.h"

#if defined(WITH_SDL)
#include "ttd.h"
#include "gfx.h"
#include "sound.h"
#include "window.h"
#include <SDL.h>
#include "player.h"
#include "hal.h"

#ifdef UNIX
#include <signal.h>
#endif

#define DYNAMICALLY_LOADED_SDL

static SDL_Surface *_sdl_screen;
static int _sdl_usage;
static bool _all_modes;

#define MAX_DIRTY_RECTS 100
static SDL_Rect _dirty_rects[MAX_DIRTY_RECTS];
static int _num_dirty_rects;

#define SDL_CALL    

#if defined(DYNAMICALLY_LOADED_SDL) && defined(WIN32)

bool LoadLibraryList(void **proc, const char *dll);

typedef struct {
	int (SDLCALL *SDL_Init)(Uint32);
	int (SDLCALL *SDL_InitSubSystem)(Uint32);
	char *(SDLCALL *SDL_GetError)();
	void (SDLCALL *SDL_QuitSubSystem)(Uint32);
	void (SDLCALL *SDL_UpdateRect)(SDL_Surface *, Sint32, Sint32, Uint32, Uint32);
	void (SDLCALL *SDL_UpdateRects)(SDL_Surface *, int, SDL_Rect *);
	int (SDLCALL *SDL_SetColors)(SDL_Surface *, SDL_Color *, int, int);
	void (SDLCALL *SDL_WM_SetCaption)(const char *, const char *);
	int (SDLCALL *SDL_ShowCursor)(int);
	void (SDLCALL *SDL_FreeSurface)(SDL_Surface *);
	int (SDLCALL *SDL_PollEvent)(SDL_Event *);
	void (SDLCALL *SDL_WarpMouse)(Uint16, Uint16);
	uint32 (SDLCALL *SDL_GetTicks)();
	int (SDLCALL *SDL_OpenAudio)(SDL_AudioSpec *, SDL_AudioSpec*);
	void (SDLCALL *SDL_PauseAudio)(int);
	void (SDLCALL *SDL_CloseAudio)();
	int (SDLCALL *SDL_LockSurface)(SDL_Surface*);
	void (SDLCALL *SDL_UnlockSurface)(SDL_Surface*);
	SDLMod (SDLCALL *SDL_GetModState)();
	void (SDLCALL *SDL_Delay)(Uint32);
	void (SDLCALL *SDL_Quit)();
	SDL_Surface *(SDLCALL *SDL_SetVideoMode)(int,int,int,Uint32);
	int (SDLCALL *SDL_EnableKeyRepeat)(int,int);
	void (SDLCALL *SDL_EnableUNICODE)(int);
	void (SDLCALL *SDL_VideoDriverName)(char *, int);
	SDL_Rect **(SDLCALL *SDL_ListModes)(void *, int);
	Uint8 *(SDLCALL *SDL_GetKeyState)(int *);
} SDLProcs;

#define M(x) x "\0"
static const char sdl_files[] = 
	M("sdl.dll")
	M("SDL_Init")
	M("SDL_InitSubSystem")
	M("SDL_GetError")
	M("SDL_QuitSubSystem")
	M("SDL_UpdateRect")
	M("SDL_UpdateRects")
	M("SDL_SetColors")
	M("SDL_WM_SetCaption")
	M("SDL_ShowCursor")
	M("SDL_FreeSurface")
	M("SDL_PollEvent")
	M("SDL_WarpMouse")
	M("SDL_GetTicks")
	M("SDL_OpenAudio")
	M("SDL_PauseAudio")
	M("SDL_CloseAudio")
	M("SDL_LockSurface")
	M("SDL_UnlockSurface")
	M("SDL_GetModState")
	M("SDL_Delay")
	M("SDL_Quit")
	M("SDL_SetVideoMode")
	M("SDL_EnableKeyRepeat")
	M("SDL_EnableUNICODE")
	M("SDL_VideoDriverName")
	M("SDL_ListModes")
	M("SDL_GetKeyState")
	M("")
;
#undef M

static SDLProcs _proc;

static char *LoadSdlDLL()
{
	if (_proc.SDL_Init != NULL) 
		return NULL;
	if (!LoadLibraryList((void**)&_proc, sdl_files))
		return "Unable to load sdl.dll";
	return NULL;
}

#undef SDL_CALL
#define SDL_CALL _proc.

#endif


#ifdef UNIX
static void SdlAbort(int sig)
{
	/* Own hand-made parachute for the cases of failed assertions. */
	SDL_CALL SDL_Quit();
}
#endif


static char *SdlOpen(uint32 x)
{
#if defined(DYNAMICALLY_LOADED_SDL) && defined(WIN32)
	{ char *s = LoadSdlDLL(); if (s) return s; }
#endif
	if (_sdl_usage++ == 0) {
		if (SDL_CALL SDL_Init(x) == -1)
			return SDL_CALL SDL_GetError();
	} else if (x) {
		if (SDL_CALL SDL_InitSubSystem(x) == -1)
			return SDL_CALL SDL_GetError();
	}

#ifdef UNIX
	signal(SIGABRT, SdlAbort);
#endif

	return NULL;
}

static void SdlClose(uint32 x)
{
	if (x)
		SDL_CALL SDL_QuitSubSystem(x);
	if (--_sdl_usage == 0) {
		SDL_CALL SDL_Quit();
#ifdef UNIX
#ifndef __MORPHOS__
		signal(SIGABRT, SIG_DFL);
#else
		signal(SIGABRT, (void (*)(int))0 );
#endif
#endif
	}
}

static void SdlVideoMakeDirty(int left, int top, int width, int height)
{
//	printf("(%d,%d)-(%d,%d)\n", left, top, width, height);
//	_pixels_redrawn += width*height;
	if (_num_dirty_rects < MAX_DIRTY_RECTS) {
		_dirty_rects[_num_dirty_rects].x = left;
		_dirty_rects[_num_dirty_rects].y = top;
		_dirty_rects[_num_dirty_rects].w = width;
		_dirty_rects[_num_dirty_rects].h = height;
	}
	_num_dirty_rects++;
}

static SDL_Color pal[256];

static void UpdatePalette(uint start, uint end) {
	uint i;
	byte *b;

	for(i = start, b = _cur_palette + start * 3; i != end; i++, b += 3) {
		pal[i].r = b[0];
		pal[i].g = b[1];
		pal[i].b = b[2];
		pal[i].unused = b[3];
	}

	SDL_CALL SDL_SetColors(_sdl_screen, pal, start, end);
}

static void InitPalette(void) {
	UpdatePalette(0, 256);
}

static void DrawSurfaceToScreen()
{
	int n;

	if(_pal_last_dirty != -1) {
		UpdatePalette(_pal_first_dirty, _pal_last_dirty + 1);
		_pal_last_dirty = -1;
	}

	if ((n=_num_dirty_rects) != 0) {
		_num_dirty_rects = 0;
		if (n > MAX_DIRTY_RECTS)
			SDL_CALL SDL_UpdateRect(_sdl_screen, 0, 0, 0, 0);
		else {
			SDL_CALL SDL_UpdateRects(_sdl_screen, n, _dirty_rects);
		}
	}
}

static int CDECL compare_res(const void *pa, const void *pb)
{
	int x = ((uint16*)pa)[0] - ((uint16*)pb)[0];
	if (x) return x;
	return ((uint16*)pa)[1] - ((uint16*)pb)[1];
}

static const uint16 default_resolutions[][2] = {
	{640,480},
	{800,600},
	{1024,768},
	{1152,864},
	{1280,960},
	{1280,1024},
	{1400,1050},
	{1600,1200},
};

static void GetVideoModes(void) {
	int i;
	SDL_Rect **modes;

	modes = SDL_CALL SDL_ListModes(NULL, SDL_SWSURFACE + (_fullscreen ? SDL_FULLSCREEN : 0));

	_all_modes = (int)modes == -1;
	
	if(modes == NULL) {
		error("sdl: no modes available");
	} else if((int) modes == -1) {
		// all modes available, put some default ones here
		memcpy(_resolutions, default_resolutions, sizeof(default_resolutions));
		_num_resolutions = sizeof(default_resolutions) / (sizeof(uint16)*2);
	} else {
		int n = 0;

		for(i = 0; modes[i] != NULL; i++) {
			int w = modes[i]->w;
			int h = modes[i]->h;
			if (IS_INT_INSIDE(w, 640, MAX_SCREEN_WIDTH+1) && 
					IS_INT_INSIDE(h, 480, MAX_SCREEN_HEIGHT+1) &&
					w%8 == 0 && h%8 == 0) { // disable screen resolutions which are not multiples of 8
				_resolutions[n][0] = w;
				_resolutions[n][1] = h;
				if (++n == sizeof(_resolutions) / (sizeof(uint16)*2)) break;
			}
		}
		_num_resolutions = n;
		qsort(_resolutions, n, sizeof(uint16)*2, compare_res);
	}
}

static int GetAvailableVideoMode(int *w, int *h)
{
	int i;


	// all modes available?
	if (_all_modes)
		return 1;

	// is the wanted mode among the available modes?
	for(i = 0; i != _num_resolutions; i++) {
		if(*w == _resolutions[i][0] && *h == _resolutions[i][1])
			return 1;
	}

	// use the default mode
	*w = _resolutions[0][0];
	*h = _resolutions[0][1];
	return 2;
}

static bool CreateMainSurface(int w, int h)
{
	SDL_Surface *newscreen;
	bool sizechange;

	GetAvailableVideoMode(&w, &h);

	sizechange = (_screen.width != w || _screen.height != h);
	_screen.pitch = _screen.width = w;
	_screen.height = h;

	DEBUG(misc, 0) ("sdl: using mode %dx%d", w, h);

	newscreen = SDL_CALL SDL_SetVideoMode(w, h, 8, SDL_SWSURFACE + SDL_HWPALETTE + (_fullscreen?SDL_FULLSCREEN:SDL_RESIZABLE));
	if(newscreen == NULL)
		return false;

	_sdl_screen = newscreen;
	InitPalette();

	SDL_CALL SDL_WM_SetCaption("OpenTTD", "OpenTTD");
	SDL_CALL SDL_ShowCursor(0);

//	if(sizechange)
		GameSizeChanged();

	return true;
}

typedef struct {
	uint16 vk_from;
	byte vk_count;
	byte map_to;
} VkMapping;

#define AS(x,z) {x,0,z}
#define AM(x,y,z,w) {x,y-x,z}

static const VkMapping _vk_mapping[] = {
	// Pageup stuff + up/down
	AM(SDLK_PAGEUP,SDLK_PAGEDOWN, WKC_PAGEUP, WKC_PAGEDOWN),
	AS(SDLK_UP,			WKC_UP),
	AS(SDLK_DOWN,		WKC_DOWN),
	AS(SDLK_LEFT,		WKC_LEFT),
	AS(SDLK_RIGHT,	WKC_RIGHT),

	AS(SDLK_HOME,		WKC_HOME),
	AS(SDLK_END,		WKC_END),

	AS(SDLK_INSERT,	WKC_INSERT),
	AS(SDLK_DELETE,	WKC_DELETE),

	// Map letters & digits
	AM(SDLK_a,SDLK_z,'A','Z'),
	AM(SDLK_0,SDLK_9,'0','9'),

	AS(SDLK_AMPERSAND, WKC_AMPERSAND), // mainly for french keyboards
	
	AS(SDLK_ESCAPE,	WKC_ESC),
	AS(SDLK_BACKSPACE,	WKC_BACKSPACE),

	AS(SDLK_SPACE,		WKC_SPACE),
	AS(SDLK_RETURN,		WKC_RETURN),
	AS(SDLK_TAB,			WKC_TAB),

	// Function keys
	AM(SDLK_F1, SDLK_F12,	WKC_F1, WKC_F12),

	// Numeric part.
	// What is the virtual keycode for numeric enter??
	AM(SDLK_KP0,SDLK_KP9, WKC_NUM_0, WKC_NUM_9),
	AS(SDLK_KP_DIVIDE,		WKC_NUM_DIV),
	AS(SDLK_KP_MULTIPLY,	WKC_NUM_MUL),
	AS(SDLK_KP_MINUS,			WKC_NUM_MINUS),
	AS(SDLK_KP_PLUS,			WKC_NUM_PLUS),
	AS(SDLK_KP_ENTER,			WKC_NUM_ENTER),
	AS(SDLK_KP_PERIOD,		WKC_NUM_DECIMAL),
	{0}	
};

static uint32 ConvertSdlKeyIntoMy(SDL_keysym *sym)
{
	const VkMapping	*map = _vk_mapping - 1;
	uint from;
	uint key = sym->sym;
	for(;;) {
		map++;
		from = map->vk_from;
		if (from == 0) {
			key = 0;
			break;		
		}
		if ((uint)(key - from) <= map->vk_count) {
			key = key - from + map->map_to;
			break;
		}
	}
	
	// META are the command keys on mac
	if (sym->mod & KMOD_META) key |= WKC_META;
	if (sym->mod & KMOD_SHIFT) key |= WKC_SHIFT;
	if (sym->mod & KMOD_CTRL) key |= WKC_CTRL;
	if (sym->mod & KMOD_ALT) key |= WKC_ALT;
	return (key << 16) + sym->unicode;
}

static int PollEvent() {
	SDL_Event ev;

	if (!SDL_CALL SDL_PollEvent(&ev))
		return -2;

	switch(ev.type) {
	case SDL_MOUSEMOTION:
		if (_cursor.fix_at) {
			int dx = ev.motion.x - _cursor.pos.x;
			int dy = ev.motion.y - _cursor.pos.y;
			if (dx != 0 || dy != 0) {
				_cursor.delta.x += dx;
				_cursor.delta.y += dy;
				SDL_CALL SDL_WarpMouse(_cursor.pos.x, _cursor.pos.y);
			}
		} else {
			_cursor.delta.x = ev.motion.x - _cursor.pos.x;
			_cursor.delta.y = ev.motion.y - _cursor.pos.y;
			_cursor.pos.x = ev.motion.x;
			_cursor.pos.y = ev.motion.y;
			_cursor.dirty = true;
		}
		break;

	case SDL_MOUSEBUTTONDOWN:
		if (_rightclick_emulate && (SDL_CALL SDL_GetModState() & (KMOD_LCTRL | KMOD_RCTRL)))
			ev.button.button = SDL_BUTTON_RIGHT;

		if (ev.button.button == SDL_BUTTON_LEFT) {
			_left_button_down = true;
		} else if (ev.button.button == SDL_BUTTON_RIGHT) {
			_right_button_down = true;
			_right_button_clicked = true;
		}
#if !defined(WIN32)		
		 else if (ev.button.button == SDL_BUTTON_WHEELUP) {
			_cursor.wheel--;
		} else if (ev.button.button == SDL_BUTTON_WHEELDOWN) {
			_cursor.wheel++;
		}
#endif
		break;

	case SDL_MOUSEBUTTONUP:
		if (_rightclick_emulate) {
			_right_button_down = false;
			_left_button_down = false;
			_left_button_clicked = false;
		} else if (ev.button.button == SDL_BUTTON_LEFT) {
			_left_button_down = false;
			_left_button_clicked = false;
		} else if (ev.button.button == SDL_BUTTON_RIGHT) {
			_right_button_down = false;
		}
		break;

	case SDL_QUIT:
		return ML_QUIT;

	case SDL_KEYDOWN:
		if ((((ev.key.keysym.sym == SDLK_RETURN) || (ev.key.keysym.sym == SDLK_f)) && (ev.key.keysym.mod & KMOD_ALT)) || (((ev.key.keysym.sym == SDLK_RETURN) || (ev.key.keysym.sym == SDLK_f)) && (ev.key.keysym.mod & KMOD_META))) {
			_fullscreen ^= true;
			GetVideoModes();
			CreateMainSurface(_screen.width, _screen.height);
			MarkWholeScreenDirty();
		} else {
			_pressed_key = ConvertSdlKeyIntoMy(&ev.key.keysym);
		}
		break;
		
	case SDL_VIDEORESIZE: {
			int w, h;
			w = ev.resize.w;
			h = ev.resize.h;
			
			w = clamp(w & ~0x7, 64, MAX_SCREEN_WIDTH);
			h = clamp(h & ~0x7, 64, MAX_SCREEN_HEIGHT);
			
			ChangeResInGame(w, h);
			
			break;
		}
	}
	return -1;
}

static const char *SdlVideoStart(char **parm)
{
    char buf[30];

	{char *s;if ((s = SdlOpen(SDL_INIT_VIDEO)) != NULL) return s;}

	SDL_CALL SDL_VideoDriverName(buf, 30);
	DEBUG(misc, 0) ("sdl: using driver '%s'", buf);

	GetVideoModes();
	CreateMainSurface(_cur_resolution[0], _cur_resolution[1]);
	MarkWholeScreenDirty();

	SDL_CALL SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	SDL_CALL SDL_EnableUNICODE(1);
	return NULL;
}

static void SdlVideoStop()
{
	SdlClose(SDL_INIT_VIDEO);
}

static int SdlVideoMainLoop()
{
	uint32 next_tick = SDL_CALL SDL_GetTicks() + 30, cur_ticks;
	int i;
	uint32 mod;
	int numkeys;
	Uint8 *keys;

	while (true) {
		InteractiveRandom(); // randomness
		
		while ((i=PollEvent()) == -1) {}
		if (i>=0) return i;

		mod = SDL_CALL SDL_GetModState();
		_ctrl_pressed = !!(mod & (KMOD_LCTRL | KMOD_RCTRL));
		_shift_pressed = !!(mod & (KMOD_LSHIFT | KMOD_RSHIFT));


		keys = SDL_CALL SDL_GetKeyState(&numkeys);

		_dirkeys = 
			(keys[SDLK_LEFT] ? 1 : 0) + 
			(keys[SDLK_UP] ? 2 : 0) + 
			(keys[SDLK_RIGHT] ? 4 : 0) + 
			(keys[SDLK_DOWN] ? 8 : 0);

		GameLoop();

		_screen.dst_ptr = _sdl_screen->pixels;
		UpdateWindows();

		if ((cur_ticks=SDL_CALL SDL_GetTicks()) > next_tick) {
			next_tick = cur_ticks;
		} else {
			DrawSurfaceToScreen();
		}

#if defined(_DEBUG)
		if (_shift_pressed) {
#else
		if (keys[SDLK_TAB]) {
#endif
			_fast_forward |= 2;
		} else if (_fast_forward&2) {
			_fast_forward = 0;
		}

		if (!_fast_forward || _pause) {
			int d = next_tick - SDL_CALL SDL_GetTicks();
			if (d > 0) {
				SDL_CALL SDL_Delay(d);
			}
		} else {
			next_tick = SDL_CALL SDL_GetTicks();
		}
		next_tick += 30;

		if (_exit_game)
			return ML_QUIT;
	}
}

static bool SdlVideoChangeRes(int w, int h)
{
	// see if the mode is available
	if (GetAvailableVideoMode(&w, &h) != 1)
		return false;
	
	CreateMainSurface(w, h);
	return true;
}

const HalVideoDriver _sdl_video_driver = {
	SdlVideoStart,
	SdlVideoStop,
	SdlVideoMakeDirty,
	SdlVideoMainLoop,
	SdlVideoChangeRes,
};

static void CDECL fill_sound_buffer(void *userdata, Uint8 *stream, int len)
{
	MxMixSamples(_mixer, stream, len >> 2);
}

static char *SdlSoundStart(char **parm)
{
	SDL_AudioSpec spec;
	
	{char *s;if ((s = SdlOpen(SDL_INIT_AUDIO)) != NULL) return s;}
	spec.freq = GetDriverParamInt(parm, "hz", 11025);
	spec.format = AUDIO_S16SYS;
	spec.channels = 2;
	spec.samples = 512;
	*(void**)&spec.callback = fill_sound_buffer;
	SDL_CALL SDL_OpenAudio(&spec, &spec);
	SDL_CALL SDL_PauseAudio(0);
	return NULL;
}

static void SdlSoundStop()
{
	SDL_CALL SDL_CloseAudio();
	SdlClose(SDL_INIT_AUDIO);
}

const HalSoundDriver _sdl_sound_driver = {
	SdlSoundStart,
	SdlSoundStop,
};


#include "viewport.h"
void redsq_debug(int tile)
{
	_thd.redsq = tile;
	MarkWholeScreenDirty();
	_screen.dst_ptr = _sdl_screen->pixels;
	UpdateWindows();

	SdlVideoMakeDirty(0,0,_screen.width,_screen.height);
	DrawSurfaceToScreen();
}

void DbgRedraw()
{
	SdlVideoMakeDirty(0,0,_screen.width,_screen.height);
	DrawSurfaceToScreen();
}
#endif // WITH_SDL
