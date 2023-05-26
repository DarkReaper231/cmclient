#ifndef FUNCTIONS_H
#define FUNCTIONS_H

/* vehicle.c */

/* window.c */


/* landscape.c */
void FindLandscapeHeight(TileInfo *ti, uint x, uint y);
void FindLandscapeHeightByTile(TileInfo *ti, uint tile);
uint GetTileSlope(uint tile, int *h);
int GetTileZ(uint tile);

void DoClearSquare(uint tile);
void CDECL ModifyTile(uint tile, uint flags, ...);
void SetMapExtraBits(uint tile, byte flags);
uint GetMapExtraBits(uint tile);
void RunTileLoop();

uint GetPartialZ(int x, int y, int corners);
uint GetSlopeZ(int x, int y);
uint32 GetTileTrackStatus(uint tile, int mode);
void GetAcceptedCargo(uint tile, AcceptedCargo *ac);
void ChangeTileOwner(uint tile, byte old_player, byte new_player);
void AnimateTile(uint tile);
void ClickTile(uint tile);
void GetTileDesc(uint tile, TileDesc *td);
void DrawTile(TileInfo *ti);

uint TileAddWrap(TileIndex tile, int add);
enum {
	TILE_WRAPPED = (uint)-1
};

bool IsValidTile(uint tile);

#if !defined(NEW_ROTATION)
static Point FORCEINLINE RemapCoords(int x, int y, int z) { Point pt = { (y-x)*2, y + x -z }; return pt; }
#else
static Point FORCEINLINE RemapCoords(int x, int y, int z) { Point pt = { (x + y)*2, x - y -z }; return pt; }
#endif

static Point FORCEINLINE RemapCoords2(int x, int y) { return RemapCoords(x, y, GetSlopeZ(x, y)); }

/* game.c */
byte *GetString(byte *buffr, uint16 string);
void InjectDparam(int amount);

int32 GetParamInt32();
int GetParamInt16();
int GetParamInt8();
int GetParamUint16();


/* sound.c */
void SndPlayTileFx(int sound, TileIndex tile);
void SndPlayVehicleFx(int sound, Vehicle *v);
void SndPlayFx(int sound);

/* clear_land.c */
void DrawHillyLandTile(TileInfo *ti);
void DrawClearLandTile(TileInfo *ti, byte set);
void DrawClearLandFence(TileInfo *ti, byte img);
void TileLoopClearHelper(uint tile);

/* station_land.c */
void StationPickerDrawSprite(int x, int y, int railtype, int image);

/* track_land.c */
void DrawTrainDepotSprite(int x, int y, int image, int railtype);

/* road_land.c */
void DrawRoadDepotSprite(int x, int y, int image);

/* water_land.c */
void DrawShipDepotSprite(int x, int y, int image);
void TileLoop_Water(uint tile);

/* players.c */
bool CheckPlayerHasMoney(int32 cost);
void SubtractMoneyFromPlayer(int32 cost);
void SubtractMoneyFromPlayerFract(byte player, int32 cost);
bool CheckOwnership(byte owner);
bool CheckTileOwnership(uint tile);
StringID GetPlayerNameString(byte player);

/* standard */
void ShowInfo(const char *str);
void CDECL ShowInfoF(const char *str, ...);
void NORETURN CDECL error(const char *str, ...);
void memswap(void *a, void *b, size_t size);

/* ttd.c */
uint32 Random();
uint RandomRange(uint max);

uint32 InteractiveRandom(); /* Used for random sequences that are not the same on the other end of the multiplayer link */
void SetDate(uint date);
/* facedraw.c */
void DrawPlayerFace(uint32 face, int color, int x, int y);

/* texteff.c */
void MoveAllTextEffects();
void AddTextEffect(StringID msg, int x, int y, uint16 duration);
void InitTextEffects();
void DrawTextEffects(DrawPixelInfo *dpi);

bool AddAnimatedTile(uint tile);
void DeleteAnimatedTile(uint tile);
void AnimateAnimatedTiles();
void InitializeAnimatedTiles();

/* tunnelbridge_cmd.c */
bool CheckTunnelInWay(uint tile, int z);
bool CheckBridge_Stuff(byte bridge_type, int bridge_len);
uint32 GetBridgeLength(TileIndex begin, TileIndex end);
int CalcBridgeLenCostFactor(int x);

/* network.c */
typedef void CommandCallback(bool success, uint tile, uint32 p1, uint32 p2);
bool DoCommandP(TileIndex tile, uint32 p1, uint32 p2, CommandCallback *callback, uint32 cmd);

void NetworkConnect(const char *hostname, int port);
void NetworkReceive();
void NetworkSend();
void NetworkProcessCommands();
void NetworkListen(int port);
void NetworkInitialize();
void NetworkShutdown();
void NetworkSendCommand(TileIndex tile, uint32 p1, uint32 p2, uint32 cmd, CommandCallback *callback);
void NetworkStartSync();
void NetworkUDPListen(int port);
void NetworkUDPReceive();
void NetworkIPListInit();
bool NetworkUDPSearchServer();

/* misc_cmd.c */
void PlaceTreesRandomly();

uint GetTileDist(TileIndex xy1, TileIndex xy2);
uint GetTileDist1D(TileIndex xy1, TileIndex xy2);
uint GetTileDist1Db(TileIndex xy1, TileIndex xy2);
uint GetTileDistAdv(TileIndex xy1, TileIndex xy2);
bool CheckDistanceFromEdge(TileIndex tile, uint distance);

void InitializeLandscapeVariables(bool only_constants);

/* misc.c */
void DeleteName(StringID id);
byte *GetName(int id, byte *buff);
StringID AllocateName(const byte *name, byte skip);
void ConvertDayToYMD(YearMonthDay *ymd, uint16 date);
uint ConvertYMDToDay(uint year, uint month, uint day);
uint ConvertIntDate(uint date);



/* misc functions */
void MarkTileDirty(int x, int y);
void MarkTileDirtyByTile(TileIndex tile);
void InvalidateWindow(byte cls, WindowNumber number);
void InvalidateWindowWidget(byte cls, WindowNumber number, byte widget_index);
void InvalidateWindowClasses(byte cls);
void DeleteWindowById(WindowClass cls, WindowNumber number);

void SetObjectToPlaceWnd(int icon, byte mode, Window *w);
void SetObjectToPlace(int icon, byte mode, byte window_class, uint16 window_num);

void ResetObjectToPlace();
bool ScrollMainWindowToTile(TileIndex tile);
bool ScrollMainWindowTo(int x, int y);
void DrawSprite(uint32 img, int x, int y);
bool EnsureNoVehicle(TileIndex tile);
bool EnsureNoVehicleZ(TileIndex tile, byte z);
void MarkAllViewportsDirty(int left, int top, int right, int bottom);
void ShowCostOrIncomeAnimation(int x, int y, int z, int32 cost);
void MarkWholeScreenDirty();

void DrawFoundation(TileInfo *ti, uint f);

bool CheckIfAuthorityAllows(uint tile);
Town *ClosestTownFromTile(uint tile, uint threshold);
void ChangeTownRating(Town *t, int add, int max);

uint GetRoadBitsByTile(TileIndex tile);
int GetTownRadiusGroup(Town *t, uint tile);
int32 GetTransportedGoodsIncome(uint num_pieces, uint dist, byte transit_days, byte cargo_type);
void ShowRenameSignWindow(SignStruct *ss);
void ShowRenameCheckpointWindow(Checkpoint *cp);
int FindFirstBit(uint32 x);
void ShowHighscoreTable(int tbl);
TileIndex AdjustTileCoordRandomly(TileIndex a, byte rng);

enum SaveOrLoadResult {
	SL_OK = 0, // completed successfully
	SL_ERROR = 1, // error that was caught before internal structures were modified
	SL_REINIT = 2, // error that was caught in the middle of updating game state, need to clear it. (can only happen during load)
};
enum SaveOrLoadMode {
	SL_INVALID = -1,
	SL_LOAD = 0,
	SL_SAVE = 1,
	SL_OLD_LOAD = 2,
};

int SaveOrLoad(const char *filename, int mode);

void AfterLoadTown();
void AskExitGame();
void AskExitToGameMenu();

void RedrawAutosave();

StringID RemapOldStringID(StringID s);

void UpdateViewportSignPos(ViewportSign *sign, int left, int top, StringID str);

enum {
	SLD_LOAD_GAME = 0,
	SLD_LOAD_SCENARIO = 1,
	SLD_SAVE_GAME = 2,
	SLD_SAVE_SCENARIO = 3,
	SLD_NEW_GAME = 4,
};
void ShowSaveLoadDialog(int mode);

void ttd_strlcpy(char *dst, const char *src, size_t len);

// callback from drivers that is called if the game size changes dynamically
void GameSizeChanged();
void ZoomInOrOutToCursor(bool in);
bool MakeScreenshot();
bool MakeWorldScreenshot(int left, int top, int width, int height, int zoom);
bool FileExists(const char *filename);
bool ReadLanguagePack(int index);
void InitializeLanguagePacks();
byte *ReadFileToMem(const char *filename, size_t *lenp, size_t maxsize);
int GetLanguageList(char **languages, int max);

const char *GetScreenshotFormatDesc(int i);
void InitializeScreenshotFormats();
void SetScreenshotFormat(int i);
void CheckSwitchToEuro();

void LoadFromConfig();
void SaveToConfig();
int ttd_main(int argc, char* argv[]);

void DeterminePaths();
char * CDECL str_fmt(const char *str, ...);

#endif /* FUNCTIONS_H */
