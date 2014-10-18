#include <SDL/SDL.h>
#include <string>

using namespace std;

class paske{
public:
	bool keyPressed(char c){
		return is_down[int(c)]&&!was_down[int(c)];	
	}
	
	void nextFrame(){
		for(int i=0; i<256; i++){
			was_down[i]=is_down[i];
			is_down[i]=0;		
		}
	}
	
	void putDown(char c){
		is_down[int(c)]=1;
	}

	paske(){for(int i=0; i<256; i++)is_down[i]=was_down[i]=0;}
private:
	bool is_down[256];
	bool was_down[256];
};

void looppi(){
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO);
	
	///...

	paske kuuntelija;	
	string syote;	

	bool loop=1;
	for(;loop;){
		kuuntelija.nextFrame();

		
		///...


		SDL_Event e;
		while(SDL_PollEvent(&e)) {
			///...
			

	
			if(e.type==SDL_KEYDOWN){
					///...

								
					
					if(e.key.keysym.sym==SDLK_RETURN) loop=0;
					if(e.key.keysym.sym>='a'&&e.key.keysym.sym<='z')
						kuuntelija.putDown(char(e.key.keysym.sym));
			}
		}
		
		
		///...

		for(char c='a'; c<='z'; c++)
			if(kuuntelija.keyPressed(c))
				syote+=c;
	}
}
