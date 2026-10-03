#include "EkopartyQrModule.h"

#if defined(EKOPARTY_BADGE) && defined(HAS_NEOPIXEL) && HAS_SCREEN

#include "EkopartyBadgeSettings.h"
#include "EkopartyQrCode.h"
#include "OLEDDisplay.h"
#include "graphics/ScreenFonts.h"

EkopartyQrModule::EkopartyQrModule() : MeshModule("ekoparty_qr"), visible(ekoparty::getBadgeSettings().manualVisible) {}

void EkopartyQrModule::requestFrameFocus()
{
    if (!visible) {
        return;
    }
    requestFocus();
    UIFrameEvent event;
    event.action = UIFrameEvent::Action::REGENERATE_FRAMESET;
    notifyObservers(&event);
}

void EkopartyQrModule::setVisible(bool value)
{
    if (visible == value) {
        return;
    }
    visible = value;
    UIFrameEvent event;
    event.action = UIFrameEvent::Action::REGENERATE_FRAMESET_BACKGROUND;
    notifyObservers(&event);
}

void EkopartyQrModule::drawFrame(OLEDDisplay *display, OLEDDisplayUiState *screenState, int16_t x, int16_t y)
{
    (void)screenState;

    display->setColor(WHITE);
    display->fillRect(x, y, ekoparty::qrCanvasSize, ekoparty::qrCanvasSize);
    display->setColor(BLACK);
    for (uint8_t row = 0; row < ekoparty::qrSize; row++) {
        for (uint8_t column = 0; column < ekoparty::qrSize; column++) {
            if ((ekoparty::qrRows[row] & (1UL << (ekoparty::qrSize - column - 1U))) != 0) {
                display->fillRect(x + ekoparty::qrOffset + column * ekoparty::qrScale,
                                  y + ekoparty::qrOffset + row * ekoparty::qrScale, ekoparty::qrScale, ekoparty::qrScale);
            }
        }
    }

    display->setColor(WHITE);
    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->setFont(FONT_SMALL);
    display->drawString(x + 96, y + 13, "MANUAL");
    display->drawString(x + 96, y + 30, "DE");
    display->drawString(x + 96, y + 47, "USUARIO");
    display->setTextAlignment(TEXT_ALIGN_LEFT);
}

#endif
