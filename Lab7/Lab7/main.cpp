/*
=================
main.cpp
Main entry point for the Card application
=================
*/

#include "GameConstants.h"
#include "GameResources.h"
#include "cD3DManager.h"
#include "cD3DXSpriteMgr.h"
#include "cD3DXTexture.h"
#include "cBalloon.h"
#include "cSprite.h"
#include "cExplosion.h"
#include "cXAudio.h"
#include "cD3DXFont.h"
#include <iostream>
#include <sstream>
   
using namespace std;

HINSTANCE hInst; // global handle to hold the application instance
HWND wndHandle; // global variable to hold the window handle

// Get a reference to the DirectX Manager
static cD3DManager* d3dMgr = cD3DManager::getInstance();

// Get a reference to the DirectX Sprite renderer Manager 
static cD3DXSpriteMgr* d3dxSRMgr = cD3DXSpriteMgr::getInstance();

D3DXVECTOR2 moleTrans = D3DXVECTOR2(300,300);

//declares vector of moles and its iter/index
vector<cBalloon*> aMole;
vector<cBalloon*>::iterator iter;
vector<cBalloon*>::iterator index;

//declares vector of game over buttons and its iter/index
vector<cBalloon*> gameOverButtons;
vector<cBalloon*>::iterator gameOverIter;
vector<cBalloon*>::iterator gameOverIndex;

//declares vector of start buttons and its iter/index
vector<cBalloon*> startButtons;
vector<cBalloon*>::iterator startIter;
vector<cBalloon*>::iterator startIndex;

RECT clientBounds;

TCHAR szTempOutput[30];

bool gHit = false;
int gMolesHit = 0;
char gMolesHitStr[50];

int molesLeft;

D3DXVECTOR3 expPos;
list<cExplosion*> gExplode;

//declares explode sound
cXAudio gExplodeSound;
//declares background music
cXAudio backgroundMusic;
//declare game over sound
cXAudio gameOverSound;

string mainMenu = "Main Menu";
string mainGame = "Main Game";
string gameOver = "Game Over";
string currentScene = mainMenu;

bool surfaceUpdate;

//declares textures for moles and buttons
cD3DXTexture* moleTextures[4];
char* moleTxtres[] = {"Images\\Mole1.png","Images\\Mole2.png","Images\\Mole3.png","Images\\explosion.png"};

cD3DXTexture* buttonTextures[2];
char* buttonTxtres[] = {"Images\\gameOverRetry.png", "Images\\gameOverQuit.png"};

cD3DXTexture* startButtonTextures[2];
char* startButtonTxtres[] = {"Images\\startButton.png", "Images\\quitButton.png"};

float timer = 5.0f;

/*
==================================================================
* LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam,
* LPARAM lParam)
* The window procedure
==================================================================
*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Check any available messages from the queue
	switch (message)
	{
		case WM_LBUTTONDOWN:
			{
				POINT mouseXY;
				mouseXY.x = LOWORD(lParam);
				mouseXY.y = HIWORD(lParam);
				
				expPos = D3DXVECTOR3((float)mouseXY.x,(float)mouseXY.y, 0.0f);

				if(currentScene == mainMenu)
				{
					startIter = startButtons.begin();
					while (startIter != startButtons.end() && !gHit)
					{
						//checks for intersection between mouse click and buttons
						if ((*startIter)->insideRect((*startIter)->getBoundingRect(),mouseXY))
						{

							if ((*startIter)->getID() == 0)
							{
								//changes scene to game scene
								gHit= true;
								surfaceUpdate = true;
								currentScene = mainGame;
								timer = 5.0f;
								gMolesHit = 0;
								sprintf_s( gMolesHitStr, 50, "Moles Hit : %d", gMolesHit);
								return 0;
							}
							else if((*startIter)->getID() == 1)
							{
								//ends game
								PostQuitMessage(0);
								return 0;

							}
							
						else {

							++startIter;
						}

					gHit = false;
					return 0;

					}
				}
				}
				
				else if (currentScene == mainGame)
				{
					
					iter = aMole.begin();
					while (iter != aMole.end() && !gHit)
					{
						//checks for intersection between mouse click and moles
						if ( (*iter)->insideRect((*iter)->getBoundingRect(),mouseXY))
						{
							OutputDebugString("Hit!\n");
							gHit = true;
							expPos = (*iter)->getSpritePos();
							gExplode.push_back(new cExplosion(expPos,moleTextures[3]));
						
							gExplodeSound.playSound(L"Sounds\\explosion.wav",false);
							iter = aMole.erase(iter);
							gMolesHit++;
							molesLeft--;

							sprintf_s( gMolesHitStr, 50, "Moles Hit : %d", gMolesHit);
							timer += 0.40f;
						}
						else
						{
							++iter;
						}
					}

					gHit = false;
					return 0;
				}

				else if (currentScene == gameOver)
				{
					gameOverIter = gameOverButtons.begin();
					while (gameOverIter != gameOverButtons.end() && !gHit)
					{
						//checks for intersection between mouse click and buttons
						if ((*gameOverIter)->insideRect((*gameOverIter)->getBoundingRect(),mouseXY))
						{
							

							if ((*gameOverIter)->getID() == 0)
							{
								//changes scene to game scene
								gHit= true;
								surfaceUpdate = true;
								currentScene = mainGame;
								timer = 5.0f;
								gMolesHit = 0;
								sprintf_s( gMolesHitStr, 50, "Moles Hit : %d", gMolesHit);
								return 0;
							}
							else if((*gameOverIter)->getID() == 1)
							{
								//ends game
								PostQuitMessage(0);
								return 0;

							}

						}
						else {

							++gameOverIter;
						}

					}

					gHit = false;
					return 0;
				}	
				
			}
		case WM_CLOSE:
			{
			// Exit the Game
				PostQuitMessage(0);
				 return 0;
			}

		case WM_DESTROY:
			{
				PostQuitMessage(0);
				return 0;
			}
	}
	// Always return the message to the default window
	// procedure for further processing
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
==================================================================
* bool initWindow( HINSTANCE hInstance )
* initWindow registers the window class for the application, creates the window
==================================================================
*/
bool initWindow( HINSTANCE hInstance )
{
	WNDCLASSEX wcex;
	// Fill in the WNDCLASSEX structure. This describes how the window
	// will look to the system
	wcex.cbSize = sizeof(WNDCLASSEX); // the size of the structure
	wcex.style = CS_HREDRAW | CS_VREDRAW; // the class style
	wcex.lpfnWndProc = (WNDPROC)WndProc; // the window procedure callback
	wcex.cbClsExtra = 0; // extra bytes to allocate for this class
	wcex.cbWndExtra = 0; // extra bytes to allocate for this instance
	wcex.hInstance = hInstance; // handle to the application instance
	wcex.hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_MyWindowIcon)); // icon to associate with the application
	wcex.hCursor = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_GUNSIGHT));// the default cursor
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1); // the background color
	wcex.lpszMenuName = NULL; // the resource name for the menu
	wcex.lpszClassName = "Whack a Mole"; // the class name being created
	wcex.hIconSm = LoadIcon(hInstance,"mallet.ico"); // the handle to the small icon

	RegisterClassEx(&wcex);
	// Create the window
	wndHandle = CreateWindow("Whack a Mole",			// the window class to use
							 "Whack a Mole",			// the title bar text
							WS_OVERLAPPEDWINDOW,	// the window style
							CW_USEDEFAULT, // the starting x coordinate
							CW_USEDEFAULT, // the starting y coordinate
							800, // the pixel width of the window
							600, // the pixel height of the window
							NULL, // the parent window; NULL for desktop
							NULL, // the menu for the application; NULL for none
							hInstance, // the handle to the application instance
							NULL); // no values passed to the window
	// Make sure that the window handle that is created is valid
	if (!wndHandle)
		return false;
	// Display the window on the screen
	ShowWindow(wndHandle, SW_SHOW);
	UpdateWindow(wndHandle);
	return true;
}

/*
==================================================================
// This is winmain, the main entry point for Windows applications
==================================================================
*/
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow )
{
	// Initialize the window
	if ( !initWindow( hInstance ) )
		return false;
	// called after creating the window
	if ( !d3dMgr->initD3DManager(wndHandle) )
		return false;
	if ( !d3dxSRMgr->initD3DXSpriteMgr(d3dMgr->getTheD3DDevice()))
		return false;

	// Grab the frequency of the high def timer
	__int64 freq = 0;				// measured in counts per second;
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	float sPC = 1.0f / (float)freq;			// number of seconds per count

	__int64 currentTime = 0;				// current time measured in counts per second;
	__int64 previousTime = 0;				// previous time measured in counts per second;

	float numFrames   = 0.0f;				// Used to hold the number of frames
	float timeElapsed = 0.0f;				// cumulative elapsed time

	GetClientRect(wndHandle,&clientBounds);

	float fpsRate = 1.0f/25.0f;
	
	//plays background sound
	backgroundMusic.playSound(L"Sounds\\backgroundMusic.wav", true);

	D3DXVECTOR3 aMolePos;

	//====================================================================
	// Loads Mole and button textures
	//====================================================================
	for (int txture = 0; txture < 4; txture++)
	{
		moleTextures[txture] = new cD3DXTexture(d3dMgr->getTheD3DDevice(), moleTxtres[txture]);
	}
	for (int txture = 0; txture < 2; txture++)
	{
		buttonTextures[txture] = new cD3DXTexture(d3dMgr->getTheD3DDevice(), buttonTxtres[txture]);
	}
	for (int txture = 0; txture < 2; txture++)
	{
		startButtonTextures[txture] = new cD3DXTexture(d3dMgr->getTheD3DDevice(), startButtonTxtres[txture]);
	}

	// Initial starting position for Mole
	D3DXVECTOR3 molePos;

	//Position of buttons
	D3DXVECTOR3 buttonPos;
	D3DXVECTOR3 startButtonPos;

	/* initialize random seed: */
	srand ( (unsigned int)time(NULL) );


	//declare number of moles that spawn at the same time
	int numMoles = 2;
	molesLeft = numMoles;

	//declare possible sprite coordinates
	float spritePositions[12] =
                {50.0f,  100.0f, 150.0f, 200.0f,
                250.0f, 300.0f, 350.0f, 400.0f,
                450.0f, 500.0f};
	
	//creates mole sprites in memory - doesn't draw
	for(int loop = 0; loop < numMoles; loop++)
	{
		
		molePos = D3DXVECTOR3(spritePositions[(rand() % 12 + 1)], spritePositions[(rand() % 12 + 1)], 0);
		aMole.push_back(new cBalloon());
		aMole[loop]->setSpritePos(molePos);
		aMole[loop]->setTranslation(D3DXVECTOR2(0,0.0f));
		aMole[loop]->setTexture(moleTextures[(loop % 3)]);
		
	}

	//creates game over buttons in memory - doesn't draw
	for(int loop = 0; loop < 2; loop++)
	{
		buttonPos = D3DXVECTOR3(250.0f + (205*loop), 300.0f, 0);
		gameOverButtons.push_back(new cBalloon());
		gameOverButtons[loop]->setSpritePos(buttonPos);
		gameOverButtons[loop]->setTranslation(D3DXVECTOR2(0,0.0f));
		gameOverButtons[loop]->setTexture(buttonTextures[loop]);
		gameOverButtons[loop]->setID(loop);

	}
	//creates start buttons in memory - doesn't draw
	for(int loop = 0; loop < 2; loop++)
	{
		startButtonPos = D3DXVECTOR3(250.0f + (205*loop), 300.0f, 0);
		startButtons.push_back(new cBalloon());
		startButtons[loop]->setSpritePos(startButtonPos);
		startButtons[loop]->setTranslation(D3DXVECTOR2(0,0.0f));
		startButtons[loop]->setTexture(startButtonTextures[loop]);
		startButtons[loop]->setID(loop);

	}


	LPDIRECT3DSURFACE9 aSurface;				// the Direct3D surfaces
	LPDIRECT3DSURFACE9 mainMenuSurface;
	LPDIRECT3DSURFACE9 howToPlaySurface;
	LPDIRECT3DSURFACE9 mainGameSurface;
	LPDIRECT3DSURFACE9 gameOverSurface;
	LPDIRECT3DSURFACE9 theBackbuffer = NULL;  // This will hold the back buffer
	
	MSG msg;
	ZeroMemory( &msg, sizeof( msg ) );

	// Create the background surfaces - one for each menu and one that the user can see
	aSurface = d3dMgr->getD3DSurfaceFromFile("Images\\mainMenuBackground.png");
	mainMenuSurface = d3dMgr->getD3DSurfaceFromFile("Images\\mainMenuBackground.png");
	mainGameSurface = d3dMgr->getD3DSurfaceFromFile("Images\\background.jpg");
	gameOverSurface = d3dMgr->getD3DSurfaceFromFile("Images\\gameOverMenu.png");

	// loads custom font
	cD3DXFont* gameFont = new cD3DXFont(d3dMgr->getTheD3DDevice(),hInstance, "MineCrafter 3");

	//sets positions of the text boxes
	RECT textPos;
	SetRect(&textPos, 50, 10, 550, 100);
	RECT timerPos;
	SetRect(&timerPos, 450, 10, 550, 100);

	QueryPerformanceCounter((LARGE_INTEGER*)&previousTime);

	while( msg.message!=WM_QUIT )
	{
		// Check the message queue
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else
		{
			//updates the surfaces based on the scene
			if(surfaceUpdate = true)
			{
				if(currentScene == mainMenu)
				{
					aSurface = mainMenuSurface;

				}
				else if (currentScene == mainGame)
				{
					aSurface = mainGameSurface;
				}
				else if (currentScene == gameOver)
				{
					
					aSurface = gameOverSurface;
					

				}
				surfaceUpdate = false;

			}
			//creates another mole if the condition is true
			if (molesLeft <= 0)
			{
				for(int loop=0; loop < numMoles; loop++)
				{
					molePos = D3DXVECTOR3((spritePositions[(rand() % 10)]), (spritePositions[(rand() % 10)]), 0);
					aMole.push_back(new cBalloon());
					aMole[loop]->setSpritePos(molePos);
					aMole[loop]->setTranslation(D3DXVECTOR2(0.0f, 0.0f));
					aMole[loop]->setTexture(moleTextures[(loop%3)]);
					molesLeft++;
				}

			}
			
			// Game code goes here
			QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
			float dt = (currentTime - previousTime)*sPC;

			// Accumulate how much time has passed.
			timeElapsed += dt;
			/*
			==============================================================
			| Update the postion of the moles/buttons
			==============================================================
			*/
			if(timeElapsed > fpsRate)
			{
				for(iter = aMole.begin(); iter != aMole.end(); ++iter)
				{
					(*iter)->update(timeElapsed);			// update mole
					aMolePos = (*iter)->getSpritePos();  // get the position of the mole
					
				}
		
				if (currentScene == gameOver)
				{	
					for(gameOverIter = gameOverButtons.begin(); gameOverIter != gameOverButtons.end(); ++gameOverIter)
					{
						(*gameOverIter)->update(timeElapsed);			// update the button
						buttonPos = (*gameOverIter)->getSpritePos();  // get the position of the button
						
					}
				}

				if (currentScene == mainMenu)
				{
					for(startIter = startButtons.begin(); startIter != startButtons.end(); ++startIter)
					{
						(*startIter)->update(timeElapsed);			// update the button
						startButtonPos = (*startIter)->getSpritePos();  // get the position of the button
						
					}
				}

				d3dMgr->beginRender();
				theBackbuffer = d3dMgr->getTheBackBuffer();
				d3dMgr->updateTheSurface(aSurface, theBackbuffer);
				d3dMgr->releaseTheBackbuffer(theBackbuffer);
				d3dxSRMgr->beginDraw();

				//draws updated buttons
				if (currentScene == mainMenu)
				{
					vector<cBalloon*>::iterator startIterB = startButtons.begin();
					for(startIterB = startButtons.begin(); startIterB!=startButtons.end(); ++startIterB)
					{
						//draws buttons
						(*startIterB)->update(timeElapsed);
						d3dxSRMgr->setTheTransform((*startIterB)->getSpriteTransformMatrix());  
						d3dxSRMgr->drawSprite((*startIterB)->getTexture(),NULL,NULL,NULL,0xFFFFFFFF);
					}

				}
			
				//draws updated moles
				else if(currentScene == mainGame)
				{	
					
					vector<cBalloon*>::iterator iterB = aMole.begin();
					for(iterB = aMole.begin(); iterB != aMole.end(); ++iterB)
					{
						//draws sprites
						d3dxSRMgr->setTheTransform((*iterB)->getSpriteTransformMatrix());  
						d3dxSRMgr->drawSprite((*iterB)->getTexture(),NULL,NULL,NULL,0xFFFFFFFF);
				
					}
					list<cExplosion*>::iterator iter = gExplode.begin();
					while(iter != gExplode.end())
					{
						if((*iter)->isActive() == false)
						{
							iter = gExplode.erase(iter);
						}
						else
						{
							//draws explosions
							(*iter)->update(timeElapsed);
							d3dxSRMgr->setTheTransform((*iter)->getSpriteTransformMatrix());  
							d3dxSRMgr->drawSprite((*iter)->getTexture(),&((*iter)->getSourceRect()),NULL,NULL,0xFFFFFFFF);
							++iter;
						
						}
					
					}
				}
				//draws updated buttons
				if (currentScene == gameOver)
				{
					
					vector<cBalloon*>::iterator gameOverIterB = gameOverButtons.begin();
					for(gameOverIterB = gameOverButtons.begin(); gameOverIterB!=gameOverButtons.end(); ++gameOverIterB)
					{
						//draws buttons
						(*gameOverIterB)->update(timeElapsed);
						d3dxSRMgr->setTheTransform((*gameOverIterB)->getSpriteTransformMatrix());  
						d3dxSRMgr->drawSprite((*gameOverIterB)->getTexture(),NULL,NULL,NULL,0xFFFFFFFF);

					}
				}
				//triggers the end game scene
				if (timer < 0){

					timer = 0.0f;
					currentScene = gameOver;
					gameOverSound.playSound(L"Sounds\\failSound.wav",false);
					surfaceUpdate = true;
					
					
				}
				d3dxSRMgr->endDraw();
				gameFont->printText(gMolesHitStr,textPos);
				if (currentScene == mainGame)
				{
					//formats float into a string rounded to 1 decimal place
					stringstream ss(stringstream::in | stringstream::out);
					ss << float(int(timer*10 + 0.5))/10;
					string timerString = ss.str();

					gameFont->printText(timerString.c_str(),timerPos);

					timer -= 0.05f;
				}

				d3dMgr->endRender();
				timeElapsed = 0.0f;
			}
			
			previousTime = currentTime;
		}
	}
	d3dxSRMgr->cleanUp();
	d3dMgr->clean();
	return (int) msg.wParam;
}
