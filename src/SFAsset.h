#ifndef SFASSET_H
#define SFASSET_H

#include <string>
#include <memory>
#include <iostream>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

using namespace std;

#include "SFCommon.h"
#include "SFEvent.h"
#include "SFBoundingBox.h"

/**
 * We could create SFPlayer, SFProjectile and SFAsset which are subclasses
 * of SFAsset.  However, I've made an implementation decision to use this
 * enum to mark the type of the SFAsset.  If we add more asset types then
 * the subclassing strategy becomes a better option.
 */
enum SFASSETTYPE {SFASSET_DEAD, SFASSET_PLAYER, SFASSET_TAIL, SFASSET_DEADPLAYER, SFASSET_COIN, 					SFASSET_WALL};

class SFAsset {
public:
  SFAsset(const SFASSETTYPE);
  SFAsset(const SFAsset&);
  virtual ~SFAsset();

  virtual void      SetPosition(Point2 &);
  virtual Point2    GetPosition();
  virtual SFAssetId GetId();
  virtual void      OnRender(SDL_Surface *);
  virtual void      GoEast();
  virtual void      GoWest();
  virtual void      GoNorth();
  virtual void    	GoSouth();
  
  virtual void      FaceEast();
  virtual void      FaceWest();
  virtual void      FaceNorth();
  virtual void    	FaceSouth();
  
  virtual bool      FacingEast();
  virtual bool      FacingWest();
  virtual bool      FacingNorth();
  virtual bool    	FacingSouth(); 
  virtual void 			CanChangeDirectionAgain();
  
  virtual void      SetNotAlive();
  virtual bool      IsAlive();
  virtual void      HandleCollision();

  virtual bool                      CollidesWith(shared_ptr<SFAsset>);
  virtual shared_ptr<SFBoundingBox> GetBoundingBox();
private:
  // it would be nice if we could make this into a smart pointer,
  // but, because we need to call SDL_FreeSurface on it, we can't.
  // (or we could use a std::shared_ptr with a custom Deleter, but
  // that's a little too much right now)
  SDL_Surface               * sprite;
  shared_ptr<SFBoundingBox>   bbox;
  SFASSETTYPE                 type;
  SFAssetId                   id;
  
  bool	facingNorth;
  bool	facingSouth;
  bool	facingEast;
  bool	facingWest;  
  bool 	canChangeDirection = true;
	
  static int SFASSETID;
};

#endif
