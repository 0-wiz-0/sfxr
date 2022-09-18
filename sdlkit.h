#ifndef SDLKIT_H
#define SDLKIT_H

#include <SDL.h>
#define ERROR(x) error(__FILE__, __LINE__, #x)
#define VERIFY(x) do { if (!(x)) ERROR(x); } while (0)

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void error (const char *file, unsigned int line, const char *msg)
{
	fprintf(stderr, "[!] %s:%u  %s\n", file, line, msg);
	exit(1);
}

typedef Uint32 DWORD;
typedef Uint16 WORD;

#define DIK_SPACE 0
#define DIK_RETURN 1
#define DDK_WINDOW 0

#define hWndMain 0
#define hInstanceMain 0

#define Sleep(x) SDL_Delay(x)

static bool keys[2];

void ddkInit();      // Will be called on startup
bool ddkCalcFrame(); // Will be called every frame, return true to continue running or false to quit
void ddkFree();      // Will be called on shutdown

class DPInput {
public:
	DPInput(int,int) {}
	~DPInput() {}
	static void Update () {}

	static bool KeyPressed(int key)
	{
		bool r = keys[key];
		keys[key] = false;
		return r;
	}

};

static Uint32 *ddkscreen32;
static Uint16 *ddkscreen16;
static int ddkpitch;
static int mouse_x, mouse_y, mouse_px, mouse_py;
static bool mouse_left = false, mouse_right = false, mouse_middle = false;
static bool mouse_leftclick = false, mouse_rightclick = false, mouse_middleclick = false;

static SDL_Window *sdl_window = NULL;
static SDL_Renderer *sdl_renderer = NULL;
static SDL_Texture *sdl_texture = NULL;
static Uint32 *pixels = NULL;
static Uint32 pitch = 0;

static void sdlupdate ()
{
	float logical_x, logical_y;
	mouse_px = mouse_x;
	mouse_py = mouse_y;
	Uint8 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
	SDL_RenderWindowToLogical(sdl_renderer, mouse_x, mouse_y, &logical_x, &logical_y);
	mouse_x = truncf(logical_x);
	mouse_y = truncf(logical_y);
	bool mouse_left_p = mouse_left;
	bool mouse_right_p = mouse_right;
	bool mouse_middle_p = mouse_middle;
	mouse_left = buttons & SDL_BUTTON(1);
	mouse_right = buttons & SDL_BUTTON(3);
	mouse_middle = buttons & SDL_BUTTON(2);
	mouse_leftclick = mouse_left && !mouse_left_p;
	mouse_rightclick = mouse_right && !mouse_right_p;
	mouse_middleclick = mouse_middle && !mouse_middle_p;
}

static void ddkLock ()
{
	ddkscreen16 = (Uint16*)pixels;
	ddkscreen32 = pixels;
}

static void ddkUnlock ()
{
	// nothing to do here
}

static void ddkSetMode (int width, int height, int bpp, int refreshrate, int fullscreen, const char *title)
{
	SDL_Surface *icon;

        //	SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &sdl_window, &sdl_renderer);
	SDL_CreateWindowAndRenderer(width, height, SDL_WINDOW_RESIZABLE, &sdl_window, &sdl_renderer);
	if (sdl_window == NULL || sdl_renderer == NULL) {
		ERROR("error creating SDL window");
	}
	// TODO: check overflow
	if ((pixels = (Uint32*)malloc(width * height * bpp)) == NULL) {
		ERROR("error creating pixel buffer");
	}
	// for bpp, only 16 and 32 are support
	if ((sdl_texture = SDL_CreateTexture(sdl_renderer, bpp == 32 ? SDL_PIXELFORMAT_ARGB8888 : SDL_PIXELFORMAT_ARGB4444,
					     SDL_TEXTUREACCESS_STATIC, width, height)) == NULL) {
		ERROR("error creating texture");
	}
	pitch = width * bpp / 8;
	ddkpitch = width;
	icon = SDL_LoadBMP("/usr/share/sfxr/sfxr.bmp");
	if (!icon) {
		icon = SDL_LoadBMP("sfxr.bmp");
	}
	if (icon) {
		SDL_SetWindowIcon(sdl_window, icon);
		SDL_FreeSurface(icon);
	}
	SDL_SetWindowTitle(sdl_window, title);
	SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255);
	SDL_RenderClear(sdl_renderer);
	SDL_RenderPresent(sdl_renderer);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(sdl_renderer, width, height);
}

#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>


static void on_file_chooser_response (GtkDialog *dialog, int response, char *fname)
{
	if (response == GTK_RESPONSE_ACCEPT) {
		GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);

		GFile *file = gtk_file_chooser_get_file (chooser);
		char *path = g_file_get_path(file);
		strncpy(fname, g_file_get_path(file), 255);
		fname[255] = 0;
		g_free(path);
	}

	gtk_window_destroy (GTK_WINDOW (dialog));
}

static bool load_file (char *fname)
{
	char *fn;
	bool ret = false;

	GtkWidget *dialog = gtk_file_chooser_dialog_new ("Load File",
							 NULL,
							 GTK_FILE_CHOOSER_ACTION_OPEN,
							 "_Cancel", GTK_RESPONSE_CANCEL,
							 "_Open", GTK_RESPONSE_ACCEPT,
							 NULL);
	GtkFileFilter *filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filter, "*.sfxr");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	gtk_window_set_modal (GTK_WINDOW(dialog), true);
	gtk_widget_show (dialog);

	fname[0] = 0;
	g_signal_connect (dialog, "response",
			  G_CALLBACK (on_file_chooser_response),
			  fname);

	while (g_list_model_get_n_items (gtk_window_get_toplevels ()) > 0) {
		g_main_context_iteration (NULL, TRUE);
	}

	return fname[0] != 0;
}

static bool save_file (char *fname, int sfxr)
{
	char *fn;
	bool ret = false;

	GtkWidget *dialog = gtk_file_chooser_dialog_new("Save file",
	                                                NULL,
	                                                GTK_FILE_CHOOSER_ACTION_SAVE,
	                                                "_Cancel", GTK_RESPONSE_CANCEL,
	                                                "_Save", GTK_RESPONSE_ACCEPT,
	                                                NULL);

	gtk_window_set_modal (GTK_WINDOW(dialog), true);
	gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER(dialog), sfxr ? "New file.sfxr" : "New file.wav");
	gtk_widget_show (dialog);

	fname[0] = 0;
	g_signal_connect (dialog, "response",
			  G_CALLBACK (on_file_chooser_response),
			  fname);

	while (g_list_model_get_n_items (gtk_window_get_toplevels ()) > 0) {
		g_main_context_iteration (NULL, TRUE);
	}

	return fname[0] != 0;
}

#define FileSelectorLoad(x,file,y) load_file(file)
#define FileSelectorSave(x,file,sfxr) save_file(file, sfxr)

static void sdlquit ()
{
	ddkFree();
	SDL_Quit();
}

static void sdlinit ()
{
	VERIFY(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO));
	atexit(sdlquit);
	memset(keys, 0, sizeof(keys));
	ddkInit();
}

static void loop (void)
{
	SDL_Event e;
	while (true)
	{
		if (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT:
				return;

			case SDL_KEYDOWN:
				switch (e.key.keysym.sym) {
				case SDLK_SPACE:
					keys[DIK_SPACE] = true;
					break;
				case SDLK_RETURN:
					keys[DIK_RETURN] = true;
					break;
				case SDLK_q:
					return;
				default:
					break;
				}
				break;

			default: break;
			}
		}

		sdlupdate();

		if (!ddkCalcFrame())
			return;

		SDL_UpdateTexture(sdl_texture, NULL, pixels, pitch);
		SDL_RenderClear(sdl_renderer);
		SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, NULL);
		SDL_RenderPresent(sdl_renderer);
	}
}

int main (int argc, char *argv[])
{
	gtk_init();
	sdlinit();
	loop();
	return 0;
}

#endif
