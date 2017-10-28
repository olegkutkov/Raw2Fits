/* 
   main.c
    - entry point of the application
    - UI routines

   Copyright 2017  Oleg Kutkov <elenbert@gmail.com>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  
 */

#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "converter.h"

static char *RAW_PATH = NULL;
static char *OUT_PATH = NULL;

static GMainContext *main_context;
static GMutex progress_bar_mutex;
static GMutex logger_mutex;

typedef struct dialog_argument {
	GtkWindow *main_window;
	GtkWidget *text_label;
	char *result_dir;
} dialog_argument_t;

typedef struct about_dialog_argument {
	GtkWindow *main_window;
	GtkDialog *dlg;
} about_dialog_argument_t;

typedef struct log_msg_threaded {
	void *arg;
	char buf[256];
} log_msg_threaded_t;

typedef struct done_cb_argument {
	GtkWidget *start_button;
	GtkWidget *stop_button;
} done_cb_argument_t;

typedef struct conv_start_argument {
	GtkEntry *entry_object;
	GtkEntry *entry_instrument;
	GtkEntry *entry_filter;
	GtkEntry *entry_exposure;
	GtkEntry *entry_temperature;
	GtkEntry *entry_telescope;
	GtkEntry *entry_observer;
	GtkEntry *entry_notes;
	GtkEntry *entry_date;
	converter_params_t conv_params;
	done_cb_argument_t done_params;
	GtkTextBuffer *logger;
	GtkButton *stop_button;
	GtkProgressBar *progrbar;
	GtkTextView *textview;
	GtkComboBox *combobox_color;
	GtkComboBox *combobox_filenaming;
	GtkCheckButton *overwrite_file;
	GtkCheckButton *autobright;
	GtkCheckButton *interpolation;
	GtkCheckButton *autoscale;
} conv_start_argument_t;

typedef struct conv_stop_argument {
	GtkButton *start_button;
	converter_params_t *conv_params;
} conv_stop_argument_t;

void select_directory(char *dlg_title, dialog_argument_t *arg)
{
	GtkWidget *dialog = gtk_file_chooser_dialog_new (dlg_title,
                                      arg->main_window,
                                      GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                      ("_Cancel"),
                                      GTK_RESPONSE_CANCEL,
                                      ("_Open"),
                                      GTK_RESPONSE_ACCEPT,
                                      NULL);

	if (GTK_RESPONSE_ACCEPT == gtk_dialog_run (GTK_DIALOG (dialog)))
 	{
		GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
    	char *filename = gtk_file_chooser_get_filename (chooser);

		arg->result_dir = malloc(strlen(filename) + 1);
		strcpy(arg->result_dir, filename);

		gtk_label_set_text(GTK_LABEL(arg->text_label), filename);

    	g_free (filename);
	} else {
		arg->result_dir = NULL;
	}

	gtk_widget_destroy (dialog);
}

void button_raw_dir_clicked_cb(GtkButton *button, dialog_argument_t *arg)
{
	select_directory("Select directory with CR2 files", arg);

	if (arg->result_dir != NULL) {
		RAW_PATH = arg->result_dir;
	}
}

void button_out_dir_clicked_cb(GtkButton *button, dialog_argument_t *arg)
{
	select_directory("Select output directory for FITS", arg);

	if (arg->result_dir != NULL) {
		OUT_PATH = arg->result_dir;
	}
}

void progress_setup(void *arg, int max_val)
{
	progress_params_t *progr = (progress_params_t*) arg;
	GtkProgressBar *pbar = GTK_PROGRESS_BAR(progr->progr_arg);

	gtk_progress_bar_set_fraction(pbar, 0.0);

	progr->fraction = 1.0 / max_val;
}

static gboolean update_progress_bar(gpointer arg)
{
	progress_params_t *progr = (progress_params_t*) arg;
	GtkProgressBar *pbar = GTK_PROGRESS_BAR(progr->progr_arg);

	gdouble fraction = gtk_progress_bar_get_fraction(pbar);
	fraction += progr->fraction;

	gtk_progress_bar_set_fraction(pbar, fraction);

	return G_SOURCE_REMOVE;
}

void progress_update(void *arg)
{
	GSource *source;

	g_mutex_lock(&progress_bar_mutex);

	source = g_idle_source_new();

	g_source_set_callback(source, update_progress_bar, arg, NULL);
	g_source_attach(source, main_context);
	g_source_unref(source);

	g_mutex_unlock(&progress_bar_mutex);
}

static gboolean update_textarea(gpointer user_data)
{
	GtkTextBuffer *buffer;
	GtkTextMark *mark;
	GtkTextIter iter;
	log_msg_threaded_t *log_arg = (log_msg_threaded_t *) user_data;

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (log_arg->arg));
	mark = gtk_text_buffer_get_insert (buffer);
	gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);
	gtk_text_buffer_insert(buffer, &iter, log_arg->buf, -1);

	gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(log_arg->arg), gtk_text_buffer_get_insert(buffer), 0.0, FALSE, 0.5, 0.5);

	free(user_data);

	return G_SOURCE_REMOVE;
}

void logger_msg_preformat(void *arg, char *str)
{
	log_msg_threaded_t *log_arg = (log_msg_threaded_t *) malloc(sizeof(log_msg_threaded_t));
	GSource *source;

	log_arg->arg = arg;
	strcpy(log_arg->buf, str);

	source = g_idle_source_new();

	g_source_set_callback(source, update_textarea, log_arg, NULL);
	g_source_attach(source, main_context);
	g_source_unref(source);
}

void logger_msg(void *arg, char *fmt, ...)
{
	g_mutex_lock(&logger_mutex);

	char buf[256];

	va_list args;

	va_start(args, fmt);
	vsnprintf(buf, 256, fmt, args);
	buf[255] = '\0';

	va_end(args);

	logger_msg_preformat(arg, buf);

	g_mutex_unlock(&logger_mutex);
}

static gboolean converting_done_update_ui(gpointer user_data)
{
	done_cb_argument_t *done_arg = (done_cb_argument_t *) user_data;

	gtk_widget_set_sensitive(done_arg->start_button, TRUE);
	gtk_widget_set_sensitive(done_arg->stop_button, FALSE);

	return G_SOURCE_REMOVE;
}

void converting_done(void *arg)
{
	GSource *source = g_idle_source_new();

	g_source_set_callback(source, converting_done_update_ui, arg, NULL);
	g_source_attach(source, main_context);
	g_source_unref(source);
}

void button_convert_clicked_cb(GtkButton *button, conv_start_argument_t *arg)
{
	if (!RAW_PATH || !OUT_PATH) {
		return;
	}

	gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(arg->stop_button), TRUE);

	converter_params_t *conv_params = &arg->conv_params;

	conv_params->converter_run = 1;

	memset(&conv_params->meta, 0, sizeof(file_metadata_t));

	const char *entry_object_text = gtk_entry_get_text(arg->entry_object);
	strncpy(conv_params->meta.object, entry_object_text, strlen(entry_object_text));

	const char *entry_instrument_text = gtk_entry_get_text(arg->entry_instrument);
	strncpy(conv_params->meta.instrument, entry_instrument_text, strlen(entry_instrument_text));

	const char *entry_filter_text = gtk_entry_get_text(arg->entry_filter);
	strncpy(conv_params->meta.filter, entry_filter_text, strlen(entry_filter_text));

	const char *entry_telescope_text = gtk_entry_get_text(arg->entry_telescope);
	strncpy(conv_params->meta.telescope, entry_telescope_text, strlen(entry_telescope_text));

	const char *entry_observer_text = gtk_entry_get_text(arg->entry_observer);
	strncpy(conv_params->meta.observer, entry_observer_text, strlen(entry_observer_text));

	const char *entry_notes_text = gtk_entry_get_text(arg->entry_notes);
	strncpy(conv_params->meta.note, entry_notes_text, strlen(entry_notes_text));

	const char *entry_date_text = gtk_entry_get_text(arg->entry_date);
	strncpy(conv_params->meta.date, entry_date_text, strlen(entry_date_text));

	conv_params->meta.exptime = atof(gtk_entry_get_text(arg->entry_exposure));
	conv_params->meta.temperature = atof(gtk_entry_get_text(arg->entry_temperature));

	conv_params->meta.bitpixel = 0;
	conv_params->meta.width = 0;
	conv_params->meta.height = 0;

	conv_params->meta.overwrite_instrument = 0;
	conv_params->meta.overwrite_observer = 0;
	conv_params->meta.overwrite_exptime = 0;
	conv_params->meta.overwrite_date = 0;

	conv_params->imsetup.mode = gtk_combo_box_get_active(arg->combobox_color);

	conv_params->fsetup.naming = gtk_combo_box_get_active(arg->combobox_filenaming);

	conv_params->fsetup.overwrite = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(arg->overwrite_file));
	conv_params->imsetup.apply_auto_bright = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(arg->autobright));
	conv_params->imsetup.apply_interpolation = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(arg->interpolation));
	conv_params->imsetup.apply_autoscale = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(arg->autoscale));

	memset(conv_params->inpath, 0, sizeof(conv_params->inpath));
	strncpy(conv_params->inpath, RAW_PATH, strlen(RAW_PATH));

	memset(conv_params->outpath, 0, sizeof(conv_params->outpath));
	strncpy(conv_params->outpath, OUT_PATH, strlen(OUT_PATH));

	conv_params->progress.progr_arg = (void *) arg->progrbar;
	conv_params->progress.progr_setup = &progress_setup;
	conv_params->progress.progr_update = &progress_update;

	conv_params->logger_arg = (void *) arg->textview;
	conv_params->logger_msg = &logger_msg;

	conv_params->complete = &converting_done;
	conv_params->done_arg = (void *) &arg->done_params;

	convert_files(conv_params);
}

void button_convert_stop_clicked_cb(GtkButton *button, conv_stop_argument_t *arg)
{
	gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(arg->start_button), TRUE);

	converter_params_t *conv_params = arg->conv_params;

	converter_stop(conv_params);
}

void on_dialog_response(GtkWidget *obj)
{
	gtk_widget_hide(obj);
}

void button_about_clicked_cb(GtkButton *button, about_dialog_argument_t *arg)
{
	gtk_window_set_modal(GTK_WINDOW(arg->dlg), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(arg->dlg), arg->main_window);

	gtk_widget_show(GTK_WIDGET(arg->dlg));
}

void on_window_main_destroy()
{
	gtk_main_quit();

	if (RAW_PATH) {
		free(RAW_PATH);
	}

	if (OUT_PATH) {
		free(OUT_PATH);
	}
}

void show_error(char *text) {
 	GtkWidget *dialog;
	dialog = gtk_message_dialog_new(NULL,
		GTK_DIALOG_MODAL,
		GTK_MESSAGE_ERROR,
		GTK_BUTTONS_OK,
		"%s",
		text);

	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

int main(int argc, char *argv[])
{
	GError *err = NULL;
	GtkBuilder *builder;
	GtkWidget *window;
	GtkWidget *about_window; 
	GtkWidget *cr2_dir_label;
	GtkWidget *out_dir_label;
	GtkWidget *button_raw_dir;
	GtkWidget *button_output_dir;
	GtkWidget *button_convert;
	GtkWidget *button_about;
	GtkWidget *button_stop;

	dialog_argument_t cr2_dlg_userdata;
	dialog_argument_t out_dlg_userdata;
	about_dialog_argument_t about_dlg_userdata;
	conv_start_argument_t conv_arg;
	conv_stop_argument_t conv_stop_arg;

	gtk_init(&argc, &argv);

	builder = gtk_builder_new();

	if (!gtk_builder_add_from_file (builder, "/usr/share/raw2fits/ui.glade", &err)) {

		err = NULL;

		if (!gtk_builder_add_from_file (builder, "glade/ui.glade", &err)) {
			show_error(err->message);
			g_object_unref(builder);
			return -1;
		}
	}

	window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
	about_window = GTK_WIDGET(gtk_builder_get_object(builder, "aboutdialog"));

	cr2_dir_label = GTK_WIDGET(gtk_builder_get_object(builder, "cr2_dir_label"));
	out_dir_label = GTK_WIDGET(gtk_builder_get_object(builder, "out_dir_label"));
	button_raw_dir = GTK_WIDGET(gtk_builder_get_object(builder, "button_raw_dir"));
	button_output_dir = GTK_WIDGET(gtk_builder_get_object(builder, "button_output_dir"));
	button_convert = GTK_WIDGET(gtk_builder_get_object(builder, "button_convert"));
	button_stop = GTK_WIDGET(gtk_builder_get_object(builder, "button_convert_stop"));
	button_about = GTK_WIDGET(gtk_builder_get_object(builder, "button_about"));

	conv_arg.entry_object = GTK_ENTRY(gtk_builder_get_object(builder, "entry_object"));
	conv_arg.entry_instrument = GTK_ENTRY(gtk_builder_get_object(builder, "entry_instrument"));
	conv_arg.entry_filter = GTK_ENTRY(gtk_builder_get_object(builder, "entry_filter"));
	conv_arg.entry_exposure = GTK_ENTRY(gtk_builder_get_object(builder, "entry_exposure"));
	conv_arg.entry_temperature = GTK_ENTRY(gtk_builder_get_object(builder, "entry_temperature"));
	conv_arg.entry_telescope = GTK_ENTRY(gtk_builder_get_object(builder, "entry_telescope"));
	conv_arg.entry_observer = GTK_ENTRY(gtk_builder_get_object(builder, "entry_observer"));
	conv_arg.entry_notes = GTK_ENTRY(gtk_builder_get_object(builder, "entry_notes"));
	conv_arg.entry_date = GTK_ENTRY(gtk_builder_get_object(builder, "entry_date"));
	conv_arg.combobox_color = GTK_COMBO_BOX(gtk_builder_get_object(builder, "combobox_color"));
	conv_arg.combobox_filenaming = GTK_COMBO_BOX(gtk_builder_get_object(builder, "combobox_outfilename"));

	conv_arg.overwrite_file = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "checkbutton_overwrite_existing_file"));
	conv_arg.autobright = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "checkbutton_autobright"));
	conv_arg.interpolation = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "checkbutton_interpolation"));
	conv_arg.autoscale = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "checkbutton_autoscale"));

	conv_arg.progrbar = GTK_PROGRESS_BAR(gtk_builder_get_object(builder, "conv_progressbar"));

	gtk_progress_bar_set_fraction (conv_arg.progrbar, 0.0);

	conv_arg.textview = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "conv_log"));
	conv_arg.stop_button = GTK_BUTTON(button_stop);

	conv_arg.done_params.start_button = button_convert;
	conv_arg.done_params.stop_button = button_stop;

	conv_stop_arg.start_button = GTK_BUTTON(button_convert);
	conv_stop_arg.conv_params = &conv_arg.conv_params;
///

	cr2_dlg_userdata.main_window = (GtkWindow*)window;
	cr2_dlg_userdata.text_label = cr2_dir_label;

	out_dlg_userdata.main_window = (GtkWindow*)window;
	out_dlg_userdata.text_label = out_dir_label;

///

	g_signal_connect (window, "destroy", G_CALLBACK (on_window_main_destroy), NULL);
	g_signal_connect(button_raw_dir, "clicked", G_CALLBACK (button_raw_dir_clicked_cb), &cr2_dlg_userdata);
	g_signal_connect(button_output_dir, "clicked", G_CALLBACK (button_out_dir_clicked_cb), &out_dlg_userdata);
	g_signal_connect(button_convert, "clicked", G_CALLBACK (button_convert_clicked_cb), &conv_arg);
	g_signal_connect(button_stop, "clicked", G_CALLBACK (button_convert_stop_clicked_cb), &conv_stop_arg);

	g_signal_connect(about_window, "response", G_CALLBACK (on_dialog_response), about_window);

	about_dlg_userdata.main_window = GTK_WINDOW(window);
	about_dlg_userdata.dlg = GTK_DIALOG(about_window);

	g_signal_connect(button_about, "clicked", G_CALLBACK (button_about_clicked_cb), &about_dlg_userdata);

	g_object_unref(builder);

	g_mutex_init(&progress_bar_mutex);
	g_mutex_init(&logger_mutex);

	main_context = g_main_context_default();

	gtk_widget_show(window);                
	gtk_main();

	converter_stop(conv_stop_arg.conv_params);

	g_mutex_clear(&logger_mutex);
	g_mutex_clear(&progress_bar_mutex);

	return 0;
}

