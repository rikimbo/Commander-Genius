#ifndef GameMenu_H_INCLUDED
#define GameMenu_H_INCLUDED

#include <widgets/GsBaseMenu.h>
#include <widgets/GsMenuController.h>

#include "widgets/Button.h"

class GameMenu : public CBaseMenu
{
public:
    GameMenu( const GsRect<float>& rect, 
              const Style &style );

    void initGalaxyBackground();
    void initVorticonBackground();
    void initBackgroundNoStyle();

    void (GameMenu::*initBackground) ();

    void setProperty( const Property newProperty ) override
    {
        mpReturnButton->setText( newProperty == CLOSEABLE ? "close" : "back" );
        auto rect = mpReturnButton->getRect();
        rect.dim.x = 0.2f;
        mpReturnButton->setRect(rect);
    }


    // Processes the stuff that the menus have in common
    virtual void ponder(const float deltaT) override;

    void render() override;

    void setMenuLabel(const std::string &label) override;
    
    Style getStyle()
    {   return mStyle;   }

private:
    GsSurface mBackground;
    GsRect<int> mCachedBgRect;
    
    const Style mStyle;
};

#endif /* GameMenu_H_INCLUDED */
