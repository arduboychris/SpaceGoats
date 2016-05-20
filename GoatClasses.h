#ifndef GOATCLASSES_h
#define GOATCLASSES_h

#include <Arduboy.h>

#define FPSMSDelay 1000/60
#define SHOTCD 250
#define PlayerMaxShots 20
#define EnemyMaxShots  10
#define MaxEnemies     8
#define MaxLevel       2

enum Buttons {
  ButtonLeft = 0,
  ButtonUp,
  ButtonRight,
  ButtonDown,
  ButtonSwap,
  ButtonFire,
  NumButtons
};
enum SpriteDataLayout {
  SpriteWidth = 0,
  SpriteHeight,
  SpriteMaxFrame,
  SpriteCyclesPerFrame,
  SpriteSpeed,
  SpriteMaxHealth,
  SpriteMasks,
  SpriteImageData
};
enum Weapons {
  wSingle,
  wSpread,
  wMissle,
  wExplosive,
  NumWeapons
};
class Shot;
class Sprite;
class SG;

class Sprite {
  public:
    Sprite ( );
    void      Draw( Arduboy * Display );
    
    bool      IsIn( int tX, int tY );
    bool      IsOffScreen( );
    virtual bool GetPixelAbsolute( int tX, int tY );
    uint8_t   Width( );
    uint8_t   Height( );
    uint8_t   Masks( );
    uint8_t   MaxFrame( );
    uint8_t   CyclesPerFrame( );
    uint8_t   Speed( );
    uint8_t   MaxHealth( );
    const unsigned char * FramePointer( );
    const unsigned char * MaskPointer( );
    int       RightX( );
    int       MiddleX( );
    int       BottomY( );
    int       MiddleY( );

    int       x, y;
    uint8_t   CurrentFrame;
    const unsigned char * SpriteData;
};
class Shot : public Sprite {
  public:
    Shot ( ) : Sprite ( ) { Direction = 0xFF; } ;
    void      Add( const unsigned char * sData, int tX, int tY, byte dir );
    bool      Active( );
    void      Remove( );
    void      Move( unsigned long Counter );
    byte      Direction;
};
class Enemy : public Shot {
  public:
    void      Think( SG * Game, unsigned long Counter );
    int       Health;
    byte      Flags;
    const byte * Pattern;
    unsigned long NextChoice;
};
class SG {
  public:
    SG ( Arduboy * display );
    void      TextIntro( const unsigned char * Text, int TextLength );
    void      LongTextSetup( );
    void      WaitForTap( );
    bool      ButtonOffCD( );
    void      ActivateButtonCD( );
    void      DrawTitleScreen( );
    void      NewGame( int level );
    void      ResetInventory( );
    void      ResetSprites( );
    
    void      Draw( );
    void      DrawEarthBackground( );
    void      DrawSpaceBackground( );
    void      FadeIn( uint8_t pMax );
    void      PatternWipe( );
    void      Cycle( );
    void      LevelUp( );
    
    void      Move( uint8_t Direction );
    void      Fire( );
    void      Swap( );
    
    uint8_t   AddEnemy( const unsigned char * sData, const byte * pData );
    void      AddEnemyShot( const unsigned char * sData, int tX, int tY, byte dir );
    void      AddShot( const unsigned char * sData, int tX, int tY, byte dir );
    void      LoadFormation( );
    void      UpdateFormation( unsigned long Counter );
    void      TestCollisions( );
    bool      TestCollision( Sprite * TestSprite1, Sprite * TestSprite2 );
    bool      TestRoughCollision( Sprite * TestSprite1, Sprite * TestSprite2 );
    bool      TestPixelCollision( Sprite * TestSprite1, Sprite * TestSprite2 );
    void      ButtonPress( uint8_t pButton );
    void      ButtonRelease( uint8_t pButton );
    int       Digits( long Number );

    unsigned long FrameCounter;
    unsigned long NextUpdate;
    uint8_t   FormationCounter;
    const byte * FormationPointer;
    uint8_t   Level;
    uint8_t   BossFight;
    
    Arduboy * Display;
    bool      ButtonState[ NumButtons ];    

    Sprite    Player;
    Shot      Shots[PlayerMaxShots];
    int       Score;
    int       Lives;
    int       ShieldCounter;
    uint8_t   Inventory[ NumWeapons ];
    uint8_t   Selection;
    unsigned long LastShotMS;

    Enemy     Enemies[ MaxEnemies ];
    Shot      EnemyShots[ EnemyMaxShots ];
};

#endif

