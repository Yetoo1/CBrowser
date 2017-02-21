/*compile with gcc `pkg-config --cflags gtk+-3.0` -o main main.c `pkg-config --libs gtk+-3.0` -ltidy -lcurl*/
//first this needs to get created then the modularity can begin in a new form
#include <gtk/gtk.h>
#include <tidy/tidy.h>
#include <tidy/buffio.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <stdio.h>
typedef struct 
{
	GtkWidget *grid, *view;
	GtkTextBuffer *buffer;
} gridntext;
static gridntext vg;
/* curl write callback, to fill tidy's input buffer...  */ 
uint write_cb(char *in, uint size, uint nmemb, TidyBuffer *out)
{
  uint r;
  r = size * nmemb;
  tidyBufAppend(out, in, r);
  return r;
}

static void display()
{
	vg.view = gtk_text_view_new();
    	vg.buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(vg.view));
        gtk_text_buffer_set_text(vg.buffer, "fuck man, this shit better work", -1);
        gtk_grid_attach(GTK_GRID(vg.grid), vg.view, 1,5,1,1);
}

/* Traverse the document tree */ 
void dumpNode(TidyDoc doc, TidyNode tnod, int indent) //probably need a struct or some shit for the widgets, I really don't know
{
 	//GtkWidget *view, *grid;
 	//GtkTextBuffer *buffer;
	//grid = gtk_grid_new();
	//gtk_container_add(GTK_CONTAINER(window),grid);
	//view = gtk_text_view_new();
	//buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
	//gtk_text_buffer_set_text(buffer, "fucking test", -1);
        //gtk_grid_attach(GTK_GRID(grid), view, 1,3,1,1);
	
	//vg.view = gtk_text_view_new();
	//vg.buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(vg.view));
	//gtk_text_buffer_set_text(vg.buffer, "fuck man, this shit better work", -1);
	//gtk_grid_attach(GTK_GRID(vg.grid), vg.view, 1,3,1,1);
	//vg.asd = "asd";
  TidyNode child;
  for(child = tidyGetChild(tnod); child; child = tidyGetNext(child) ) {
    ctmbstr name = tidyNodeGetName(child);
    if(name) {
      /* if it has a name, then it's an HTML tag ... */ 
      TidyAttr attr;
      printf("%*.*s%s ", indent, indent, "<", name);
      /* walk the attribute list */ 
      for(attr=tidyAttrFirst(child); attr; attr=tidyAttrNext(attr) ) {
        printf(tidyAttrName(attr));
        tidyAttrValue(attr)?printf("=\"%s\" ",
                                   tidyAttrValue(attr)):printf(" ");
      }
      printf(">\n");
    }
    else {
      /* if it doesn't have a name, then it's probably text, cdata, etc... */ 
      TidyBuffer buf;
      tidyBufInit(&buf);
      tidyNodeGetText(doc, child, &buf);
      printf("%*.*s\n", indent, indent, buf.bp?(char *)buf.bp:"");
      tidyBufFree(&buf);
    }
    dumpNode(doc, child, indent + 4); /* recursive */ 
  }
}
static void do_shit(GtkWidget * widget, GtkWidget *entry)
{
  const gchar *entry_text;
  entry_text = gtk_entry_get_text (GTK_ENTRY (entry));
  //g_print("%s\n", entry_text);
  CURL *curl;
  char curl_errbuf[CURL_ERROR_SIZE];
  TidyDoc tdoc;
  TidyBuffer docbuf = {0};
  TidyBuffer tidy_errbuf = {0};
  int err;
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, entry_text);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_errbuf);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
 
    tdoc = tidyCreate();
    tidyOptSetBool(tdoc, TidyForceOutput, yes); /* try harder */ 
    tidyOptSetInt(tdoc, TidyWrapLen, 4096);
    tidySetErrorBuffer(tdoc, &tidy_errbuf);
    tidyBufInit(&docbuf);
 
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &docbuf);
    err=curl_easy_perform(curl);
    if(!err) {
      err = tidyParseBuffer(tdoc, &docbuf); /* parse the input */ 
      if(err >= 0) {
        err = tidyCleanAndRepair(tdoc); /* fix any problems */ 
        if(err >= 0) {
          err = tidyRunDiagnostics(tdoc); /* load tidy error buffer */ 
          if(err >= 0) {
            dumpNode(tdoc, tidyGetRoot(tdoc), 0); /* walk the tree */ 
            fprintf(stderr, "%s\n", tidy_errbuf.bp); /* show errors */ 
          }
        }
      }
    }
    else
      fprintf(stderr, "%s\n", curl_errbuf);
 
    /* clean-up */ 
    curl_easy_cleanup(curl);
    tidyBufFree(&docbuf);
    tidyBufFree(&tidy_errbuf);
    tidyRelease(tdoc);
    //return err;
	
}

static void
activate (GtkApplication* app,
          gpointer        user_data)
{
	const gchar *text;
 	const gchar *texto;
	GtkWidget *window, *label, *button, *entry;
  	//GtkTextBuffer *buffer;
	window = gtk_application_window_new (app);
  	gtk_window_set_title (GTK_WINDOW (window), "Window");
  	gtk_window_set_default_size (GTK_WINDOW (window), 400, 400);
  
	/* Create a 1x2 table */
	vg.grid = gtk_grid_new();
	gtk_container_add(GTK_CONTAINER (window), vg.grid);
	//left, right, up , down
	//tell the user what to do (bad philosophy if you want the user to feel at home, unless you want them to feel infantile)
	label = gtk_label_new("Enter the url:");
	gtk_grid_attach(GTK_GRID(vg.grid), label, 0, 1, 1, 1);
	entry = gtk_entry_new();
	//find a way to get the text in here to be appended to the yeah shit
	//texto = gtk_entry_buffer_get_text(gtk_entry_buffer_new(text, 10));
	//g_print("%c", texto);
	//gtk_entry_set_max_length(GTK_ENTRY(entry), 50);
	g_signal_connect(entry, "activate", G_CALLBACK (do_shit), entry); /*when enter is pressed, shit is sent */
	gtk_grid_attach(GTK_GRID(vg.grid), entry, 1, 1, 1, 1);
	//this does the same thing as enter but in button form
	button = gtk_button_new_with_label("Enter");
	g_signal_connect(button, "clicked", G_CALLBACK(do_shit), entry);
	gtk_grid_attach(GTK_GRID(vg.grid), button, 1,2,1,1);

	button = gtk_button_new_with_label("Test");
	
	g_signal_connect(button, "clicked", G_CALLBACK(display), vg.view);
	gtk_grid_attach(GTK_GRID(vg.grid), button, 1, 4, 1, 1);
	
	
	
	//vg.view = gtk_text_view_new();
	//vg.buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(vg.view));
	//gtk_text_buffer_set_text(vg.buffer, "fuck man, this shit better work", -1);
	//gtk_text_buffer_set_text(vg.buffer, vg.entry_text, -1);
	//gtk_grid_attach(GTK_GRID(vg.grid), vg.view, 1,3,1,1);
	display();
	
	gtk_widget_show_all (window);
}

int
main (int    argc,  char **argv)
{
  GtkApplication *app;
  int status;

  app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);
	
  return status;
}
