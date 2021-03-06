#include "SFApp.h"

SFApp::SFApp() : points(0), is_running(true) {
	
  surface = SDL_GetVideoSurface();
  app_box = make_shared<SFBoundingBox>(Vector2(surface->w/2, surface->h/2), surface->w/2, surface->h/2);
  player  = make_shared<SFAsset>(SFASSET_PLAYER);
  // Player is 30x30, Point2 -'s are to get them aligned to grid properly
  auto player_pos = Point2((surface->w/2)-16, (surface->h/2)-18);
  player->SetPosition(player_pos);

  for(int i=1; i<=points+2; i++) {
  	auto playerTail = make_shared<SFAsset>(SFASSET_TAIL);
  	auto pos 				= Point2((surface->w/2)-16, ((surface->h/2)-18)-(32*i));
  	playerTail->SetPosition(pos);
  	playerTails.push_back(playerTail);
  }
  
  srand (time(NULL)); // initialize random seed

	NewCoin();	
	paused = true;
}

SFApp::~SFApp() {
}

/**
 * Handle all events that come from SDL.
 * These are timer or keyboard events.
 */
void SFApp::OnEvent(SFEvent& event) {
  SFEVENT the_event = event.GetCode();
  switch (the_event) {
  case SFEVENT_QUIT:
    is_running = false;
    restart = false;
    break;
  case SFEVENT_RESTART:
  	//is_running = false;
  	restart = true;
  	break;
  case SFEVENT_UPDATE:
  	if (!paused) {
		  OnUpdateWorld();
		}
		OnRender();
    break;
  case SFEVENT_PLAYER_UP:
  	paused = false;
   	player->FaceNorth();
    break;
  case SFEVENT_PLAYER_DOWN:
  	paused = false;
    player->FaceSouth();
    break;
  case SFEVENT_PLAYER_LEFT:
  	paused = false;
    player->FaceWest();
    break;
  case SFEVENT_PLAYER_RIGHT:
  	paused = false;
    player->FaceEast();
    break;
  }
}

int SFApp::OnExecute() {
  // Execute the app
  SDL_Event event;
  while (SDL_WaitEvent(&event) && is_running) {
    // if this is an update event, then handle it in SFApp,
    // otherwise punt it to the SFEventDispacher.
    SFEvent sfevent((const SDL_Event) event);
    OnEvent(sfevent);
  }
}

void SFApp::OnUpdateWorld() {

	// Tick
	gameTick++;
	if (gameTick > gameSpeed) {
		gameTick = 0;
	}

	// Make player move every tick
	if (gameTick == 0) {
		UpdateTail();
		player->CanChangeDirectionAgain();
		if (player->FacingNorth()) {
			player->GoNorth();
		}
		if (player->FacingSouth()) {
			player->GoSouth();
		}
		if (player->FacingEast()) {
			player->GoEast();
		}
		if (player->FacingWest()) {
			player->GoWest();
		}
	}
 
  // Detect coin collisions
  for(auto c : coins) {
  	// Detect if coin is placed (or somehow ends up in) a tail section or wall
    for(auto t : playerTails) {
  		if(c->CollidesWith(t)) {
  			c->HandleCollision();
  			NewCoin();
  		}
  	}
  	for(auto w : walls) {
  		if(c->CollidesWith(w)) {
    		c->HandleCollision();
  			NewCoin();
  		}
  	}
  	// Detect if the player noms a coin
  	if(player->CollidesWith(c)) {
			points++;
			cout << "Number of points : " << (points) << ". Game speed: " << gameSpeed << endl;
			if (gameSpeed == 1) {
				cout << "CONGRADULATIONS! You win!" << endl;
				// Replace following line with some message or fireworks or crap showing you won
				cout << "The test is now over, YOU DON'T NEED ANY MORE POINTS." << endl;
				std::stringstream sstm;
	  		sstm << "You win! Points: " << points;
	  		SDL_WM_SetCaption(sstm.str().c_str(),  sstm.str().c_str());
			} else {
				if (gameSpeed > 1) {
					gameSpeed--;
					// The more points you have, the faster the game will get
					std::stringstream sstm;
		  		sstm << "Points: " << points;
		  		SDL_WM_SetCaption(sstm.str().c_str(),  sstm.str().c_str());
				}
			}			
  		c->HandleCollision(); // Remove old coin
  		NewCoin(); // Spawn a new coin
  	}
  }

	// Detect if player collides with wall
	for(auto w : walls) {
		if(player->CollidesWith(w)){
			cout << "You crashed you dead :(" << endl;
			paused = true;
			player->SetNotAlive();
			deadPlayer  = make_shared<SFAsset>(SFASSET_DEADPLAYER);
			auto deadPlayer_pos = player->GetPosition();
			deadPlayer->SetPosition(deadPlayer_pos);
		}
	}

	// Detect if player collides with tail section
	for(auto t : playerTails) {
		if(player->CollidesWith(t)){
			cout << "You crashed you dead :(" << endl;
			paused = true;
			player->SetNotAlive();
			deadPlayer  = make_shared<SFAsset>(SFASSET_DEADPLAYER);
			auto deadPlayer_pos = player->GetPosition();
			deadPlayer->SetPosition(deadPlayer_pos);
		}
	}
	
	// Remove collected coins
  list<shared_ptr<SFAsset>> coinsTmp;
	for(auto c : coins) {
		if(c->IsAlive()) {
			coinsTmp.push_back(c);
		}
	}
	coins.clear();
  coins = list<shared_ptr<SFAsset>>(coinsTmp);

}

void SFApp::OnRender() {
  // clear the surface
  SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 54, 54, 54) );

  for(auto t: playerTails) {
    if(t->IsAlive()) {t->OnRender(surface);}
  }

  for(auto c: coins) {
    if(c->IsAlive()) {c->OnRender(surface);}
  }
  
  // If player is alive, render it, otherwise render the dead player. Positioned after other OnRenders
  // because it has to be ON TOP of other things if they overlap.
  if(player->IsAlive()) {
  	player->OnRender(surface);
  } else {
  	deadPlayer->OnRender(surface);
  }

  // Switch the off-screen buffer to be on-screen
  SDL_Flip(surface);
}

void SFApp::UpdateTail() {
// Makes the tail follow the players head around.
	auto headTail = make_shared<SFAsset>(SFASSET_TAIL); // Make new tail section
	auto v = player->GetPosition();											// Get position of players head
	headTail->SetPosition(v);														// Give new tail section the location of players head
	playerTails.emplace_front(headTail);								// Put new tail section at front of tail list
	// Found out about emplace_front from: http://www.cplusplus.com/reference/list/list/emplace_front/ Is nice.
	
	if (points < playerTails.size()-2) { // Player starts with 2 tail sections
		playerTails.pop_back();	// Remove last tail section
	} else {
	// Player got a point so tail is one longer
	}
}

void SFApp::NewCoin() {
// remember 20x20 grid
  // Generate 2 random numbers between 1 and 20.
	int randomX = rand() % 20 + 1;
	int randomY = rand() % 20 + 1;
  auto coin = make_shared<SFAsset>(SFASSET_COIN);
  auto pos  = Point2((randomX*32)-16, (randomY*32)+14); // coin.png is 30x30, grid sections 32x32
  coin->SetPosition(pos);
  coins.push_back(coin);
  cout << "Coin placed at (" << randomX << "," << randomY << ")" << endl;
}
