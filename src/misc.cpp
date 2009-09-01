/* MISC.C
  All KINDS of assorted crap :) Has most of the in-game dialog boxes
  such as the status box etc.

  Also like I said there's all kinds of assorted crap in here.
  That's why it's called "misc.c" (get it? :))
*/

#include "keen.h"
#ifdef BUILD_SDL
#include <SDL.h>
#include "sdl/joydrv.h"
#include "sdl/CInput.h"
#include "sdl/CVideoDriver.h"
#include "sdl/sound/CSound.h"
#endif

#include "include/misc.h"
#include "include/game.h"
#include "include/gamedo.h"
#include "fileio/CSavedGame.h"
#include "CLogFile.h"
#include "graphics/CGfxEngine.h"
#include "CGraphics.h"
#include "StringUtils.h"
#include "FindFile.h"

#include "fileio/CExeFile.h"

#include "dialog/CWindow.h"
#include "dialog/CTextViewer.h"

#include "common/palette.h"

#include <fstream>

void banner(void)
{
char buf[80];

  sprintf(buf, "%s  *Unknown* build check banner()", REVISION);
  #ifdef TARGET_WIN32
    sprintf(buf, "%s  Windows build", REVISION);
  #endif
  #ifdef TARGET_LNX
    sprintf(buf, "%s  Linux build", REVISION);
  #endif
  printf("%s", buf);
  printf(" (%d bit)", static_cast<int> (sizeof(int*)*8));

  printf("\nby The CloneKeenPlus Team 2009\n");
  printf("\n");
  printf("BY A FAN, FOR FANS. ALL \"COMMANDER KEEN\" GRAPHICS,\n");
  printf("SOUND, AND LEVEL FILES ARE THE PROPERTY OF ID SOFTWARE.\n");
  printf("\n");
}

void cleanup(stCloneKeenPlus *CKP)
{
	if(CKP->GameData){ delete[] CKP->GameData; CKP->GameData = NULL; }

	g_pLogFile->fltextOut(BLACK,true," Freed %d strings.<br>", freestrings());

	JoyDrv_Stop(&(CKP->Joystick));
	g_pLogFile->textOut(BLACK,true," Joystick driver shut down.<br>");
	g_pSound->stopAllSounds();
	g_pSound->destroy();
	g_pLogFile->textOut(BLACK,true," Sound driver shut down.<br>");

    if (demofile)
    {
    	fclose(demofile);
    	g_pLogFile->fltextOut(BLACK,true," Demo file closed.<br>");
    }

    g_pGraphics->stopGraphics();
    g_pLogFile->fltextOut(BLACK,true," Graphics driver shut down.<br>");

    g_pGraphics->freemem();

    g_pLogFile->ftextOut("<br>");
}

// draw an empty dialog box, for youseeinyourmind(), etc.
void dialogbox(int x1, int y1, int w, int h)
{
int x,y,i,j;

  SDL_Surface *boxsurface = g_pVideoDriver->FGLayerSurface;

  g_pGfxEngine->Font.drawCharacter(boxsurface, 1, x1*8, y1*8);
  g_pGfxEngine->Font.drawCharacter(boxsurface, 3, (x1+w)*8, y1*8);
  for(x=(x1*8)+8,i=0;i<w-1;i++)
  {
	  g_pGfxEngine->Font.drawCharacter(boxsurface, 2, x, y1*8);
	  x+=8;
  }
  y=(y1+1)*8;
  for(j=0;j<h-2;j++)
  {
    for(x=(x1*8),i=0;i<=w;i++)
    {
      if (i==0) g_pGfxEngine->Font.drawCharacter(boxsurface, 4, x, y );
      else if (i==w) g_pGfxEngine->Font.drawCharacter(boxsurface, 5, x, y );
      else g_pGfxEngine->Font.drawCharacter(boxsurface, ' ', x, y );
      x+=8;
    }
    y+=8;
  }
    for(x=(x1*8),i=0;i<=w;i++)
    {
      if (i==0) g_pGfxEngine->Font.drawCharacter(boxsurface, 6, x, y);
      else if (i==w) g_pGfxEngine->Font.drawCharacter(boxsurface, 8, x, y);
      else g_pGfxEngine->Font.drawCharacter(boxsurface, 7, x, y);
      x+=8;
    }
}

bool showGameHint(int mpx, int mpy, int episode, int level)
{
	std::string strname;

	if(episode == 1)
	{
		if(map.mapdata[mpx][mpy] >= 435 && map.mapdata[mpx][mpy] <= 438)
			// it's a garg statue
    		map_chgtile(mpx, mpy, 434);
		else
			map_chgtile(mpx, mpy, 485);

		strname =  "EP1_YSIYM_LVL" + itoa(level);
	}
	else if(episode == 2)
	{
        // make the switch stop glowing
	    switch(level)
	    {
	    case 8:
	    	strname = "EP2_VE_NOJUMPINDARK";
			break;
	    case 10:
	    	strname = "EP2_VE_EVILBELTS";
			break;
	    default:
	    	return false;
	    }
        map_chgtile(mpx, mpy+1, 432);
	}

	CTextBox* TextBox;
	CWindow *InfoTextWindow = new CWindow( g_pVideoDriver->FGLayerSurface, 50, 50, 220, 100 );

	TextBox = new CTextBox( g_pVideoDriver->FGLayerSurface, 50, 50, 204, 84, getstring(strname), true);
	TextBox->setFontDimensions(8, 8);
	InfoTextWindow->addObject(TextBox);

	// The Text will be too big, so resize in knowing the height of the first text.
	InfoTextWindow->Resize(InfoTextWindow->getWidth(),
			(InfoTextWindow->m_TextBox[0]->getNumberOfTextlines()+2)*8);

	g_pInput->flushAll();

    do
    {
    	g_pInput->pollEvents();
	InfoTextWindow->render();
    	g_pVideoDriver->update_screen();
    } while(!g_pInput->getPressedAnyCommand());

    delete InfoTextWindow;
    return true;
}

void inventory_draw_ep1(int p)
{
int x,t,i,j;
	std::string tempbuf;
int dlgX,dlgY,dlgW,dlgH;

  SDL_Surface *boxsurface = g_pVideoDriver->FGLayerSurface;

  dlgX = GetStringAttribute("EP1_StatusBox", "LEFT");
  dlgY = GetStringAttribute("EP1_StatusBox", "TOP");
  dlgW = GetStringAttribute("EP1_StatusBox", "WIDTH");
  dlgH = GetStringAttribute("EP1_StatusBox", "HEIGHT");

  dialogbox(dlgX,dlgY,dlgW,dlgH);
  g_pGfxEngine->Font.drawFont( boxsurface, getstring("EP1_StatusBox"), (dlgX+1)<<3, (dlgY+1)<<3, 0);

  // fill in what we have
  // 321: joystick/battery/vacuum/fuel not gotten
  // 414: raygun, 415, pogo
  // 424: yellow/red/green/blue cards
  // 448: ship parts, gotten
  // raygun icon
  g_pGfxEngine->Tilemap.drawTile(boxsurface, (dlgX+4)<<3, ((dlgY+8)<<3)+3, 414);
  // pogo
  if (player[p].inventory.HasPogo) g_pGfxEngine->Tilemap.drawTile(boxsurface, ((dlgX+12)<<3)+4, ((dlgY+9)<<3)+3, 415);
  // cards
  if (player[p].inventory.HasCardYellow)
  {
	  g_pGfxEngine->Tilemap.drawTile(boxsurface, (dlgX+21)<<3, ((dlgY+8)<<3)+3, 424);
	  if(player[p].inventory.HasCardYellow > 1)
		  g_pGfxEngine->Font.drawFont( boxsurface, itoa(player[p].inventory.HasCardYellow),(dlgX+20)<<3,((dlgY+8)<<3)+3,0);
  }
  if (player[p].inventory.HasCardRed)
  {
	  g_pGfxEngine->Tilemap.drawTile(boxsurface, (dlgX+25)<<3, ((dlgY+8)<<3)+3, 425);

	  if(player[p].inventory.HasCardRed > 1)
		  g_pGfxEngine->Font.drawFont( boxsurface, itoa(player[p].inventory.HasCardRed),(dlgX+24)<<3,((dlgY+8)<<3)+3,0);
  }
  if (player[p].inventory.HasCardGreen)
  {
	  g_pGfxEngine->Tilemap.drawTile(boxsurface, (dlgX+21)<<3, ((dlgY+10)<<3)+4, 426);

	  if (player[p].inventory.HasCardGreen > 1)
		  g_pGfxEngine->Font.drawFont( boxsurface, itoa(player[p].inventory.HasCardGreen),(dlgX+20)<<3,((dlgY+10)<<3)+3,0);
  }
  if (player[p].inventory.HasCardBlue)
  {
	  g_pGfxEngine->Tilemap.drawTile(boxsurface, (dlgX+25)<<3, ((dlgY+10)<<3)+4, 427);

	  if(player[p].inventory.HasCardBlue > 1)
		  g_pGfxEngine->Font.drawFont( boxsurface, itoa(player[p].inventory.HasCardBlue),(dlgX+24)<<3,((dlgY+10)<<3)+3,0);
  }
  // ship parts
  if (player[p].inventory.HasJoystick) t=448; else t=321;
  g_pGfxEngine->Tilemap.drawTile(boxsurface, (dlgX+18)<<3, ((dlgY+4)<<3)+3, t);
  if (player[p].inventory.HasBattery) t=449; else t=322;
  g_pGfxEngine->Tilemap.drawTile(boxsurface, (dlgX+21)<<3, ((dlgY+4)<<3)+3, t);
  if (player[p].inventory.HasVacuum) t=450; else t=323;
  g_pGfxEngine->Tilemap.drawTile(boxsurface, (dlgX+24)<<3, ((dlgY+4)<<3)+3, t);
  if (player[p].inventory.HasWiskey) t=451; else t=324;
  g_pGfxEngine->Tilemap.drawTile(boxsurface, (dlgX+27)<<3, ((dlgY+4)<<3)+3, t);
  // ray gun charges
  i = player[p].inventory.charges;
  if (i>999) i=999;
  tempbuf = itoa(i);
  g_pGfxEngine->Font.drawFont( boxsurface, tempbuf, (dlgX+4)<<3, (dlgY+12)<<3, 0);

  // score
  i = player[p].inventory.score;
  tempbuf = itoa(i);
  g_pGfxEngine->Font.drawFont( boxsurface, tempbuf, (dlgX+12-tempbuf.size())<<3, (dlgY+2)<<3, 0);
  // extra life at
  i = player[p].inventory.extralifeat;
  tempbuf = itoa(i);
  g_pGfxEngine->Font.drawFont( boxsurface, tempbuf, (dlgX+28-tempbuf.size())<<3, (dlgY+2)<<3, 0);
  // lives
  i = player[p].inventory.lives;
  x = ((dlgX+1)<<3)+4;
  if (i>7) i=7;
  for(j=0;j<i;j++)
  {
	  g_pGraphics->drawSprite_direct(x, (dlgY+4)<<3, playerbaseframes[p]);
	  x += sprites[0].xsize;
  }
}

void inventory_draw_ep2(int p, stCloneKeenPlus *pCKP)
{
int x,/*y,t,*/i,j;
	std::string tempbuf;
int dlgX,dlgY,dlgW,dlgH;

  stLevelControl *p_levelcontrol;
  SDL_Surface *boxsurface = g_pVideoDriver->FGLayerSurface;

  p_levelcontrol = &(pCKP->Control.levelcontrol);

  dlgX = GetStringAttribute("EP2_StatusBox", "LEFT");
  dlgY = GetStringAttribute("EP2_StatusBox", "TOP");
  dlgW = GetStringAttribute("EP2_StatusBox", "WIDTH");
  dlgH = GetStringAttribute("EP2_StatusBox", "HEIGHT");

  dialogbox(dlgX,dlgY,dlgW,dlgH);
  g_pGfxEngine->Font.drawFont( boxsurface, getstring("EP2_StatusBox"), (dlgX+1)<<3, (dlgY+1)<<3, 0);

  // cards
  if (player[p].inventory.HasCardYellow)
  {
	  g_pGfxEngine->Tilemap.drawTile(boxsurface, ((dlgX+21)<<3)-4, ((dlgY+8)<<3)+3, 424);

	  if(player[p].inventory.HasCardYellow > 1)
		  g_pGfxEngine->Font.drawFont( boxsurface, itoa(player[p].inventory.HasCardYellow),(dlgX+20)<<3,((dlgY+8)<<3)+3);

  }
  if (player[p].inventory.HasCardRed)
  {
	  g_pGfxEngine->Tilemap.drawTile(boxsurface, ((dlgX+25)<<3)-4, ((dlgY+8)<<3)+3, 425);

	  if(player[p].inventory.HasCardRed > 1)
		  g_pGfxEngine->Font.drawFont( boxsurface, itoa(player[p].inventory.HasCardRed),(dlgX+24)<<3,((dlgY+8)<<3)+3,0);

  }
  if (player[p].inventory.HasCardGreen)
  {
	  g_pGfxEngine->Tilemap.drawTile(boxsurface, ((dlgX+21)<<3)-4, ((dlgY+10)<<3)+4, 426);

	  if(player[p].inventory.HasCardGreen > 1)
		  g_pGfxEngine->Font.drawFont( boxsurface, itoa(player[p].inventory.HasCardGreen),(dlgX+20)<<3,((dlgY+10)<<3)+3,0);
  }
  if (player[p].inventory.HasCardBlue)
  {
	  g_pGfxEngine->Tilemap.drawTile(boxsurface, ((dlgX+25)<<3)-4, ((dlgY+10)<<3)+4, 427);

	  if(player[p].inventory.HasCardBlue > 1)
		  g_pGfxEngine->Font.drawFont( boxsurface, itoa(player[p].inventory.HasCardBlue),(dlgX+24)<<3,((dlgY+10)<<3)+3,0);

  }
  // cities saved
  if (p_levelcontrol->levels_completed[4]) g_pGfxEngine->Font.drawFont( boxsurface, getstring("EP2_LVL4_TargetName"), (dlgX+1)<<3, (dlgY+8)<<3, 0);
  if (p_levelcontrol->levels_completed[6]) g_pGfxEngine->Font.drawFont( boxsurface, getstring("EP2_LVL6_TargetName"), (dlgX+8)<<3, (dlgY+8)<<3, 0);
  if (p_levelcontrol->levels_completed[7]) g_pGfxEngine->Font.drawFont( boxsurface, getstring("EP2_LVL7_TargetName"), (dlgX+1)<<3, (dlgY+9)<<3, 0);
  if (p_levelcontrol->levels_completed[13]) g_pGfxEngine->Font.drawFont( boxsurface, getstring("EP2_LVL13_TargetName"), (dlgX+8)<<3, (dlgY+9)<<3, 0);
  if (p_levelcontrol->levels_completed[11]) g_pGfxEngine->Font.drawFont( boxsurface, getstring("EP2_LVL11_TargetName"), (dlgX+1)<<3, (dlgY+10)<<3, 0);
  if (p_levelcontrol->levels_completed[9]) g_pGfxEngine->Font.drawFont( boxsurface, getstring("EP2_LVL9_TargetName"), (dlgX+8)<<3, (dlgY+10)<<3, 0);
  if (p_levelcontrol->levels_completed[15]) g_pGfxEngine->Font.drawFont( boxsurface, getstring("EP2_LVL15_TargetName"), (dlgX+1)<<3, (dlgY+11)<<3, 0);
  if (p_levelcontrol->levels_completed[16]) g_pGfxEngine->Font.drawFont( boxsurface, getstring("EP2_LVL16_TargetName"), (dlgX+8)<<3, (dlgY+11)<<3, 0);

  // raygun icon
  g_pGfxEngine->Tilemap.drawTile(boxsurface, (dlgX+20)<<3, ((dlgY+5)<<3)-5, 414);

  // ray gun charges text
  i = player[p].inventory.charges;
  if (i>999) i=999;
  g_pGfxEngine->Font.drawFont( boxsurface, itoa(i), (dlgX+27-tempbuf.size())<<3, ((dlgY+5)<<3)-1, 0);

  // score
  i = player[p].inventory.score;
  g_pGfxEngine->Font.drawFont( boxsurface, itoa(i), (dlgX+12-tempbuf.size())<<3, (dlgY+2)<<3, 0);

  // extra life at
  i = player[p].inventory.extralifeat;
  g_pGfxEngine->Font.drawFont( boxsurface, itoa(i), (dlgX+28-tempbuf.size())<<3, (dlgY+2)<<3, 0);

  // lives
  i = player[p].inventory.lives;
  x = ((dlgX + 1)<<3)+4;
  if (i>7) i=7;
  for(j=0;j<i;j++)
  {
	  g_pGraphics->drawSprite_direct(x, (dlgY+4)<<3, playerbaseframes[p]);
    x += sprites[0].xsize;
  }

}

void inventory_draw_ep3(int p)
{
//int x,y,t,i,j;
int i,j,x;
int ankhtimepercent;
	std::string tempbuf;
int dlgX,dlgY,dlgW,dlgH;

  SDL_Surface *boxsurface = g_pVideoDriver->FGLayerSurface;

  dlgX = GetStringAttribute("EP3_StatusBox", "LEFT");
  dlgY = GetStringAttribute("EP3_StatusBox", "TOP");
  dlgW = GetStringAttribute("EP3_StatusBox", "WIDTH");
  dlgH = GetStringAttribute("EP3_StatusBox", "HEIGHT");

  dialogbox(dlgX,dlgY,dlgW,dlgH);
  g_pGfxEngine->Font.drawFont( boxsurface, getstring("EP3_StatusBox"), (dlgX+1)<<3, (dlgY+1)<<3, 0);

  // calculate % ankh time left
  ankhtimepercent = (int)((float)player[p].ankhtime / (PLAY_ANKH_TIME/100));
  // ankh time
  g_pGfxEngine->Tilemap.drawTile(boxsurface, (dlgX+4)<<3, ((dlgY+8)<<3)+3, 214);

  g_pGfxEngine->Font.drawFont( boxsurface, itoa(ankhtimepercent), (dlgX+8)<<3, ((dlgY+8)<<3)+7, 0);

  // raygun icon
  g_pGfxEngine->Tilemap.drawTile(boxsurface, (dlgX+23)<<3, ((dlgY+5)<<3)-5, 216);

  // ray gun charges text
  i = player[p].inventory.charges;
  if (i>999) i=999;
  tempbuf = itoa(i);
  g_pGfxEngine->Font.drawFont( boxsurface, tempbuf, (dlgX+26)<<3, ((dlgY+5)<<3)-1, 0);

  // cards
  if (player[p].inventory.HasCardYellow)
  {
	  g_pGfxEngine->Tilemap.drawTile(boxsurface, ((dlgX+14)<<3)+4, ((dlgY+8)<<3)+4, 217);

	  if(player[p].inventory.HasCardYellow > 1)
		  g_pGfxEngine->Font.drawFont( boxsurface, itoa(player[p].inventory.HasCardYellow),(dlgX+13)<<3,((dlgY+8)<<3)+3,0);

  }
  if (player[p].inventory.HasCardRed)
  {
	  g_pGfxEngine->Tilemap.drawTile(boxsurface, ((dlgX+18)<<3)+4, ((dlgY+8)<<3)+4, 218);

	  if(player[p].inventory.HasCardRed > 1)
		  g_pGfxEngine->Font.drawFont(boxsurface, itoa(player[p].inventory.HasCardRed),(dlgX+17)<<3,((dlgY+8)<<3)+3,0);

  }
  if (player[p].inventory.HasCardGreen)
  {
	  g_pGfxEngine->Tilemap.drawTile(boxsurface, ((dlgX+22)<<3)+4, ((dlgY+8)<<3)+4, 219);

	  if(player[p].inventory.HasCardGreen > 1)
		  g_pGfxEngine->Font.drawFont(boxsurface, itoa(player[p].inventory.HasCardGreen),(dlgX+21)<<3,((dlgY+8)<<3)+3,0);

  }
  if (player[p].inventory.HasCardBlue)
  {
	  g_pGfxEngine->Tilemap.drawTile(boxsurface, ((dlgX+26)<<3)+4, ((dlgY+8)<<3)+4, 220);

	  if(player[p].inventory.HasCardBlue > 1)
		  g_pGfxEngine->Font.drawFont(boxsurface, itoa(player[p].inventory.HasCardBlue),(dlgX+25)<<3,((dlgY+8)<<3)+3,0);

  }

  // score
  i = player[p].inventory.score;
  g_pGfxEngine->Font.drawFont(boxsurface, itoa(i), (dlgX+12-tempbuf.size())<<3, (dlgY+2)<<3);
  // extra life at
  i = player[p].inventory.extralifeat;
  g_pGfxEngine->Font.drawFont(boxsurface, itoa(i), (dlgX+28-tempbuf.size())<<3, (dlgY+2)<<3);
  // lives
  i = player[p].inventory.lives;
  x = ((dlgX+1)<<3)+4;
  if (i>9) i=9;
  for(j=0;j<i;j++)
  {
	  g_pGraphics->drawSprite_direct(x, (dlgY+4)<<3, playerbaseframes[p]);
	  x += sprites[0].xsize;
  }
}

void showinventory(int p, stCloneKeenPlus *pCKP)
{
	stLevelControl *p_levelcontrol;
	p_levelcontrol = &(pCKP->Control.levelcontrol);

  // draw the episode-specific stuff
  if (p_levelcontrol->episode==1)
    inventory_draw_ep1(p);

  else if (p_levelcontrol->episode==2)
    inventory_draw_ep2(p, pCKP);

  else if (p_levelcontrol->episode==3)
    inventory_draw_ep3(p);


  g_pVideoDriver->update_screen();

  // wait for any button pressed or any action triggered
  bool close=false;

  g_pInput->flushAll();

  while(!close)
  {
	g_pInput->pollEvents();

	if(g_pInput->getPressedAnyCommand(p) || g_pInput->getPressedAnyKey()) close=true;
  }
}

void YourShipNeedsTheseParts(stCloneKeenPlus *pCKP)
{
int cp = 0;
int dlgX,dlgY,dlgW,dlgH;

  dlgX = GetStringAttribute("EP1_SHIP", "LEFT");
  dlgY = GetStringAttribute("EP1_SHIP", "TOP");
  dlgW = GetStringAttribute("EP1_SHIP", "WIDTH");
  dlgH = GetStringAttribute("EP1_SHIP", "HEIGHT");

  dialogbox(dlgX,dlgY,dlgW,dlgH);

  SDL_Surface *boxsurface = g_pVideoDriver->FGLayerSurface;

  g_pGfxEngine->Font.drawFont( boxsurface, getstring("EP1_SHIP"), (dlgX+1)<<3, (dlgY+1)<<3,0);

  // draw needed parts
  if (!player[cp].inventory.HasJoystick)
	  g_pGfxEngine->Tilemap.drawTile(boxsurface, (dlgX+9)<<3, (dlgY+3)<<3, 448);

  if (!player[cp].inventory.HasBattery)
	  g_pGfxEngine->Tilemap.drawTile(boxsurface, (dlgX+12)<<3, (dlgY+3)<<3, 449);

  if (!player[cp].inventory.HasVacuum)
	  g_pGfxEngine->Tilemap.drawTile(boxsurface, (dlgX+15)<<3, (dlgY+3)<<3, 450);

  if (!player[cp].inventory.HasWiskey)
	  g_pGfxEngine->Tilemap.drawTile(boxsurface, (dlgX+18)<<3, (dlgY+3)<<3, 451);

  g_pVideoDriver->update_screen();

  // wait for any key!
  g_pInput->flushKeys();
  while(!g_pInput->getPressedAnyKey())
  {
	g_pInput->pollEvents();

	if(g_pInput->getPressedAnyCommand()) break;
  }
}

void ShipEp3(stCloneKeenPlus *pCKP)
{
char strname[80];
int twirlframe, twirltimer;
int dlgX,dlgY,dlgW,dlgH,twirlX,twirlY;
const int twirlspeed = 100;

	SDL_Surface *boxsurface = g_pVideoDriver->FGLayerSurface;

  // display one of four random strings
  sprintf(strname, "EP3_SHIP%d", (rand()%4)+1);

  dlgX = GetStringAttribute(strname, "LEFT");
  dlgY = GetStringAttribute(strname, "TOP");
  dlgW = GetStringAttribute(strname, "WIDTH");
  dlgH = GetStringAttribute(strname, "HEIGHT");
  twirlX = GetStringAttribute(strname, "TWIRLX");
  twirlY = GetStringAttribute(strname, "TWIRLY");

  dialogbox(dlgX,dlgY,dlgW,dlgH);
  g_pGfxEngine->Font.drawFont( boxsurface, getstring(strname), (dlgX+1)<<3, (dlgY+1)<<3,0);

  g_pVideoDriver->update_screen();

  g_pInput->flushAll();

    twirlframe = 0;
    twirltimer = twirlspeed+1;
    g_pInput->flushKeys();
    // wait for any command or key
    do
    {
	if (twirltimer>twirlspeed)
	{
		g_pGfxEngine->Font.drawCharacter( boxsurface, (dlgX+twirlX)<<3, (dlgY+twirlY)<<3, twirlframe+9);
		g_pVideoDriver->update_screen();
		twirlframe++;
		if (twirlframe>5) twirlframe=0;
		twirltimer=0;
	} else twirltimer++;
	if(g_pInput->getPressedAnyCommand()) break;
	g_pInput->pollEvents();
    } while(!g_pInput->getPressedAnyKey());
}

// pops up the "which slot do you want to save to" box.
// is issave=1, it's a save box, if issave=0, it's a load box
// returns either the selected slot or 0 if canceled
char save_slot_box(int issave, stCloneKeenPlus *pCKP)
{
char saveslot;
FILE *fp;
	std::string fname;
char slotexists;
int x, bmnum;
int dlgX,dlgY,dlgW,dlgH;

stLevelControl *p_levelcontrol = &(pCKP->Control.levelcontrol);
SDL_Surface *boxsfc = g_pVideoDriver->FGLayerSurface;

top: ;
  if (issave)
  {
     dlgX = GetStringAttribute("WhichSlotSave", "LEFT");
     dlgY = GetStringAttribute("WhichSlotSave", "TOP");
     dlgW = GetStringAttribute("WhichSlotSave", "WIDTH");
     dlgH = GetStringAttribute("WhichSlotSave", "HEIGHT");
  }
  else
  {
     dlgX = GetStringAttribute("WhichSlotLoad", "LEFT");
     dlgY = GetStringAttribute("WhichSlotLoad", "TOP");
     dlgW = GetStringAttribute("WhichSlotLoad", "WIDTH");
     dlgH = GetStringAttribute("WhichSlotLoad", "HEIGHT");
     map_redraw();
     bmnum = g_pGraphics->getBitmapNumberFromName("TITLE");
     x = (320/2)-(bitmaps[bmnum].xsize/2);
     g_pGraphics->drawBitmap(x, 0, bmnum);
  }

  saveslot = 0;
  do
  {
	gamedo_render_drawobjects();

	g_pGfxEngine->drawDialogBox(boxsfc, dlgX,dlgY,dlgW,dlgH);
	if (issave)
	{
		g_pGfxEngine->Font.drawFont(boxsfc, getstring("WhichSlotSave"),(dlgX+1)<<3,(dlgY+1)<<3);
	}
	else
	{
		g_pGfxEngine->Font.drawFont(boxsfc, getstring("WhichSlotLoad"),(dlgX+1)<<3,(dlgY+1)<<3);
		gamedo_AnimatedTiles();
	}

	for (int i=0 ; i<9  ; i++)
	{
		if (g_pInput->getPressedKey(KNUM1+i)) saveslot = 1+i;
	}

	gamedo_frameskipping_blitonly();
	gamedo_render_eraseobjects();

	g_pInput->pollEvents();
  } while(!g_pInput->getPressedKey(KQUIT) && !saveslot);

  /* check if the selected save file exists */
	fname = "ep";
	fname += p_levelcontrol->episode+'0';
	fname += "save";
	fname += saveslot+'0';
	fname += ".dat";
  slotexists = 0;
  fp = OpenGameFile(fname.c_str(), "rb");
  if (fp)
  {
    fclose(fp);
    slotexists = 1;
  }

  if ((issave && !slotexists) || (!issave && slotexists))
  {
    map_redraw();
    return saveslot;
  }

  if (issave)
  {
     dlgX = GetStringAttribute("SaveSlotOverwrite", "LEFT");
     dlgY = GetStringAttribute("SaveSlotOverwrite", "TOP");
     dlgW = GetStringAttribute("SaveSlotOverwrite", "WIDTH");
     dlgH = GetStringAttribute("SaveSlotOverwrite", "HEIGHT");
  }
  else
  {
     dlgX = GetStringAttribute("LoadNoSuchSlot", "LEFT");
     dlgY = GetStringAttribute("LoadNoSuchSlot", "TOP");
     dlgW = GetStringAttribute("LoadNoSuchSlot", "WIDTH");
     dlgH = GetStringAttribute("LoadNoSuchSlot", "HEIGHT");
  }

  // either we're trying to save over an existing game, or we're
  // loading a game that doesn't exist.
  do
  {
	  gamedo_render_drawobjects();

	  g_pGfxEngine->drawDialogBox(boxsfc, dlgX,dlgY,dlgW,dlgH);
	  if (issave)
	  {
		g_pGfxEngine->Font.drawFont(boxsfc, getstring("SaveSlotOverwrite"),(dlgX+1)<<3,(dlgY+1)<<3);
	    if (g_pInput->getPressedKey(KN))
	    {
		  map_redraw();
		  goto top;
	    }
	    else if (g_pInput->getPressedKey(KY))
	    {
		  map_redraw();
		  return saveslot;
	    }
	  }
	  else
	  {
		  g_pGfxEngine->Font.drawFont(boxsfc, getstring("LoadNoSuchSlot"),(dlgX+1)<<3,(dlgY+1)<<3);

		  if (g_pInput->getPressedAnyKey())
		  {
			  map_redraw();
			  goto top;
		  }

		  gamedo_AnimatedTiles();
	  }

	  gamedo_frameskipping_blitonly();
	  gamedo_render_eraseobjects();

	  g_pInput->pollEvents();
  } while(!g_pInput->getPressedKey(KQUIT));

  map_redraw();
  return 0;
}

void game_save_interface(stCloneKeenPlus *pCKP)
{
int waittimer;
char saveslot;
int dlgX,dlgY,dlgW,dlgH;

  dlgX = GetStringAttribute("GameSaveSuccess", "LEFT");
  dlgY = GetStringAttribute("GameSaveSuccess", "TOP");
  dlgW = GetStringAttribute("GameSaveSuccess", "WIDTH");
  dlgH = GetStringAttribute("GameSaveSuccess", "HEIGHT");

  SDL_Surface *sfc = g_pVideoDriver->FGLayerSurface;

  saveslot = save_slot_box(1, pCKP);
  if (!saveslot) return;                // canceled

  /* save the game */
  CSavedGame *SavedGame = new CSavedGame(&(pCKP->Control.levelcontrol));
  SavedGame->save(saveslot);
  delete SavedGame;

  /* display the "your game has been saved" box */
  waittimer = 0;
  do
  {
	waittimer++;
	if (waittimer > 5000) break;

	gamedo_render_drawobjects();

	g_pGfxEngine->drawDialogBox( sfc, dlgX,dlgY,dlgW,dlgH);
	g_pGfxEngine->Font.drawFont( sfc, getstring("GameSaveSuccess"), (dlgX+1)<<3, (dlgY+1)<<3 );

	gamedo_frameskipping_blitonly();
	gamedo_render_eraseobjects();

	g_pInput->pollEvents();
  } while(!g_pInput->getPressedAnyKey());

  map_redraw();
}

int VerifyQuit()
{
int dlgX,dlgY,dlgW,dlgH;
	std::string text;

  if (fade_in_progress()) return NO_QUIT;

  text = getstring("VerifyQuit");
  dlgX = GetStringAttribute("VerifyQuit", "LEFT");
  dlgY = GetStringAttribute("VerifyQuit", "TOP");
  dlgW = GetStringAttribute("VerifyQuit", "WIDTH");
  dlgH = GetStringAttribute("VerifyQuit", "HEIGHT");

  SDL_Surface *sfc = g_pVideoDriver->FGLayerSurface;
  // either we're trying to save over an existing game, or we're
  // loading a game that doesn't exist.
  do
  {
	gamedo_render_drawobjects();
	gamedo_AnimatedTiles();

	g_pGfxEngine->drawDialogBox( sfc, dlgX,dlgY,dlgW,dlgH);
	g_pGfxEngine->Font.drawFont( sfc, text, (dlgX+1)<<3, (dlgY+1)<<3 );

	if (g_pInput->getPressedKey(KQ))
	{
	  map_redraw();
	  QuitState = QUIT_PROGRAM;
	  return 0;
	}
	else if (g_pInput->getPressedKey(KT))
	{
	  map_redraw();
	  QuitState = QUIT_TO_TITLE;
	  return QuitState;
	}
	else if (g_pInput->getPressedKey(KQUIT))
	{
	  map_redraw();
	  QuitState = NO_QUIT;
	  return QuitState;
	}

	g_pInput->pollEvents();

	gamedo_frameskipping_blitonly();
	gamedo_render_eraseobjects();
  } while(1);
}

void AllPlayersInvisible(void)
{
int i;

  for(i=0;i<MAX_PLAYERS;i++)
  {
    if (player[i].isPlaying)
    {
      objects[player[i].useObject].onscreen = 0;
      player[i].hideplayer = 1;
    }
  }
}

char gameiswon(stCloneKeenPlus *pCKP)
{
int partcount;
int i;
	stLevelControl *p_levelcontrol;
	p_levelcontrol = &(pCKP->Control.levelcontrol);

       if (p_levelcontrol->episode==1)
       {
          /* episode 1: game is won when all parts are collected */

           // count the number of parts the players have acquired
           partcount = 0;
           for(i=0;i<MAX_PLAYERS;i++)
           {
             if (player[i].isPlaying)
             {
               if (player[i].inventory.HasJoystick) partcount++;
               if (player[i].inventory.HasBattery) partcount++;
               if (player[i].inventory.HasWiskey) partcount++;
               if (player[i].inventory.HasVacuum) partcount++;
             }
           }

           // go to end sequence if all the parts have been got
           if (partcount >= 4)
           {
             return 1;
           }
           else return 0;
       }
       else if (p_levelcontrol->episode==2)
       {
         /* episode 2: game is won when all cities are saved */
         if (!p_levelcontrol->levels_completed[4]) return 0;
         if (!p_levelcontrol->levels_completed[6]) return 0;
         if (!p_levelcontrol->levels_completed[7]) return 0;
         if (!p_levelcontrol->levels_completed[13]) return 0;
         if (!p_levelcontrol->levels_completed[11]) return 0;
         if (!p_levelcontrol->levels_completed[9]) return 0;
         if (!p_levelcontrol->levels_completed[15]) return 0;
         if (!p_levelcontrol->levels_completed[16]) return 0;
         return 1;
       }
       else if (p_levelcontrol->episode==3)
       {
         /* episode 3: game is won when mortimer is defeated */
         if (p_levelcontrol->levels_completed[16])
         {
           return 1;
         }
         else
         {
           return 0;
         }
       }

return 0;
}

void usage(void)
{
	// TODO: Many options has to be changed. For now it is disabled
	/*
  printf("Usage: keen [lvlnum] [-*player] [-nopk] [-ep*] [-dtm] [-nocheat] [-rec] -[eseq]<br>\n");
  printf("lvlnum          specify a level number (such as 2) to go directly to that level<br>");
  printf("-*player        select number of players (1-4); defaults to 1<br>");
  printf("-nopk           do not allow players to kill each other in multiplayer games<br>");
  printf("-game*            select game of data base; if not given, start menu is opened<br>");
  printf("-dtm            go directly to the world map, bypassing intro and title screen<br>");
  printf("-mean           increase game difficulty<br>");
  printf("-cheat          enable function key cheat/debug codes<br>");
  printf("-rec            record player actions to demo.dat for making a demo<br>");
  printf("-eseq           for the impatient--cut directly to the ending sequence<br>");
#ifdef BUILD_SDL
  printf("-fs             use fullscreen mode<br>");
  printf("-dbl            zoom image 2x<br>");
  printf("-ogl            hardware acceleration<br>");
  printf("-showfps        show FPS in upper-right of screen<br>");
#endif

  printf("<br>");
  printf("Examples:<br>");
  printf("  keen 3 -ep2                play ep 2, level 3 in 1-player mode<br>");
  printf("  keen -ep3 -dtm -2player    play ep3, skip title&intro, 2-player mode<br>");
  printf("  keen -ep3                  play a normal game of ep3<br>");
  */
}

unsigned int rnd(void)
{
  //random_seed = random_seed * 1103515245 + 12345;
  //return (uint)(random_seed / 65536) % 32768;
	return rand();
}

void radar(void)
{
unsigned int x,y,o;
unsigned int x1,y1,x2,y2;
unsigned int yoff;
  // draw the map
  for(y=0;y<map.ysize;y++)
  {
    yoff = ((y+4+scrolly_buf)&511)<<9;
    for(x=0;x<map.xsize;x++)
    	g_pGraphics->getScrollbuffer()[yoff+((4+x+scrollx_buf)&511)] = map.mapdata[x][y]&15;

  }

  // draw objects
  for(o=0;o<MAX_OBJECTS;o++)
  {
    if (objects[o].exists)
    {
      x = objects[o].x >> CSF >> 4;
      y = objects[o].y >> CSF >> 4;

      yoff = ((y+4+scrolly_buf)&511)<<9;
      g_pGraphics->getScrollbuffer()[yoff+((4+x+scrollx_buf)&511)] = objects[o].type&15;
    }
  }

  // draw the area that is visible in the scrollbuffer
  x1 = mapx; y1 = mapy;
  x2 = x1+32; y2 = y1+32;
  for(y=y1;y<y2;y++)
  {
     if (y<map.ysize)
     {
       yoff = ((y+4+scrolly_buf)&511)<<9;
       g_pGraphics->getScrollbuffer()[yoff+((4+x1+scrollx_buf)&511)] = 10;
       if (x2<map.xsize)
    	   g_pGraphics->getScrollbuffer()[yoff+((4+x2+scrollx_buf)&511)] = 10;
     }
  }
  for(x=x1;x<=x2;x++)
  {
     if (y1 < map.ysize && x < map.xsize)
     {
       yoff = ((y1+4+scrolly_buf)&511)<<9;
       g_pGraphics->getScrollbuffer()[yoff+((4+x+scrollx_buf)&511)] = 10;
     }
     if (y2 < map.ysize && x < map.xsize)
     {
       yoff = ((y2+4+scrolly_buf)&511)<<9;
       g_pGraphics->getScrollbuffer()[yoff+((4+x+scrollx_buf)&511)] = 10;
     }
  }

  // draw the area that is visible on the screen
  // 320x200 = 20x12.5 tiles
  x1 = scroll_x>>4; y1 = scroll_y>>4;
  x2 = x1+20; y2 = y1+12;
  for(y=y1;y<y2;y++)
  {
     if (y<map.ysize)
     {
       yoff = ((y+4+scrolly_buf)&511)<<9;
       g_pGraphics->getScrollbuffer()[yoff+((4+x1+scrollx_buf)&511)] = 12;
       if (x2<map.xsize)
    	   g_pGraphics->getScrollbuffer()[yoff+((4+x2+scrollx_buf)&511)] = 12;
     }
  }
  for(x=x1;x<=x2;x++)
  {
     if (x < map.xsize)
     {
       if (y1 < map.ysize)
       {
         yoff = ((y1+4+scrolly_buf)&511)<<9;
         g_pGraphics->getScrollbuffer()[yoff+((4+x+scrollx_buf)&511)] = 12;
       }
       if (y2 < map.ysize)
       {
         yoff = ((y2+4+scrolly_buf)&511)<<9;
         g_pGraphics->getScrollbuffer()[yoff+((4+x+scrollx_buf)&511)] = 12;
       }
     }
  }
}

void SetAllCanSupportPlayer(int o, int state)
{
	unsigned int i;
	for(i=0;i<numplayers;i++)
		objects[o].cansupportplayer[i] = state;
}

void showTextMB(const std::string& Text)
{
	CTextBox* TextBox;
	CWindow *InfoTextWindow = new CWindow( g_pVideoDriver->FGLayerSurface, 60, 50, 200, 100 );

	TextBox = new CTextBox( g_pVideoDriver->FGLayerSurface, 60, 50, 200, 100, Text, true);
	TextBox->setFontDimensions(8, 8);
	InfoTextWindow->addObject(TextBox);

	// The Text will be too big, so resize in knowing the height of the first text.
	InfoTextWindow->Resize(InfoTextWindow->getWidth(),
			 (InfoTextWindow->m_TextBox[0]->getNumberOfTextlines()+2)*8 );

	g_pInput->flushAll();
    do
    {
		g_pInput->pollEvents();
		InfoTextWindow->render();
		g_pVideoDriver->update_screen();
    } while(!g_pInput->getPressedAnyCommand());

    delete InfoTextWindow;
}

// This function shows up the helping text when F1 is pushed
void showF1HelpText(int episode, std::string DataDirectory)
{
   std::string helptext;

   if(episode==1)
   {
	   // We suppose that we are using version 131. Maybe it must be extended
		   std::string filename = "data/" + DataDirectory;
		   if(DataDirectory != "")
			   filename += "/";

		   filename += "helptext.ck1";

		   std::ifstream File; OpenGameFileR(File, filename, std::ios::binary);

		   if(!File) return;

		   while(!File.eof())
			helptext.push_back(File.get());

		File.close();
   }
   else
   {
	   // Here the Text file is within the EXE-File
	   unsigned long startflag=0, endflag=0;
	   unsigned char *text_data = NULL;

	   CExeFile *ExeFile = new CExeFile(episode, DataDirectory);
	   ExeFile->readData();

	   if(!ExeFile->getData()) return;

	   if(episode == 2)
	   {
		   startflag = 0x15DC0-512;
		   endflag = 0x1659F-512;
	   }
	   else // Episode 3
	   {
		   startflag = 0x17BD0-512;
		   endflag = 0x1839F-512;
	   }

	   text_data = ExeFile->getData();

	   for(unsigned long i=startflag ; i<endflag ; i++ )
		   helptext.push_back(text_data[i]);

	   delete ExeFile;
   }

   helptext += "\n\nNOTE: The controls in CKP may be different than in the help described.\nCheck out the Readme.txt!\n";

   CTextViewer *Textviewer = new CTextViewer(g_pVideoDriver->FGLayerSurface, 0, 0, 320, 140);

   Textviewer->loadText(helptext);

   Textviewer->processCycle();

   delete Textviewer;
}

