#include "SelectionMenu.h"

CPlayerSpriteVarSelection::
CPlayerSpriteVarSelection(const GsControl::Style style) :
    GameMenu( GsRect<float>(0.25f, 0.2f, 0.5f, 0.5f), style )
{

    mpMenuDialog->addControl( new GameButton( "Start",
                                          new SelectPlayerSpriteVarEvent(mCurId),
                                          style ) );
}

void
CPlayerSpriteVarSelection::
ponder(const float deltaT)
{


    // TODO: Code for Player selection here!
    GameMenu::ponder(deltaT);
}
