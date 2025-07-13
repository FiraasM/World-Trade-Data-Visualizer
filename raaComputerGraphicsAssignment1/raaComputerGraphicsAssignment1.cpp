#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>
#include <map>
#include <conio.h>

#include <raaCamera/raaCamera.h>
#include <raaUtilities/raaUtilities.h>
#include <raaMaths/raaMaths.h>
#include <raaMaths/raaVector.h>
#include <raaSystem/raaSystem.h>
#include <raaPajParser/raaPajParser.h>
#include <raaText/raaText.h>

#include "raaConstants.h"
#include "raaParse.h"
#include "raaControl.h"

#include <iostream>
#include <vector>

#include "raaText/raaTextUI.h"
#include <time.h>

// NOTES
// look should look through the libraries and additional files I have provided to familarise yourselves with the functionallity and code.
// The data is loaded into the data structure, managed by the linked list library, and defined in the raaSystem library.
// You will need to expand the definitions for raaNode and raaArc in the raaSystem library to include additional attributes for the siumulation process
// If you wish to implement the mouse selection part of the assignment you may find the camProject and camUnProject functions usefull


// core system global data
raaCameraInput g_Input; // structure to hadle input to the camera comming from mouse/keyboard events
raaCamera g_Camera; // structure holding the camera position and orientation attributes
raaSystem g_System; // data structure holding the imported graph of data - you may need to modify and extend this to support your functionallity
raaControl g_Control; // set of flag controls used in my implmentation to retain state of key actions

// global var: parameter name for the file to load
const static char csg_acFileParam[] = {"-input"};

// global var: file to load data from
char g_acFile[256];

// core functions -> reduce to just the ones needed by glut as pointers to functions to fulfill tasks
void display(); // The rendering function. This is called once for each frame and you should put rendering code here
void idle(); // The idle function is called at least once per frame and is where all simulation and operational code should be placed
void reshape(int iWidth, int iHeight); // called each time the window is moved or resived
void keyboard(unsigned char c, int iXPos, int iYPos); // called for each keyboard press with a standard ascii key
void keyboardUp(unsigned char c, int iXPos, int iYPos); // called for each keyboard release with a standard ascii key
void sKeyboard(int iC, int iXPos, int iYPos); // called for each keyboard press with a non ascii key (eg shift)
void sKeyboardUp(int iC, int iXPos, int iYPos); // called for each keyboard release with a non ascii key (eg shift)
void mouse(int iKey, int iEvent, int iXPos, int iYPos); // called for each mouse key event
void motion(int iXPos, int iYPos); // called for each mouse motion event


// Non glut functions
void myInit(); // the myinit function runs once, before rendering starts and should be used for setup
void nodeDisplay(raaNode *pNode); // callled by the display function to draw nodes
void arcDisplay(raaArc *pArc); // called by the display function to draw arcs
void buildGrid(); // 

//Data representation functions
void getContinentColour(unsigned int continent, float* pOut);
void drawCorrespondingShape(unsigned int worldSystem, float fMass, bool isPinned, bool isSelected);
void drawTextForNode(raaNode *pNode);

//Simulation functions and variables
static float springRestingLength = DEFAULT_SPRING_RESTING_LENGTH;
static float coefficientOfRestitution = DEFAULT_COEFFICIENT_OF_RESTITUTION;
static float dampingCoefficienct = DEFAULT_DAMPING_COEFFICIENT;
static float timeStepMultiplier = DEFAULT_TIME_STEP_MULTIPLIER;

void increaseSpringRestingLength();
void decreaseSpringRestingLength();
void increaseCoefficientOfRestitution();
void decreaseCoefficientOfRestitution();
void increaseDampingCoefficienct();
void decreaseDampingCoefficienct();
void increaseTimeStepMultiplier();
void decreaseTimeStepMultiplier();

void springSimulation();
void resetForce(raaNode* pNode);
void calculateForce(raaArc* pArc);
void calculateNewPosition(raaNode* pNode);
bool isSimulationRunning = false;
void toggleSpringSimulation();
void restartSpringSimulation();//resets the simulation parameters to its default and resets all the nodes forces and velocity
void resetNodeSimulationValues(raaNode* pNode);

float kineticEnergyCount = 0.0f; //Energy is iteratively added up to this variable, when it's done, the final value will be asssigned to totalKineticEnergy
float totalKineticEnergy = 0.0f; //total kinetic energy per simulation call



//Grid function
void toggleGrid();



//Menu functions
void initMenu();
void onMenuEntryClicked(int menuEntry);

//Functions for layouts
int ascendingMass(void* a, void* b);
int descendingMass(void* a, void* b);
void randomizePositions(raaNode* pNode);
void worldSystemLayout();
void continentLayout();

//Functions for pinning nodes
void pinNode(raaNode* pNode);
void unpinNode(raaNode* pNode);
void togglePin(raaNode* pNode);


//Functions and variables for projection (To allow mouse selection)
bool project(float x, float y, bool readPixelDepth);
void pointIntersectWith(raaNode* pNode);

static float projectionCoordinates[3];
static bool isSpacebarPressed = false;
static raaNode* pSelectedNode;
static float currentDepth;




//Functions for HUD / UI
void initUIs();
void initHelpUI();
void initInfoUI();
void initStatusUI();
void initSelectedNodeUI();
void initSpringSimulationInfoUI();
void initPerformanceInfoUI();
void drawRequiredUI();

void updateSelectedNodeUI();
void updateSpringSimulationInfoUI();
void updatePerformanceInfoUI();

void toggleHelpUI();
void toggleInfoUI();
void toggleStatusUI();
void toggleSpringSimulationInfoUI();
void togglePerformanceInfoUI();


static raaTextUI* statusUI;
static raaTextUI* helpUI;
static raaTextUI* infoUI;
static raaTextUI* springSimulationInfoUI;
static raaTextUI* performanceInfoUI;
static raaTextUI* selectedNodeUI;

static bool isStatusUIRequired = false;
static bool isHelpUIRequired = false;
static bool isInfoUIRequired = false;
static bool isSimulationInfoUIRequired = false;
static bool isPerformanceInfoUIRequired = false;

void setLayoutText(char* text);
void setRunningTextColour(float colour[4]);
void setGridTextColour(float colour[4]);

static float simulationTime = 0.0f;
static float renderTime = 0.0f;

//For calculating framerate
static int frameCount = 0;
static int framerate = 0;
static clock_t startTime;




//Centers the camera based on all of the nodes' position
void centreCamera();




void nodeDisplay(raaNode *pNode) // function to render a node (called from display())
{
	float* position = pNode->m_afPosition;
	unsigned int continent = pNode->m_uiContinent;
	unsigned int worldSystem = pNode->m_uiWorldSystem;


	glPushMatrix();
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	//This section of code draws the shape corresponding to their world system and colour corresponding to their continent
	float afCol[] = { 0.0f, 1.0f, 0.0f, 1.0f };
	getContinentColour(continent, afCol);
	utilitiesColourToMat(afCol, 2.0f);
	glTranslated(position[0], position[1], position[2]);
	drawCorrespondingShape(worldSystem, pNode->m_fMass, pNode->isPinned, pNode->isSelected);

	//Draws the country name above the node
	drawTextForNode(pNode);
	
	
	glPopMatrix();
	glPopAttrib();
	

	
}

void arcDisplay(raaArc *pArc) // function to render an arc (called from display())
{
	raaNode* m_pNode0 = pArc->m_pNode0;
	raaNode* m_pNode1 = pArc->m_pNode1;

	float* arcPos0 = m_pNode0->m_afPosition;
	float* arcPos1 = m_pNode1->m_afPosition;



	glEnable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);
	glBegin(GL_LINES);

	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(arcPos0[0], arcPos0[1], arcPos0[2]);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(arcPos1[0], arcPos1[1], arcPos1[2]);

	glEnd();


}

//(1 - core, 2 - semiperiphery, 3 - periphery).
void drawCorrespondingShape(unsigned int worldSystem, float fMass, bool isPinned, bool isSelected) {
	float pinSizeOffset = 2.0f;
	float afCol[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float selectedColour[] = { 0.0f, 1.0f, 1.0f, 1.0f };

	switch (worldSystem) {

		case 1:
			glutSolidSphere(mathsRadiusOfSphereFromVolume(fMass), 15, 15);
			if (isPinned) {
				utilitiesColourToMat(afCol, 2.0f, true);
				glutWireSphere(mathsRadiusOfSphereFromVolume(fMass) + pinSizeOffset, 15, 15);
			}
			break;

		case 2:
			glutSolidCube(mathsDimensionOfCubeFromVolume(fMass));
			if (isPinned) {
				utilitiesColourToMat(afCol, 2.0f, true);
				glutWireCube(mathsDimensionOfCubeFromVolume(fMass) + pinSizeOffset);
			}
			break;

		case 3:
			glRotatef(270, 1, 0, 0);
			glutSolidCone(mathsRadiusOfConeFromVolume(fMass), mathsHeightOfConeFromVolumeAndRadius(fMass, mathsRadiusOfConeFromVolume(fMass)), 15, 15);
			if (isPinned) {
				utilitiesColourToMat(afCol, 2.0f, true);
				glutWireCone(mathsRadiusOfConeFromVolume(fMass) + pinSizeOffset, mathsHeightOfConeFromVolumeAndRadius(fMass + pinSizeOffset, mathsRadiusOfConeFromVolume(fMass)), 15, 15);
			}
			glRotatef(-270, 1, 0, 0);
			break;

		default:
			printf("Warning: can not get shape for world system %d, drawing a torus instead... \n", worldSystem);
			glutSolidTorus(10, 1, 15, 15);
	}

	if (isSelected) {
		utilitiesColourToMat(selectedColour, 2.0f, true);
		glutWireSphere(mathsRadiusOfSphereFromVolume(fMass) + (pinSizeOffset*10.0f), 15, 15);
	}
}

//Draws the text (country) above the node
void drawTextForNode(raaNode* pNode) {
	float height = 5.0f;

	int continent = pNode->m_uiContinent;
	int worldSystem = pNode->m_uiWorldSystem;
	float fMass = pNode->m_fMass;

	switch (worldSystem) {

	case 1:
		height = mathsRadiusOfSphereFromVolume(fMass);
		break;

	case 2:
		height = mathsDimensionOfCubeFromVolume(fMass);
		break;

	case 3:
		height = mathsHeightOfConeFromVolumeAndRadius(fMass, mathsRadiusOfConeFromVolume(fMass));
		break;

	}

	glTranslated(0.0f, height, 0.0f);
	glMultMatrixf(camRotMatInv(g_Camera));
	glScalef(16, 16, 0.1f);
	float afCol[] = { 0.0f, 1.0f, 0.0f, 1.0f };
	getContinentColour(continent, afCol);
	utilitiesColourToMat(afCol, 2.0f);
	outlinePrint(pNode->m_acName, true);

}

//(1 - Africa, 2 - Asia, 3 - Europe, 4 - North America, 5 - Oceania, 6 - South America)
void getContinentColour(unsigned int continent, float* pOut) {
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;
	float a = 1.0f;

	switch (continent) {

		case 1:
			r = 1.0f;
			break;

		case 2:
			g = 1.0f;
			break;

		case 3:
			b = 1.0f;
			break;

		case 4:
			r = 1.0f;
			b = 1.0f;
			break;

		case 5:
			r = 1.0f;
			g = 1.0f;
			break;

		case 6:
			g = 1.0f;
			b = 1.0f;
			break;

		default:
			printf("Warning: can not get colour for continent %d, given black colour instead... \n", continent);
	}

	pOut[0] = r;
	pOut[1] = g;
	pOut[2] = b;
	pOut[3] = a;

}

void resetNodeSimulationValues(raaNode* pNode) {
	float* vectorForce = pNode->resultantForce;
	float* vectorVelocity = pNode->velocity;

	vectorForce[0] = vectorForce[1] = vectorForce[2] = 0.0f;
	vectorVelocity[0] = vectorVelocity[1] = vectorVelocity[2] = 0.0f;
}

void resetForce(raaNode* pNode) {
	float* vectorForce = pNode->resultantForce;

	vectorForce[0] = vectorForce[1] = vectorForce[2] = 0.0f;
}

void calculateForce(raaArc* pArc) {
	raaNode* m_pNode0 = pArc->m_pNode0;
	raaNode* m_pNode1 = pArc->m_pNode1;

	float* arcPos0 = m_pNode0->m_afPosition;
	float* arcPos1 = m_pNode1->m_afPosition;


	float arcDistance = vecDistance(arcPos0, arcPos1); //calculates the distance

	//calculating direction vector from node0 to node1
	float directionVector0[] = { 0.0f, 0.0f, 0.0f };
	vecSub(arcPos1, arcPos0, directionVector0);
	vecNormalise(directionVector0, directionVector0);


	//calculating direction vector pointing from node1 to node0
	float directionVector1[] = { 0.0f, 0.0f, 0.0f };
	vecSub(arcPos0, arcPos1, directionVector1);
	vecNormalise(directionVector1, directionVector1);

	float extension = arcDistance - springRestingLength; //calculates extension

	float springForceScalar = coefficientOfRestitution * extension; //calculates spring force scalar

	float* resultantForce0 = m_pNode0->resultantForce;
	float* resultantForce1 = m_pNode1->resultantForce;


	//calculating respective vector forces and adding to each node
	float springForce0[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	vecScalarProduct(directionVector0, springForceScalar, springForce0);
	if(!m_pNode0->isPinned && !m_pNode0->isSelected) vecAdd(resultantForce0, springForce0, resultantForce0);

	float springForce1[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	vecScalarProduct(directionVector1, springForceScalar, springForce1);
	if (!m_pNode1->isPinned && !m_pNode1->isSelected) vecAdd(resultantForce1, springForce1, resultantForce1);


}

void calculateNewPosition(raaNode* pNode) {

	float mass = pNode->m_fMass;
	float* vectorResultantForce = pNode->resultantForce;

	float acceleration[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	//From newton 2nd law of motion:
	//(1)force = mass * acceleration

	//therefore:
	//(2)acceleration = force / mass

	//therefore:
	//(3)acceleration = force * (1/mass)

	//NOTE: there was no function for dividing vector with a scalar value so formula (3) was used to calculate acceleration
	vecScalarProduct(vectorResultantForce, 1 / mass, acceleration);


	//Evaluated timestep based in measurement of period between each update frame with use of user chosen multipliers
	float time = renderTime * timeStepMultiplier;

	// position is calculated by: S = UT + ( at^2 / 2 )
	// 
	//UT part
	float UT_result[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	vecScalarProduct(pNode->velocity, time, UT_result);

	// AT^2 / 2  part
	float ATT2_result[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	vecScalarProduct(acceleration, time * time, ATT2_result);
	vecScalarProduct(ATT2_result, 0.5, ATT2_result);

	//Combine both part of equation to get S
	float finalPosition[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	vecAdd(UT_result, ATT2_result, finalPosition);
	vecAdd(pNode->m_afPosition, finalPosition, pNode->m_afPosition);


	float velocityCalculated[4] = { 0.0f, 0.0f, 0.0f, 0.0f};
	vecScalarProduct(acceleration, time, velocityCalculated);
	vecAdd(pNode->velocity, velocityCalculated, pNode->velocity);
	vecScalarProduct(pNode->velocity, dampingCoefficienct, pNode->velocity);

	float kineticEnergy = pNode->m_fMass * sqrt((pNode->velocity[0] * pNode->velocity[0]) + (pNode->velocity[1] * pNode->velocity[1]) + (pNode->velocity[2] * pNode->velocity[2])) / 2.0f;
	kineticEnergyCount += kineticEnergy;

}

void springSimulation() {
	clock_t start_time, end_time;
	start_time = clock();
	
	visitNodes(&g_System, resetForce);

	visitArcs(&g_System, calculateForce);

	visitNodes(&g_System, calculateNewPosition);

	totalKineticEnergy = kineticEnergyCount;
	kineticEnergyCount = 0.0f;

	end_time = clock();
	simulationTime = ((float)(end_time - start_time)) / CLOCKS_PER_SEC;
}

void restartSpringSimulation() {

	visitNodes(&g_System, resetNodeSimulationValues);
	springRestingLength = DEFAULT_SPRING_RESTING_LENGTH;
	coefficientOfRestitution = DEFAULT_COEFFICIENT_OF_RESTITUTION;
	dampingCoefficienct = DEFAULT_DAMPING_COEFFICIENT;
	timeStepMultiplier = DEFAULT_TIME_STEP_MULTIPLIER;

}

void toggleSpringSimulation() {
	float colour[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	isSimulationRunning = !(isSimulationRunning);
	if (isSimulationRunning) {
		vecSet(0.0f, 1.0f, 0.0f, colour);
	}
	else {
		vecSet(1.0f, 0.0f, 0.0f, colour);
	}

	setRunningTextColour(colour);

}



void toggleGrid() {
	controlToggle(g_Control, csg_uiControlDrawGrid);

	float colour[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	if (controlActive(g_Control, csg_uiControlDrawGrid)) {
		vecSet(0.0f, 1.0f, 0.0f, colour);
	}
	else {
		vecSet(1.0f, 0.0f, 0.0f, colour);
	}

	setGridTextColour(colour);

}

void initMenu() {
	int layoutSubMenuHandle = glutCreateMenu(onMenuEntryClicked);
	glutAddMenuEntry("Randomize Positions", MENU_RANDOMIZE_POSITIONS);
	glutAddMenuEntry("World System Layout", MENU_WORLD_SYSTEM_LAYOUT);
	glutAddMenuEntry("Continent Layout", MENU_CONTINENT_LAYOUT);

	int pinControlSubMenuHandle = glutCreateMenu(onMenuEntryClicked);
	glutAddMenuEntry("Pin All Nodes", MENU_PIN_ALL_NODES);
	glutAddMenuEntry("Clear Pinned Nodes", MENU_CLEAR_PINNED_NODES);
	glutAddMenuEntry("Swap Pinned Nodes", MENU_SWAP_PINNED_NODES);

	int headUpDisplaySubMenuHandle = glutCreateMenu(onMenuEntryClicked);
	glutAddMenuEntry("Toggle Status", MENU_TOGGLE_STATUS);
	glutAddMenuEntry("Toggle Info", MENU_TOGGLE_INFO);
	glutAddMenuEntry("Toggle Help", MENU_TOGGLE_HELP);
	glutAddMenuEntry("Toggle Performance Info", MENU_TOGGLE_PERFORMANCE_INFO);
	glutAddMenuEntry("Toggle Simulation Parameters", MENU_TOGGLE_SIMULATION_PARAMETERS);


	int simulationControlSubMenuHandle = glutCreateMenu(onMenuEntryClicked);
	glutAddMenuEntry("Toggle Spring Simulation", MENU_TOGGLE_SPRING_SIMULATION);
	glutAddMenuEntry("Toggle Simulation Parameters", MENU_TOGGLE_SIMULATION_PARAMETERS);
	glutAddMenuEntry("Reset Spring Simulation Parameters To Default Values", MENU_RESTART_SPRING_SIMULATION_VALUES);

	glutAddMenuEntry("Increase Time Step Multiplier", MENU_INCREASE_TIME_STEP_MULTIPLIER);
	glutAddMenuEntry("Decrease Time Step Multiplier", MENU_DECREASE_TIME_STEP_MULTIPLIER);

	glutAddMenuEntry("Increase Damping Coefficient", MENU_INCREASE_DAMPING_COEFFICIENT);
	glutAddMenuEntry("Decrease Damping Coefficient", MENU_DECREASE_DAMPING_COEFFICIENT);

	glutAddMenuEntry("Increase Coefficient Of Restitution", MENU_INCREASE_COEFFICIENT_OF_RESTITUTION);
	glutAddMenuEntry("Decrease Coefficient Of Restitution", MENU_DECREASE_COEFFICIENT_OF_RESTITUTION);

	glutAddMenuEntry("Increase Spring Resting Length", MENU_INCREASE_SPRING_RESTING_LENGTH);
	glutAddMenuEntry("Decrease Spring Resting Length", MENU_DECREASE_SPRING_RESTING_LENGTH);


	int mainMenuHandle = glutCreateMenu(onMenuEntryClicked);
	glutAddMenuEntry("Centre Camera", MENU_CENTRE_CAMERA);
	glutAddSubMenu("Spring Simulation", simulationControlSubMenuHandle);
	glutAddMenuEntry("Toggle Grid", MENU_TOGGLE_GRID);

	glutAddSubMenu("Layout", layoutSubMenuHandle);
	glutAddSubMenu("Pin Control", pinControlSubMenuHandle);
	glutAddSubMenu("Head Up Display", headUpDisplaySubMenuHandle);


	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void onMenuEntryClicked(int menuEntry) {
	switch (menuEntry) {
	case MENU_CENTRE_CAMERA:
		centreCamera();
		break;


	case MENU_TOGGLE_SPRING_SIMULATION:
		toggleSpringSimulation();
		break;
	case MENU_TOGGLE_SIMULATION_PARAMETERS:
		toggleSpringSimulationInfoUI();
		break;
	case MENU_RESTART_SPRING_SIMULATION_VALUES:
		restartSpringSimulation();
		break;

	case MENU_INCREASE_SPRING_RESTING_LENGTH:
		increaseSpringRestingLength();
		break;
	case MENU_DECREASE_SPRING_RESTING_LENGTH:
		decreaseSpringRestingLength();
		break;
	case MENU_INCREASE_COEFFICIENT_OF_RESTITUTION:
		increaseCoefficientOfRestitution();
		break;
	case MENU_DECREASE_COEFFICIENT_OF_RESTITUTION:
		decreaseCoefficientOfRestitution();
		break;
	case MENU_INCREASE_DAMPING_COEFFICIENT:
		increaseDampingCoefficienct();
		break;
	case MENU_DECREASE_DAMPING_COEFFICIENT:
		decreaseDampingCoefficienct();
		break;
	case MENU_INCREASE_TIME_STEP_MULTIPLIER:
		increaseTimeStepMultiplier();
		break;
	case MENU_DECREASE_TIME_STEP_MULTIPLIER:
		decreaseTimeStepMultiplier();
		break;



	case MENU_TOGGLE_GRID:
		toggleGrid();
		break;
	case MENU_RANDOMIZE_POSITIONS:
		visitNodes(&g_System, randomizePositions);
		camReset(g_Camera);
		setLayoutText("Layout: Randomise");
		break;
	case MENU_WORLD_SYSTEM_LAYOUT:
		worldSystemLayout();
		camReset(g_Camera);
		setLayoutText("Layout: World Order");
		break;
	case MENU_CONTINENT_LAYOUT:
		continentLayout();
		camReset(g_Camera);
		setLayoutText("Layout: Continent");
		break;
	case MENU_PIN_ALL_NODES:
		visitNodes(&g_System, pinNode);
		break;
	case MENU_CLEAR_PINNED_NODES:
		visitNodes(&g_System, unpinNode);
		break;
	case MENU_SWAP_PINNED_NODES:
		visitNodes(&g_System, togglePin);
		break;
	case MENU_TOGGLE_STATUS:
		toggleStatusUI();
		break;
	case MENU_TOGGLE_INFO:
		toggleInfoUI();
		break;
	case MENU_TOGGLE_HELP:
		toggleHelpUI();
		break;
	case MENU_TOGGLE_PERFORMANCE_INFO:
		togglePerformanceInfoUI();
		break;
	}
}


void randomizePositions(raaNode* pNode) {
	float* position = pNode->m_afPosition;

	position[0] = randFloat(250.0f, 600.0f);
	position[1] = randFloat(0.0f, 800.0f);
	position[2] = randFloat(0.0f, 800.0f);
}

void worldSystemLayout() {

	raaLinkedList linkedList = g_System.m_llNodes;
	raaVector nodes;

	initVector(&nodes, 50);
	linkedListToVector(&linkedList, &nodes);
	bubbleSortVector(&nodes, descendingMass);

	int ws1Count = 0;
	int ws2Count = 0;
	int ws3Count = 0;

	for (int i = 0; i < nodes.size; ++i) {
		raaNode* node = (raaNode*) nodes.data[i];

		int worldSystem = node->m_uiWorldSystem;
		float* position = node->m_afPosition;

		float min_x = 300.0f;

		switch (worldSystem) {

		case 1:
			vecSet(min_x + (150 * (worldSystem - 1)), 50.0f*ws1Count, 400.0f, position);
			ws1Count++;
			break;

		case 2:
			vecSet(min_x + (150 * (worldSystem - 1)), 50.0f * ws2Count, 400.0f, position);
			ws2Count++;
			break;

		case 3:
			vecSet(min_x + (150 * (worldSystem - 1)), 50.0f*ws3Count, 400.0f, position);
			ws3Count++;
			break;
		}
	}

	freeVector(&nodes);



}

void continentLayout() {
	raaLinkedList linkedList = g_System.m_llNodes;
	raaVector nodes;

	initVector(&nodes, 50);
	linkedListToVector(&linkedList, &nodes);
	bubbleSortVector(&nodes, descendingMass);

	int c1Count = 0;
	int c2Count = 0;
	int c3Count = 0;
	int c4Count = 0;
	int c5Count = 0;
	int c6Count = 0;

	for (int i = 0; i < nodes.size; ++i) {
		raaNode* node = (raaNode*) nodes.data[i];

		// printf("mass: %f \n", node->m_fMass);

		int continent = node->m_uiContinent;
		float* position = node->m_afPosition;

		float min_x = 50.0f;

		switch (continent) {

		case 1:
			vecSet(min_x + (150 * (continent-1)), 50.0f * c1Count, 400.0f, position);
			c1Count++;
			break;

		case 2:
			vecSet(min_x + (150 * (continent - 1)), 50.0f * c2Count, 400.0f, position);
			c2Count++;
			break;

		case 3:
			vecSet(min_x + (150 * (continent - 1)), 50.0f * c3Count, 400.0f, position);
			c3Count++;
			break;

		case 4:
			vecSet(min_x + (150 * (continent - 1)), 50.0f * c4Count, 400.0f, position);
			c4Count++;
			break;

		case 5:
			vecSet(min_x + (150 * (continent - 1)), 50.0f * c5Count, 400.0f, position);
			c5Count++;
			break;

		case 6:
			vecSet(min_x + (150 * (continent - 1)), 50.0f * c6Count, 400.0f, position);
			c6Count++;
			break;
		


		}
	}

	freeVector(&nodes);


}


int ascendingMass(void* a, void* b) {
	raaNode* nodeA = (raaNode*)a;
	raaNode* nodeB = (raaNode*)b;
	return nodeA->m_fMass > nodeB->m_fMass;
}

int descendingMass(void* a, void* b) {
	raaNode* nodeA = (raaNode*)a;
	raaNode* nodeB = (raaNode*)b;
	return nodeA->m_fMass < nodeB->m_fMass;
}

void pinNode(raaNode* pNode) {
	pNode->isPinned = true;
}

void unpinNode(raaNode* pNode) {
	pNode->isPinned = false;
}

void togglePin(raaNode* pNode) {
	pNode->isPinned = !pNode->isPinned;
}


// draw the scene. Called once per frame and should only deal with scene drawing (not updating the simulator)
void display() 
{
	clock_t start_time, end_time;
	start_time = clock();

	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT); // clear the rendering buffers

	glLoadIdentity(); // clear the current transformation state
	glMultMatrixf(camObjMat(g_Camera)); // apply the current camera transform

	// draw the grid if the control flag for it is true	
	if (controlActive(g_Control, csg_uiControlDrawGrid)) glCallList(gs_uiGridDisplayList);

	glPushAttrib(GL_ALL_ATTRIB_BITS); // push attribute state to enable constrained state changes

	visitNodes(&g_System, nodeDisplay); // loop through all of the nodes and draw them with the nodeDisplay function
	visitArcs(&g_System, arcDisplay); // loop through all of the arcs and draw them with the arcDisplay function
	glPopAttrib();


	// draw a simple sphere
	float afCol[] = { 0.3f, 1.0f, 0.5f, 1.0f };
	utilitiesColourToMat(afCol, 2.0f);

	glPushMatrix();
	glTranslatef(0.0f, 30.0f, 0.0f);
	glutSolidSphere(5.0f, 10, 10);
	glPopMatrix();

	drawRequiredUI();

	glFlush(); // ensure all the ogl instructions have been processed
	glutSwapBuffers(); // present the rendered scene to the screen

	end_time = clock();
	renderTime = ((float)(end_time - start_time)) / CLOCKS_PER_SEC;
	frameCount++;

	
}

// processing of system and camera data outside of the renderng loop
void idle() 
{
	if (isSimulationRunning) {
		springSimulation();
	}

	clock_t currentTime = clock();
	float elapsedTime = (float)(currentTime - startTime) / CLOCKS_PER_SEC;

	if (elapsedTime >= 1.0) {
		framerate = frameCount;
		frameCount = 0;
		startTime = currentTime;
	}
	
	

	controlChangeResetAll(g_Control); // re-set the update status for all of the control flags
	camProcessInput(g_Input, g_Camera); // update the camera pos/ori based on changes since last render
	camResetViewportChanged(g_Camera); // re-set the camera's viwport changed flag after all events have been processed
	glutPostRedisplay();// ask glut to update the screen


}

// respond to a change in window position or shape
void reshape(int iWidth, int iHeight)  
{

	glViewport(0, 0, iWidth, iHeight);  // re-size the rendering context to match window
	camSetViewport(g_Camera, 0, 0, iWidth, iHeight); // inform the camera of the new rendering context size
	glMatrixMode(GL_PROJECTION); // switch to the projection matrix stack 
	glLoadIdentity(); // clear the current projection matrix state
	gluPerspective(csg_fCameraViewAngle, ((float)iWidth)/((float)iHeight), csg_fNearClip, csg_fFarClip); // apply new state based on re-sized window
	glMatrixMode(GL_MODELVIEW); // swap back to the model view matrix stac
	glGetFloatv(GL_PROJECTION_MATRIX, g_Camera.m_afProjMat); // get the current projection matrix and sort in the camera model
	glutPostRedisplay(); // ask glut to update the screen
}

// detect key presses and assign them to actions
void keyboard(unsigned char c, int iXPos, int iYPos)
{

	switch(c)
	{
	case 'w':
		camInputTravel(g_Input, tri_pos); // mouse zoom
		break;
	case 's':
		camInputTravel(g_Input, tri_neg); // mouse zoom
		break;
	case 'c':
		camPrint(g_Camera); // print the camera data to the comsole
		break;
	case 'g':
		toggleGrid(); // toggle the drawing of the grid
		break;
	case 'u':
		toggleStatusUI();
		break;
	case 'h':
		toggleHelpUI();
		break;
	case 'i':
		toggleInfoUI();
		break;
	case 'p':
		camPrint(g_Camera);
		break;
	case 'r':
		toggleSpringSimulation();
		break;
	case 't':
		togglePerformanceInfoUI();
		break;
	case 'm':
		restartSpringSimulation();
		break;
	case 'x':
		toggleSpringSimulationInfoUI();
		break;
	case 'z':
		if (isSpacebarPressed && pSelectedNode != nullptr) togglePin(pSelectedNode);
	case 32:
		//if space bar is pressed:
		if (!isSpacebarPressed) {
			isSpacebarPressed = true;
		}
		break;

	case '1':
		decreaseTimeStepMultiplier();
		break;

	case '2':
		increaseTimeStepMultiplier();
		break;

	case '3':
		decreaseDampingCoefficienct();
		break;

	case '4':
		increaseDampingCoefficienct();
		break;

	case '5':
		decreaseCoefficientOfRestitution();
		break;

	case '6':
		increaseCoefficientOfRestitution();
		break;

	case '7':
		decreaseSpringRestingLength();
		break;

	case '8':
		increaseSpringRestingLength();
		break;



	}
}

// detect standard key releases
void keyboardUp(unsigned char c, int iXPos, int iYPos) 
{
	switch(c)
	{
		
		// end the camera zoom action
		case 'w': 
		case 's':
			camInputTravel(g_Input, tri_null);
			break;
		case 32:
			if (isSpacebarPressed) {
				isSpacebarPressed = false;
			}
			break;


	}
}

void sKeyboard(int iC, int iXPos, int iYPos)
{
	// detect the pressing of arrow keys for ouse zoom and record the state for processing by the camera
	switch(iC)
	{
		case GLUT_KEY_UP:
			camInputTravel(g_Input, tri_pos);
			break;
		case GLUT_KEY_DOWN:
			camInputTravel(g_Input, tri_neg);
			break;

	}


}

void sKeyboardUp(int iC, int iXPos, int iYPos)
{


	// detect when mouse zoom action (arrow keys) has ended
	switch(iC)
	{
		case GLUT_KEY_UP:
		case GLUT_KEY_DOWN:
			camInputTravel(g_Input, tri_null);
			break;

	}

}

void mouse(int iKey, int iEvent, int iXPos, int iYPos)
{
	// capture the mouse events for the camera motion and record in the current mouse input state
	if (iKey == GLUT_LEFT_BUTTON)
	{
		//camInputMouse(g_Input, (iEvent == GLUT_DOWN) ? true : false);

		if (iEvent == GLUT_DOWN) {

			if (!isSpacebarPressed) {
				camInputMouse(g_Input, true);
				camInputSetMouseStart(g_Input, iXPos, iYPos);
			}
			else {
				float x = iXPos;
				float y = iYPos;

				if(project(x, y, true)) visitNodes(&g_System, pointIntersectWith);
			}
			


		}

		if (iEvent == GLUT_UP) { 
			camInputMouse(g_Input, false);

			if (pSelectedNode != nullptr) {
				pSelectedNode->isSelected = false;
				pSelectedNode = nullptr;
			}
		}

	}

	else if (iKey == GLUT_MIDDLE_BUTTON)
	{
		camInputMousePan(g_Input, (iEvent == GLUT_DOWN) ? true : false);
		if (iEvent == GLUT_DOWN)camInputSetMouseStart(g_Input, iXPos, iYPos);
	}

}

bool project(float x, float y, bool readPixelDepth) {
	float mvMatrix[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, mvMatrix);

	float pvMatrix[16];
	glGetFloatv(GL_PROJECTION_MATRIX, pvMatrix);

	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	float fZ;

	if (readPixelDepth) {
		glReadPixels(x, viewport[3] - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &fZ);
		currentDepth = fZ;
	}
	else {
		fZ = currentDepth;
	}
	

	return (renderUnProject(x, viewport[3] - y, fZ, mvMatrix, pvMatrix, viewport, projectionCoordinates));

}


void pointIntersectWith(raaNode* pNode) {
	float* position = pNode->m_afPosition;
	unsigned int worldSystem = pNode->m_uiWorldSystem;

	switch (worldSystem) {
		case 1:
			if (pointIntersectWithSphere(projectionCoordinates, position, mathsRadiusOfSphereFromVolume(pNode->m_fMass))) {
				pSelectedNode = pNode;
				pNode->isSelected = true;
			}
			break;

		case 2:
			if (pointIntersectWithCube(projectionCoordinates, position, mathsDimensionOfCubeFromVolume(pNode->m_fMass))) {
				pNode->isSelected = true;
				pSelectedNode = pNode;
			}
			break;

		case 3:
			float fRadius = mathsRadiusOfConeFromVolume(pNode->m_fMass);
			if (pointIntersectWithCone(projectionCoordinates, position, fRadius, mathsHeightOfConeFromVolumeAndRadius(pNode->m_fMass, fRadius))) {
				pNode->isSelected = true;
				pSelectedNode = pNode;
			}
			break;
	}
}


void motion(int iXPos, int iYPos)
{
	// if mouse is in a mode that tracks motion pass this to the camera model
	if (!isSpacebarPressed) {
		if (g_Input.m_bMouse || g_Input.m_bMousePan) camInputSetMouseLast(g_Input, iXPos, iYPos);
	}

	if (pSelectedNode != nullptr && project(iXPos,  iYPos, false)) {

		float* pNodePosition = pSelectedNode->m_afPosition;
		vecSet(projectionCoordinates[0], projectionCoordinates[1], projectionCoordinates[2], pNodePosition);

	}
}


void myInit()
{
	// setup my event control structure
	controlInit(g_Control);

	// initalise the maths library
	initMaths();

	// Camera setup
	camInit(g_Camera); // initalise the camera model
	camInputInit(g_Input); // initialise the persistant camera input data 
	camInputExplore(g_Input, true); // define the camera navigation mode

	// opengl setup - this is a basic default for all rendering in the render loop
	glClearColor(csg_afColourClear[0], csg_afColourClear[1], csg_afColourClear[2], csg_afColourClear[3]); // set the window background colour
	glEnable(GL_DEPTH_TEST); // enables occusion of rendered primatives in the window
	glEnable(GL_LIGHT0); // switch on the primary light
	glEnable(GL_LIGHTING); // enable lighting calculations to take place
	glEnable(GL_BLEND); // allows transparency and fine lines to be drawn
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // defines a basic transparency blending mode
	glEnable(GL_NORMALIZE); // normalises the normal vectors used for lighting - you may be able to switch this iff (performance gain) is you normalise all normals your self
	glEnable(GL_CULL_FACE); // switch on culling of unseen faces
	glCullFace(GL_BACK); // set culling to not draw the backfaces of primatives

	// build the grid display list - display list are a performance optimization 
	buildGrid();

	// initialise the data system and load the data file
	initSystem(&g_System);
	parse(g_acFile, parseSection, parseNetwork, parseArc, parsePartition, parseVector);
}

int main(int argc, char* argv[])
{
	// check parameters to pull out the path and file name for the data file
	for (int i = 0; i<argc; i++) if (!strcmp(argv[i], csg_acFileParam)) sprintf_s(g_acFile, "%s", argv[++i]);


	if (strlen(g_acFile)) 
	{ 
		// if there is a data file

		glutInit(&argc, (char**)argv); // start glut (opengl window and rendering manager)

		glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA); // define buffers to use in ogl
		glutInitWindowPosition(csg_uiWindowDefinition[csg_uiX], csg_uiWindowDefinition[csg_uiY]);  // set rendering window position
		glutInitWindowSize(csg_uiWindowDefinition[csg_uiWidth], csg_uiWindowDefinition[csg_uiHeight]); // set rendering window size
		glutCreateWindow("raaAssignment1-2017");  // create rendering window and give it a name

		buildFont(); // setup text rendering (use outline print function to render 3D text


		myInit(); // application specific initialisation

		// provide glut with callback functions to enact tasks within the event loop
		glutDisplayFunc(display);
		glutIdleFunc(idle);
		glutReshapeFunc(reshape);
		glutKeyboardFunc(keyboard);
		glutKeyboardUpFunc(keyboardUp);
		glutSpecialFunc(sKeyboard);
		glutSpecialUpFunc(sKeyboardUp);
		glutMouseFunc(mouse);
		glutMotionFunc(motion);
		initMenu();
		initUIs();

		startTime = clock();
		glutMainLoop(); // start the rendering loop running, this will only ext when the rendering window is closed 

		killFont(); // cleanup the text rendering process

		return 0; // return a null error code to show everything worked
	}
	else
	{
		// if there isn't a data file 

		printf("The data file cannot be found, press any key to exit...\n");
		_getch();
		return 1; // error code
	}
}

void buildGrid()
{
	if (!gs_uiGridDisplayList) gs_uiGridDisplayList= glGenLists(1); // create a display list

	glNewList(gs_uiGridDisplayList, GL_COMPILE); // start recording display list

	glPushAttrib(GL_ALL_ATTRIB_BITS); // push attrib marker
	glDisable(GL_LIGHTING); // switch of lighting to render lines

	glColor4fv(csg_afDisplayListGridColour); // set line colour

	// draw the grid lines
	glBegin(GL_LINES);
	for (int i = (int)csg_fDisplayListGridMin; i <= (int)csg_fDisplayListGridMax; i++)
	{
		glVertex3f(((float)i)*csg_fDisplayListGridSpace, 0.0f, csg_fDisplayListGridMin*csg_fDisplayListGridSpace);
		glVertex3f(((float)i)*csg_fDisplayListGridSpace, 0.0f, csg_fDisplayListGridMax*csg_fDisplayListGridSpace);
		glVertex3f(csg_fDisplayListGridMin*csg_fDisplayListGridSpace, 0.0f, ((float)i)*csg_fDisplayListGridSpace);
		glVertex3f(csg_fDisplayListGridMax*csg_fDisplayListGridSpace, 0.0f, ((float)i)*csg_fDisplayListGridSpace);
	}
	glEnd(); // end line drawing

	glPopAttrib(); // pop attrib marker (undo switching off lighting)

	glEndList(); // finish recording the displaylist
}

void initUIs() {
	initStatusUI();
	initHelpUI();
	initInfoUI();
	initPerformanceInfoUI();
	initSpringSimulationInfoUI();
	initSelectedNodeUI();
}

void initStatusUI() {
	raaTextUI* textUI = createTextUI(0.01f, 0.95f);
	raaTextInfo* text1 = createTextInfo("Running", 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	raaTextInfo* text2 = createTextInfo("Grid", 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, -25.0f);
	raaTextInfo* text3 = createTextInfo("Layout: Default", 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, -50.0f);

	addTextToUI(textUI, text1);
	addTextToUI(textUI, text2);
	addTextToUI(textUI, text3);

	statusUI = textUI;


}

void setRunningTextColour(float colour[4]) {
	raaTextInfo* runningText = (raaTextInfo*)statusUI->texts->data[0];
	vecSet(colour[0], colour[1], colour[2], runningText->colour);
}

void setGridTextColour(float colour[4]) {
	raaTextInfo* gridText = (raaTextInfo*)statusUI->texts->data[1];
	vecSet(colour[0], colour[1], colour[2], gridText->colour);
}

void setLayoutText(char* text) {
	raaTextInfo* layoutText = (raaTextInfo*)statusUI->texts->data[2];
	layoutText->text = text;
}



void initHelpUI() {
	raaTextUI* textUI = createTextUI(0.01f, 0.95f);
	raaTextInfo* mouseText = createTextInfo("Mouse", 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f);
	raaTextInfo* mouseText1 = createTextInfo("Left Button (Drag) -> rotate", 1.0f, 1.0f, 1.0f, 1.0f, 25.0f, -25.0f);
	raaTextInfo* mouseText2 = createTextInfo("Middle Button (Drag) -> pan", 1.0f, 1.0f, 1.0f, 1.0f, 25.0f, -50.0f);
	raaTextInfo* mouseText3 = createTextInfo("<Space> Left Button -> select/drag node", 1.0f, 1.0f, 1.0f, 1.0f, 25.0f, -75.0f);
	raaTextInfo* mouseText4 = createTextInfo("<Space> Left Button + 'z' -> pin/unpin node", 1.0f, 1.0f, 1.0f, 1.0f, 25.0f, -100.0f);
	raaTextInfo* keyText = createTextInfo("Keys", 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -125.0f);
	raaTextInfo* keyText1 = createTextInfo("'w' -> zoom in", 1.0f, 1.0f, 1.0f, 1.0f, 25.0f, -150.0f);
	raaTextInfo* keyText2 = createTextInfo("'s' -> zoom out", 1.0f, 1.0f, 1.0f, 1.0f, 25.0f, -175.0f);
	raaTextInfo* keyText3 = createTextInfo("'r' -> toggle spring simulation", 1.0f, 1.0f, 1.0f, 1.0f, 25.0f, -200.0f);
	raaTextInfo* keyText4 = createTextInfo("'g' -> toggle grid visibility", 1.0f, 1.0f, 1.0f, 1.0f, 25.0f, -225.0f);
	raaTextInfo* keyText5 = createTextInfo("'u' -> toggle status", 1.0f, 1.0f, 1.0f, 1.0f, 25.0f, -250.0f);
	raaTextInfo* keyText6 = createTextInfo("'i' -> toggle info", 1.0f, 1.0f, 1.0f, 1.0f, 25.0f, -275.0f);
	raaTextInfo* keyText7 = createTextInfo("'h' -> toggle help", 1.0f, 1.0f, 1.0f, 1.0f, 25.0f, -300.0f);
	raaTextInfo* keyText8 = createTextInfo("'p' -> print camera vals to console", 1.0f, 1.0f, 1.0f, 1.0f, 25.0f, -325.0f);
	raaTextInfo* keyText9 = createTextInfo("'t' -> toggle performance info", 1.0f, 1.0f, 1.0f, 1.0f, 25.0f, -350.0f);
	raaTextInfo* keyText10 = createTextInfo("Simulation", 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -375.0f);
	raaTextInfo* keyText11 = createTextInfo("1/2 -> -/+ Timestep Multiplier", 1.0f, 1.0f, 1.0f, 1.0f, 25.0f, -400.0f);
	raaTextInfo* keyText12 = createTextInfo("3/4 -> -/+ Damping Coefficient", 1.0f, 1.0f, 1.0f, 1.0f, 25.0f, -425.0f);
	raaTextInfo* keyText13 = createTextInfo("5/6 -> -/+ Coefficienct of Restitution", 1.0f, 1.0f, 1.0f, 1.0f, 25.0f, -450.0f);
	raaTextInfo* keyText14 = createTextInfo("7/8 -> -/+ Spring Resting Length", 1.0f, 1.0f, 1.0f, 1.0f, 25.0f, -475.0f);
	raaTextInfo* keyText15 = createTextInfo("'m' -> Restart spring simulation", 1.0f, 1.0f, 1.0f, 1.0f, 25.0f, -500.0f);
	raaTextInfo* keyText16 = createTextInfo("'x' -> Toggle simulation info", 1.0f, 1.0f, 1.0f, 1.0f, 25.0f, -525.0f);

	addTextToUI(textUI, mouseText); 
	addTextToUI(textUI, mouseText1);
	addTextToUI(textUI, mouseText2);
	addTextToUI(textUI, mouseText3);
	addTextToUI(textUI, mouseText4);
	addTextToUI(textUI, keyText);
	addTextToUI(textUI, keyText1);
	addTextToUI(textUI, keyText2);
	addTextToUI(textUI, keyText3);
	addTextToUI(textUI, keyText4);
	addTextToUI(textUI, keyText5);
	addTextToUI(textUI, keyText6);
	addTextToUI(textUI, keyText7);
	addTextToUI(textUI, keyText8);
	addTextToUI(textUI, keyText9);
	addTextToUI(textUI, keyText10);
	addTextToUI(textUI, keyText11);
	addTextToUI(textUI, keyText12);
	addTextToUI(textUI, keyText13);
	addTextToUI(textUI, keyText14);
	addTextToUI(textUI, keyText15);
	addTextToUI(textUI, keyText16);

	helpUI = textUI;

}

void initInfoUI() {
	raaTextUI* textUI = createTextUI(0.01f, 0.95f);
	raaTextInfo* text1 = createTextInfo("Shape indicates world order", 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f);
	raaTextInfo* text2 = createTextInfo("Sphere -> 1st World", 1.0f, 1.0f, 1.0f, 1.0f, 10.0f, -25.0f);
	raaTextInfo* text3 = createTextInfo("Cube -> 2nd World", 1.0f, 1.0f, 1.0f, 1.0f, 10.0f, -50.0f);
	raaTextInfo* text4 = createTextInfo("Cone -> 3rd World", 1.0f, 1.0f, 1.0f, 1.0f, 10.0f, -75.0f);
	raaTextInfo* text5 = createTextInfo("Colour indicates continent (Approximate)", 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -100.0f);
	raaTextInfo* text6 = createTextInfo("Blue -> Europe", 1.0f, 1.0f, 1.0f, 1.0f, 10.0f, -125.0f);
	raaTextInfo* text7 = createTextInfo("Pink -> North America", 1.0f, 1.0f, 1.0f, 1.0f, 10.0f, -150.0f);
	raaTextInfo* text8 = createTextInfo("Green -> Asia", 1.0f, 1.0f, 1.0f, 1.0f, 10.0f, -175.0f);
	raaTextInfo* text9 = createTextInfo("Pale Blue -> South America", 1.0f, 1.0f, 1.0f, 1.0f, 10.0f, -200.0f);
	raaTextInfo* text10 = createTextInfo("Red -> Africa", 1.0f, 1.0f, 1.0f, 1.0f, 10.0f, -225.0f);
	raaTextInfo* text11 = createTextInfo("Lines indicate country's amount of trade", 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -250.0f);
	raaTextInfo* text12 = createTextInfo("Red End -> Import", 1.0f, 1.0f, 1.0f, 1.0f, 10.0f, -275.0f);
	raaTextInfo* text13 = createTextInfo("Green End -> Export", 1.0f, 1.0f, 1.0f, 1.0f, 10.0f, -300.0f);
	raaTextInfo* text14 = createTextInfo("Sizes indicates country's amount of trade", 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -325.0f);

	addTextToUI(textUI, text1);
	addTextToUI(textUI, text2);
	addTextToUI(textUI, text3);
	addTextToUI(textUI, text4);
	addTextToUI(textUI, text5);
	addTextToUI(textUI, text6);
	addTextToUI(textUI, text7);
	addTextToUI(textUI, text8);
	addTextToUI(textUI, text9);
	addTextToUI(textUI, text10);
	addTextToUI(textUI, text11);
	addTextToUI(textUI, text12);
	addTextToUI(textUI, text13);
	addTextToUI(textUI, text14);


	infoUI = textUI;

}

void initPerformanceInfoUI() {
	raaTextUI* textUI = createTextUI(0.7f, 0.8f);
	raaTextInfo* text1 = createTextInfo(nullptr, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f);
	raaTextInfo* text2 = createTextInfo(nullptr, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 25.0f);
	raaTextInfo* text3 = createTextInfo(nullptr, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 50.0f);
	raaTextInfo* text4 = createTextInfo(nullptr, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 75.0f);
	addTextToUI(textUI, text1);
	addTextToUI(textUI, text2);
	addTextToUI(textUI, text3);
	addTextToUI(textUI, text4);

	performanceInfoUI = textUI;

}


void updatePerformanceInfoUI() {
	raaTextInfo* framerateText = (raaTextInfo*)performanceInfoUI->texts->data[0];
	raaTextInfo* renderTimeText = (raaTextInfo*)performanceInfoUI->texts->data[1];
	raaTextInfo* simulationTimeText = (raaTextInfo*)performanceInfoUI->texts->data[2];
	raaTextInfo* energyText = (raaTextInfo*)performanceInfoUI->texts->data[3];

	char strFramerate[100] = "FPS: ";
	char strRenderTime[100] = "Render Time: ";
	char strSimulationTime[100] = "Simulation Time: ";
	char strEnergy[100] = "Energy: ";

	char strFramerateNum[20];
	sprintf(strFramerateNum, "%d", framerate);
	strcat(strFramerate, strFramerateNum);

	char strRenderTimeNum[20];
	sprintf(strRenderTimeNum, "%.2f ms", renderTime*1000.0f);
	strcat(strRenderTime, strRenderTimeNum);

	char strSimulationTimeNum[20];
	sprintf(strSimulationTimeNum, "%.2f ms", simulationTime*1000.0f);
	strcat(strSimulationTime, strSimulationTimeNum);

	char strEnergyNum[80];
	sprintf(strEnergyNum, "%.2f J", totalKineticEnergy);
	strcat(strEnergy, strEnergyNum);

	strcpy(framerateText->text, strFramerate);
	strcpy(renderTimeText->text, strRenderTime);
	strcpy(simulationTimeText->text, strSimulationTime);
	strcpy(energyText->text, strEnergy);

}

void initSpringSimulationInfoUI() {
	raaTextUI* textUI = createTextUI(0.45f, 0.1f);
	raaTextInfo* text1 = createTextInfo(nullptr, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f);
	raaTextInfo* text2 = createTextInfo(nullptr, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 25.0f);
	raaTextInfo* text3 = createTextInfo(nullptr, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 50.0f);
	raaTextInfo* text4 = createTextInfo(nullptr, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 75.0f);
	addTextToUI(textUI, text1);
	addTextToUI(textUI, text2);
	addTextToUI(textUI, text3);
	addTextToUI(textUI, text4);

	springSimulationInfoUI = textUI;
}

void updateSpringSimulationInfoUI() {
	raaTextInfo* springRestingLengthText = (raaTextInfo*)springSimulationInfoUI->texts->data[0];
	raaTextInfo* coefficientOfRestitutionText = (raaTextInfo*)springSimulationInfoUI->texts->data[1];
	raaTextInfo* dampingCoefficienctText = (raaTextInfo*)springSimulationInfoUI->texts->data[2];
	raaTextInfo* timeStepMultiplierText = (raaTextInfo*)springSimulationInfoUI->texts->data[3];

	char strSpringRestingLength[100] = "Spring resting length:  ";
	char strCoefficientOfRestitution[100] = "Coefficienct of restitution: ";
	char strDampingCoefficienct[100] = "Damping coefficient: ";
	char strTimeStepMultiplier[100] = "Time step multiplier: ";


	char strSpringRestingLengthNum[20];
	sprintf(strSpringRestingLengthNum, "%.1f m", springRestingLength);
	strcat(strSpringRestingLength, strSpringRestingLengthNum);

	char strCoefficientOfRestitutionNum[20];
	sprintf(strCoefficientOfRestitutionNum, "%.1f", coefficientOfRestitution);
	strcat(strCoefficientOfRestitution, strCoefficientOfRestitutionNum);

	char strDampingCoefficienctNum[20];
	sprintf(strDampingCoefficienctNum, "%.1f", dampingCoefficienct);
	strcat(strDampingCoefficienct, strDampingCoefficienctNum);

	char strTimeStepMultiplierNum[20];
	sprintf(strTimeStepMultiplierNum, "%.1fx", timeStepMultiplier);
	strcat(strTimeStepMultiplier, strTimeStepMultiplierNum);

	strcpy(springRestingLengthText->text, strSpringRestingLength);
	strcpy(coefficientOfRestitutionText->text, strCoefficientOfRestitution);
	strcpy(dampingCoefficienctText->text, strDampingCoefficienct);
	strcpy(timeStepMultiplierText->text, strTimeStepMultiplier);

}

void initSelectedNodeUI() {
	raaTextUI* textUI = createTextUI(0.5f, 0.6f);
	raaTextInfo* text1 = createTextInfo(nullptr, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	raaTextInfo* text2 = createTextInfo(nullptr, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, -25.0f);
	raaTextInfo* text3 = createTextInfo(nullptr, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, -50.0f);
	raaTextInfo* text4 = createTextInfo(nullptr, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, -75.0f);
	raaTextInfo* text5 = createTextInfo(nullptr, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, -100.0f);
	raaTextInfo* text6 = createTextInfo(nullptr, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, -125.0f);
	raaTextInfo* text7 = createTextInfo(nullptr, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, -150.0f);
	addTextToUI(textUI, text1);
	addTextToUI(textUI, text2);
	addTextToUI(textUI, text3);
	addTextToUI(textUI, text4);
	addTextToUI(textUI, text5);
	addTextToUI(textUI, text6);
	addTextToUI(textUI, text7);

	selectedNodeUI = textUI;
}

void updateSelectedNodeUI() {
	raaTextInfo* countryText = (raaTextInfo *) selectedNodeUI->texts->data[0];
	raaTextInfo* continentText = (raaTextInfo*)selectedNodeUI->texts->data[1];
	raaTextInfo* worldSystemText = (raaTextInfo*)selectedNodeUI->texts->data[2];
	raaTextInfo* massText = (raaTextInfo*)selectedNodeUI->texts->data[3];
	raaTextInfo* xText = (raaTextInfo*)selectedNodeUI->texts->data[4];
	raaTextInfo* yText = (raaTextInfo*)selectedNodeUI->texts->data[5];
	raaTextInfo* zText = (raaTextInfo*)selectedNodeUI->texts->data[6];

	char strCountry[100] = "Country: ";
	char strContinent[100] = "Continent: ";
	char strWorldSystem[100] = "World System: ";
	char strMass[100] = "GDP (Mass): ";

	char strX[100] = "X: ";
	char strY[100] = "Y: ";
	char strZ[100] = "Z: ";


	strcat(strCountry, pSelectedNode->m_acName);
	strcat(strContinent, continentIndexToName(pSelectedNode->m_uiContinent));
	strcat(strWorldSystem, worldSystemIndexToName(pSelectedNode->m_uiWorldSystem));


	char strMassNum[20];
	sprintf(strMassNum, "%.1f", pSelectedNode->m_fMass);
	strcat(strMass, strMassNum);

	char strXNum[20];
	sprintf(strXNum, "%.2f", pSelectedNode->m_afPosition[0]);
	strcat(strX, strXNum);

	char strYNum[20];
	sprintf(strYNum, "%.2f", pSelectedNode->m_afPosition[1]);
	strcat(strY, strYNum);

	char strZNum[20];
	sprintf(strZNum, "%.2f", pSelectedNode->m_afPosition[2]);
	strcat(strZ, strZNum);

	strcpy(countryText->text, strCountry);
	strcpy(continentText->text, strContinent);
	strcpy(worldSystemText->text, strWorldSystem);
	strcpy(massText->text, strMass);

	strcpy(xText->text, strX);
	strcpy(yText->text, strY);
	strcpy(zText->text, strZ);

}



void toggleStatusUI() {
	isStatusUIRequired = !isStatusUIRequired;
}

void toggleInfoUI() {
	isInfoUIRequired = !isInfoUIRequired;
}

void toggleHelpUI() {
	isHelpUIRequired = !isHelpUIRequired;
}

void toggleSpringSimulationInfoUI() {
	isSimulationInfoUIRequired = !isSimulationInfoUIRequired;
}

void togglePerformanceInfoUI() {
	isPerformanceInfoUIRequired = !isPerformanceInfoUIRequired;
}





void drawRequiredUI() {
	float extraOffsetX = 0.0f;
	float extraOffsetY = 0.0f;

	if (isStatusUIRequired) {
		drawTextUI(statusUI, extraOffsetX, extraOffsetY);
		extraOffsetY -= 100.0f;
	}

	if (isInfoUIRequired) {
		drawTextUI(infoUI, extraOffsetX, extraOffsetY);
		extraOffsetX += 350.0f;
	}

	if (isHelpUIRequired) {
		drawTextUI(helpUI, extraOffsetX, extraOffsetY);
	}

	if (isPerformanceInfoUIRequired) {
		updatePerformanceInfoUI();
		drawTextUI(performanceInfoUI, 0.0f, 0.0f);
	}

	if (pSelectedNode != nullptr) {
		updateSelectedNodeUI();
		drawTextUI(selectedNodeUI, 0.0f, 0.0f);
	}

	if (isSimulationInfoUIRequired) {
		updateSpringSimulationInfoUI();
		drawTextUI(springSimulationInfoUI, 0.0f, 0.0f);
	}


	
	
}


void centreCamera() {

	raaLinkedList linkedList = g_System.m_llNodes;
	raaVector nodes;

	initVector(&nodes, 50);
	linkedListToVector(&linkedList, &nodes);

	float totalX = 0.0f;
	float totalY = 0.0f;
	float totalZ = 0.0f;

	for (int i = 0; i < nodes.size; ++i) {
		raaNode* node = (raaNode*)nodes.data[i];
		
		float* position = node->m_afPosition;
		totalX += position[0];
		totalY += position[1];
		totalZ += position[2];

	}

	float centreX = totalX / nodes.size;
	float centreY = totalY / nodes.size;
	float centreZ = totalZ / nodes.size;

	vecSet(centreX, centreY, centreZ, g_Camera.m_fVP);
	freeVector(&nodes);



}


void increaseSpringRestingLength() {
	springRestingLength += 10;
}

void decreaseSpringRestingLength() {
	springRestingLength -= 10;
	if (springRestingLength < 10) springRestingLength = 10;
}

void increaseCoefficientOfRestitution() {
	coefficientOfRestitution += 0.1f;
}
void decreaseCoefficientOfRestitution() {
	coefficientOfRestitution -= 0.1f;
	if (coefficientOfRestitution < 0.0f) coefficientOfRestitution = 0.0f;
}

void increaseDampingCoefficienct() {
	dampingCoefficienct += 0.1f;
}
void decreaseDampingCoefficienct() {
	dampingCoefficienct -= 0.1f;
	if (dampingCoefficienct < 0.0f) dampingCoefficienct = 0.0f;
}

void increaseTimeStepMultiplier() {
	timeStepMultiplier += 2.0f;
	
}
void decreaseTimeStepMultiplier() {
	timeStepMultiplier -= 2.0f;
	if (timeStepMultiplier < 0.0f) timeStepMultiplier = 0.0f;
}

