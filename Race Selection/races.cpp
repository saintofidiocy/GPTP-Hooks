#include "races.h"

namespace hooks {

  // Replaces the table used to populate the race dropdown menus in gluCustm and gluChat
  RaceTable CustomRaceList[] = {
//  { Race ID,         network.tbl string index },
    { RaceId::Zerg,    7 },
    { RaceId::Terran,  8 },
    { RaceId::Protoss, 9 },
    { RaceId::Random,  10 },
  };
  u32 CustomRaceCount = sizeof(CustomRaceList) / sizeof(RaceTable);

  // Race ID to be selected if a player has an invalid race
  const u8 DefaultRace = RaceId::Random;

  // Races that can be selected by RaceId::Random -- can have duplicates, and is independent of CustomRaceList
  u8 RandomRaceList[] = {
    RaceId::Zerg,
    RaceId::Terran,
    RaceId::Protoss,
  };
  const u32 RandomRaceCount = sizeof(RandomRaceList);
  u32 RandomRaceMax = RandomRaceCount - 1;


  // Holds custom race IDs
  u8 playerVirtualRaces[8] = { 0 };
  u8 virtualRaceShuffle[8] = { 0 };


  // Used to keep track of Random race selection
  u32 shuffleCount;
  bool shuffleSel[RandomRaceCount];



  // Stores custom virtual Race ID's while setting a valid Race ID
  PLAYER* activePlayers = (PLAYER*)(0x0057EEE0); // redefine because it's const in scbwdata.h
  void setVirtualRaces() {
    for (int i = 0; i < 8; i++) {
      playerVirtualRaces[i] = activePlayers[i].race;
      virtualRaceShuffle[i] = activePlayers[i].race;

      if (activePlayers[i].race > RaceId::Protoss) {
        // TODO: Set virtual races to appropriate valid IDs !!!
        activePlayers[i].race = RaceId::Terran;
      }
    }
  }

  // Virtual race ID's accessible from anywhere
  u32 getVirtualRace(u8 player) {
    if (player >= 8) return RaceId::None;
    return playerVirtualRaces[player];
  }

  // Keeps virtual race ID's in sync with in-game player ID's with random start locations
  void playerShuffle(u8 newID, u8 oldID) {
    playerVirtualRaces[newID] = virtualRaceShuffle[oldID];
  }


  // Shuffles races, attempting to give a relatively even distribution
  u32* innerShuffleCount = (u32*)(0x0059BF60);
  u32 getRandomRace(u32 index, u32 count) {

    // initialize race shuffling when it has been initialized internally
    if (*innerShuffleCount == 3) {
      *innerShuffleCount = 0;
      shuffleCount = RandomRaceCount;
      for (int i = 0; i < RandomRaceCount; i++) {
        shuffleSel[i] = false;
      }
    }

    u32 raceSel;
    if (shuffleCount == 1) { // 1 choice -- pick whatever has not already been selected
      for (int i = 0; i < RandomRaceCount; i++) {
        if (shuffleSel[i] == false) {
          raceSel = i;
          break;
        }
      }
      *innerShuffleCount = 3; // re-initialize

      // reshuffle the last player if the number of players is a multiple of the number of races
      if (RandomRaceCount > 1 && (index % RandomRaceCount) == RandomRaceMax && (count % RandomRaceCount) == 0) {
        raceSel = scbw::randBetween(0, RandomRaceMax);
      }
    } else { // select a value that hasn't already been selected
      raceSel = scbw::randBetween(0, shuffleCount - 1);

      // skip values that have been selected
      for (u32 i = 0; i <= raceSel; i++) {
        if (shuffleSel[i] == true) {
          raceSel++;
        }
      }

      // I actually have no idea
      if (index == count - 1 && count == RandomRaceMax) {
        if (scbw::randBetween(0, 7) == 4) { // 1/8 chance ????
          raceSel = scbw::randBetween(0, RandomRaceMax);
        }
      }

      shuffleCount--;
      shuffleSel[raceSel] = true;
    }

    return RandomRaceList[raceSel];
  }


  // Prevents maps from loading an unavailable race
  PLAYER* playerTableLoad = (PLAYER*)(0x0059BDB0);
  u8* userSelectable = (u8*)(0x0059BDA8);
  void loadMapRaces() {
    u32 i;
    for (i = 0; i < 8; i++) {
      playerTableLoad[i].id = i;
      playerTableLoad[i].actions = 0xFFFFFFFF;

      if (i < 8) {
        if (playerTableLoad[i].race == RaceId::Select) {
          playerTableLoad[i].race = DefaultRace;
          userSelectable[i] = 1;
        } else {
          if(isValidRace(playerTableLoad[i].race) == false){
            playerTableLoad[i].race = DefaultRace;
          }
        }
      }
    }
    for (; i < 12; i++) {
      playerTableLoad[i].type = 0;
      playerTableLoad[i].race = 0;
      playerTableLoad[i].force = 0;
    }
  }

  // checks if raceID is in the race list
  bool isValidRace(u8 raceID) {
    for (u32 i = 0; i < CustomRaceCount; i++) {
      if (CustomRaceList[i].raceId == raceID) {
        return true;
      }
    }
    return false;
  }


} // hooks