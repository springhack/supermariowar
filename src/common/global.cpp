//#include "global.h"

#include "FileIO.h"
#include "FileList.h"
#include "GameValues.h"
#include "Game.h"
#include "gfx.h"
#include "map.h"
#include "MapList.h"
#include "MovingObjectTypes.h"
#include "ResourceManager.h"
#include "TilesetManager.h"

#include <cassert>
#include <cmath>

//1.8.0.0 == Release to staff
//1.8.0.1 == Second release to staff
//1.8.0.2 == beta1
//1.8.0.3 == beta2
//1.8.0.4 == final
//1.9.0.0 == neagix work-in-progress, not released
//2.0.0.0 == fluffypillow netplay code
int g_iVersion[] = {2, 0, 0, 0};

// main game directory, read from command line argument
#ifdef ANDROID
std::string RootDataDirectory = GetHomeDirectory() + "data";
#else
std::string RootDataDirectory = "data";
#endif

CGame *smw;
CResourceManager *rm;

bool VersionIsEqual(int iVersion[], short iMajor, short iMinor, short iMicro, short iBuild)
{
    return iVersion[0] == iMajor && iVersion[1] == iMinor && iVersion[2] == iMicro && iVersion[3] == iBuild;
}

bool VersionIsEqualOrBefore(int iVersion[], short iMajor, short iMinor, short iMicro, short iBuild)
{
    if (iVersion[0] < iMajor)
        return true;

    if (iVersion[0] == iMajor) {
        if (iVersion[1] < iMinor)
            return true;

        if (iVersion[1] == iMinor) {
            if (iVersion[2] < iMicro)
                return true;

            if (iVersion[2] == iMicro) {
                return iVersion[3] <= iBuild;
            }
        }
    }

    return false;
}

bool VersionIsEqualOrAfter(int iVersion[], short iMajor, short iMinor, short iMicro, short iBuild)
{
    if (iVersion[0] > iMajor)
        return true;

    if (iVersion[0] == iMajor) {
        if (iVersion[1] > iMinor)
            return true;

        if (iVersion[1] == iMinor) {
            if (iVersion[2] > iMicro)
                return true;

            if (iVersion[2] == iMicro) {
                return iVersion[3] >= iBuild;
            }
        }
    }

    return false;
}

CGameValues game_values;

FiltersList *filterslist;  //Filters list must be initiallized before maps list because it is used in maplist constructor
MapList *maplist;
SkinList *skinlist;
AnnouncerList *announcerlist;
MusicList *musiclist;
WorldMusicList *worldmusiclist;
GraphicsList *menugraphicspacklist;
GraphicsList *worldgraphicspacklist;
GraphicsList *gamegraphicspacklist;
SoundsList *soundpacklist;
TourList *tourlist;
WorldList *worldlist;

CMap      *g_map;
CTilesetManager *g_tilesetmanager;


bool g_fLoadMessages = true;

//Network stuff
int g_iNextNetworkID = 0;
int g_iNextMessageID = 0;
char szIPString[32] = "";

//Joystick-Init
SDL_Joystick **joysticks = NULL;
short joystickcount = 0;


const char * GameInputNames[NUM_KEYS] = {"Left", "Right", "Jump", "Down", "Turbo", "Use Item", "Pause", "Exit"};
const char * MenuInputNames[NUM_KEYS] = {"Up", "Down", "Left", "Right", "Select", "Cancel", "Random", "Fast Map"};

/*
0 == poison mushroom
1 == 1up
2 == 2up
3 == 3up
4 == 5up
5 == flower
6 == star
7 == clock
8 == bobomb
9 == pow
10 == bulletbill
11 == hammer
12 == green shell
13 == red shell
14 == spike shell
15 == buzzy shell
16 == mod
17 == feather
18 == mystery mushroom
19 == boomerang
20 == tanooki
21 == ice wand
22 == podoboo
23 == bombs
24 == leaf
25 == pwings
*/


short g_iDefaultPowerupPresets[NUM_POWERUP_PRESETS][NUM_POWERUPS] = {
//   0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25
    {5,10, 4, 2, 1,10, 8, 4, 4, 2, 2, 4, 8, 4, 2, 4, 2, 4, 5, 6, 6, 3, 4, 4, 5, 3}, //Custom 1
    {5,10, 4, 2, 1,10, 8, 4, 4, 2, 2, 4, 8, 4, 2, 4, 2, 4, 5, 6, 6, 3, 4, 4, 5, 3}, //Custom 2
    {5,10, 4, 2, 1,10, 8, 4, 4, 2, 2, 4, 8, 4, 2, 4, 2, 4, 5, 6, 6, 3, 4, 4, 5, 3}, //Custom 3
    {5,10, 4, 2, 1,10, 8, 4, 4, 2, 2, 4, 8, 4, 2, 4, 2, 4, 5, 6, 6, 3, 4, 4, 5, 3}, //Custom 4
    {5,10, 4, 2, 1,10, 8, 4, 4, 2, 2, 4, 8, 4, 2, 4, 2, 4, 5, 6, 6, 3, 4, 4, 5, 3}, //Custom 5
    {5,10, 5, 3, 1,10, 2, 3, 4, 3, 3, 4, 9, 6, 2, 4, 4, 7, 5, 6, 6, 3, 2, 2, 5, 5}, //Balanced
    {5, 0, 0, 0, 0,10, 0, 0, 0, 0, 0, 7, 9, 6, 3, 4, 0, 0, 0, 4, 0, 2, 0, 2, 0, 0}, //Weapons Only
    {0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0}, //Koopa Bros Weapons
    {5,10, 7, 5, 2, 0, 6, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 5, 0, 3, 0, 0, 0, 8, 6}, //Support Items
    {3, 3, 1, 0, 0, 0, 0, 0, 4, 2, 2, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 3, 3, 0, 0}, //Booms and Shakes
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 2, 0, 0, 0, 8, 3}, //Fly and Glide
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 4, 2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //Shells
    {5, 8, 4, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0}, //Mushrooms Only
    {3, 5, 0, 0, 0, 5, 2, 0, 0, 0, 3, 0, 6, 4, 1, 3, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0}, //Super Mario Bros 1
    {0, 5, 0, 0, 0, 0, 2, 4, 3, 2, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0}, //Super Mario Bros 2
    {0, 3, 0, 0, 0, 8, 4, 0, 0, 0, 5, 2,10, 9, 4, 5, 0, 0, 0, 4, 3, 0, 4, 0, 8, 5}, //Super Mario Bros 3
    {0,10, 0, 0, 0,10, 6, 0, 0, 0, 2, 0, 8, 4, 2, 4, 0, 4, 0, 0, 0, 0, 5, 0, 0, 0}, //Super Mario World
};

//Koopa Bros Weapons
//Mushrooms Only
//Super Mario Bros (All mushrooms ((poison was in lost levels, that counts as SMB1)), fireflower, star, and maybe the shells)
//Super Mario Bros 2 (All mushrooms but poison, star, clock, pow, green/red shell, bombs)
//One I'd like to see is SMWorld (esc{sp}) only stuff...
//0,10,0,1,0,10,6,0,0,0,2,0,8,4,2,4,0,10,0,5,2,0,0

short g_iCurrentPowerupPresets[NUM_POWERUP_PRESETS][NUM_POWERUPS];


//Conversion to switch 1.6 tiles into 1.7 tiles
short g_iTileConversion[] =    {0,1,2,3,4,5,6,7,8,9,
                                10,11,12,13,14,15,575,670,702,703,
                                32,33,34,35,36,37,38,39,40,41,
                                42,43,44,45,46,47,332,331,330,637,
                                64,65,66,67,68,69,70,71,72,73,
                                74,75,76,77,78,79,365,299,366,853,
                                537,595,505,658,659,656,657,774,775,776,
                                540,96,97,98,643,644,645,26,27,506,
                                122,123,124,690,691,688,689,745,746,747,
                                569,128,129,130,704,677,907,90,91,572,
                                931,602,539,885,728,729,730,731,186,187,
                                188,160,161,162,736,192,194,30,31,898,
                                737,738,739,800,760,761,762,763,218,219,
                                220,864,865,509,768,224,226,62,63,930,
                                769,770,771,508,598,599,600,507,601,510,
                                603,896,897,192,193,260,259,193,194,541,
                                627,699,697,940,941,942,860,861,862,250,
                                252,543,158,498,499,500,922,924,854,886,
                                605,125,126,127,720,721,752,754,753,722,
                                723,928,929,563,531,532,923,571,882,851,
                                309,310,311,343,278,341,99,100,101,489,
                                490,491,384,385,386,147,113,148,914,664,
                                373,374,375,376,310,377,131,132,133,553,
                                554,555,416,417,418,179,145,180,946,570,
                                867,868,869,213,214,215,163,164,165,566,
                                567,568,448,449,450,863,530,504,892,883,
                                899,900,901,245,246,247,777,778,779,250,
                                251,252,856,857,858,859,562,712,893,905,
                                908,909,910,624,625,626,592,593,594,282,
                                283,284,888,889,890,891,710,711,894,937
                               };

void _load_drawmsg(const std::string& f)
{
    if (g_fLoadMessages) {
        /*
        static SDL_Rect r;
        r.x = 0;
        r.y = 0;
        r.w = 500;
        r.h = (Uint16)menu_font_small.getHeight();
        Uint32 col = SDL_MapRGB(screen->format, 189, 251, 255);
        SDL_FillRect(screen, &r, col);      //fill empty area
        */

        rm->menu_font_small.draw(0, 0, f.c_str());
    }
}

void _load_waitforkey()
{
#ifndef __EMSCRIPTEN__
    SDL_Event event;
    while (true) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN)
                return;
            if (event.type == SDL_JOYBUTTONDOWN)
                return;
        }

        SDL_Delay(10);
    }
#endif
}



short iScoreboardPlayerOffsetsX[3][3] = {{40, 0, 0}, {19, 59, 0}, {6, 40, 74}};
short iKingOfTheHillZoneLimits[4][4] = {{0, 0, 1, 2}, {0, 1, 2, 4}, {0, 2, 4, 7}, {0, 2, 5, 12}};

const char * g_szBackgroundConversion[26] = {
    "Land_Classic.png",
    "Castle_Dungeon.png",
    "Desert_Pyramids.png",
    "Ghost_GhostHouse.png",
    "Underground_Cave.png",
    "Clouds_AboveTheClouds.png",
    "Castle_GoombaHall.png",
    "Platforms_GreenSpottedHills.png",
    "Snow_SnowTrees.png",
    "Desert_Desert.png",
    "Underground_BrownRockWall.png",
    "Land_CastleWall.png",
    "Clouds_Clouds.png",
    "Land_GreenMountains.png",
    "Land_InTheTrees.png",
    "Battle_Manor.png",
    "Platforms_JaggedGreenStones.png",
    "Underground_RockWallAndPlants.png",
    "Underground_DarkPipes.png",
    "Bonus_StarryNight.png",
    "Platforms_CloudsAndWaterfalls.png",
    "Battle_GoombaPillars.png",
    "Bonus_HillsAtNight.png",
    "Castle_CastlePillars.png",
    "Land_GreenHillsAndClouds.png",
    "Platforms_BlueSpottedHills.png"
};

//Conversion from backgrounds to music categories created by NMcCoy
short g_iMusicCategoryConversion[26] = {0, 3, 8, 5, 1, 9, 3, 4, 10, 8, 1, 0, 9, 0, 0, 7, 4, 1, 1, 6, 4, 7, 6, 3, 0, 4};
const char * g_szMusicCategoryNames[MAXMUSICCATEGORY] = {"Land", "Underground", "Underwater", "Castle", "Platforms", "Ghost", "Bonus", "Battle", "Desert", "Clouds", "Snow"};
short g_iDefaultMusicCategory[MAXMUSICCATEGORY] = {0, 0, 0, 0, 0, 1, 2, 3, 0, 0, 0};

const char * g_szWorldMusicCategoryNames[MAXWORLDMUSICCATEGORY] = {"Grass", "Desert", "Water", "Giant", "Sky", "Ice", "Pipe", "Dark", "Space"};

const char * g_szAutoFilterNames[NUM_AUTO_FILTERS] = {"Death Tiles", "Warps", "Ice", "Item Boxes", "Breakable Blocks", "Throwable Blocks", "On/Off Blocks", "Platforms", "Hazards", "Item Destroyable Blocks", "Hidden Blocks", "Map Items"};
short g_iAutoFilterIcons[NUM_AUTO_FILTERS] = {37, 29, 33, 1, 0, 6, 40, 73, 19, 87, 17, 118};


void LoadCurrentMapBackground()
{
    char filename[128];
    sprintf(filename, "gfx/packs/backgrounds/%s", g_map->szBackgroundFile);
    std::string path = convertPath(filename, gamegraphicspacklist->current_name());

    //if the background file doesn't exist, use the classic background
    if (!File_Exists(path))
        path = convertPath("gfx/packs/backgrounds/Land_Classic.png", gamegraphicspacklist->current_name());

    gfx_loadimagenocolorkey(&rm->spr_background, path);
}

//TODO  - Review what is colliding with what and remove duplicates (i.e. shell vs. throwblock and throwblock vs. shell should only detect one way)
short g_iCollisionMap[MOVINGOBJECT_LAST][MOVINGOBJECT_LAST] = {
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_none = 0
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_powerup = 1
    {0,0,0,1,1,0,0,1,1,0,0,0,1,1,0,0,0,0,0,0,0,1,0,1,1,0,0,0,0,0,1,0,0,1}, //movingobject_fireball = 2
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_goomba = 3
    {0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,1,0,0,0,0,1,1,0,0,0,0,0,1,0,0,0}, //movingobject_bulletbill = 4
    {0,0,0,1,1,0,0,1,1,0,0,0,1,1,0,0,0,0,0,0,0,1,0,1,1,0,0,0,0,0,1,0,0,1}, //movingobject_hammer = 5
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_poisonpowerup = 6
    {0,0,0,1,1,0,0,1,0,0,0,0,1,1,0,0,0,0,1,0,1,1,0,1,1,0,0,0,0,0,1,0,0,0}, //movingobject_shell = 7
    {0,0,0,1,1,0,0,1,1,0,0,0,1,1,0,0,0,0,1,0,1,1,0,1,1,0,0,0,0,0,1,0,0,0}, //movingobject_throwblock = 8
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0}, //movingobject_egg = 9
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_star = 10
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0}, //movingobject_flag = 11
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_cheepcheep = 12
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_koopa = 13
    {0,0,0,1,1,0,0,1,1,0,0,0,1,1,0,0,0,0,0,0,0,1,0,1,1,0,0,0,0,0,1,0,0,1}, //movingobject_boomerang = 14
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_carried = 15
    {0,0,0,1,0,0,0,1,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1}, //movingobject_iceblast = 16
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_bomb = 17
    {0,0,0,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0}, //movingobject_podobo = 18
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_treasurechest = 19
    {0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,1,1,0,0,0,0,0,1,0,0,0}, //movingobject_attackzone = 20
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_pirhanaplant = 21
    {0,0,0,1,1,0,0,1,1,0,0,0,1,1,0,0,0,0,0,0,0,1,0,1,1,0,0,0,0,0,1,0,0,1}, //movingobject_explosion = 22
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_buzzybeetle = 23
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_spiny = 24
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_phantokey = 25
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_flagbase = 26
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_yoshi = 27
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_coin = 28
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_collectioncard = 29
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_sledgebrother = 30
    {0,0,0,1,1,0,0,1,1,0,0,0,1,1,0,0,0,0,0,0,0,1,0,1,1,0,0,0,0,0,0,0,0,1}, //movingobject_sledgehammer = 31
    {0,0,0,1,1,0,0,1,1,0,0,0,1,1,0,0,0,0,0,0,0,1,0,1,1,0,0,0,0,0,0,0,0,1}, //movingobject_superfireball = 32
    {0,0,0,1,1,0,0,1,1,0,0,0,1,1,0,0,0,0,0,0,1,1,0,1,1,0,0,0,0,0,1,0,0,1}, //movingobject_throwbox = 33
};


SDL_Rect iCountDownNumbers[4][4][2] =
    {   {{{0, 0, 64, 64},{288, 208, 64, 64}},
        {{0, 64, 48, 48},{296, 216, 48, 48}},
        {{192, 64, 32, 32},{304, 224, 32, 32}},
        {{0, 112, 16, 16},{312, 232, 16, 16}}
    },

    {   {{64, 0, 64, 64},{288, 208, 64, 64}},
        {{48, 64, 48, 48},{296, 216, 48, 48}},
        {{224, 64, 32, 32},{304, 224, 32, 32}},
        {{16, 112, 16, 16},{312, 232, 16, 16}}
    },

    {   {{128, 0, 64, 64},{288, 208, 64, 64}},
        {{96, 64, 48, 48},{296, 216, 48, 48}},
        {{192, 96, 32, 32},{304, 224, 32, 32}},
        {{32, 112, 16, 16},{312, 232, 16, 16}}
    },

    {   {{192, 0, 64, 64},{288, 208, 64, 64}},
        {{144, 64, 48, 48},{296, 216, 48, 48}},
        {{224, 96, 32, 32},{304, 224, 32, 32}},
        {{48, 112, 16, 16},{312, 232, 16, 16}}
    }
};

short iCountDownTimes[28] = {3, 3, 3, 15, 3, 3, 3, 3, 3, 3, 15, 3, 3, 3, 3, 3, 3, 15, 3, 3, 3, 3, 3, 3, 45, 3, 3, 3};
short iCountDownRectSize[28] = {3, 2, 1, 0, 1, 2, 3, 3, 2, 1, 0, 1, 2, 3, 3, 2, 1, 0, 1, 2, 3, 3, 2, 1, 0, 1, 2, 3};
short iCountDownRectGroup[28] = {0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3};
short iCountDownAnnounce[28] = {-1, -1, -1, 12, -1, -1, -1, -1, -1, -1, 13, -1, -1, -1, -1, -1, -1, 14, -1, -1, -1, -1, -1, -1, 15, -1, -1, -1};

TileType GetIncrementedTileType(TileType type)
{
    switch (type) {
    case tile_nonsolid:
        return tile_solid;
    case tile_solid:
        return tile_solid_on_top;
    case tile_solid_on_top:
        return tile_ice;
    case tile_ice:
        return tile_death;
    case tile_death:
        return tile_death_on_top;
    case tile_death_on_top:
        return tile_death_on_bottom;
    case tile_death_on_bottom:
        return tile_death_on_left;
    case tile_death_on_left:
        return tile_death_on_right;
    case tile_death_on_right:
        return tile_ice_on_top;
    case tile_ice_on_top:
        return tile_ice_death_on_bottom;
    case tile_ice_death_on_bottom:
        return tile_ice_death_on_left;
    case tile_ice_death_on_left:
        return tile_ice_death_on_right;
    case tile_ice_death_on_right:
        return tile_super_death;
    case tile_super_death:
        return tile_super_death_top;
    case tile_super_death_top:
        return tile_super_death_bottom;
    case tile_super_death_bottom:
        return tile_super_death_left;
    case tile_super_death_left:
        return tile_super_death_right;
    case tile_super_death_right:
        return tile_player_death;
    case tile_player_death:
        return tile_nonsolid;
    }

    return tile_nonsolid;
}
