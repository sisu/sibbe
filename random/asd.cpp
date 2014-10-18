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

					if(e.key.keysym.sym==SDLK_a) kuuntelija.putDown('a');
					if(e.key.keysym.sym==SDLK_b) kuuntelija.putDown('b');
					if(e.key.keysym.sym==SDLK_c) kuuntelija.putDown('c');
					if(e.key.keysym.sym==SDLK_d) kuuntelija.putDown('d');
					if(e.key.keysym.sym==SDLK_e) kuuntelija.putDown('e');
					if(e.key.keysym.sym==SDLK_f) kuuntelija.putDown('f');
					if(e.key.keysym.sym==SDLK_g) kuuntelija.putDown('g');
					if(e.key.keysym.sym==SDLK_h) kuuntelija.putDown('h');	
					if(e.key.keysym.sym==SDLK_i) kuuntelija.putDown('i');	
					if(e.key.keysym.sym==SDLK_j) kuuntelija.putDown('j');	
					if(e.key.keysym.sym==SDLK_k) kuuntelija.putDown('k');	
					if(e.key.keysym.sym==SDLK_l) kuuntelija.putDown('l');	
					if(e.key.keysym.sym==SDLK_m) kuuntelija.putDown('m');	
					if(e.key.keysym.sym==SDLK_n) kuuntelija.putDown('n');	
					if(e.key.keysym.sym==SDLK_o) kuuntelija.putDown('o');	
					if(e.key.keysym.sym==SDLK_p) kuuntelija.putDown('p');	
					if(e.key.keysym.sym==SDLK_q) kuuntelija.putDown('q');	
					if(e.key.keysym.sym==SDLK_r) kuuntelija.putDown('r');	
					if(e.key.keysym.sym==SDLK_s) kuuntelija.putDown('s');	
					if(e.key.keysym.sym==SDLK_t) kuuntelija.putDown('t');
					if(e.key.keysym.sym==SDLK_u) kuuntelija.putDown('u');	
					if(e.key.keysym.sym==SDLK_v) kuuntelija.putDown('v');	
					if(e.key.keysym.sym==SDLK_w) kuuntelija.putDown('w');	
					if(e.key.keysym.sym==SDLK_x) kuuntelija.putDown('x');	
					if(e.key.keysym.sym==SDLK_y) kuuntelija.putDown('y');	
					if(e.key.keysym.sym==SDLK_z) kuuntelija.putDown('z');				
			}
		}
		
		
		///...

		for(char c='a'; c<='z'; c++)
			if(kuuntelija.keyPressed(c))
				syote+=c;
	}
}
