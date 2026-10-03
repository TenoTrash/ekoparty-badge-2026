#pragma once

#include "configuration.h"

#if defined(EKOPARTY_BADGE) && defined(HAS_NEOPIXEL) && HAS_SCREEN

#include "Observer.h"
#include "mesh/MeshModule.h"

class EkopartyQrModule : public MeshModule, public Observable<const UIFrameEvent *>
{
  public:
    EkopartyQrModule();
    void requestFrameFocus();
    void setVisible(bool value);
    bool isVisible() const { return visible; }

  protected:
    virtual bool wantPacket(const meshtastic_MeshPacket *) override { return false; }
    virtual void drawFrame(OLEDDisplay *display, OLEDDisplayUiState *screenState, int16_t x, int16_t y) override;
    virtual bool wantUIFrame() override { return visible; }
    virtual Observable<const UIFrameEvent *> *getUIFrameObservable() override { return this; }
    virtual bool suppressNavigationBar() override { return true; }

  private:
    bool visible;
};

#endif
