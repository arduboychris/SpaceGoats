#include <SPI.h>
#include <EEPROM.h>
#include <Arduboy.h>
#include "GoatClasses.h"
#include "ProgramData.h"

Arduboy display;
SG      Game( &display );
unsigned long currTime;
unsigned long prevTime = 0;

void setup() {
  Serial.begin(9600);
  display.begin( );
  Game.TextIntro( IntroText1, 56 );
  Game.TextIntro( IntroText2, 76 );
}
void loop( ) {
  display.clear( );
  Game.DrawTitleScreen( );
  Game.WaitForTap( );
  Game.PatternWipe( );
  Game.Score = 0;
  Game.Lives = 4;
  Game.ResetInventory();
  Game.NewGame( 1 );
  for ( int a = 16; a >= 0; a-- ) {
    display.clear( );
    Game.Draw( );
    Game.FadeIn( a );
    display.display( );
    delay( 30 );
  }
  while ( Game.Lives ) {
    byte input = display.buttonsState( );
    if ( input & B_BUTTON )       { Game.ButtonPress( ButtonSwap ); }   else { Game.ButtonRelease( ButtonSwap ); }
    if ( input & A_BUTTON )       { Game.ButtonPress( ButtonFire ); }   else { Game.ButtonRelease( ButtonFire ); }
    if ( input & UP_BUTTON )      { Game.ButtonPress( ButtonUp ); }     else { Game.ButtonRelease( ButtonUp ); }
    if ( input & DOWN_BUTTON )    { Game.ButtonPress( ButtonDown ); }   else { Game.ButtonRelease( ButtonDown ); }
    if ( input & LEFT_BUTTON )    { Game.ButtonPress( ButtonLeft ); }   else { Game.ButtonRelease( ButtonLeft ); }
    if ( input & RIGHT_BUTTON )   { Game.ButtonPress( ButtonRight ); }  else { Game.ButtonRelease( ButtonRight ); }
    currTime = millis();
    if ( currTime > prevTime + FPSMSDelay ) {
      prevTime = currTime;
      display.clear( );
      Game.Cycle( );
      Game.Draw( );
      display.display( );
    }
  }
  while ( Game.ShieldCounter ) {
    display.clear( );
    Game.Cycle( );
    Game.Draw( );
    display.display( );
  }
  Game.PatternWipe( );
  display.drawBitmap( 30, 12, gameover, 68, 16, BLACK );
  Game.WaitForTap( );
  Game.PatternWipe( );

  if (Game.Score) enterHighScore(1);
  while ( !displayHighScores(1) ) {
    delay(250);
  }

  for ( int a = 16; a >= 0; a-- ) {
    display.clear();
    Game.DrawTitleScreen( );
    Game.FadeIn( a );
    display.display( );
    delay(30);
  }
}
//Function by nootropic design to add high scores
void enterHighScore(byte file) {
  // Each block of EEPROM has 10 high scores, and each high score entry
  // is 5 bytes long:  3 bytes for initials and two bytes for score.
  int address = file * 10 * 5;
  byte hi, lo;
  char initials[3];
  char text[16];
  char tmpInitials[3];
  unsigned int tmpScore = 0;

  // High score processing
  for (byte i = 0; i < 10; i++) {
    hi = EEPROM.read(address + (5*i));
    lo = EEPROM.read(address + (5*i) + 1);
    if ((hi == 0xFF) && (lo == 0xFF))
    {
      // The values are uninitialized, so treat this entry
      // as a score of 0.
      tmpScore = 0;
    } else
    {
      tmpScore = (hi << 8) | lo;
    }
    if (Game.Score > tmpScore) {
      char index = 0;
      initials[0] = ' ';
      initials[1] = ' ';
      initials[2] = ' ';
      while (true) {  //    enterInitials();
        display.clear();
        display.setCursor(16,0); display.print(F("HIGH SCORE"));
        sprintf(text, "%u", Game.Score);
        display.setCursor(88, 0); display.print(text);
        display.setCursor(56, 20); display.print(initials[0]);
        display.setCursor(64, 20); display.print(initials[1]);
        display.setCursor(72, 20); display.print(initials[2]);
        for (byte i = 0; i < 3; i++) display.drawLine(56 + (i*8), 27, 56 + (i*8) + 6, 27, 1);
        display.drawLine(56, 28, 88, 28, 0);
        display.drawLine(56 + (index*8), 28, 56 + (index*8) + 6, 28, 1);
        display.display(); 
        delay(150);
        if ( display.pressed(LEFT_BUTTON) || display.pressed(B_BUTTON)) {
          index--;
          if (index < 0) index = 0;
          else display.tunes.tone(1046, 250);
        }
        if (display.pressed(RIGHT_BUTTON)) {
          index++;
          if (index > 2) index = 2;
          else display.tunes.tone(1046, 250);
        }
        if (display.pressed(DOWN_BUTTON)) {
          initials[index]++;
          display.tunes.tone(523, 250);
          // A-Z 0-9 :-? !-/ ' '
          if (initials[index] == '0') initials[index] = ' ';
          if (initials[index] == '!') initials[index] = 'A';
          if (initials[index] == '[') initials[index] = '0';
          if (initials[index] == '@') initials[index] = '!';
        }
        if (display.pressed(UP_BUTTON)) {
          initials[index]--;
          display.tunes.tone(523, 250);
          if (initials[index] == ' ') initials[index] = '?';
          if (initials[index] == '/') initials[index] = 'Z';
          if (initials[index] == 31) initials[index] = '/';
          if (initials[index] == '@') initials[index] = ' ';
        }
        if (display.pressed(A_BUTTON)) {
          if (index < 2) {
            index++;
            display.tunes.tone(1046, 250);
          } else {
            display.tunes.tone(1046, 250);
            break;
          }
        }
      }
      
      for(byte j=i;j<10;j++) {
        hi = EEPROM.read(address + (5*j));
        lo = EEPROM.read(address + (5*j) + 1);
        if ((hi == 0xFF) && (lo == 0xFF)) tmpScore = 0;
        else tmpScore = (hi << 8) | lo;
        tmpInitials[0] = (char)EEPROM.read(address + (5*j) + 2);
        tmpInitials[1] = (char)EEPROM.read(address + (5*j) + 3);
        tmpInitials[2] = (char)EEPROM.read(address + (5*j) + 4);

        // write score and initials to current slot
        EEPROM.write(address + (5*j), ((Game.Score >> 8) & 0xFF));
        EEPROM.write(address + (5*j) + 1, (Game.Score & 0xFF));
        EEPROM.write(address + (5*j) + 2, initials[0]);
        EEPROM.write(address + (5*j) + 3, initials[1]);
        EEPROM.write(address + (5*j) + 4, initials[2]);

        // tmpScore and tmpInitials now hold what we want to
        //write in the next slot.
        Game.Score = tmpScore;
        initials[0] = tmpInitials[0];
        initials[1] = tmpInitials[1];
        initials[2] = tmpInitials[2];
      }
      return;
    }
  }
}
boolean displayHighScores(byte file) {
  char text[16];
  char initials[3];
  unsigned int Score;
  byte y = 10;
  byte x = 24;
  // Each block of EEPROM has 10 high scores, and each high score entry
  // is 5 bytes long:  3 bytes for initials and two bytes for score.
  int address = file*10*5;
  byte hi, lo;
  display.clear();
  display.setCursor(32, 0);
  display.print(F("HIGH SCORES"));
//display.display();

  for(int i = 0; i < 10; i++)
  {
    sprintf(text, "%2d", i+1);
    display.setCursor(x,y+(i*8));
    display.print(text);
//  display.display();
    hi = EEPROM.read(address + (5*i));
    lo = EEPROM.read(address + (5*i) + 1);

    if ((hi == 0xFF) && (lo == 0xFF)) Score = 0;
    else Score = (hi << 8) | lo;

    initials[0] = (char)EEPROM.read(address + (5*i) + 2);
    initials[1] = (char)EEPROM.read(address + (5*i) + 3);
    initials[2] = (char)EEPROM.read(address + (5*i) + 4);

    if (Score > 0)
    {
      sprintf(text, "%c%c%c %u", initials[0], initials[1], initials[2], Score);
      display.setCursor(x + 24, y + (i*8));
      display.print(text);
//    display.display();
    }
  }
  display.display();
  if ( display.buttonsState() ) return true;
  return false;
}
