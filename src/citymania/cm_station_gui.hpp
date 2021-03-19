#ifndef CM_STATION_GUI_HPP
#define CM_STATION_GUI_HPP

#include "../core/geometry_type.hpp"
#include "../command_type.h"
#include "../station_gui.h"
#include "../station_type.h"

namespace citymania {

enum class StationBuildingStatus {
    IMPOSSIBLE = 0,
    QUERY = 1,
    JOIN = 2,
    NEW = 3,
};

// void SetStationBiildingStatus(StationBuildingStatus status);
// void SetStationTileSelectSize(int w, int h, int catchment);
bool UseImprovedStationJoin();
void OnStationTileSetChange(const Station *station, bool adding, StationType type);
void OnStationPartBuilt(const Station *station, TileIndex tile, uint32 p1, uint32 p2);
void PlaceRoadStop(TileIndex start_tile, TileIndex end_tile, uint32 p2, uint32 cmd);
void HandleStationPlacement(TileIndex start, TileIndex end);
void PlaceRail_Station(TileIndex tile);
void PlaceDock(TileIndex tile);
void PlaceAirport(TileIndex tile);

void SelectStationToJoin(const Station *station);
// const Station *GetStationToJoin();
void MarkCoverageHighlightDirty();
bool CheckRedrawStationCoverage();
void AbortStationPlacement();

std::string GetStationCoverageProductionText(TileIndex tile, int w, int h, int rad, StationCoverageType sct);

} // namespace citymania

#endif
