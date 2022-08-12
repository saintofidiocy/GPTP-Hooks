Allows for the customization of race selection in lobbies and Random race selection on map start.

- `CustomRaceList` is the race IDs and network.tbl string indexes that get added to the dropdown list in the lobby. Entries can be removed or reordered as desired.
- `RandomRaceList` is the list of race IDs that get chosen from when Random race is selected, and is fully independent of `CustomRaceList`. Entries can be duplicated within this list or the `getRandomRace` function can be modified to change the distribution (StarCraft normally tries to keep the distribution roughly equal).

### To Install
- Put the files somewhere in your GPTP project (e.g. \/hooks\/)
- Include `races.h` and call `hooks::injectCustomizedRaces();` in initialize.cpp
