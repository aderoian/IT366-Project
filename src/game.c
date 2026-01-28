#include <SDL.h>
#include "simple_logger.h"

#include "gfc_input.h"
#include "gfc_config_def.h"

#include "gf2d_graphics.h"
#include "gf2d_sprite.h"

#include "animation.h"
#include "entity.h"
#include "player_entity.h"

uint8_t __DEBUG = 0;

void parse_arguments(int argc,char *argv[]);

int main(int argc, char * argv[])
{
    /*variable declarations*/
    int done = 0;
    const Uint8 * keys;
    Sprite *sprite;
    
    int mx,my;
    float then, now, mf = 0;
    Sprite *mouse;
    GFC_Color mouseGFC_Color = gfc_color8(255,100,255,200);
    
    /*program initializtion*/
    parse_arguments(argc,argv);
    init_logger("gf2d.log",0);
    gfc_input_init("def/input.json");
    gfc_config_def_init();

    slog("---==== BEGIN ====---");
    gf2d_graphics_initialize(
        "gf2d",
        1200,
        720,
        1200,
        720,
        gfc_vector4d(0,0,0,255),
        0);
    gf2d_graphics_set_frame_delay(16);
    gf2d_sprite_init(1024);
    animation_manager_init(256);
    entity_init(1024);
    phys_init(1024);
    SDL_ShowCursor(SDL_DISABLE);

    slog_sync();
    
    /*demo setup*/
    sprite = gf2d_sprite_load_image("images/backgrounds/bg_flat.png");
    mouse = gf2d_sprite_load_all("images/pointer.png",32,32,16,0);

    Entity * ent = player_spawn(gfc_vector2d(200, 200), "images/pointer.png");
    player_spawn_immobile(gfc_vector2d(300, 200), "images/pointer.png");
    if (!ent) slog("failed to spawn player");
    slog("press [escape] to quit");
    /*main game loop*/

    then = (float) SDL_GetTicks() * 0.001f;
    while(!done)
    {
        gfc_input_update();
        // SDL_PumpEvents();   // update SDL's internal event structures
        // keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
        /*update things here*/
        //SDL_GetMouseState(&mx,&my);
        mf+=0.1;
        if (mf >= 16.0)mf = 0;

        now = (float) SDL_GetTicks() * 0.001f;
        entity_think_all();
        entity_update_all(now - then);

        phys_step(now - then);
        then = now;
        
        gf2d_graphics_clear_screen();// clears drawing buffers
        // all drawing should happen betweem clear_screen and next_frame
            //backgrounds drawn first
            gf2d_sprite_draw_image(sprite,gfc_vector2d(0,0));

            entity_draw_all();
            
            //UI elements last
            gf2d_sprite_draw(
                mouse,
                gfc_vector2d(mx,my),
                NULL,
                NULL,
                NULL,
                NULL,
                &mouseGFC_Color,
                (int)mf);

        gf2d_graphics_next_frame();// render current draw frame and skip to the next frame
        
        if (gfc_input_command_down("exit"))done = 1; // exit condition
        //slog("Rendering at %f FPS",gf2d_graphics_get_frames_per_second());
    }
    slog("---==== END ====---");
    return 0;
}

void parse_arguments(int argc,char *argv[]) {
    int a;
    for (a = 1; a < argc;a++) {
        if (strcmp(argv[a],"--debug") == 0) {
            __DEBUG = 1;
        }
    }    
}
/*eol@eof*/
