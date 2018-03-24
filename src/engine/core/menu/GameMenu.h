#ifndef GameMenu_H_INCLUDED
#define GameMenu_H_INCLUDED

#include <widgets/GsBaseMenu.h>
#include <widgets/GsMenuController.h>

#include "widgets/Button.h"

class GameMenu : public CBaseMenu
{
public:
    GameMenu( const GsRect<float>& rect, 
              const GsControl::Style &style ) :
        CBaseMenu( rect, CGUIDialog::FXKind(0) ),
        mStyle(style)
    {
        //GsRect<float> localRect(0.30f, 0.282f, 0.5f, 0.5f);
        GsRect<float> localRect(0.25f, 0.282f, 0.6f, 0.5f);
        mpMenuDialog->setRect(localRect);


        if(style == GsControl::GALAXY)
        {
            mpReturnButton = new GameButton( "x",
                                             new CloseMenuEvent(),
                                             style);

            mpMenuDialog->addControl( mpReturnButton,
                                      GsRect<float>(-0.1f, 0.1f,
                                                    0.03f/localRect.w,
                                                    0.03f/localRect.h) );

        }
        else if(style == GsControl::VORTICON)
        {
            mpReturnButton = new GsButton( "x",
                                           new CloseMenuEvent(),
                                           GsControl::Style::UNSET);

            mpMenuDialog->addControl( mpReturnButton,
                                      GsRect<float>(-0.05f, -0.05f,
                                                    0.08f/localRect.w,
                                                    0.08f/localRect.h) );
        }


        mpReturnButton->setDown(true);

        if(style == GsControl::GALAXY)
        {
            createGalaxyBackground();
        }
        else if(style == GsControl::VORTICON)
        {
            initVorticonBackground();
        }

        
    }

    void createGalaxyBackground();

    void initVorticonBackground();

    // Processes the stuff that the menus have in common
    virtual void ponder(const float deltaT) override;

    void render() override;

    void setMenuLabel(const std::string &label) override;
    
    GsControl::Style Style() 
    {   return mStyle;   }

private:
    GsSurface mBackground;
    
    const GsControl::Style mStyle;

};

#endif /* GameMenu_H_INCLUDED */
