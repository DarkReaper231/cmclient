#include "stdafx.h"
#include "ttd.h"
#include "gfx.h"
#include "table/palettes.h"
#include "hal.h"

static void GfxMainBlitter(byte *sprite, int x, int y, int mode);
void GfxInitPalettes();

static int _stringwidth_out;
static byte _cursor_backup[64*64];
static Rect _invalid_rect;
static byte *_color_remap_ptr;
static byte _string_colorremap[3];

#define DIRTY_BYTES_PER_LINE (MAX_SCREEN_WIDTH/64)
static byte _dirty_blocks[DIRTY_BYTES_PER_LINE * MAX_SCREEN_HEIGHT / 8];


static void memcpy_pitch(void *d, void *s, int w, int h, int spitch, int dpitch)
{
	byte *dp = (byte*)d;
	byte *sp = (byte*)s;

	assert(h >= 0);
	if (h != 0) do {
		memcpy(dp, sp, w);
		dp += dpitch;
		sp += spitch;
	} while (--h);
}


void GfxScroll(int left, int top, int width, int height, int xo, int yo) {
	byte *src, *dst;
	int p;
	int ht;

	if (xo == 0 && yo == 0)
		return;

	if (_cursor.visible)
		UndrawMouseCursor();

	p = _screen.pitch;

	if (yo > 0 || (yo == 0 && xo > 0)) {
		// Calculate pointers
		dst = _screen.dst_ptr + (top+height-1) * p + (left+width-1);
		src = dst - yo * p;

		// Decrease height and increase top
		top += yo;
		height -= yo;
		assert(height > 0);

		// Adjust left & width
		if (xo >= 0) {
			left += xo;
			src -= xo;
			width -= xo;
		} else {
			dst += xo;
			width += xo;
		}

		// Offset pointers to fit into the memmove call
		dst += -width + 1;
		src += -width + 1;
		ht = height;

		do {
			memmove(dst, src, width);
			src -= p;
			dst -= p;
		} while (--ht);


		// This part of the screen is now dirty.
		_video_driver->make_dirty(left, top, width, height);

	} else {
		// Calculate pointers to mem.
		dst = _screen.dst_ptr + top * p + left;
		src = dst - yo * p;

		// Decrese height. (yo is <=0).
		height += yo;
		assert(height > 0);

		// Adjust left & width
		if (xo >= 0) {
			dst += xo;
			left += xo;
			width -= xo;
		} else {
			src -= xo;
			width += xo;
		}

		ht = height;

		do {
			memcpy(dst, src, width);
			src += p;
			dst += p;
		} while (--ht);

		// This part of the screen is now dirty.
		_video_driver->make_dirty(left, top, width, height);
	}
}


void GfxFillRect(int left, int top, int right, int bottom, int color) {
	DrawPixelInfo *dpi = _cur_dpi;
	byte *dst,*ctab;

	if (dpi->zoom != 0)
		return;

	if (left > right || top > bottom)
		return;

	if (right < dpi->left || left >= dpi->left + dpi->width)
		return;

	if (bottom < dpi->top || top >= dpi->top + dpi->height)
		return;

	if ( (left -= dpi->left) < 0) left = 0;
	right = right - dpi->left + 1;
	if (right > dpi->width) right=dpi->width;
	right -= left;
	assert(right > 0);

	if ( (top -= dpi->top) < 0) top = 0;
	bottom = bottom - dpi->top + 1;
	if (bottom > dpi->height) bottom=dpi->height;
	bottom -= top;
	assert(bottom > 0);

	dst = dpi->dst_ptr + top * dpi->pitch + left;

	if (!(color & 0x8000)) {
		if (!(color & 0x4000)) {
			do {
				memset(dst, color, right);
				dst += dpi->pitch;
			} while (--bottom);
		} else {
			/* use colortable mode */
			ctab = GetSpritePtr(color & 0x3FFF) + 1;
			do {
				int i;
				for(i=0; i!=right;i++)
					dst[i] = ctab[dst[i]];
				dst += dpi->pitch;
			} while (--bottom);
		}
	} else {
		byte bo = 0;
		do {
			int i;
			byte b = (bo^=1);
			for(i=0; i!=right;i++)
				if ((b^=1) != 0)
					dst[i] = (byte)color;
			dst += dpi->pitch;
		} while (--bottom);
	}
}

void GfxSetPixel(int x, int y, int color)
{
	DrawPixelInfo *dpi = _cur_dpi;
	if ((x-=dpi->left) < 0 || x>=dpi->width || (y-=dpi->top)<0 || y>=dpi->height)
		return;
	dpi->dst_ptr[y*dpi->pitch+x] = color;
}

void GfxDrawLine(int x, int y, int x2, int y2, int color)
{
	int dy;
	int dx;
	int stepx, stepy;
	int frac;

	// Check clipping first
	{
		DrawPixelInfo *dpi = _cur_dpi;
		int t;

		if (x < dpi->left && x2 < dpi->left)
			return;

		if (y < dpi->top && y2 < dpi->top)
			return;

		t = dpi->left + dpi->width;
		if (x > t && x2 > t)
			return;

		t = dpi->top + dpi->height;
		if (y > t && y2 > t)
			return;
	}

	dy = (y2 - y) * 2;
	if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }

	dx = (x2 - x) * 2;
	if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }

	GfxSetPixel(x, y, color);
	if (dx > dy) {
		frac = dy - (dx >> 1);
		while (x != x2) {
			if (frac >= 0) {
				y += stepy;
				frac -= dx;
			}
			x += stepx;
			frac += dy;
			GfxSetPixel(x, y, color);
		}
	} else {
		frac = dx - (dy >> 1);
		while (y != y2) {
			if (frac >= 0) {
				x += stepx;
				frac -= dy;
			}
			y += stepy;
			frac += dx;
			GfxSetPixel(x, y, color);
		}
	}
}

// ASSIGNMENT OF ASCII LETTERS < 32
// 0 - end of string
// 1 - SETX <BYTE>
// 2 - SETXY <BYTE> <BYTE>
// 3-7 - 
// 8 - TINYFONT
// 9 - BIGFONT
// 10 - newline
// 11-14 - 
// 15-31 - 17 colors


enum {
	ASCII_LETTERSTART = 32,
	
	ASCII_SETX = 1,
	ASCII_SETXY = 2,

	ASCII_TINYFONT = 8,
	ASCII_BIGFONT = 9,
	ASCII_NL = 10,

	ASCII_COLORSTART = 15,
};


/* returns right coordinate */
int DrawString(int x, int y, uint16 str, byte color)
{
	GetString(str_buffr, str);
	assert(strlen(str_buffr) < sizeof(str_buffr) - 1);
	return DoDrawString(str_buffr, x, y, color);
}


void DrawStringRightAligned(int x, int y, uint16 str, byte color)
{
	GetString(str_buffr, str);
	assert(strlen(str_buffr) < sizeof(str_buffr) - 1);
	DoDrawString(str_buffr, x - GetStringWidth(str_buffr), y, color);
}


int DrawStringCentered(int x, int y, uint16 str, byte color)
{
	int w;

	GetString(str_buffr, str);
	assert(strlen(str_buffr) < sizeof(str_buffr) - 1);

	w = GetStringWidth(str_buffr);
	DoDrawString(str_buffr, x - (w>>1), y, color);

	return w;
}

void DrawStringCenterUnderline(int x, int y, uint16 str, byte color)
{
	int w = DrawStringCentered(x, y, str, color);
	GfxFillRect(x-(w>>1), y+10, x-(w>>1)+w, y+10, _string_colorremap[1]);
}

uint32 FormatStringLinebreaks(byte *str, int maxw) {
	int num = 0;
	int base = _stringwidth_base;
	int w;
	byte *last_space;
	byte c;

	for(;;) {
		w = 0;
		last_space = NULL;

		for(;;) {
			c = *str++;
			if (c == ' ') last_space = str;

			if (c >= ASCII_LETTERSTART) {
				w += _stringwidth_table[base + ((byte)c) - 0x20];
				if (w > maxw) {
					str = last_space;
					if (str == NULL)
						return num + (base << 16);
					break;
				}
			} else {
				if (c == 0) return num + (base << 16);
				if (c == ASCII_NL) break;

				if (c == ASCII_SETX) str++;
				else if (c == ASCII_SETXY) str += 2;
				else if (c == ASCII_TINYFONT) base = 224;
				else if (c == ASCII_BIGFONT) base = 448;
			}
		}

		num++;
		str[-1] = 0;
	}
}


void DrawStringMultiCenter(int x, int y, uint16 str, int maxw)
{
	uint32 tmp;
	int num, w, mt, t;
	byte *src;
	byte c;

	GetString(str_buffr, str);
	assert(strlen(str_buffr) < sizeof(str_buffr) - 1);

	tmp = FormatStringLinebreaks(str_buffr, maxw);
	num = (uint16)tmp;
	t = tmp >> 16;

	mt = 10;
	if (t != 0) {
		mt = 6;
		if (t != 244) mt = 18;
	}

	y -= (mt >> 1) * num;

	src = str_buffr;

	for(;;) {
		w = GetStringWidth(src);
		DoDrawString(src, x - (w>>1), y, 0xFE);
		_stringwidth_base = _stringwidth_out;

		for(;;) {
			c = *src++;
			if (c == 0) {
				y += mt;
				if (--num < 0) {
					_stringwidth_base = 0;
					return;
				}
				break;
			} else if (c == ASCII_SETX) {
				src++;
			} else if (c == ASCII_SETXY) {
				src+=2;
			}
		}
	}
}

void DrawStringMultiLine(int x, int y, uint16 str, int maxw) {
	uint32 tmp;
	int num, w, mt, t;
	byte *src;
	byte c;

	GetString(str_buffr, str);
	assert(strlen(str_buffr) < sizeof(str_buffr) - 1);

	tmp = FormatStringLinebreaks(str_buffr, maxw);
	num = (uint16)tmp;
	t = tmp >> 16;
	mt = 10;
	if (t != 0) {
		mt = 6;
		if (t != 244) mt = 18;
	}

	src = str_buffr;

	for(;;) {
		w = GetStringWidth(src);
		DoDrawString(src, x, y, 0xFE);
		_stringwidth_base = _stringwidth_out;

		for(;;) {
			c = *src++;
			if (c == 0) {
				y += mt;
				if (--num < 0) {
					_stringwidth_base = 0;
					return;
				}
				break;
			} else if (c == ASCII_SETX) {
				src++;
			} else if (c == ASCII_SETXY) {
				src+=2;
			}
		}
	}
}

int GetStringWidth(const byte *str) {
	int w = -1;
	byte c;
	int base = _stringwidth_base;

	for(;;) {
		c = *str++;
		if (c == 0)
			return w;
		if (c >= ASCII_LETTERSTART) {
			w += _stringwidth_table[base + c - ASCII_LETTERSTART];
		} else {
			if (c == ASCII_SETX) str++;
			else if (c == ASCII_SETXY) str += 2;
			else if (c == ASCII_TINYFONT) base = 224;
			else if (c == ASCII_BIGFONT) base = 448;
		}
	}
}

void DrawFrameRect(int left, int top, int right, int bottom, int ctab, int flags) {
	byte color_2 = _color_list[ctab].window_color_1a;
	byte color_interior = _color_list[ctab].window_color_bga;
	byte color_3 = _color_list[ctab].window_color_bgb;
	byte color = _color_list[ctab].window_color_2;

	if (!(flags & 0x8))	{
		if (!(flags & 0x20)) {
			GfxFillRect(left, top, left, bottom-1, color);
			GfxFillRect(left+1, top, right-1, top, color);
			GfxFillRect(right, top, right, bottom-1, color_2);
			GfxFillRect(left, bottom, right, bottom, color_2);
			if (!(flags & 0x10)) {
				GfxFillRect(left+1, top+1, right-1, bottom-1, color_interior);
			}
		} else {
			GfxFillRect(left, top, left, bottom, color_2);
			GfxFillRect(left+1, top, right, top, color_2);
			GfxFillRect(right, top+1, right, bottom-1, color);
			GfxFillRect(left+1, bottom, right, bottom, color);
			if (!(flags & 0x10)) {
				GfxFillRect(left+1, top+1, right-1, bottom-1,
					flags&0x40 ? color_interior : color_3);
			}
		}
	} else if (flags & 0x1) {
		// transparency
		GfxFillRect(left, top, right, bottom, 0x4322);
	} else {
		GfxFillRect(left, top, right, bottom, color_interior);
	}
}

int DoDrawString(const byte *string, int x, int y, byte color) {
	DrawPixelInfo *dpi = _cur_dpi;
	int base = _stringwidth_base;
	byte c, *b;
	int xo = x, yo = y;

	if (color != 0xFE) {
		if (x >= dpi->left + dpi->width ||
				x + _screen.width*2 <= dpi->left ||
				y >= dpi->top + dpi->height ||
				y + _screen.height <= dpi->top)
					return x;

		if (color != 0xFF) {
switch_color:;
			b = GetSpritePtr(674);
			_string_colorremap[1] = b[color*2+8];
			_string_colorremap[2] = b[color*2+9];
			_color_remap_ptr = _string_colorremap;
		}
	}

check_bounds:
	if (y + 19 <= dpi->top || dpi->top + dpi->height <= y) {
skip_char:;
		for(;;) {
			c = *string++;
			if (c < ASCII_LETTERSTART) goto skip_cont;			
		}
	}

	for(;;) {
		c = *string++;
skip_cont:;
		if (c == 0) {
			_stringwidth_out = base;
			return x;
		}
		if (c >= ASCII_LETTERSTART) {
			if (x >= dpi->left + dpi->width) goto skip_char;
			if (x + 26 >= dpi->left) {
				GfxMainBlitter(GetSpritePtr(base + 2 + c - ASCII_LETTERSTART), x, y, 1);
			}
			x += _stringwidth_table[base + c - ' '];
		} else if (c == ASCII_NL) { // newline = {}
			x = xo;
			y += 10;
			if (base != 0) {
				y -= 4;
				if (base != 0xE0)
					y += 12;
			}
			goto check_bounds;
		} else if (c >= ASCII_COLORSTART) { // change color?
			color = (byte)(c - ASCII_COLORSTART);
			goto switch_color;
		} else if (c == ASCII_SETX) { // {SETX}
			x = xo + *string++;
		} else if (c == ASCII_SETXY) {// {SETXY}
			x = xo + *string++;
			y = yo + *string++;
		} else if (c == ASCII_TINYFONT) { // {TINYFONT}
			base = 0xE0;
		} else if (c == ASCII_BIGFONT) { // {BIGFONT}
			base = 0x1C0;
		} else {
			printf("Unknown string command character %d\n", c);
		}
	}
}

void DrawSprite(uint32 img, int x, int y) {
	if (img & 0x8000) {
		_color_remap_ptr = GetSpritePtr(img >> 16) + 1;
		GfxMainBlitter(GetSpritePtr(img & 0x3FFF), x, y, 1);
	} else if (img & 0x4000) {
		_color_remap_ptr = GetSpritePtr(img >> 16) + 1;
		GfxMainBlitter(GetSpritePtr(img & 0x3FFF), x, y, 2);
	} else {
		GfxMainBlitter(GetSpritePtr(img & 0x3FFF), x, y, 0);
	}
}

typedef struct BlitterParams {
	int start_x, start_y;
	byte *sprite, *sprite_org;
	byte *dst;
	int mode;
	int width, height;
	int width_org, height_org;
	int pitch;
	byte info;
} BlitterParams;

static void GfxBlitTileZoomIn(BlitterParams *bp)
{
	byte *src_o = bp->sprite, *src;
	int num, skip;
	byte done;
	byte *dst, *ctab;

	if (bp->mode & 1) {
		src_o += READ_LE_UINT16(src_o + bp->start_y * 2);
		
		do {
			do {
				done = src_o[0];
				num = done & 0x7F;
				skip = src_o[1];
				src = src_o + 2;
				src_o += num + 2;

				dst = bp->dst;

				if ( (skip -= bp->start_x) > 0) {
					dst += skip;
				} else {
					src -= skip;
					num += skip;
					if (num <= 0)
						continue;
					skip = 0;
				}

				skip = skip + num - bp->width;
				if (skip > 0) {
					num -= skip;
					if (num <= 0)
						continue;
				}

				ctab = _color_remap_ptr;

				while (num >= 4) {
					dst[3] = ctab[src[3]];
					dst[2] = ctab[src[2]];
					dst[1] = ctab[src[1]];
					dst[0] = ctab[src[0]];
					dst += 4;
					src += 4;
					num -= 4;
				}
				while (num) {
					*dst++ = ctab[*src++];
					num--;
				}
			} while (!(done & 0x80));

			bp->dst += bp->pitch;
		} while (--bp->height);
	} else if (bp->mode & 2) {
		src_o += READ_LE_UINT16(src_o + bp->start_y * 2);
		do {
			do {
				done = src_o[0];
				num = done & 0x7F;
				skip = src_o[1];
				src_o += num + 2;

				dst = bp->dst;

				if ( (skip -= bp->start_x) > 0) {
					dst += skip;
				} else {
					num += skip;
					if (num <= 0)
						continue;
					skip = 0;
				}

				skip = skip + num - bp->width;
				if (skip > 0) {
					num -= skip;
					if (num <= 0)
						continue;
				}

				ctab = _color_remap_ptr;
				while (num) {
					*dst = ctab[*dst];
					dst++;
					num--;
				}
			} while (!(done & 0x80));

			bp->dst += bp->pitch;
		} while (--bp->height);

	} else {
		src_o += READ_LE_UINT16(src_o + bp->start_y * 2);
		do {
			do {
				done = src_o[0];
				num = done & 0x7F;
				skip = src_o[1];
				src = src_o + 2;
				src_o += num + 2;

				dst = bp->dst;

				if ( (skip -= bp->start_x) > 0) {
					dst += skip;
				} else {
					src -= skip;
					num += skip;
					if (num <= 0)
						continue;
					skip = 0;
				}

				skip = skip + num - bp->width;
				if (skip > 0) {
					num -= skip;
					if (num <= 0)
						continue;
				}
#if defined(_WIN32)
				if (num & 1) *dst++ = *src++;
				if (num & 2) { *(uint16*)dst = *(uint16*)src; dst += 2; src += 2; }
				if (num >>= 2) {
					do {
						*(uint32*)dst = *(uint32*)src;
						dst += 4;
						src += 4;
					} while (--num);
				}
#else
				memcpy(dst, src, num);
#endif
			} while (!(done & 0x80));

			bp->dst += bp->pitch;
		} while (--bp->height);
	}
}

static void GfxBlitZoomInUncomp(BlitterParams *bp)
{
	byte *src = bp->sprite;
	byte *dst = bp->dst;
	int height = bp->height;
	int width = bp->width;
	int i;

	assert(height > 0);
	assert(width > 0);

	if (bp->mode & 1) {
		if (bp->info & 1) {
			byte *ctab = _color_remap_ptr;
			byte b;

			do {
				for(i=0; i!=width; i++) {
					if ((b=ctab[src[i]]) != 0)
						dst[i] = b;
				}
				src += bp->width_org;
				dst += bp->pitch;
			} while (--height);
		}
	} else if (bp->mode & 2) {
		if (bp->info & 1) {
			byte *ctab = _color_remap_ptr;
			do {
				for(i=0; i!=width; i++)
					if (src[i])
						dst[i] = ctab[dst[i]];
				src += bp->width_org;
				dst += bp->pitch;
			} while (--height);
		}
	} else {
		if (!(bp->info & 1)) {
			do {
				memcpy(dst, src, width);
				src += bp->width_org;
				dst += bp->pitch;
			} while (--height);
		} else {
			do {
				int n = width;
				while (n >= 4) {
					if (src[0]) dst[0] = src[0];
					if (src[1]) dst[1] = src[1];
					if (src[2]) dst[2] = src[2];
					if (src[3]) dst[3] = src[3];

					dst += 4;
					src += 4;
					n -= 4;
				}

				while (n) {
					if (src[0]) dst[0] = src[0];
					src++;
					dst++;
					n--;
				}

				src += bp->width_org - width;
				dst += bp->pitch - width;

			} while (--height);
		}
	}
}

static void GfxBlitTileZoomMedium(BlitterParams *bp)
{
	byte *src_o = bp->sprite, *src;
	int num, skip;
	byte done;
	byte *dst, *ctab;

	if (bp->mode & 1) {
		src_o += READ_LE_UINT16(src_o + bp->start_y * 2);
		do {
			do {
				done = src_o[0];
				num = done & 0x7F;
				skip = src_o[1];
				src = src_o + 2;
				src_o += num + 2;

				dst = bp->dst;

				if (skip & 1) {
					skip++;
					src++;
					if (!--num)
						continue;
				}

				if ( (skip -= bp->start_x) > 0) {
					dst += skip >> 1;
				} else {
					src -= skip;
					num += skip;
					if (num <= 0)
						continue;
					skip = 0;
				}

				skip = skip + num - bp->width;
				if (skip > 0) {
					num -= skip;
					if (num <= 0)
						continue;
				}

				ctab = _color_remap_ptr;
				num = (num + 1) >> 1;
				if (num) {
					do {
						*dst = ctab[*src];
						dst++;
						src+=2;
					} while (--num);
				}
			} while (!(done & 0x80));
			bp->dst += bp->pitch;
			if (!--bp->height)
				return;

			do {
				done = src_o[0];
				src_o += (done & 0x7F) + 2;
			} while (!(done & 0x80));
		} while (--bp->height);
	} else if (bp->mode & 2) {
		src_o += READ_LE_UINT16(src_o + bp->start_y * 2);
		do {
			do {
				done = src_o[0];
				num = done & 0x7F;
				skip = src_o[1];
				src_o += num + 2;

				dst = bp->dst;

				if (skip & 1) {
					skip++;
					if (!--num)
						continue;
				}

				if ( (skip -= bp->start_x) > 0) {
					dst += skip >> 1;
				} else {
					num += skip;
					if (num <= 0)
						continue;
					skip = 0;
				}

				skip = skip + num - bp->width;
				if (skip > 0) {
					num -= skip;
					if (num <= 0)
						continue;
				}

				ctab = _color_remap_ptr;
				num = (num + 1) >> 1;
				if (num) {
					do {
						*dst = ctab[*dst];
						dst++;
					} while (--num);
				}

			} while (!(done & 0x80));
			bp->dst += bp->pitch;
			if (!--bp->height)
				return;

			do {
				done = src_o[0];
				src_o += (done & 0x7F) + 2;
			} while (!(done & 0x80));
		} while (--bp->height);

	} else {
		src_o += READ_LE_UINT16(src_o + bp->start_y * 2);
		do {
			do {
				done = src_o[0];
				num = done & 0x7F;
				skip = src_o[1];
				src = src_o + 2;
				src_o += num + 2;

				dst = bp->dst;

				if (skip & 1) {
					skip++;
					src++;
					if (!--num)
						continue;
				}

				if ( (skip -= bp->start_x) > 0) {
					dst += skip >> 1;
				} else {
					src -= skip;
					num += skip;
					if (num <= 0)
						continue;
					skip = 0;
				}

				skip = skip + num - bp->width;
				if (skip > 0) {
					num -= skip;
					if (num <= 0)
						continue;
				}

				num = (num + 1) >> 1;

				if (num) {
					do {
						*dst = *src;
						dst++;
						src+=2;
					} while (--num);
				}

			} while (!(done & 0x80));

			bp->dst += bp->pitch;
			if (!--bp->height)
				return;

			do {
				done = src_o[0];
				src_o += (done & 0x7F) + 2;
			} while (!(done & 0x80));

		} while (--bp->height);
	}
}

static void GfxBlitZoomMediumUncomp(BlitterParams *bp)
{
	byte *src = bp->sprite;
	byte *dst = bp->dst;
	int height = bp->height;
	int width = bp->width;
	int i;

	assert(height > 0);
	assert(width > 0);

	if (bp->mode & 1) {
		if (bp->info & 1) {
			byte *ctab = _color_remap_ptr,b;
			height >>= 1;
			if (height)	do {
				for(i=0; i!=width>>1; i++)
					if ((b=ctab[src[i*2]]) != 0)
						dst[i] = b;
				src += bp->width_org * 2;
				dst += bp->pitch;
			} while (--height);
		}
	} else if (bp->mode & 2) {
		if (bp->info & 1) {
			byte *ctab = _color_remap_ptr;
			height >>= 1;
			if (height)	do {
				for(i=0; i!=width>>1; i++)
					if (src[i*2])
						dst[i] = ctab[dst[i]];
				src += bp->width_org * 2;
				dst += bp->pitch;
			} while (--height);
		}
	} else {
		if (bp->info & 1) {
			height >>= 1;
			if (height)	do {
				for(i=0; i!=width>>1; i++)
					if (src[i*2])
						dst[i] = src[i*2];
				src += bp->width_org * 2;
				dst += bp->pitch;
			} while (--height);
		}
	}
}

static void GfxBlitTileZoomOut(BlitterParams *bp)
{
	byte *src_o = bp->sprite, *src;
	int num, skip;
	byte done;
	byte *dst, *ctab;

	if (bp->mode & 1) {
		src_o += READ_LE_UINT16(src_o + bp->start_y * 2);
		for(;;) {
			do {
				done = src_o[0];
				num = done & 0x7F;
				skip = src_o[1];
				src = src_o + 2;
				src_o += num + 2;

				dst = bp->dst;

				if (skip & 1) {
					skip++;
					src++;
					if (!--num)
						continue;
				}

				if (skip & 2) {
					skip+=2;
					src+=2;
					if ((num-=2) <= 0)
						continue;
				}

				if ( (skip -= bp->start_x) > 0) {
					dst += skip >> 2;
				} else {
					src -= skip;
					num += skip;
					if (num <= 0)
						continue;
					skip = 0;
				}

				skip = skip + num - bp->width;
				if (skip > 0) {
					num -= skip;
					if (num <= 0)
						continue;
				}

				ctab = _color_remap_ptr;
				num = (num + 3) >> 2;
				if (num) {
					do {
						*dst = ctab[*src];
						dst++;
						src+=4;
					} while (--num);
				}
			} while (!(done & 0x80));
			bp->dst += bp->pitch;
			if (!--bp->height)
				return;

			do {
				done = src_o[0];
				src_o += (done & 0x7F) + 2;
			} while (!(done & 0x80));
			if (!--bp->height)
				return;

			do {
				done = src_o[0];
				src_o += (done & 0x7F) + 2;
			} while (!(done & 0x80));
			if (!--bp->height)
				return;

			do {
				done = src_o[0];
				src_o += (done & 0x7F) + 2;
			} while (!(done & 0x80));
			if (!--bp->height)
				return;
		}
	} else if (bp->mode & 2) {
		src_o += READ_LE_UINT16(src_o + bp->start_y * 2);
		for(;;) {
			do {
				done = src_o[0];
				num = done & 0x7F;
				skip = src_o[1];
				src_o += num + 2;

				dst = bp->dst;

				if (skip & 1) {
					skip++;
					if (!--num)
						continue;
				}

				if (skip & 2) {
					skip+=2;
					if ((num-=2) <= 0)
						continue;
				}

				if ( (skip -= bp->start_x) > 0) {
					dst += skip >> 2;
				} else {
					num += skip;
					if (num <= 0)
						continue;
					skip = 0;
				}

				skip = skip + num - bp->width;
				if (skip > 0) {
					num -= skip;
					if (num <= 0)
						continue;
				}

				ctab = _color_remap_ptr;
				num = (num + 3) >> 2;
				if (num) {
					do {
						*dst = ctab[*dst];
						dst++;
					} while (--num);
				}

			} while (!(done & 0x80));
			bp->dst += bp->pitch;
			if (!--bp->height)
				return;

			do {
				done = src_o[0];
				src_o += (done & 0x7F) + 2;
			} while (!(done & 0x80));
			if (!--bp->height)
				return;

			do {
				done = src_o[0];
				src_o += (done & 0x7F) + 2;
			} while (!(done & 0x80));
			if (!--bp->height)
				return;

			do {
				done = src_o[0];
				src_o += (done & 0x7F) + 2;
			} while (!(done & 0x80));
			if (!--bp->height)
				return;
		}
	} else {
		src_o += READ_LE_UINT16(src_o + bp->start_y * 2);
		for(;;) {
			do {
				done = src_o[0];
				num = done & 0x7F;
				skip = src_o[1];
				src = src_o + 2;
				src_o += num + 2;

				dst = bp->dst;

				if (skip & 1) {
					skip++;
					src++;
					if (!--num)
						continue;
				}

				if (skip & 2) {
					skip+=2;
					src+=2;
					if ((num-=2) <= 0)
						continue;
				}

				if ( (skip -= bp->start_x) > 0) {
					dst += skip >> 2;
				} else {
					src -= skip;
					num += skip;
					if (num <= 0)
						continue;
					skip = 0;
				}

				skip = skip + num - bp->width;
				if (skip > 0) {
					num -= skip;
					if (num <= 0)
						continue;
				}

				num = (num + 3) >> 2;

				if (num) {
					do {
						*dst = *src;
						dst++;
						src+=4;
					} while (--num);
				}

			} while (!(done & 0x80));

			bp->dst += bp->pitch;
			if (!--bp->height)
				return;

			do {
				done = src_o[0];
				src_o += (done & 0x7F) + 2;
			} while (!(done & 0x80));
			if (!--bp->height)
				return;

			do {
				done = src_o[0];
				src_o += (done & 0x7F) + 2;
			} while (!(done & 0x80));
			if (!--bp->height)
				return;

			do {
				done = src_o[0];
				src_o += (done & 0x7F) + 2;
			} while (!(done & 0x80));
			if (!--bp->height)
				return;
		}
	}
}

static void GfxBlitZoomOutUncomp(BlitterParams *bp)
{
	byte *src = bp->sprite;
	byte *dst = bp->dst;
	int height = bp->height;
	int width = bp->width;
	int i;

	assert(height > 0);
	assert(width > 0);

	if (bp->mode & 1) {
		if (bp->info & 1) {
			byte *ctab = _color_remap_ptr,b;
			height >>= 2;
			if (height)	do {
				for(i=0; i!=width>>2; i++)
					if ((b=ctab[src[i*4]]) != 0)
						dst[i] = b;
				src += bp->width_org * 4;
				dst += bp->pitch;
			} while (--height);
		}
	} else if (bp->mode & 2) {
		if (bp->info & 1) {
			byte *ctab = _color_remap_ptr;
			height >>= 2;
			if (height)	do {
				for(i=0; i!=width>>2; i++)
					if (src[i*4])
						dst[i] = ctab[dst[i]];
				src += bp->width_org * 4;
				dst += bp->pitch;
			} while (--height);
		}
	} else {
		if (bp->info & 1) {
			height >>= 2;
			if (height)	do {
				for(i=0; i!=width>>2; i++)
					if (src[i*4])
						dst[i] = src[i*4];
				src += bp->width_org * 4;
				dst += bp->pitch;
			} while (--height);
		}
	}
}

typedef void (*BlitZoomFunc)(BlitterParams *bp);

static void GfxMainBlitter(byte *sprite, int x, int y, int mode)
{
	DrawPixelInfo *dpi = _cur_dpi;
	int start_x, start_y;
	byte info;
	BlitterParams bp;
	int zoom_mask = ~((1 << (dpi->zoom))-1);

	static const BlitZoomFunc zf_tile[3] =
	{
		GfxBlitTileZoomIn,
		GfxBlitTileZoomMedium,
		GfxBlitTileZoomOut
	};
	static const BlitZoomFunc zf_uncomp[3] =
	{
		GfxBlitZoomInUncomp,
		GfxBlitZoomMediumUncomp,
		GfxBlitZoomOutUncomp
	};

	/* decode sprite header */
	x += (int16)READ_LE_UINT16(&((SpriteHdr*)sprite)->x_offs);
	y += (int16)READ_LE_UINT16(&((SpriteHdr*)sprite)->y_offs);
	bp.width_org = bp.width = READ_LE_UINT16(&((SpriteHdr*)sprite)->width);
	bp.height_org = bp.height = ((SpriteHdr*)sprite)->height;
	info = ((SpriteHdr*)sprite)->info;
	bp.info = info;
	bp.sprite_org = bp.sprite = sprite + sizeof(SpriteHdr);
	bp.dst = dpi->dst_ptr;
	bp.mode = mode;
	bp.pitch = dpi->pitch;

	assert(bp.height > 0);
	assert(bp.width > 0);

	if (info & 8) {
		/* tile blit */
		start_y = 0;

		if (dpi->zoom > 0) {
			start_y += bp.height &~ zoom_mask;
			bp.height &= zoom_mask; 
			if (bp.height == 0) return;
			y&=zoom_mask;
		}

		if ( (y -= dpi->top) < 0) {
			if ((bp.height += y) <= 0)
				return;
			start_y -= y;
			y = 0;
		} else {
			bp.dst += bp.pitch * (y>>(dpi->zoom));
		}
		bp.start_y = start_y;

		if ( (y = y + bp.height - dpi->height) > 0) {
			if ( (bp.height -= y) <= 0)
				return;
		}

		start_x = 0;
		x &= zoom_mask;
		if ( (x -= dpi->left) < 0) {
			if ((bp.width += x) <= 0)
				return;
			start_x -= x;
			x = 0;
		}
		bp.start_x = start_x;
		bp.dst += x>>(dpi->zoom);

		if ( (x = x + bp.width - dpi->width) > 0) {
			if ( (bp.width -= x) <= 0)
				return;
		}

		zf_tile[dpi->zoom](&bp);
	} else {

		bp.sprite += bp.width * (bp.height & ~zoom_mask);
		bp.height &= zoom_mask;
		if (bp.height == 0)
			return;

		y &= zoom_mask;

		if ( (y -= dpi->top) < 0) {
			if ((bp.height += y) <= 0)
				return;
			bp.sprite -= bp.width * y;
			y = 0;
		} else {
			bp.dst += bp.pitch * (y>>(dpi->zoom));
		}

		if ( (y = y + bp.height - dpi->height) > 0) {
			if ( (bp.height -= y) <= 0)
				return;
		}

		start_x = 0;

		x &= zoom_mask;

		if ( (x -= dpi->left) < 0) {
			if ((bp.width += x) <= 0)
				return;
			start_x -= x;
			bp.sprite -= x;
			x = 0;
		}
		bp.dst += x>>(dpi->zoom);

		if ( (x = x + bp.width - dpi->width) > 0) {
			if ( (bp.width -= x) <= 0)
				return;
			start_x += x;
		}
		bp.start_x = start_x;

		if (info&2) {
			int totpix = bp.height_org * bp.width_org;
			byte *dst = (byte*)alloca(totpix);
			byte *src = bp.sprite_org;
			signed char b;

			bp.sprite += (dst - src);

			while (totpix != 0) {
				assert(totpix > 0);
				b = *src++;
				if (b >= 0) {
					int i, count=b;
					for(i=0; i!=count; i++)
						dst[i] = src[i];
					dst += count;
					src += count;
					totpix -= count;
				} else {
					byte *tmp = dst- (((b&7)<<8)|(*src++));
					int i;
					int count = -(b >> 3);

					for(i=0; i!=count; i++)
						dst[i] = tmp[i];
					dst += count;
					totpix -= count;
				}
			}
		}
		zf_uncomp[dpi->zoom](&bp);
	}
}


void GfxScalePalette(int pal, byte scaling)
{
	byte *dst, *src;
	size_t count;

	GfxInitPalettes();

	dst = _cur_palette;
	src = GET_PALETTE(pal);
	count = 256;
	do {
		dst[0] = (byte)(src[0] * scaling >> 8);
		dst[1] = (byte)(src[1] * scaling >> 8);
		dst[2] = (byte)(src[2] * scaling >> 8);
		dst += 3;
		src += 3;
	} while (--count);
}

void GfxInitPalettes()
{
	memcpy(_cur_palette, _palettes[0], 256*3);
	_pal_first_dirty = 0;
	_pal_last_dirty = 255;
}


#define EXTR(p,q) (((uint16)(_timer_counter * (p)) * (q)) >> 16)
#define EXTR2(p,q) (((uint16)(~_timer_counter * (p)) * (q)) >> 16)
#define COPY_TRIPLET do {d[0]=s[0+j]; d[1]=s[1+j]; d[2]=s[2+j];d+=3;}while(0)

void DoPaletteAnimations()
{
	const byte *s;
	byte *d;
	int j;
	int i;
	const ExtraPaletteValues *ev = &_extra_palette_values;
	byte old_val[28*3];

	d = _cur_palette + 217*3;
	memcpy(old_val, d, 28*3);

	// Dark blue water
	s = ev->a;
	if (_opt.landscape == LT_CANDY) s = ev->ac;
	j = EXTR(320,5) * 3;
	for(i=0; i!=5; i++) {
		COPY_TRIPLET;
		j+=3;
		if (j == 15) j = 0;
	}

	// Glittery water
	s = ev->b;
	if (_opt.landscape == LT_CANDY) s = ev->bc;
	j = EXTR(128, 15) * 3;
	for(i=0; i!=5; i++) {
		COPY_TRIPLET;
		j += 9;
		if (j >= 45) j -= 45;
	}

	s = ev->e;
	j = EXTR2(512, 5) * 3;
	for(i=0; i!=5; i++) {
		COPY_TRIPLET;
		j += 3;
		if (j == 3*5) j = 0;
	}

	// Oil refinery fire animation
	s = ev->oil_ref;
	j = EXTR2(512, 7) * 3;
	for(i=0; i!=7; i++) {
		COPY_TRIPLET;
		j += 3;
		if (j == 3*7) j = 0;
	}

	// Radio tower blinking
	{
		byte i,v;
		i = (_timer_counter >> 1) & 0x7F;
		(v = 255, i < 0x3f) ||
		(v = 128, i < 0x4A || i >= 0x75) ||
		(v = 20);
		d[0] = v;
		d[1] = d[2] = 0;
		d += 3;

		i ^= 0x40;
		(v = 255, i < 0x3f) ||
		(v = 128, i < 0x4A || i >= 0x75) ||
		(v = 20);
		d[0] = v;

		d[1] = d[2] = 0;
		d += 3;
	}

	// Handle lighthouse and stadium animation
	s = ev->lighthouse;
	j = EXTR(256, 4) * 3;
	for(i=0; i!=4; i++) {
		COPY_TRIPLET;
		j += 3;
		if (j == 3*4) j = 0;
	}

	if (memcmp(old_val, _cur_palette + 217*3, 28*3)) {
		if (_pal_first_dirty > 217) _pal_first_dirty = 217;
		if (_pal_last_dirty < 217+29-1) _pal_last_dirty = 217+29-1;
	}
}


void LoadStringWidthTable()
{
	int i;
	byte *b;

	b = _stringwidth_table;

	// 2 equals space.
	for(i=2; i != 0xE2; i++) {
		*b++ = (byte)((i < 93 || i >= 129 || i == 98) ? GetSpritePtr(i)[2] : 0);
	}

	for(i=0xE2; i != 0x1C2; i++) {
		*b++ = (byte)((i < 317 || i >= 353) ? GetSpritePtr(i)[2]+1 : 0);
	}

	for(i=0x1C2; i != 0x2A2; i++) {
		*b++ = (byte)((i < 541 || i >= 577) ? GetSpritePtr(i)[2]+1 : 0);
	}
}

void ScreenSizeChanged()
{
	// check the dirty rect
	if (_invalid_rect.right >= _screen.width) _invalid_rect.right = _screen.width;
	if (_invalid_rect.bottom >= _screen.height) _invalid_rect.bottom = _screen.height;

	// screen size changed and the old bitmap is invalid now, so we don't want to undraw it
	_cursor.visible = false;
}

void UndrawMouseCursor()
{
	if (_cursor.visible) {
		_cursor.visible = false;
		memcpy_pitch(
			_screen.dst_ptr + _cursor.draw_pos.x + _cursor.draw_pos.y * _screen.pitch,
			_cursor_backup,
			_cursor.draw_size.x, _cursor.draw_size.y, _cursor.draw_size.x, _screen.pitch);
		
		_video_driver->make_dirty(_cursor.draw_pos.x, _cursor.draw_pos.y, _cursor.draw_size.x, _cursor.draw_size.y);
	}
}

void DrawMouseCursor()
{
	int x,y,w,h;

	// Don't draw the mouse cursor if it's already drawn
	if (_cursor.visible) {
		if (!_cursor.dirty)
			return;
		UndrawMouseCursor();
	}

	w = _cursor.size.x;
	x = _cursor.pos.x + _cursor.offs.x;
	if (x < 0) { w += x; x=0; }
	if (w>_screen.width-x) { w = _screen.width-x; }
	if (w <= 0) return;
	_cursor.draw_pos.x = x;
	_cursor.draw_size.x = w;

	h = _cursor.size.y;
	y = _cursor.pos.y + _cursor.offs.y;
	if (y < 0) { h += y; y=0; }
	if (h>_screen.height-y) { h = _screen.height-y; }
	if (h <= 0) return;
	_cursor.draw_pos.y = y;
	_cursor.draw_size.y = h;

	assert(w*h < sizeof(_cursor_backup));

	// Make backup of stuff below cursor
	memcpy_pitch(
		_cursor_backup,
		_screen.dst_ptr + _cursor.draw_pos.x + _cursor.draw_pos.y * _screen.pitch,
		_cursor.draw_size.x, _cursor.draw_size.y, _screen.width, _cursor.draw_size.x);

	// Draw cursor on screen
	_cur_dpi = &_screen;
	DrawSprite(_cursor.sprite, _cursor.pos.x, _cursor.pos.y);

	_video_driver->make_dirty(_cursor.draw_pos.x, _cursor.draw_pos.y, _cursor.draw_size.x, _cursor.draw_size.y);

	_cursor.visible = true;
	_cursor.dirty = false;
}

void DbgScreenRect(int left, int top, int right, int bottom)
{
	DrawPixelInfo dp,*old;

	old = _cur_dpi;
	_cur_dpi = &dp;
	dp = _screen;
	GfxFillRect(left, top, right-1, bottom-1, rand() & 255);
	_cur_dpi = old;
}

extern bool _dbg_screen_rect;

void RedrawScreenRect(int left, int top, int right, int bottom)
{
	assert(right <= _screen.width && bottom <= _screen.height);
	if (_cursor.visible) {
		if (right > _cursor.draw_pos.x &&
				left < _cursor.draw_pos.x + _cursor.draw_size.x &&
				bottom > _cursor.draw_pos.y &&
				top < _cursor.draw_pos.y + _cursor.draw_size.y) {
			UndrawMouseCursor();
		}
	}

#if defined(_DEBUG)
	if (_dbg_screen_rect)
		DbgScreenRect(left, top, right, bottom);
	else
#endif
		DrawOverlappedWindowForAll(left, top, right, bottom);

	_video_driver->make_dirty(left, top, right-left, bottom-top);
}

void DrawDirtyBlocks()
{
	byte *b = _dirty_blocks;
	int x=0,y=0;
	int w = (_screen.width + 63) & ~63;
	int h = _screen.height;

	do {
		if (*b != 0) {
			int left,top;
			int right = x + 64;
			int bottom = y;
			byte *p = b;
			int h2;
			// First try coalescing downwards
			do {
				*p = 0;
				p += DIRTY_BYTES_PER_LINE;
				bottom += 8;
			} while (bottom != h && *p);

			// Try coalescing to the right too.
			h2 = (bottom - y) >> 3;
			assert(h2>0);
			p = b;

			while (right != w) {
				byte *p2 = ++p;
				int h = h2;
				// Check if a full line of dirty flags is set.
				do {
					if (!*p2) goto no_more_coalesc;
					p2 += DIRTY_BYTES_PER_LINE;
				} while (--h);

				// Wohoo, can combine it one step to the right!
				// Do that, and clear the bits.
				right += 64;

				h = h2;
				p2 = p;
				do {
					*p2 = 0;
					p2 += DIRTY_BYTES_PER_LINE;
				} while (--h);
			}
			no_more_coalesc:;

			left = x;
			top = y;

			if (left < _invalid_rect.left)left = _invalid_rect.left;
			if (top < _invalid_rect.top)	top = _invalid_rect.top;
			if (right > _invalid_rect.right)right = _invalid_rect.right;
			if (bottom > _invalid_rect.bottom)bottom = _invalid_rect.bottom;

			if (left < right && top < bottom) {
				RedrawScreenRect(left, top, right, bottom);
			}

		}
	} while (b++, (x+=64) != w || (x=0,b+=-(w>>6)+DIRTY_BYTES_PER_LINE,(y+=8) != h));

	_invalid_rect.left = w;
	_invalid_rect.top = h;
	_invalid_rect.right = 0;
	_invalid_rect.bottom = 0;
}


void SetDirtyBlocks(int left, int top, int right, int bottom)
{
	byte *b;
	int width,height,i;

	if (left < 0) left = 0;
	if (top < 0) top = 0;
	if (right > _screen.width) right = _screen.width;
	if (bottom > _screen.height) bottom = _screen.height;

	if (left >= right || top >= bottom)
		return;

	if (left < _invalid_rect.left)	_invalid_rect.left = left;
	if (top < _invalid_rect.top)  	_invalid_rect.top = top;
	if (right > _invalid_rect.right)_invalid_rect.right = right;
	if (bottom > _invalid_rect.bottom)_invalid_rect.bottom = bottom;

	left >>= 6;
	top >>= 3;

	b = _dirty_blocks + top * DIRTY_BYTES_PER_LINE + left;

	width = ((right-1) >> 6) - left + 1;
	height = ((bottom-1) >> 3) - top + 1;

	assert(width > 0 && height > 0);

	do {
		i=width;
		do b[--i] = 0xFF; while (i);

		b += DIRTY_BYTES_PER_LINE;
	} while (--height);
}

void MarkWholeScreenDirty()
{
	SetDirtyBlocks(0, 0, _screen.width, _screen.height);
}

bool FillDrawPixelInfo(DrawPixelInfo *n, DrawPixelInfo *o, int left, int top, int width, int height)
{
	int t;

	if (o == NULL)
		o = _cur_dpi;

	n->zoom = 0;

	assert(width > 0);
	assert(height > 0);

	n->left = 0;
	if ((left -= o->left) < 0) {
		if ((width += left) < 0)
			return false;
		n->left = -left;
		left = 0;
	}

	if ((t=width + left - o->width) > 0) {
		if ((width -= t) < 0)
			return false;
	}
	n->width = width;

	n->top = 0;
	if ((top -= o->top) < 0) {
		if ((height += top) < 0)
			return false;
		n->top = -top;
		top = 0;
	}

	n->dst_ptr = o->dst_ptr + left + top * (n->pitch = o->pitch);

	if ((t=height + top - o->height) > 0) {
		if ((height-=t) < 0)
			return false;
	}
	n->height = height;


	return true;
}

static void SetCursorSprite(uint cursor)
{
	CursorVars *cv = &_cursor;
	byte *p;

	if (cv->sprite == cursor)
		return;

	p =	GetSpritePtr(cursor & 0x3FFF);
	cv->sprite = cursor;
	cv->size.y = *(byte*)(p+1);
	cv->size.x = READ_LE_UINT16(p+2);
	cv->offs.x = (int16)READ_LE_UINT16(p+4);
	cv->offs.y = (int16)READ_LE_UINT16(p+6);

	cv->dirty = true;
}

static void SwitchAnimatedCursor()
{
	CursorVars *cv = &_cursor;
	const uint16 *cur;
	uint sprite;

	cur = cv->animate_cur;
	if (cur == NULL || *cur == 0xFFFF)
		cur = cv->animate_list;
	sprite = cur[0];
	cv->animate_timeout = cur[1];
	cv->animate_cur = cur + 2;

	SetCursorSprite(sprite);
}

void CursorTick() {
	CursorVars *cv = &_cursor;
	if (cv->animate_timeout && !--cv->animate_timeout)
		SwitchAnimatedCursor();
}

void SetMouseCursor(uint cursor)
{
	// Turn off animation
	_cursor.animate_timeout = 0;
	// Set cursor
	SetCursorSprite(cursor);
}

void SetAnimatedMouseCursor(const uint16 *table)
{
	_cursor.animate_list = table;
	_cursor.animate_cur = NULL;
	SwitchAnimatedCursor();
}

bool ChangeResInGame(int w, int h)
{
	if ((_screen.width != w || _screen.height != h) && !_video_driver->change_resolution(w, h))
		return false;

	_cur_resolution[0] = w;
	_cur_resolution[1] = h;
	return true;
}
