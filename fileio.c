#include "stdafx.h"
#include "ttd.h"
#if defined(UNIX)
#include <ctype.h> // required for tolower()
#endif

/*************************************************/
/* FILE IO ROUTINES ******************************/
/*************************************************/

#define FIO_BUFFER_SIZE 512

typedef struct {
	byte *buffer, *buffer_end;
	uint32 pos;
	FILE *cur_fh;
	FILE *handles[32];
	byte buffer_start[512];
} Fio;

static Fio _fio;

// Get current position in file
uint32 FioGetPos()
{
	return _fio.pos + (_fio.buffer - _fio.buffer_start) - FIO_BUFFER_SIZE;
}

void FioSeekTo(uint32 pos, int mode)
{
	if (mode == SEEK_CUR) pos += FioGetPos();
	_fio.buffer = _fio.buffer_end = _fio.buffer_start + FIO_BUFFER_SIZE;
	fseek(_fio.cur_fh, (_fio.pos=pos), SEEK_SET);
}

// Seek to a file and a position
void FioSeekToFile(uint32 pos)
{
	FILE *f = _fio.handles[pos >> 24];
	assert(f != NULL);
	_fio.cur_fh = f;
	FioSeekTo(pos & 0xFFFFFF, SEEK_SET);
}

byte FioReadByte()
{
	if (_fio.buffer == _fio.buffer_end) {
		_fio.pos += FIO_BUFFER_SIZE;
		fread(_fio.buffer = _fio.buffer_start, 1, FIO_BUFFER_SIZE, _fio.cur_fh);
	}
	return *_fio.buffer++;
}

void FioSkipBytes(int n)
{
	for(;;) {
		int m = min(_fio.buffer_end - _fio.buffer, n);
		_fio.buffer += m;
		n -= m;
		if (n == 0) break;
		FioReadByte();
		n--;
	}
}


uint16 FioReadWord()
{
	byte b = FioReadByte();
	return (FioReadByte() << 8) | b;
}

uint32 FioReadDword()
{
	uint b = FioReadWord();
	return (FioReadWord() << 16) | b;
}

void FioReadBlock(void *ptr, uint size)
{
	FioSeekTo(FioGetPos(), SEEK_SET);
	_fio.pos += size;
	fread(ptr, 1, size, _fio.cur_fh);
}

void FioCloseAll()
{
	int i;

	for(i=0; i!=lengthof(_fio.handles); i++) {
		if (_fio.handles[i] != NULL) {
			fclose(_fio.handles[i]);
			_fio.handles[i] = NULL;
		}
	}
}

void FioOpenFile(int slot, const char *filename)
{
	FILE *f;
	char buf[MAX_PATH];

	sprintf(buf, "%s%s", _path.data_dir, filename);
	
	f = fopen(buf, "rb");
#if !defined(WIN32)
	if (f == NULL) {
		char *s;
		// Make lower case and try again
		for(s=buf + strlen(_path.data_dir) - 1; *s != 0; s++)
			*s = tolower(*s);
		f = fopen(buf, "rb");
	}
#endif

	if (f == NULL)
		error("Cannot open file '%s'", buf);
	
	_fio.handles[slot] = f;
	FioSeekToFile(slot << 24);
}


