#include <stdlib.h>
#include <stdint.h>
#include <gtk/gtk.h>
#include <time.h>

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#endif

/******************************** settings *****************************/
#define MODE_WORK 0
#define MODE_PAUZE 1

#define WORK_INTERVAL 60*60	/** 1 hodina */
#define PAUSE_DELAY 60*10	/** 10 minut */
#define PAUZE_INTERVAL 60*5	/** 5 minut */

/******************************** declaration **************************/

struct _computer_health_
{
	GtkWidget* window;

	int32_t deduction;
	uint8_t mode;
	uint32_t last_time;
	uint16_t label_position[2];
	GdkRectangle geometry;
};

typedef struct _computer_health_ computer_health;

/*
 * function: computer_health_new - constructor for computer_health class (defining window)
 * return: computer_health* - new compute_health class object
 */ 
computer_health* computer_health_new();

/*
 * function: computer_health_callback - core function, cyclic called function, counting the work and pauze time
 * param: gpointer param - parametr given inside the callback function
 * return: gboolean - return flag, if return is true continue calling, if return false stop calling the function 
 */ 
gboolean computer_health_callback(gpointer param);

/*
 * function: draw_callback - drawing function
 */ 
gboolean draw_callback(GtkWidget *widget, cairo_t *cr, gpointer param);

/*
 * function: computer_health_count_time - return time value in integer form, number of secounds
 * return: uint32_t - time in secounds
 */ 
uint32_t computer_health_count_time();

/*
 * function: computer_health_set_pauze_state - procedure for setting pauze state
 * param: computer_health* this - pointer to computer_health class object
 */ 
void computer_health_set_pauze_state(computer_health* this);

/*
 * function: computer_health_set_work_state - procedure for settings work state 
 * param: computer_health* this - pointer to computer_halth class object
 */ 
void computer_health_set_work_state(computer_health* this, int32_t deduction);

/*
 * key press event callback, if is set the pause state, then is available press 
 * escape key for delay the pause for 10 minutes
 */
gboolean computer_health_key_press (GtkWidget *widget, GdkEventKey *event, gpointer param);


/**************************** definition ******************************/

int main(int argv, char** argc)
{
#if defined(WIN32) || defined(WIN64)
	HWND var = GetConsoleWindow();
	ShowWindow(var, SW_HIDE);
#endif	

	gtk_init(&argv, &argc);
	
	computer_health* ch_instance = computer_health_new();
	computer_health_set_work_state(ch_instance, WORK_INTERVAL);
	
	gtk_main();

#if defined(WIN32) || defined(WIN64)	
	ShowWindow(var, SW_SHOW);
#endif
	
	return 0;
}

computer_health* computer_health_new()
{
	computer_health* this = (computer_health*) malloc(sizeof(computer_health));

	this->last_time = computer_health_count_time();

	GdkDisplay* display = gdk_display_get_default(); 
	GdkMonitor* monitor = gdk_display_get_monitor(display, 0); 
	gdk_monitor_get_geometry(monitor, &(this->geometry));

	this->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(this->window), this->geometry.width/2, this->geometry.height/2);
	//gtk_window_maximize(GTK_WINDOW(this->window));
	gtk_window_set_resizable(GTK_WINDOW(this->window), FALSE);
	gtk_window_fullscreen(GTK_WINDOW(this->window));
	
/*
	GdkCursor * cursor = gdk_cursor_new_for_display(gdk_display_get_default(), GDK_BLANK_CURSOR);
	GdkWindow * win = gtk_widget_get_window((this->window));
	gdk_window_set_cursor(win, cursor);
*/

	g_signal_connect(G_OBJECT(this->window), "draw", G_CALLBACK(draw_callback), this);
	g_signal_connect(G_OBJECT(this->window), "key_release_event", G_CALLBACK(computer_health_key_press), this);
	g_signal_connect(G_OBJECT(this->window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	
	g_timeout_add(1000, computer_health_callback, this);

	gtk_widget_show_all(GTK_WIDGET(this->window));

	return this;
}

gboolean computer_health_key_press (GtkWidget *widget, GdkEventKey *event, gpointer param)
{
	computer_health* this = (computer_health*) param;
	
	if(event->keyval == GDK_KEY_Escape)
	{
		computer_health_set_work_state(this, PAUSE_DELAY);
	}

printf("key pressed\n");
	return TRUE;
}

uint32_t computer_health_count_time()
{
	time_t my_time;
	struct tm* time_info;

	time(&my_time);
	time_info = localtime(&my_time);

	uint32_t time_val = 0;

	time_val += time_info->tm_sec;
        time_val += (time_info->tm_min*60);
	time_val += (time_info->tm_hour*3600);
	time_val += (time_info->tm_mday*24*3600);
	time_val += ((time_info->tm_mon+1)*30*24*3600);
	time_val += (time_info->tm_year+1900);
	return time_val;
}

void computer_health_set_pauze_state(computer_health* this)
{
	this->mode = MODE_PAUZE;
	this->deduction = PAUZE_INTERVAL;
	gtk_widget_set_visible(GTK_WIDGET(this->window), TRUE);
}

void computer_health_set_work_state(computer_health* this, int32_t deduction)
{
	this->mode = MODE_WORK;
	this->deduction = deduction;
	gtk_widget_set_visible(GTK_WIDGET(this->window), FALSE);
}

gboolean computer_health_callback(gpointer param)
{
	computer_health* this = (computer_health*) param;
	uint32_t current_time = computer_health_count_time();
	uint32_t time_difference = (current_time - this->last_time);

	if(this->mode == MODE_WORK)
	{
		gtk_window_set_keep_above(GTK_WINDOW(this->window), FALSE);

		if(this->deduction > 0)
		{
			if(time_difference > 1)
			{
				if((time_difference + this->deduction) < WORK_INTERVAL)
				{
					this->deduction += time_difference;
				}	
				else
				{
					this->deduction = WORK_INTERVAL;
				}
			}
			else
			{
				this->deduction--;
			}
		}
		else
		{
			computer_health_set_pauze_state(this);
		}
	}
	else if(this->mode == MODE_PAUZE)
	{
		gtk_window_set_keep_above(GTK_WINDOW(this->window), TRUE);

		if(this->deduction > 0)
		{
			if(time_difference > 1)
			{
				if((time_difference + this->deduction) < PAUZE_INTERVAL)
				{
					this->deduction -= time_difference;
				}
				else
				{
					computer_health_set_work_state(this, WORK_INTERVAL);
				}
			}
			else
			{
				this->deduction --;
			}
		}
		else
		{
			computer_health_set_work_state(this, WORK_INTERVAL);
		}
	}
	else
	{
		computer_health_set_work_state(this, WORK_INTERVAL);
	}

	this->last_time = current_time;
	gtk_widget_queue_draw(GTK_WIDGET(this->window));
	//printf("deduction: %d\n", this->deduction);
	return TRUE;
}

gboolean draw_callback(GtkWidget *widget, cairo_t *cr, gpointer param)
{
	computer_health* this = (computer_health*) param;
	cairo_set_source_rgb(cr, 0.43,0.65 ,0);
	cairo_rectangle(cr, 0, 0, this->geometry.width, this->geometry.height);
	cairo_fill(cr);
	
	char label[6];
	label[0] = (this->deduction/60) > 9 ? ((this->deduction/60)/10)+48 : '0';
	label[1] = (this->deduction/60) > 9 ? ((this->deduction/60)%10)+48 : this->deduction/60 > 0 ? (this->deduction/60)+48 : '0';
	label[2] = ':';
	label[3] = (this->deduction%60) > 9 ? ((this->deduction%60)/10+48) : '0';
	label[4] = ((this->deduction%60)%10)+48;
	label[5] = '\0';

	cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr, 70);
	cairo_text_extents_t extents;
	cairo_text_extents(cr, label, &extents);

	if(this->deduction%10 == 0)
	{
		this->label_position[0] = rand()%((int)(this->geometry.width-extents.width));
		this->label_position[1] = extents.height+(rand()%((int)(this->geometry.height-extents.height)));
	}

	cairo_move_to(cr, this->label_position[0], this->label_position[1]);
	cairo_set_source_rgb(cr, 0.9, 0.9 ,0.9);
	cairo_show_text(cr, label);

	return TRUE;
}
