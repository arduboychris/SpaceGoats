#include <Arduboy.h>
#include "GoatClasses.h"
#include "ProgramData.h"

void Shot::Add( const unsigned char * sData, int tX, int tY, byte dir ) {
  SpriteData = sData;
  x = tX;
  y = tY;
  Direction = dir;
}
bool Shot::Active( ) {
  if ( Direction == 0xFF ) return false;
  return true;
}
void Shot::Remove( ) {
  SpriteData = NULL;
  CurrentFrame = 0;
  x = 0;
  y = 0; 
  Direction = 0xFF;
}
void Shot::Move( unsigned long Counter ) {
  if ( Direction > 12 && Direction != 0xFF ) {
    Direction++;
    if ( Direction == 0xFF ) {
      Remove( );
      return;
    }
  }
  for ( uint8_t a = 0; a < Speed(); a++ ) {
    bool DirectionOsc = !( ( Counter + a ) % 3 );
    switch ( Direction ) {
      case 0:    // Up
        y--;
        break;
      case 1: 
        if ( DirectionOsc ) x++;
        else y--;
        break;
      case 2:
        if ( DirectionOsc ) y--;
        else x++;
        break;
      case 3:    // Right
        x++;
        break;
      case 4:
        if ( DirectionOsc ) y++;
        else x++;
        break;
      case 5:
        if ( DirectionOsc ) x++;
        else y++;
        break;
      case 6:    // Down
        y++;
        break;
      case 7:
        if ( DirectionOsc ) x--;
        else y++;
        break;
      case 8:
        if ( DirectionOsc ) y++;
        else x--;
        break;
      case 9:    // Left
        x--;
        break;
      case 10:
        if ( DirectionOsc ) y--;
        else x--;
        break;
      case 11:
        if ( DirectionOsc ) x--;
        else y--;
        break;
    }
    if ( IsOffScreen( ) ) Remove( );
  }
}

void Enemy::Think ( SG * Game, unsigned long Counter ) {
  if ( Counter >= NextChoice ) {
    byte Packet = pgm_read_byte ( Pattern );
    if ( Packet < 13 ) { 
      Direction = Packet;
      NextChoice = ( pgm_read_byte( Pattern+1 ) / Speed() ) + Counter;
      Pattern+=2;
    }
    else if ( Packet == 0xFE ) {
      uint8_t pPattern = pgm_read_byte( Pattern+1 ) & 0x0F;
      uint8_t pSprite = ( pgm_read_byte( Pattern+1 ) & 0xF0 ) >> 4;
      Game->AddEnemy( ( const unsigned char * ) pgm_read_word( &Sprites[pSprite] ),
                ( byte * )                pgm_read_word( &Patterns[pPattern] ) );
      Pattern+=2;
    }
    else if ( Packet == 0xFF ) { // Shooting I guess?
      uint8_t pDirection = pgm_read_byte( Pattern+1 ) & 0x0F;
      uint8_t pType = ( pgm_read_byte( Pattern+1 ) & 0xF0 ) >> 4;
      const unsigned char * pData;
      switch ( pType ) {
        case wSingle:     pData = SingleShotSprite; break;
        case wMissle:     pData = MissleShotSprite; break;
        case wExplosive:  pData = ExplosiveShotSprite; break;
      }
      Game->AddEnemyShot ( pData, x, MiddleY(), pDirection );
      Pattern+=2;
    } 
    else {
      Pattern -= ( Packet - 12 ) * 2;
    }
  }
}

Sprite::Sprite ( ) {
  SpriteData = NULL;
  x = 0;
  y = 0;
  CurrentFrame = 0;
}
void Sprite::Draw ( Arduboy * Display ) {
  if ( Masks() ) Display->drawBitmap( x, y, MaskPointer( ),  Width(), Height(), BLACK );
                 Display->drawBitmap( x, y, FramePointer( ), Width(), Height(), WHITE );
  if ( CurrentFrame+1 < MaxFrame() * CyclesPerFrame() ) CurrentFrame++;
  else CurrentFrame = 0;
}
bool Sprite::IsIn ( int tX, int tY ) {
  if ( tX >= x &&
       tX <= RightX() &&
       tY >= y &&
       tY <= BottomY() ) return true;
  return false;
}
bool Sprite::IsOffScreen( ) {
  if ( x > WIDTH || y > HEIGHT || RightX() < 0 || BottomY() < 0 ) return true;
  return false;
}
bool Sprite::GetPixelAbsolute( int tX, int tY ) {
  int RelativeX = tX - x;
  int RelativeY = tY - y;
  if ( RelativeX < 0 || RelativeY < 0 || RelativeX > Width() || RelativeY > Height() ) return false;
  uint8_t row = RelativeY / 8;
  uint8_t offset = RelativeY % 8;
  byte selection = pgm_read_byte( MaskPointer() + ( row * Width() ) + RelativeX );
  return ( selection & ( 1 << offset ) );
}
uint8_t Sprite::Width ( ) {
  return pgm_read_byte( SpriteData + SpriteWidth );
}
uint8_t Sprite::Height ( ) {
  return pgm_read_byte( SpriteData + SpriteHeight );
}
uint8_t Sprite::Masks ( ) {
  return pgm_read_byte( SpriteData + SpriteMasks );
}
uint8_t Sprite::MaxFrame ( ) {
  return pgm_read_byte( SpriteData + SpriteMaxFrame );
}
uint8_t Sprite::CyclesPerFrame ( ) {
  return pgm_read_byte( SpriteData + SpriteCyclesPerFrame );
}
uint8_t Sprite::Speed ( ) {
  return pgm_read_byte( SpriteData + SpriteSpeed );
}
uint8_t Sprite::MaxHealth ( ) {
  return pgm_read_byte( SpriteData + SpriteMaxHealth );
}
const unsigned char * Sprite::FramePointer ( ) {
  int FrameSize = Height() * Width() / 8;
  return SpriteData + SpriteImageData + ( FrameSize * ( CurrentFrame / CyclesPerFrame() ) );
}
const unsigned char * Sprite::MaskPointer ( ) {
  int FrameSize = Height() * Width() / 8;
  return SpriteData + SpriteImageData + ( FrameSize * MaxFrame() );
}
int Sprite::RightX ( ) {
  return x+Width()-1;
}
int Sprite::MiddleX ( ) {
  return x + ( Width() / 2 );
}
int Sprite::BottomY ( ) {
  return y+Height()-1;
}
int Sprite::MiddleY ( ) {
  return y + ( Height() / 2 );
}

SG::SG ( Arduboy * display ) {
  Display = display;
}
void SG::TextIntro( const unsigned char * Text, int TextLength ) {
  LongTextSetup( );
  for ( int a = 0; a < TextLength; a++ ) {
    char Selection = ( char ) pgm_read_byte ( Text + a );
    Display->print( Selection );
    Display->display();
    if ( Selection != 0x0A && Selection != 0x20 ) {
      Display->tunes.tone( 220, 20 );
      delay(30);
    }
  }
  WaitForTap();
}
void SG::LongTextSetup( ) {
  Display->clear();
  Display->setCursor(0,4);
  Display->drawLine(0,1,127,1,1);
  Display->drawLine(0,62,127,62,1);
}
void SG::WaitForTap( ) {
  Display->display();
  ActivateButtonCD();
  while ( true ) {
    if ( ButtonOffCD() && Display->buttonsState() ) return;
  }
}
bool SG::ButtonOffCD( ) {
  if ( millis() > LastShotMS + SHOTCD ) return true;
  return false;
}
void SG::ActivateButtonCD( ) {
  LastShotMS = millis();
}
void SG::DrawTitleScreen ( ) {
  Display->drawBitmap(0,0,StarFieldTop,64,32,WHITE);
  Display->drawBitmap(0,32,StarFieldTop,64,8,WHITE);
  Display->drawBitmap(64,0,StarFieldTop,64,32,WHITE);
  Display->drawBitmap(64,32,StarFieldTop,64,8,WHITE);
  Display->drawBitmap(0,32,Plants,128,32,WHITE);
  Display->fillRect(29,4,71,13,BLACK);
  Display->fillRect(21,18,87,13,BLACK);
  Display->drawBitmap(30,5,TitleSpace,69,16,WHITE);
  Display->drawBitmap(22,19,TitleGoats,85,16,WHITE);
}
void SG::NewGame ( int level ) {
  FrameCounter = 0;
  NextUpdate = 120;
  FormationPointer = NULL;
  for ( int a = 0; a < NumButtons; a++ ) ButtonState[a] = false;
  Player.SpriteData = SpaceGoatSprite;
  Player.x = -23;
  Player.y = 8;
  Player.CurrentFrame = 0;
  Level = level;
  BossFight = 0xFF;
  FormationCounter = 0;
  ShieldCounter = 98;
  LastShotMS = millis();
  ResetSprites();
  LoadFormation();
}
void SG::ResetInventory( ) {
  for ( int a = 0; a < NumWeapons; a++ ) Inventory[a] = 0;
  Inventory[wSingle] = 1;
  Selection = wSingle;
}
void SG::ResetSprites( ) {
  for ( int a = 0; a < PlayerMaxShots; a++ ) Shots[a].Remove();
  for ( int a = 0; a < MaxEnemies; a++ ) {
    Enemies[a].Pattern = NULL;
    Enemies[a].NextChoice = 0;
    Enemies[a].Flags = 0;
    Enemies[a].Remove();
  }
  for ( int a = 0; a < EnemyMaxShots; a++ ) EnemyShots[a].Remove();
}
void SG::Draw ( ) {
  if ( Level % 2 ) DrawEarthBackground( );
  else DrawSpaceBackground( );

  // Draw Sprites/Shots
  for ( uint8_t a = 0; a < PlayerMaxShots; a++ )
    if ( Shots[a].Active() )
      Shots[a].Draw( Display );

  for ( uint8_t a = 0; a < EnemyMaxShots; a++ )
    if ( EnemyShots[a].Active() )
      EnemyShots[a].Draw( Display );

  for ( uint8_t a = 0; a < MaxEnemies; a++ )
    if ( Enemies[a].Active() )
      Enemies[a].Draw ( Display );

  if ( ShieldCounter % 4 < 2 ) Player.Draw( Display );
  // Draw UI
  Display->setCursor(64 - ( Digits(Score)*3 ) , 0);
  Display->print( Score );
  for ( uint8_t a = 0; a < min( Lives, 4 ); a++ ) {
    Display->drawBitmap( a*8, 0, TileMask, 8, 8, BLACK );
    Display->drawBitmap( a*8, 0, GoatIcon, 8, 8, WHITE );
  }
  Display->drawBitmap(96, 0, TileMask, 8, 8, BLACK );
  Display->drawBitmap(96, 0, ( const unsigned char * ) pgm_read_word( &InventoryIcons[ Selection ] ), 8, 8, WHITE );
  for ( uint8_t a = 0; a < 3; a++ ) {
    uint8_t x = 104 + ( 8 * a );
    Display->drawBitmap( x, 0, TileMask, 8, 8, BLACK );
    if ( Inventory[Selection] > a ) Display->drawBitmap ( x, 0, FilledSquare, 8, 8, WHITE );
    else Display->drawBitmap ( x, 0, EmptySquare, 8, 8, WHITE );
  }
}
void SG::DrawEarthBackground( ) {
  for ( uint8_t x = 0; x < WIDTH; x+=8 ) {
    for ( uint8_t y = 8; y < 32; y+=8 ) {
      Display->drawBitmap( x, y, ShadeTile, 8, 8, WHITE );
    }
  }
  Display->drawBitmap( 0 -     ((FrameCounter/10 ) % WIDTH ), 32, Plants, 128, 32, WHITE );
  Display->drawBitmap( WIDTH - ((FrameCounter/10 ) % WIDTH ), 32, Plants, 128, 32, WHITE );

  Display->drawBitmap( WIDTH - ((FrameCounter/30) % 192), 10, CloudMask, 20, 8, WHITE );
  Display->drawBitmap( 150   - ((FrameCounter/38) % 180), 12, CloudMask, 20, 8, WHITE );
  Display->drawBitmap( 256   - ((FrameCounter/26) % 384), 16, CloudMask, 20, 8, WHITE );
  Display->drawBitmap( 180   - ((FrameCounter/24) % 210), 20, CloudMask, 20, 8, WHITE );
  for ( int x = 0 - ((FrameCounter/22)%20); x < WIDTH; x+=20 ) {
    Display->drawBitmap( x, 25, CloudMask, 20, 8, WHITE );
    Display->drawBitmap( x, 25, Mountain, 20, 8, BLACK );
  }
  Display->drawFastHLine( 0, 33, WIDTH, BLACK );
}
void SG::DrawSpaceBackground( ) {
    // Starfield
  Display->drawBitmap( 0 - ( ( FrameCounter / 90 ) % WIDTH ), 0, StarFieldTop, 64, 32, WHITE );
  Display->drawBitmap( WIDTH - ( ( FrameCounter / 90 ) % WIDTH ), 0, StarFieldTop, 64, 32, WHITE );
  Display->drawBitmap( 0 - ( ( FrameCounter / 80 ) % WIDTH ), 32, StarFieldTop, 64, 32, WHITE );
  Display->drawBitmap( WIDTH - ( ( FrameCounter / 80 ) % WIDTH ), 32, StarFieldTop, 128, 32, WHITE );

  Display->drawBitmap( 64 + 0 - ( ( FrameCounter / 90 ) % WIDTH ), 0, StarFieldTop, 64, 32, WHITE );
  Display->drawBitmap( 64 + WIDTH - ( ( FrameCounter / 90 ) % WIDTH ), 0, StarFieldTop, 64, 32, WHITE );
  Display->drawBitmap( 64 + 0 - ( ( FrameCounter / 80 ) % WIDTH ), 32, StarFieldTop, 64, 32, WHITE );
  Display->drawBitmap( 64 + WIDTH - ( ( FrameCounter / 80 ) % WIDTH ), 32, StarFieldTop, 64, 32, WHITE );
  // Far objects
  Display->drawBitmap( WIDTH - ((FrameCounter/30) % 192), 10, FarStarMask, 16, 16, BLACK );
  Display->drawBitmap( WIDTH - ((FrameCounter/30) % 192), 10, FarStar, 16, 16, WHITE );
  Display->drawBitmap( 256 - ((FrameCounter/28) % 384), 14, FarStarMask, 16, 16, BLACK );
  Display->drawBitmap( 256 - ((FrameCounter/28) % 384), 14, FarStar, 16, 16, WHITE );
  // Near objects
  Display->drawBitmap( 196 - ((FrameCounter/20) % 256), 40, NearMoonMask, 32, 32, BLACK );
  Display->drawBitmap( 196 - ((FrameCounter/20) % 256), 40, NearMoon, 32, 32, WHITE );

}
void SG::FadeIn( uint8_t pMax ) {
	for (uint8_t pCount = 0; pCount < pMax; pCount++) {
		uint8_t patternX = patternCoords[pCount*2];
		uint8_t patternY = patternCoords[(pCount*2)+1];
		for (uint8_t x = patternX; x < WIDTH; x+=4) {
			for (uint8_t y = patternY; y < HEIGHT; y+=4) {
				Display->drawPixel(x,y,WHITE);
			}
		}
	}
}
void SG::PatternWipe( ) {
  for (uint8_t pCount = 0; pCount < 17; pCount++) {
//  uint8_t patternX = patternCoords[pCount*2];
//  uint8_t patternY = patternCoords[(pCount*2)+1];
//  for (uint8_t x = patternX; x < WIDTH; x+=4) {
//    for (uint8_t y = patternY; y < HEIGHT; y+=4) {
//      Display->drawPixel(x,y,WHITE);
//    }
//  }
    FadeIn ( pCount );
    Display->display();
    delay(62);
  }
}
void SG::Cycle ( ) {
  if ( ShieldCounter < 74 && Lives ) {
    if ( ButtonState[ButtonLeft] )  Move( ButtonLeft );
    if ( ButtonState[ButtonRight] ) Move( ButtonRight );
    if ( ButtonState[ButtonUp] )    Move( ButtonUp );
    if ( ButtonState[ButtonDown] )  Move( ButtonDown );
    if ( ButtonState[ButtonFire] )  Fire( );
    if ( ButtonState[ButtonSwap] )  Swap( );
  }
  if ( ShieldCounter == 99 ) {
    Lives--;
    ResetInventory();
    Player.SpriteData = SpaceGoatSprite;
    Player.x = -24;
    Player.y = 8;
    Player.CurrentFrame = 0;
  }
  if ( ShieldCounter < 99 && ShieldCounter > 74 && Lives ) Player.x+=2;

  for ( uint8_t a = 0; a < PlayerMaxShots; a++ )
    if ( Shots[a].Active() )
      Shots[a].Move( FrameCounter );

  for ( uint8_t a = 0; a < EnemyMaxShots; a++ )
    if ( EnemyShots[a].Active() )
      EnemyShots[a].Move( FrameCounter );

  for ( uint8_t a = 0; a < MaxEnemies; a++ )
    if ( Enemies[a].Active() ) {
      Enemies[a].Think( this, FrameCounter );
      Enemies[a].Move( FrameCounter );
    }
    
  if ( Lives ) {
    if ( BossFight == 0xFF ) UpdateFormation( FrameCounter );
    TestCollisions( );
  }
  
  if ( ShieldCounter ) ShieldCounter--;
  FrameCounter++;
  
  if ( BossFight != 0xFF && !Enemies[BossFight].Active() ) {
    // Level up stuff
    BossFight = 0xFF;
    LevelUp( );
  }
}
void SG::LevelUp( ) {
  unsigned long pTime = 0, cTime;
  int Counter = 0;
  while ( Counter < 60 ) {
    cTime = millis();
    if ( cTime > pTime + FPSMSDelay ) {
      pTime = cTime;
      Cycle( );
      Display->clear();
      Draw( );
      Display->display();
      Counter++;
    }
  }
  for ( int a = 0; a < 17; a++ ) {
    Display->clear( );
    Cycle( );
    Draw( );
    FadeIn( a );
    Display->display( );
    delay( 35 );
  }
  
  if ( Level < MaxLevel ) {
    NewGame( Level+1 );
    return;
  }
  // Endgame condition
  NewGame( 1 );  // Loop for now
  
  for ( int a = 16; a >= 0; a-- ) {
    Display->clear( );
    Draw( );
    FadeIn( a );
    Display->display( );
    delay( 35 );
  }
}
void SG::Move ( uint8_t Direction ) {
  for ( uint8_t a = 0; a < Player.Speed(); a++ ) {
    switch ( Direction ) {
      case ButtonLeft:  if ( Player.x > 0 ) Player.x--; break;
      case ButtonUp:    if ( Player.y > 8 ) Player.y--; break;
      case ButtonRight: if ( Player.RightX() < WIDTH-1 ) Player.x++; break;
      case ButtonDown:  if ( Player.BottomY() < HEIGHT-1 ) Player.y++; break;
    }
  }
}
void SG::Fire ( ) {
  unsigned long CurrentTime = millis();
  if ( CurrentTime < LastShotMS + SHOTCD ) return;
  LastShotMS = CurrentTime;
  switch ( Selection ) {
    case wSingle: 
      AddShot ( SingleShotSprite, Player.RightX(), Player.MiddleY(), 3 );
      LastShotMS -= 50 * ( Inventory[wSingle] - 1 );
      break;
    case wSpread:
      LastShotMS += SHOTCD; // Double CD
      AddShot ( SingleShotSprite, Player.RightX(), Player.MiddleY(), 2 );
      AddShot ( SingleShotSprite, Player.RightX(), Player.MiddleY(), 4 );
      if ( Inventory[wSpread] > 1 ) {
        AddShot ( SingleShotSprite, Player.RightX(), Player.MiddleY(), 3 );
      }
      if ( Inventory[wSpread] > 2 ) {
        AddShot ( SingleShotSprite, Player.RightX(), Player.MiddleY(), 1 );
        AddShot ( SingleShotSprite, Player.RightX(), Player.MiddleY(), 5 );
      }
      break;
    case wMissle:
      AddShot ( MissleShotSprite, Player.RightX(), Player.y + 4, 3 );
      LastShotMS += SHOTCD;
      break;
    case wExplosive:
      AddShot ( ExplosiveShotSprite, Player.RightX(), Player.y + 4, 3 );
      LastShotMS += SHOTCD*2;
      break;
  }
}
void SG::Swap ( ) {
  unsigned long CurrentTime = millis();
  if ( CurrentTime < LastShotMS + SHOTCD ) return;
  LastShotMS = CurrentTime;  
  for ( uint8_t a = 0; a < NumWeapons; a++ ) {
    uint8_t NextSlot = ( Selection + a + 1 ) % NumWeapons;
    if ( Inventory[NextSlot] ) {
      Selection = NextSlot;
      return;
    }
  }
}

uint8_t SG::AddEnemy( const unsigned char * sData, const byte * pData ) {
  for ( uint8_t a = 0; a < MaxEnemies; a++ )
    if ( !Enemies[a].Active() ) {
      Enemies[a].Add( sData, pgm_read_byte(pData), pgm_read_byte(pData+1), pgm_read_byte(pData+2) );
      Enemies[a].NextChoice = FrameCounter + ( pgm_read_byte(pData+3) / Enemies[a].Speed() );
      Enemies[a].Pattern = pData + 4;
      Enemies[a].Flags = 0;
      Enemies[a].Health = Enemies[a].MaxHealth();
      return a;
    }
}
void SG::AddShot( const unsigned char * sData, int tX, int tY, byte dir ) {
  for ( uint8_t a = 0; a < PlayerMaxShots; a++ ) {
    if ( !Shots[a].Active() ) {
      Shots[a].Add ( sData, tX, tY, dir );
      return;
    }
  }
}
void SG::AddEnemyShot( const unsigned char * sData, int tX, int tY, byte dir ) {
  for ( uint8_t a = 0; a < EnemyMaxShots; a++ ) {
    if ( !EnemyShots[a].Active() ) {
      EnemyShots[a].Add ( sData, tX, tY, dir );
      return;
    }
  }

}
void SG::LoadFormation( ) {
  switch ( Level ) {
    case 1: 
      if ( FormationCounter < 1 ) FormationPointer = ( byte * ) pgm_read_word( &Level1[FormationCounter] );
      else if ( FormationCounter == 1 ) {
        if ( BossFight == 0xFF ) BossFight = AddEnemy( BeeBossSprite, BeeBossPattern );
        FormationCounter++;
      }
      break;
    case 2: 
      if ( FormationCounter < 1 ) FormationPointer = ( byte * ) pgm_read_word( &Level2[FormationCounter] );
      else if ( FormationCounter == 1 ) {
        if ( BossFight == 0xFF ) BossFight = AddEnemy( DogBossSprite, BeeBossPattern );
        FormationCounter++;
      }
      break;
  }
}
void SG::UpdateFormation( unsigned long Counter ) {
  if ( Counter >= NextUpdate ) {
    byte Packet = pgm_read_byte(FormationPointer);
    byte Flag = 0;
    if ( Packet == 0xFF ) {
      byte Command = pgm_read_byte(FormationPointer+1);
      if ( Command == 0xFF ) {
        FormationCounter++;
        LoadFormation();
        return;
      }
      Flag = Command;
      FormationPointer+=2;
    }
    Packet = pgm_read_byte(FormationPointer);
    FormationPointer++;
    uint8_t pPattern = Packet & 0x0F;
    uint8_t pSprite = ( Packet & 0xF0 ) >> 4;
    NextUpdate = Counter + pgm_read_byte(FormationPointer++);
    uint8_t index = AddEnemy( ( const unsigned char * ) pgm_read_word( &Sprites[pSprite] ),
                              ( byte * )                pgm_read_word( &Patterns[pPattern] ) );
    Enemies[index].Flags = Flag;
  }
}
void SG::TestCollisions( ) {
  if ( !ShieldCounter ) {
    for ( uint8_t a = 0; a < EnemyMaxShots; a++ ) {
      if ( !EnemyShots[a].Active() ) continue;
      if ( TestCollision( &Player, &EnemyShots[a] ) ) {
        Player.SpriteData = SpaceGoatDeathSprite;
        Player.CurrentFrame = 0;
        ShieldCounter = 137;
        break;
      }
    }
  }
  for ( uint8_t b = 0; b < MaxEnemies; b++ ) {
    if ( !Enemies[b].Active() ) continue;
    for ( uint8_t a = 0; a < PlayerMaxShots; a++ ) {
      if ( !Shots[a].Active() ) continue;
      if ( TestCollision( &Shots[a], &Enemies[b] ) ) {
        Enemies[b].Health -= Shots[a].MaxHealth();
        if ( Enemies[b].Health < 1 ) {
          if ( Enemies[b].Flags ) {
            const unsigned char * sData;
            switch ( Enemies[b].Flags ) {
              case 0b1:     sData = SingleFrameSprite; break;
              case 0b10:    sData = SpreadFrameSprite; break;
              case 0b100:   sData = MissleFrameSprite; break;
              case 0b1000:  sData = ExplosiveFrameSprite; break;
            }
            Enemies[b].SpriteData = sData;
            Enemies[b].Direction = 9;
            Enemies[b].NextChoice = FrameCounter + 128;
          }
          else {
            Score += 10 * Enemies[b].MaxHealth();
            Enemies[b].Flags = 0;
            Enemies[b].Remove( );
          }
        }
        if ( !Enemies[b].Flags ) { 
          if ( Shots[a].SpriteData == MissleShotSprite ) {
            Shots[a].SpriteData = ExplosionSprite;
            Shots[a].CurrentFrame = 0;
            Shots[a].x -= 4;
            Shots[a].y -= 4;
            Shots[a].Direction = 0xFF - 60;
          }
          if ( Shots[a].SpriteData == ExplosiveShotSprite ) {
            for ( byte dir = 0; dir < 12; dir++ ) {
              AddShot( SingleShotSprite, Shots[a].MiddleX(), Shots[a].MiddleY(), dir );
            }
            Shots[a].Remove( );
          }
          else Shots[a].Remove( );
        }
        break;
      }
    }
    if ( !Enemies[b].Active() ) continue;
    if ( !ShieldCounter ) {
      if ( TestCollision( &Player, &Enemies[b] ) ) {
        if ( Enemies[b].SpriteData == SingleFrameSprite ) {
          if ( Inventory[wSingle] < 3 ) Inventory[wSingle]++;
          Enemies[b].Remove( );
          Score += 20;
        }
        else if ( Enemies[b].SpriteData == SpreadFrameSprite ) {
          if ( Inventory[wSpread] < 3 ) Inventory[wSpread]++;
          Enemies[b].Remove( );
          Score += 20;
        }
        else if ( Enemies[b].SpriteData == MissleFrameSprite ) {
          if ( Inventory[wMissle] < 3 ) Inventory[wMissle]++;
          Enemies[b].Remove( );
          Score += 20;
        }
        else if ( Enemies[b].SpriteData == ExplosiveFrameSprite ) {
          if ( Inventory[wExplosive] < 3 ) Inventory[wExplosive]++;
          Enemies[b].Remove( );
          Score += 20;
        }
        else {
          Player.SpriteData = SpaceGoatDeathSprite;
          Player.CurrentFrame = 0;
          ShieldCounter = 137;
        }
      }
    }
  }
}
bool SG::TestCollision( Sprite * TestSprite1, Sprite * TestSprite2 ) {
  if ( TestRoughCollision( TestSprite1, TestSprite2 ) || TestRoughCollision( TestSprite2, TestSprite1 ) )
    if ( TestPixelCollision( TestSprite1, TestSprite2 ) ) return true;
  return false;
}
bool SG::TestRoughCollision( Sprite * TestSprite1, Sprite * TestSprite2 ) {
  if ( TestSprite1->IsIn ( TestSprite2->x,       TestSprite2->y ) ||
       TestSprite1->IsIn ( TestSprite2->RightX(),TestSprite2->y ) ||
       TestSprite1->IsIn ( TestSprite2->x,       TestSprite2->BottomY() ) ||
       TestSprite1->IsIn ( TestSprite2->RightX(),TestSprite2->BottomY() ) ) {
    return true;
  }
  return false;
}
bool SG::TestPixelCollision( Sprite * TestSprite1, Sprite * TestSprite2 ) {
  for ( int a = max( TestSprite1->x, TestSprite2->x ); a <= min( TestSprite1->RightX(), TestSprite2->RightX() ); a++ ) {
    for ( int b = max( TestSprite1->y, TestSprite2->y ); b <= min( TestSprite1->BottomY(), TestSprite2->BottomY() ); b++ ) {
      if ( TestSprite1->GetPixelAbsolute(a,b) ) {
        if ( TestSprite2->GetPixelAbsolute(a,b) ) return true;
      }
    }
  }
  return false;
}
void SG::ButtonPress( uint8_t pButton ) {
  if ( ButtonState[ pButton ] ) return;
  ButtonState[ pButton ] = true;
}
void SG::ButtonRelease( uint8_t pButton ) {
  if ( !ButtonState[ pButton ] ) return;
  ButtonState[ pButton ] = false;
}
int SG::Digits( long Number ) {
  int Count = 1;
  long Divided = Number;
  while (true) {
    Divided /= 10;
    if ( !Divided ) return Count;
    Count++;
  }
}

