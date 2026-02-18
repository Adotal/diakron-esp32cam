#include "button_driver.h"
#include "service_ui.h"
#include "display_driver.h"

static unsigned long pressStartTime = 0;
static bool lastButtonState = HIGH;
static bool longPressDetected = false;

void handleButton()
{
    bool currentStateBtn = digitalRead(SERVICE_BUTTON_PIN);
    // Detecta el inicio de la pulsación
    if (lastButtonState == HIGH && currentStateBtn == LOW)
    {
        pressStartTime = millis();
        longPressDetected = false;
    }
    // Mientras este presionado y no se haya detectado el long press, verifica si se ha superado el tiempo para considerar un long press
    if (lastButtonState == LOW && currentStateBtn == LOW)
    {
        if (!longPressDetected && (millis() - pressStartTime >= 2000))
        {
            longPressDetected = true;
            ui_select();
        }
    }
    // Cuando el botón se suelta, si no se detectó un long press, se considera una pulsación corta
    if (lastButtonState == LOW && currentStateBtn == HIGH)
    {
        if (!longPressDetected)
        {
            ui_nextItem();
        }
    }

    lastButtonState = currentStateBtn;
}
