/*
 TiLDA Mk2

 GUITask

 The MIT License (MIT)

 Copyright (c) 2014 Electromagnetic Field LTD

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#include "GUITask.h"
#include "ButtonSubscription.h"
#include "debug.h"
#include "Tilda.h"
#include <glcd.h>
#include <fonts/allFonts.h>
#include <M2tk.h>

static ButtonSubscription* _buttons_p;
static Orientation_t _current_orientation;
enum ButtonMap_t{
    BUTTONMAP_UP,
    BUTTONMAP_DOWN,
    BUTTONMAP_LEFT,
    BUTTONMAP_RIGHT
};

//HELD,RIGHT,HUNG,LEFT
    //UP,DOWN,LEFT,RIGHT
static uint8_t keymap[4][4] = {
  {M2_KEY_DATA_UP, M2_KEY_DATA_DOWN, M2_KEY_PREV, M2_KEY_NEXT},
  {M2_KEY_PREV, M2_KEY_NEXT, M2_KEY_DATA_DOWN, M2_KEY_DATA_UP},
  {M2_KEY_DATA_DOWN, M2_KEY_DATA_UP, M2_KEY_NEXT, M2_KEY_PREV},
  {M2_KEY_NEXT, M2_KEY_PREV, M2_KEY_DATA_UP, M2_KEY_DATA_DOWN}
};

uint8_t m2_es_button_subscription(m2_p ep, uint8_t msg)
{
  //static ButtonSubscription* _buttons_p;
  if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
    if (_buttons_p == 0) {
      _buttons_p = new ButtonSubscription();
      _buttons_p->addButtons(UP | DOWN | LEFT | RIGHT | A | B | CENTER);
    }
  }

  switch(msg)
  {
    case M2_ES_MSG_GET_KEY:
    {
      Button button;
      if (_buttons_p) {
       button = _buttons_p->waitForPress(portMAX_DELAY);
      } else {
        // read button state manually

      }
      switch (button)
      {
      case UP:
        return keymap[_current_orientation][BUTTONMAP_UP] | M2_KEY_EVENT_MASK;
        break;
      case DOWN:
        return keymap[_current_orientation][BUTTONMAP_DOWN] | M2_KEY_EVENT_MASK;
        break;
      case LEFT:
        return keymap[_current_orientation][BUTTONMAP_LEFT] | M2_KEY_EVENT_MASK;
        break;
      case RIGHT:
        return keymap[_current_orientation][BUTTONMAP_RIGHT] | M2_KEY_EVENT_MASK;
        break;
      case A:
        return M2_KEY_SELECT | M2_KEY_EVENT_MASK;
        break;
      case B:
        return M2_KEY_EXIT | M2_KEY_EVENT_MASK;
        break;
      case CENTER:
        return M2_KEY_SELECT | M2_KEY_EVENT_MASK;
        break;
      }
      /* return M2_KEY_NONE if there is no button pressed. */
      return M2_KEY_NONE;
    }
    case M2_ES_MSG_INIT:
      /* code, which will be executed once at startup of the controller */
      return 0;
  }
  return 0;
}

String GUITask::getName() const
{
  return "GUI";
}

void GUITask::task() {
    // Main M2tkloop
    _m2 = new M2tk(&m2_null_element, m2_es_button_subscription, m2_eh_6bs, m2_gh_glcd_uffs);  // Create M2 with no root element, it can be added when required.
    _m2->setFont(0,System5x7);
    _m2->setFont(1,Arial14);
    _m2->setFont(3,Wendy3x5);
    while(true) {
        _m2->checkKey(); // This will block until there is a key press.
        if ( _m2->handleKey() ) {
            _m2->draw();
        }
    }
}

void GUITask::setM2Root(m2_rom_void_p newRoot, uint8_t next_cnt, uint8_t cv, bool setHome) {
    debug::log("GUITask::setM2Root");

    if (_m2->getRoot() == &m2_null_element) {
        _m2->getKey(); //clear key buffer
    }
    _m2->setRoot(newRoot, next_cnt, cv);
    if (setHome)
      _m2->setHome(newRoot);
    if (_buttons_p != 0) {
        _buttons_p->clear();
        _buttons_p->wake();
    }
}

void GUITask::clearRoot() {
    _m2->setRoot(&m2_null_element);
    if (_buttons_p != 0)
        _buttons_p->wake();
    Tilda::delay(250); //Give the screen chance to update.
}

void GUITask::setOrientation(Orientation_t orientation) {
  _current_orientation = orientation;
  switch (orientation)
  {
    case ORIENTATION_HELD:
      GLCD.SetRotation(ROTATION_90);
      break;
    case ORIENTATION_HUNG:
      GLCD.SetRotation(ROTATION_270);
      break;
    case ORIENTATION_RIGHT:
      GLCD.SetRotation(ROTATION_180);
      break;
    case ORIENTATION_LEFT:
      GLCD.SetRotation(ROTATION_0);
      break;
  }
}
