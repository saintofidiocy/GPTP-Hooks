Allows for the customization of race selection in lobbies and Random race selection on map start.

- `CustomRaceList` is the race IDs and network.tbl string indexes that get added to the dropdown list in the lobby. Entries can be removed or reordered as desired.
- `RandomRaceList` is the list of race IDs that get chosen from when Random race is selected, and is fully independent of `CustomRaceList`. Entries can be duplicated within this list or the `getRandomRace` function can be modified to change the distribution (StarCraft normally tries to keep the distribution roughly equal).

### To Install
- Put the files somewhere in your GPTP project (e.g. \/hooks\/)
- Include `races.h` and call `hooks::injectCustomizedRaces();` in initialize.cpp


### Virtual Races
The race list can have custom race ID's for additional race and starting options. These custom race IDs will be replaced by a valid ID during map load (since I'm not about to implement true additional races), but the custom values remain accessible by calling `hooks::getVirtualRace(playerID)`.
The most basic steps to use virtual races:
- Add entries to `CustomRaceList` and, if desired, `RandomRaceList` using unused IDs (>7).
- Set valid race IDs for virtual races in the `setVirtualRaces` function (otherwise the game will crash when it tries to load assests for the invalid race).
- Modify the functions of `create_init_units.cpp` using `getVirtualRace` rather than the given race ID to set different starting units for the custom races.
