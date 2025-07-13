#pragma once
#include <string>

//generic
const static unsigned int csg_uiX = 0;
const static unsigned int csg_uiY = 1;
const static unsigned int csg_uiZ = 2;
const static unsigned int csg_uiWidth = 2;
const static unsigned int csg_uiHeight = 3;
const static float csg_fCameraViewAngle = 30.0f;
const static unsigned int csg_uiDefaultStringLength = 128;
const static float csg_fNearClip = 0.1f;
const static float csg_fFarClip = 10000.0f;
const static int csg_uiWindowDefinition[] = { 0,0,512,384 };
// materials
const static bool csg_bMaterialEmissiveOn = true;
const static bool csg_bMaterialEmissiveOff = false;

const static float csg_afMaterialNeturalColour[] = { 1.0f, 1.0f, 0.1f, 0.6f };
const static float csg_fMaterialNeturalShininess = 15.0f;

const static float csg_afColourTransBrightRed[] = { 1.0f, 0.2f, 0.2f, 0.6f };

const static float csg_afColourClear[] = { 0.6f, 0.7f, 0.8f, 0.7f }; // screen background colour

//control
const static unsigned int csg_uiControlDrawGrid = 0;
const static unsigned int csg_uiControlNumFlags = 1;

// parser
std::string constantContinentIndexToName(int i);
char* continentIndexToName(int i);
char* worldSystemIndexToName(int i);

const static unsigned int csg_uiParseNetwork = 1;
const static unsigned int csg_uiParseArcs = 2;
const static unsigned int csg_uiParseVector = 3;
const static unsigned int csg_uiParsePartition = 4;

const static unsigned int csg_uiParseXCoord = 1;
const static unsigned int csg_uiParseGDP = 2;

const static unsigned int csg_uiParseContinent = 1;
const static unsigned int csg_uiParseWorldSystem = 2;
const static float csg_afParseLayoutScale[] = { 800.0f, 800.0f, 800.0f };
const static float csg_fParseDefaultMass = 100.0f;
const static float csg_fParseDefaultSize = 1.0f;

// grid
const static float csg_afDisplayListGridColour[] = { 0.3f, 0.3f, 0.1f, 0.3f };
const static float csg_fDisplayListGridMin = -500.0f;
const static float csg_fDisplayListGridMax = 500.0f;
const static float csg_fDisplayListGridSpace = 50.0f;
static unsigned int gs_uiGridDisplayList = 0;

//spring simulation 
const static float DEFAULT_SPRING_RESTING_LENGTH = 300.0f;
const static float DEFAULT_COEFFICIENT_OF_RESTITUTION = 0.1f;
const static float DEFAULT_DAMPING_COEFFICIENT = 0.2f;
const static float DEFAULT_TIME_STEP_MULTIPLIER = 20.0f;


//Menu 
const static unsigned int MENU_CENTRE_CAMERA = 1;
const static unsigned int MENU_TOGGLE_SPRING_SIMULATION = 2;
const static unsigned int MENU_TOGGLE_GRID = 3;
const static unsigned int MENU_RANDOMIZE_POSITIONS = 4;
const static unsigned int MENU_WORLD_SYSTEM_LAYOUT = 5;
const static unsigned int MENU_CONTINENT_LAYOUT = 6;
const static unsigned int MENU_PIN_ALL_NODES = 7;
const static unsigned int MENU_CLEAR_PINNED_NODES = 8;
const static unsigned int MENU_SWAP_PINNED_NODES = 9;
const static unsigned int MENU_TOGGLE_STATUS = 10;
const static unsigned int MENU_TOGGLE_INFO = 11;
const static unsigned int MENU_TOGGLE_HELP = 12;
const static unsigned int MENU_TOGGLE_PERFORMANCE_INFO = 13;
const static unsigned int MENU_RESTART_SPRING_SIMULATION_VALUES = 14;

const static unsigned int MENU_INCREASE_SPRING_RESTING_LENGTH = 15;
const static unsigned int MENU_DECREASE_SPRING_RESTING_LENGTH = 16;

const static unsigned int MENU_INCREASE_COEFFICIENT_OF_RESTITUTION = 17;
const static unsigned int MENU_DECREASE_COEFFICIENT_OF_RESTITUTION = 18;

const static unsigned int MENU_INCREASE_DAMPING_COEFFICIENT = 19;
const static unsigned int MENU_DECREASE_DAMPING_COEFFICIENT = 20;

const static unsigned int MENU_INCREASE_TIME_STEP_MULTIPLIER = 21;
const static unsigned int MENU_DECREASE_TIME_STEP_MULTIPLIER = 22;

const static unsigned int MENU_TOGGLE_SIMULATION_PARAMETERS = 23;

