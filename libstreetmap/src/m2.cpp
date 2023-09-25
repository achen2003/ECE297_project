/* 
 * Copyright 2023 University of Toronto
 *
 * Permission is hereby granted, to use this software and associated 
 * documentation files (the "Software") in course work at the University 
 * of Toronto, or for personal use. Other uses are prohibited, in 
 * particular the distribution of the Software either publicly or to third 
 * parties.
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


#include "m1.h"
#include "m2.h"
#include "m3.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include <climits>
#include <limits>
#include <cmath>
#include <set>
#include <string>
#include "m2_helper.h"
#include "globals.h"
#include <fstream>
#include <chrono>
#include <thread>
#include <fstream>
#include <sstream>
#include <numeric>
#include <filesystem>


//Global gtkCssProvider functions:
/*Any time you want to add a button, or any other ui element, add its subsequent cssprovider here. This will allow me to add css style to it*/



//https://www.youtube.com/watch?v=UGx1AzlXd70

// g -> set_color(colorType[show_elements_global.dark][ISLAND][0], colorType[show_elements_global.dark][ISLAND][1], colorType[show_elements_global.dark][ISLAND][2]);

// Functions:
int colorType[2][11][3] = {{{54,69,79}, {100, 126, 104}, {184, 189, 112}, {112, 189, 184}, {112, 163, 189}, {102, 153, 150}, {93, 120, 137}, {100, 171, 95}, {79, 150, 114}, {92, 173, 160}, {132, 205, 193}},
{{0,0,0},{105, 206, 120}, {228, 240, 66},{158, 255, 249}, {158, 223, 255}, {157, 236, 230}, {165, 207, 233}, {137, 238, 129}, {128, 234, 130}, {126, 226, 209}, {155, 237, 224}}};
void dark_mode(GtkWidget* /*widget*/, ezgl::application *application);
void show_subway_stations(GtkWidget* /*widget*/, ezgl::application *application);
void updateDropDown(GtkWidget* /*widget*/, ezgl::application *app);
void updateDirections(GtkWidget* /*widget*/, ezgl::application *app);
void act_on_mouse_click(ezgl::application *app, GdkEventButton* /* event */ , double x, double y);
void dialog_response_callback(GtkDialog *dialog, gint response_id, gpointer user_data);
void initial_setup (ezgl::application *application, bool /*new_window*/);
void show_poi(GtkWidget* /*widget*/, ezgl::application *application);
void searching(GtkWidget* /*widget*/, ezgl::application *app); 
void centre_on(GtkWidget* /*widget*/, ezgl::application *app);
void clear_search(GtkWidget* /*widget*/, ezgl::application *app);
void change_cities(GtkWidget* /*widget*/, ezgl::application &app);
void draw_new_canvas(ezgl::application *app);
void drawMapHelper(ezgl::application &app, std::string city_name);
void draw_main_canvas(ezgl::renderer *g);
void allFilesinPath(GtkWidget* /*widget*/, ezgl::application *app);
std::string process(std::string const& s, int type);
void activate_search_path(GtkWidget* /*widget*/, ezgl::application *app);
void search_path_response_callback(GtkDialog *dialog, gint response_id, gpointer /* user_data */);
void help_delete(GtkWidget* /*widget*/, ezgl::application *app);


typedef struct {
   ezgl::application* app;
   IntersectionIdx id_;  
} UserData;

// Change the global variable to change between high/low contrast
void dark_mode(GtkWidget* /*widget*/, ezgl::application *application) {
   show_elements_global.dark = !show_elements_global.dark;
   application -> refresh_drawing();
}

// Change the global variable to show POIs
void show_poi(GtkWidget* /*widget*/, ezgl::application *application) {
   show_elements_global.show_poi = !show_elements_global.show_poi;
   application -> refresh_drawing();
}

// Change the global variable to show the subway station
void show_subway_stations(GtkWidget* /*widget*/, ezgl::application *app) {
   show_elements_global.subways = !show_elements_global.subways;
   app -> refresh_drawing();
}



void changeSearchActive(GtkWidget* /*widget*/, ezgl::application *app);
void help_popup(GtkWidget* /*widget*/, ezgl::application *app);
std::string processCity(std::string const& s, std::string const& n);
void directionMenuPop(GtkWidget* /*widget*/, ezgl::application *app);

// Setting up connections of all buttons/elements in the application
// Only called when the map is opened the first time
// i.e. will not be called wehen changing to a new map
void initial_setup (ezgl::application *application, bool /*new window */) {
   // Create new buttons
   allFilesinPath(NULL, application);
   GObject *high_contrast = application->get_object("HighContrast");
   GObject *poi_button = application->get_object("PoiSwitch");
   GObject *entry_bar = application ->get_object("SearchEntry");
   GObject *result_box = application ->get_object("ResultBox");
   GObject *pathfinder_button = application->get_object("FindPath");
   GObject *map_changer = application->get_object("cities");
   GObject *help_button = application ->get_object("HelpButton");
   GObject *help_done = application ->get_object("HelpDone");
   GObject *clear_path = application ->get_object("ClearPath");
   GObject *subway_button = application -> get_object("SubwayButton");

   g_signal_connect(
      high_contrast,
      "clicked",
      G_CALLBACK(dark_mode),
      application
   );
   g_signal_connect(
      poi_button,
      "clicked",
      G_CALLBACK(show_poi),
      application
   );
   g_signal_connect(
      entry_bar,
      "changed",
      G_CALLBACK(updateDropDown),
      application
   );
   g_signal_connect(
      result_box,     // pointer to the UI widget
      "changed", // Signal name (from the widget documentation) e.g. "clicked"
      G_CALLBACK(searching), // name of callback function (you write this function)
      application // pointer to (arbitrary) data to pass to callback my_func or NULL for no data
   );
   g_signal_connect(
      pathfinder_button, 
      "clicked", 
      G_CALLBACK(activate_search_path),
      application
   );

   // map search entry not found
   g_signal_connect(
      map_changer,
      "changed",
      G_CALLBACK(change_cities),
      application
   );
   g_signal_connect(
      help_button,
      "clicked",
      G_CALLBACK(help_popup),
      application
   );   
   g_signal_connect(
      help_done, 
      "clicked",
      G_CALLBACK(help_delete),
      application
   );
   
   g_signal_connect(
      clear_path,
      "clicked",
      G_CALLBACK(clear_search),
      application
   );
   g_signal_connect(
      subway_button, 
      "clicked", 
      G_CALLBACK(show_subway_stations),
      application
   );




   // Adding Css styling
   GtkCssProvider *provider = gtk_css_provider_new();
//   GtkStyleContext *context = gtk_style_context_new();
   gtk_css_provider_load_from_path (provider,
   "libstreetmap/resources/theme.css", NULL);
   GdkScreen *screen =  gdk_screen_get_default();
   gtk_style_context_add_provider_for_screen (screen,
                                 GTK_STYLE_PROVIDER(provider),
                                 GTK_STYLE_PROVIDER_PRIORITY_USER);
   
   GtkWindow* window = GTK_WINDOW(application->get_object("MainWindow"));
   gtk_window_fullscreen(GTK_WINDOW(window));
}

bool directShow = false;

// Update the directions according to the from/to
void updateDirections(GtkWidget* /*widget*/, ezgl::application *app){
   GObject *text = app ->get_object("DirectionText");
   if(directShow){
      gtk_label_set_text(GTK_LABEL(text), "Test Directions");
      directShow = false;
   }
   else{
      gtk_label_set_text(GTK_LABEL(text), "");
   }
}

// Close the help window
void help_delete(GtkWidget* /*widget*/, ezgl::application *app){
   GtkWidget *help_menu = app->find_widget("HelpWindow");
   // gdk_window_hide(GDK_WINDOW(help_menu));
   gtk_widget_set_visible(help_menu, FALSE);
}
int helpState = 0;

// Set up the direction menu
void directionMenuPop(GtkWidget* /*widget*/, ezgl::application *app){
   if (helpState ==1){
      GtkWidget *widge = app->find_widget("TemporaryName");
      gtk_widget_destroy(widge);
      helpState = 0;
      return;
   }
   GtkGrid *in_grid = (GtkGrid *)app->get_object("OuterGrid");
   GtkWidget* new_label = gtk_label_new("TemporaryName");
   gtk_widget_set_name(new_label, "TemporaryName");
   // add the new button
   gtk_grid_attach(in_grid, new_label, 0, 0, 1, 1);
   // show the button
   gtk_widget_show(new_label);
   helpState = 1;
}
int popupState = 0;

// The callback function when the help button is clicked
// Will popup the help texts
void help_popup(GtkWidget* /*widget*/, ezgl::application *app){
   std::ifstream infile("libstreetmap/src/help.txt");
   std::ostringstream ss;
   if (infile.is_open())
      ss << infile.rdbuf();
   infile.close();
   std::string fileContents = ss.str();
   const char* finalContents = fileContents.c_str();
   GtkWidget *help_menu = app->find_widget("HelpWindow");
   GtkWidget *info = gtk_label_new(finalContents);
   gtk_widget_set_name(info, "info");
   GtkGrid *in_grid = (GtkGrid *)app->get_object("HelpGridText");
   gtk_grid_attach(in_grid, info, 1,1,10,10);
   if (popupState == 1){
      gtk_widget_hide(help_menu);
   }
   gtk_window_set_default_size (GTK_WINDOW(help_menu),1200,500);
   gtk_widget_show_all(help_menu);
   popupState = 1;
   
}  


// Update the content in the drop down menu
// when the input is changed
bool toggleSearch = false;
void changeSearchActive(GtkWidget* /*widget*/, ezgl::application *app){
   GObject *search_entry = app ->get_object("SearchEntry");
   GObject *box = app ->get_object("ResultBox");
   toggleSearch = true;
   const char* currActive = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(box));
   gtk_entry_set_text(GTK_ENTRY(search_entry), currActive);
}



// Stores all of the matched results (the street names and indices) according to the input of
// the search entry
std::vector<std::pair<std::vector<StreetIdx>, std::string>> options;


// clear all of the searched streets/intersections
void clear_search(GtkWidget* /*widget*/, ezgl::application *app) {
   highlight_global.from_highlighted = false;
   highlight_global.to_highlighted = false;
   highlight_global.from_id = -1;
   highlight_global.to_id = -1;
   highlight_global.all_intersections = false;
   highlight_global.two_streets_intersections.clear();
   highlight_global.single_street_searched = -1;

   found_path_global.found_path.clear();
   found_path_global.draw_found_path = false;

   GObject *text = app ->get_object("DirectionText");
   gtk_label_set_text(GTK_LABEL(text), "");

   GObject *from_box = app -> get_object("FromBox");
   GObject *to_box = app-> get_object("ToBox");
   std::string from_text, to_text;

   from_text = "From: ";

   const char* from_c_text = from_text.c_str();

   to_text = "To: ";   
   const char* to_c_text = to_text.c_str();  


   gtk_entry_set_text(GTK_ENTRY(from_box), from_c_text);
   gtk_entry_set_text(GTK_ENTRY(to_box), to_c_text);

   app -> refresh_drawing();
}

// Only called when the user update the input in the entry
// Will also call the searching function after the drop down is updated
void updateDropDown(GtkWidget* /*widget*/, ezgl::application *app){
      if (toggleSearch ==true){
         toggleSearch = false;
      }
      else{
         GObject *box = app -> get_object("ResultBox");
         GObject* search_entry = app -> get_object("SearchEntry");
         

         options.clear();
         GtkEntry* search_entry_text = GTK_ENTRY(search_entry);
         const gchar* text = gtk_entry_get_text(search_entry_text); 

         // Defensive
         if (box != nullptr) {
            gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(box));
         } else {
            std::cerr << "Error: box pointer is null." << std::endl;
         }
      
         gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(box));

         if (strlen(text) > 1){
            std::string street_name_entered(text);

         std::string first_name;
         std::string second_name;
         if(street_name_entered.find(" and ") != std::string::npos){
            first_name = process(street_name_entered, 0);
            second_name = process(street_name_entered, 1);
         }

         // single street is input
         if(street_name_entered.find(" and ") == std::string::npos){
            // all of the matched results
            std::vector<StreetIdx> matched_streets = findStreetIdsFromPartialStreetName(street_name_entered);
            for (int i = 0; i < matched_streets.size(); i++){
               if (i == 8){
                  break;
               }
               std::string street_name = getStreetName(matched_streets[i]);
               std::vector<StreetIdx> single_street(1);
               single_street[0] = matched_streets[i];

               options.push_back({single_street, street_name});

            }
            int num = 0;
            for(auto it : options){
               std::string single_street_temp = std::to_string(it.first[0]);
               const char* single_street_c_temp = single_street_temp.c_str();
               gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(box),num,single_street_c_temp,it.second.c_str());
               ++num;
            }

            //gtk_combo_box_set_active(GTK_COMBO_BOX(box), 0);

         }

         // should get the intersections
         else{
            std::vector<StreetIdx> firstStreets = findStreetIdsFromPartialStreetName(first_name);
            std::vector<StreetIdx> secondStreets = findStreetIdsFromPartialStreetName(second_name);
            if ((firstStreets.size()>0 && secondStreets.size()>0)) {
               for (int i = 0; i < firstStreets.size(); i++){
                  for (int j = 0; j < secondStreets.size(); j++){
                     if (findIntersectionsOfTwoStreets(firstStreets[i], secondStreets[j]).size()>0){
                        std::vector<StreetIdx> two_streets(2);
                        two_streets[0] = firstStreets[i];
                        two_streets[1] = secondStreets[j];
                        std::string street_name =  getStreetName(firstStreets[i]) + " and " + getStreetName(secondStreets[j]);
                        options.push_back({two_streets, street_name});
                     }
                  }
               }

               int num = 0;
               for(auto it : options){
                  std::string first_street_id = std::to_string(it.first[0]);
                  std::string second_street_id = std::to_string(it.first[1]);

                  std::string two_street_ids = first_street_id + " " + second_street_id;
                  const char* c_string_streets = two_street_ids.c_str();
                  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(box), num, c_string_streets ,it.second.c_str());
                  ++num;
               }
               //gtk_combo_box_set_active(GTK_COMBO_BOX(box), 0);
                  //gtk_combo_box_popup(GTK_COMBO_BOX(box));
                  gtk_entry_grab_focus_without_selecting(GTK_ENTRY(search_entry));
               }
            }
         }

      
      }
}

void draw_new_canvas(ezgl::application *app) {
   ezgl::renderer *g = app -> get_renderer();
   std::cout << "Drawing new map" << std::endl;
   draw_main_canvas(g);
}
std::vector<std::string>filler;

// Process the drop down menu of the cities
// Help to change the cities
std::string processCity(std::string const& s, std::string const& n){

   std::string::size_type pos = s.find("maps/");
   std::string::size_type pos2 = n.find(".streets");

      if (pos != std::string::npos)
      {
         return s.substr((pos+5),pos2-(pos+5));
      }
      else
      {
         return s;
      }

}

//https://icon-icons.com/icon/zoom-fit-best/104297
void allFilesinPath(GtkWidget* /*widget*/, ezgl::application *app){
   std::string path = "/cad2/ece297s/public/maps/";
   for (const auto & entry : std::filesystem::directory_iterator(path)){
      filler.push_back(std::filesystem::absolute( entry.path() ).string());
   }
   GObject *box = app->get_object("cities");
   for(auto it : filler){
      if((it.find(".osm.bin")) == std::string::npos && it.find(".osm2bin") == std::string::npos && it.find(".xml") == std::string::npos &&it.find(".log") == std::string::npos){
         std::string temp =processCity(it, it);
         gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(box),0,NULL,temp.c_str());
      }
   }
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(box),0,NULL,"Choose a city/country");  
   gtk_combo_box_set_active(GTK_COMBO_BOX(box), 0);
   
}



// Change the city according to the text entry
// after the search button is pressed
void change_cities(GtkWidget* /*widget*/, ezgl::application &app) {
   app.create_popup_message("Please wait", "Loading the new map...");
   app.refresh_drawing();
   GObject* map_name_entry = app.get_object("cities");
   const gchar* city_text= gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(map_name_entry));
   std::string city_name_entered(city_text);
   
   std::string map_path = "/cad2/ece297s/public/maps/" + city_name_entered + ".streets.bin";
   std::ifstream ifile;
   ifile.open(map_path);
   if (!ifile) {
      app.create_popup_message("Change Map Error", 
      "Not able to change the map,\nEither because the map is not in our database or because you entered the wrong name\nPlease make sure you entered your map in the following format:\ncity-name_country_name\ne.g. new-york_usa");
      ifile.close();
      return;
   }

   ifile.close();
   closeMap();
   reload_map = true;

   highlight_global.from_highlighted = false;
   highlight_global.to_highlighted = false;
   highlight_global.from_id = -1;
   highlight_global.to_id = -1;
   highlight_global.all_intersections = false;
   highlight_global.two_streets_intersections.clear();
   highlight_global.single_street_searched = -1;

   found_path_global.found_path.clear();
   found_path_global.draw_found_path = false;

   GObject *text = app.get_object("DirectionText");
   gtk_label_set_text(GTK_LABEL(text), "");
   GObject *from_box = app.get_object("FromBox");
   GObject *to_box = app.get_object("ToBox");
   std::string from_text, to_text;

   from_text = "From: ";

   const char* from_c_text = from_text.c_str();

   to_text = "To: ";   
   const char* to_c_text = to_text.c_str();  

   gtk_entry_set_text(GTK_ENTRY(from_box), from_c_text);
   gtk_entry_set_text(GTK_ENTRY(to_box), to_c_text);

   GObject *entry_bar = app.get_object("SearchEntry");

   GtkEntry *entry = GTK_ENTRY(entry_bar);
   gtk_entry_set_text(entry, "");



   bool load_new_map = loadMap(map_path);
   if (!load_new_map) {
      app.create_popup_message("Change Map Error", 
      "Not able to change the map,\neither because the map is not in our database or because you entered the wrong name\nplease make sure you entered your map in the following format:\ncity-name_country_name\ne.g. new-york_usa");
      return;
   }

   else {
      app.create_popup_message("Successful!", "New map loaded");
      reload_map = true;
      drawMapHelper(app, city_name_entered);
   }
}

// Called when the search button is pressed
// Will center the screen to the object that was searched
void centre_on(GtkWidget* /*widget*/, ezgl::application *app){
   GObject *temp = app->get_object("ResultBox");
   gchar *curr = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(temp));
   if (curr == NULL){
      std::cout << "Empty";
   }
   else{
      StreetIdx from = findStreetIdsFromPartialStreetName(curr)[0];
      LatLon square = getIntersectionPosition(from);
      float x = x_from_lon(square.longitude());
      float y = y_from_lat(square.latitude());
      ezgl::point2d top{x - 500, y - 500};
      ezgl::point2d bottom{x + 500, y + 500};
      ezgl::rectangle newVis{top, bottom};
      app -> get_renderer() -> set_visible_world(newVis);
      app -> refresh_drawing();
   }
}


// Ececuted when the item in the drop down menu is chosen
// After user changes their input in the entry
void searching(GtkWidget* /*widget*/, ezgl::application *app) {
//   if (!search_activated)  return;
   GObject *result_temp = app -> get_object("ResultBox");
   GtkComboBox* result_box = GTK_COMBO_BOX(result_temp);

   int active_index = gtk_combo_box_get_active(result_box);
   // Ignore the event if the active index is negative or the text in the combo box doesn't match the active item
   // Won't be triggered when the combo box is cleared
   if (active_index < 0) {
      return;
   }

   const char* active_id = gtk_combo_box_get_active_id(result_box);
   std::string active_id_string(active_id);

   // when there are two streets
   if (active_id_string.find(" ") != std::string::npos) {
      std::stringstream ss(active_id_string);
      StreetIdx firstStreet, secondStreet;
      ss >> firstStreet >> secondStreet;
      highlight_global.two_streets_intersections = findIntersectionsOfTwoStreets(firstStreet, secondStreet);
      highlight_global.all_intersections = true;

      // Can't handle if there are more than one intersection
      if (highlight_global.two_streets_intersections.size() > 1) {
         app -> create_popup_message ("Sorry!", "Multiple intersections are found.\nCan you please click on the intersection on the map to specify that intersection?");
         return;
      }


      else {
         std::string window_text_string = "The selected intersection is: " + intersections[highlight_global.two_streets_intersections[0]].name +".";     
         const char *dialog_title = "Declare the Intersection";
         const char *window_text = window_text_string.c_str();

         const gchar *window_temp = "MainWindow";
         GtkWindow* window = GTK_WINDOW(app -> get_object(window_temp));

         const gint GTK_RESPONSE_FROM = -4;
         const gint GTK_RESPONSE_TO = -5;
         const gint GTK_RESPONSE_CANCEL = -6;

         GtkWidget* dialog = gtk_dialog_new_with_buttons(
            dialog_title,   //title
            window,         //window
            GTK_DIALOG_MODAL, //Button and return_id pairs
            ("FROM"),
            GTK_RESPONSE_FROM,
            ("TO"),
            GTK_RESPONSE_TO,
            ("CANCEL"),
            GTK_RESPONSE_CANCEL, 
            NULL
         );

         auto content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
         GtkWidget* label = gtk_label_new(window_text);
         gtk_container_add(GTK_CONTAINER(content_area), label);


         UserData* user_data = new UserData{app, highlight_global.two_streets_intersections[0]};
         g_signal_connect(
            GTK_DIALOG(dialog), 
            "response", 
            G_CALLBACK(dialog_response_callback), 
            user_data
         );
         gtk_widget_show_all(dialog);
         app -> refresh_drawing();
      }
   }

   // when only one street is input (no "and")
   else {
      std::stringstream ss (active_id_string);
      ss >> highlight_global.single_street_searched;
      app -> refresh_drawing();
   }

   
}

// Clicking on any place on the screen will popup a window providing the information 
// of the nearest intersection
// User can set that intersection as either start or end for path searching
void act_on_mouse_click(ezgl::application *app,
                        GdkEventButton*  /* event */ ,
                        double x, double y){

   LatLon pos = LatLon(lat_from_y(y), lon_from_x(x));
   IntersectionIdx id_ = findClosestIntersection(pos);
   std::string window_text_string = "The closest intersection is: " + intersections[id_].name;
   const char *dialog_title = "Declare the Intersection";
   const char *window_text = window_text_string.c_str();


   const gchar *window_temp = "MainWindow";
   GtkWindow* window = GTK_WINDOW(app -> get_object(window_temp));

   const gint GTK_RESPONSE_FROM = -4;
   const gint GTK_RESPONSE_TO = -5;
   const gint GTK_RESPONSE_CANCEL = -6;

   GtkWidget* dialog = gtk_dialog_new_with_buttons(
      dialog_title,   //title
      window,         //window
      GTK_DIALOG_MODAL, //Button and return_id pairs
      ("FROM"),
      GTK_RESPONSE_FROM,
      ("TO"),
      GTK_RESPONSE_TO,
      ("CANCEL"),
      GTK_RESPONSE_CANCEL, 
      NULL
  );

   auto content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
   GtkWidget* label = gtk_label_new(window_text);
   gtk_container_add(GTK_CONTAINER(content_area), label);


   UserData* user_data = new UserData{app, id_};

   g_signal_connect(
      GTK_DIALOG(dialog), 
      "response", 
      G_CALLBACK(dialog_response_callback), 
      user_data
   );
   gtk_widget_show_all(dialog);

}


// Executed when the search_path button is pressed
// Will first check if the search is valid 
// ("i.e: Both from and to are input")
// Then it will ask whether user wants to search for path,
// and generate the path accordingly
void activate_search_path(GtkWidget* /*widget*/, ezgl::application *app) {
   if (!highlight_global.from_highlighted || !highlight_global.to_highlighted) {
      app -> create_popup_message("Error", "Not able to find the path because you are missing either the starting or ending point");
      return;
   }

   std::string window_text_string = "Start finding the path from " + intersections[highlight_global.from_id].name
                              + " to " + intersections[highlight_global.to_id].name + ".";
   const char *dialog_title = "Proceed to Path Finding?";
   const char *window_text = window_text_string.c_str();


   const gchar *window_temp = "MainWindow";
   GtkWindow* window = GTK_WINDOW(app -> get_object(window_temp));
   const gint GTK_RESPONSE_YES = -4;
   const gint GTK_RESPONSE_NO = -5;
   GtkWidget* dialog = gtk_dialog_new_with_buttons(
      dialog_title,   //title
      window,         //window
      GTK_DIALOG_MODAL, //Button and return_id pairs
      ("YES"),
      GTK_RESPONSE_YES,
      ("NO"),
      GTK_RESPONSE_NO,
      NULL
   );

   auto content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
   GtkWidget* label = gtk_label_new(window_text);
   gtk_container_add(GTK_CONTAINER(content_area), label);

   g_signal_connect(
      GTK_DIALOG(dialog), 
      "response", 
      G_CALLBACK(search_path_response_callback), 
      app
   );
   gtk_widget_show_all(dialog);
   directShow = true;
   updateDirections(NULL, app);

}



// Called by activate_search_path
// Safe to assume that this function will not be called
// if either "from" or "to" is missing
void search_path_response_callback(GtkDialog *dialog, gint response_id, gpointer user_data) {
   auto app = static_cast<ezgl::application*>(user_data);
   double turn_penalty = 20;
   const gint GTK_RESPONSE_YES = -4;
   const gint GTK_RESPONSE_NO = -5;
   bool has_path = true;
   switch (response_id) {
      case GTK_RESPONSE_YES: 
      {    
         // Handle the YES button press for this specific dialog ID
         std::pair<IntersectionIdx, IntersectionIdx> from_to = {highlight_global.from_id, highlight_global.to_id};
         std::vector<StreetSegmentIdx> path_found = findPathBetweenIntersections(from_to, turn_penalty);     
         if (path_found.size() == 0) {
            has_path = false;
         }
         break;
      }

      case GTK_RESPONSE_NO:
      {
         // Handle the NO button press for this specific dialog ID
         gtk_widget_destroy(GTK_WIDGET(dialog));
         return;
      }

      default:
      {  
         gtk_widget_destroy(GTK_WIDGET(dialog));
         return;
      }
         
   }


   gtk_widget_destroy(GTK_WIDGET(dialog));

   // handle the cases if no path is found
   if (!has_path) {
      app -> create_popup_message("Error!", "No path is found between two intersections!");
      return;
   }

   GObject *text = app ->get_object("DirectionText");
   std::string result = std::accumulate(instructions.begin(), instructions.end(), std::string(""));
   const char* text_c = result.c_str();

   // Set the directions into the gtk_label
   gtk_label_set_text(GTK_LABEL(text), text_c);

   app -> refresh_drawing();

}


// only called when the searching is updated
// Either executing searching or mouse clicking
// only called when the searching is updated
// Either executing searching or mouse clicking
void dialog_response_callback(GtkDialog *dialog, gint response_id, gpointer user_data) {

   UserData* data = static_cast<UserData*>(user_data);
   ezgl::application* app = data -> app;
   IntersectionIdx _id_ = data -> id_;
   const gint GTK_RESPONSE_FROM = -4;
   const gint GTK_RESPONSE_TO = -5;
   const gint GTK_RESPONSE_CANCEL = -6;
   switch (response_id) {
      case GTK_RESPONSE_FROM:
      {      
         // Handle the FROM button press for this specific dialog ID
         highlight_global.from_id = _id_;
         highlight_global.from_highlighted = true;
         break;
      }
      case GTK_RESPONSE_TO:
      {
         // Handle the TO button press for this specific dialog ID
         highlight_global.to_id = _id_;
         highlight_global.to_highlighted = true;
         break;
      }   
      case GTK_RESPONSE_CANCEL:
      {      
         // Handle the CANCEL button press for this specific dialog ID
         delete data;
         gtk_widget_destroy(GTK_WIDGET(dialog));
         return;
      }

      default:
      {
         // Handle other button press events, if any

         break;
      }
   }

   delete data;
   gtk_widget_destroy(GTK_WIDGET(dialog));

   GObject *from_box = app -> get_object("FromBox");
   GObject *to_box = app-> get_object("ToBox");

   std::string from_text, to_text;

   if (found_path_global.draw_found_path) {
      found_path_global.draw_found_path = false;
      found_path_global.found_path.clear();
   }
   // empty the direction text
   GObject *direction_text = app ->get_object("DirectionText");
   gtk_label_set_text(GTK_LABEL(direction_text), "");

   if (highlight_global.from_id == -1)     from_text = "From: ";
   else {
      from_text = "From: " + getIntersectionName(highlight_global.from_id) + "   ";
   }
   const char* from_c_text = from_text.c_str();

   if (highlight_global.to_id == -1)    to_text = "To: ";   
   else {
      to_text = "To: " + getIntersectionName(highlight_global.to_id);
   }
   const char* to_c_text = to_text.c_str();  

   gtk_entry_set_text(GTK_ENTRY(from_box), from_c_text);
   gtk_entry_set_text(GTK_ENTRY(to_box), to_c_text);



   app -> refresh_drawing();
}



// Image initializers
ezgl::surface* subway_stations;
ezgl::surface* to_from_intersections;

ezgl::surface* poi_image;

void draw_main_canvas(ezgl::renderer *g) {
   // Set the background to grey (245)
   g -> set_color(138, 138, 138);
   g -> draw_rectangle({0, 0}, {1500, 1500});
   g -> fill_rectangle({0, 0}, {1500, 1500});

   // Default at 0.0108933 when first opening map
   ezgl::rectangle visible_screen = g -> get_visible_screen();

   ezgl::rectangle visible_world = g -> get_visible_world();   
   double zoom_scale = (visible_screen.height()) / (visible_world.height());

   // Default line width
   g -> set_line_width(zoom_scale * 10);
   g -> set_color(0, 0, 0);

   // if (g -> get_visible_world().contains(ezgl::2dpoint))
   bool contains_feature_curve_point;


   // Commented out the curve points check because 
   // of edge cases (St. Helena)
   // Bunch of things disappear
   // Keep them for now
   
   // Draw the lakes regardless
   for (int i = 0; i < features_lake.size(); ++i) {
      // contains_feature_curve_point = false;
      // for (int p = 0; p < (features_lake[i].feature_curve_points).size(); ++p) {
      //    if (g -> get_visible_world().contains((features_lake[i].feature_curve_points)[p])) {
      //       contains_feature_curve_point = true;
      //       break;
      //    }
      // }

      // && (contains_feature_curve_point == true)
      if ((features_lake[i].feature_num_curve_points > 1)) {
         g -> set_color(colorType[show_elements_global.dark][LAKE][0], colorType[show_elements_global.dark][LAKE][1], colorType[show_elements_global.dark][LAKE][2]);
         g -> fill_poly(features_lake[i].feature_curve_points);
      }
   }
   
   // Draw the island
   for (int i = 0; i < features_island.size(); ++i) {
      // contains_feature_curve_point = false;
      // for (int p = 0; p < (features_island[i].feature_curve_points).size(); ++p) {
      //    if (g -> get_visible_world().contains((features_island[i].feature_curve_points)[p])) {
      //       contains_feature_curve_point = true;
      //       break;
      //    }
      // }

      //  && (contains_feature_curve_point == true)
      if ((features_island[i].feature_num_curve_points > 1)) {
         g -> set_color(colorType[show_elements_global.dark][0][0], colorType[show_elements_global.dark][0][1], colorType[show_elements_global.dark][0][2]);
         g -> fill_poly(features_island[i].feature_curve_points);
      }

      // Check if there are any lakes smaller than the current island
      for (int j = 0; j < features_lake.size(); ++j) {
         // contains_feature_curve_point = false;
         // for (int p = 0; p < (features_lake[i].feature_curve_points).size(); ++p) {
         //    if (g -> get_visible_world().contains((features_lake[i].feature_curve_points)[p])) {
         //       contains_feature_curve_point = true;
         //       break;
         //    }
         // }

         // && (contains_feature_curve_point == true)
         if ((features_lake[j].feature_num_curve_points > 1) && (features_island[i].feature_area > features_lake[j].feature_area)) {
            g -> set_color(colorType[show_elements_global.dark][LAKE][0], colorType[show_elements_global.dark][LAKE][1], colorType[show_elements_global.dark][LAKE][2]);
            g -> fill_poly(features_lake[j].feature_curve_points);
         }
      }
   }

   // Draw the greenspace regardless
   for (int i = 0; i < features_greenspace.size(); ++i) {
      // contains_feature_curve_point = false;
      // for (int p = 0; p < (features_greenspace[i].feature_curve_points).size(); ++p) {
      //    if (g -> get_visible_world().contains((features_greenspace[i].feature_curve_points)[p])) {
      //       contains_feature_curve_point = true;
      //       break;
      //    }
      // }

      // && (contains_feature_curve_point == true)
      if ((features_greenspace[i].feature_num_curve_points > 1)) {
         g -> set_color(colorType[show_elements_global.dark][GREENSPACE][0], colorType[show_elements_global.dark][GREENSPACE][1], colorType[show_elements_global.dark][GREENSPACE][2]);
         g -> fill_poly(features_greenspace[i].feature_curve_points);
      }
   }

   if (zoom_scale > 0.001) { 
      for (int i = 0; i < features_park.size(); ++i) {
         // Iterate through all curve points, checking if they are in the screen
         // contains_feature_curve_point = false;
         // for (int p = 0; p < (features_park[i].feature_curve_points).size(); ++p) {
         //    if (g -> get_visible_world().contains((features_park[i].feature_curve_points)[p])) {
         //       contains_feature_curve_point = true;
         //       break;
         //    }
         // }

         // && (contains_feature_curve_point == true)
         if ((features_park[i].feature_num_curve_points > 1)) {
            g -> set_color(colorType[show_elements_global.dark][PARK][0], colorType[show_elements_global.dark][PARK][1], colorType[show_elements_global.dark][PARK][2]);
            g -> fill_poly(features_park[i].feature_curve_points);
         }
      }
   }
   
   // Draw the glacier
   if (zoom_scale > 0.001) { 
      for (int i = 0; i < features_glacier.size(); ++i) {
         // contains_feature_curve_point = false;
         // for (int p = 0; p < (features_glacier[i].feature_curve_points).size(); ++p) {
         //    if (g -> get_visible_world().contains((features_glacier[i].feature_curve_points)[p])) {
         //       contains_feature_curve_point = true;
         //       break;
         //    }
         // }

         //  && (contains_feature_curve_point == true)
         if ((features_glacier[i].feature_num_curve_points > 1)) {
            g -> set_color(colorType[show_elements_global.dark][GLACIER][0], colorType[show_elements_global.dark][GLACIER][1], colorType[show_elements_global.dark][GLACIER][2]);
            g -> fill_poly(features_glacier[i].feature_curve_points);
         }
      }      
   } 
   
   if (zoom_scale > 0.01) {
      // Draw the beach
      for (int i = 0; i < features_beach.size(); ++i) {
         // contains_feature_curve_point = false;
         // for (int p = 0; p < (features_beach[i].feature_curve_points).size(); ++p) {
         //    if (g -> get_visible_world().contains((features_beach[i].feature_curve_points)[p])) {
         //       contains_feature_curve_point = true;
         //       break;
         //    }
         // }

         //  && (contains_feature_curve_point == true)
         if ((features_beach[i].feature_num_curve_points > 1)) {
            g -> set_color(colorType[show_elements_global.dark][BEACH][0], colorType[show_elements_global.dark][BEACH][1], colorType[show_elements_global.dark][BEACH][2]);
            g -> fill_poly(features_beach[i].feature_curve_points);
         }
      }

      // Draw golfcourse
      for (int i = 0; i < features_golfcourse.size(); ++i) {
         contains_feature_curve_point = false;
         for (int p = 0; p < (features_golfcourse[i].feature_curve_points).size(); ++p) {
            if (g -> get_visible_world().contains((features_golfcourse[i].feature_curve_points)[p])) {
               contains_feature_curve_point = true;
               break;
            }
         }
         if ((features_golfcourse[i].feature_num_curve_points > 1) && (contains_feature_curve_point == true)) {
            g -> set_color(colorType[show_elements_global.dark][GOLFCOURSE][0], colorType[show_elements_global.dark][GOLFCOURSE][1], colorType[show_elements_global.dark][GOLFCOURSE][2]);
            g -> fill_poly(features_golfcourse[i].feature_curve_points);
         }
      }

      // Draw rivers
      for (int i = 0; i < features_river.size(); ++i) {
         contains_feature_curve_point = false;
         for (int p = 0; p < (features_river[i].feature_curve_points).size(); ++p) {
            if (g -> get_visible_world().contains((features_river[i].feature_curve_points)[p])) {
               contains_feature_curve_point = true;
               break;
            }
         }
         if ((features_river[i].feature_num_curve_points > 1) && (contains_feature_curve_point == true)) {
            g -> set_color(colorType[show_elements_global.dark][RIVER][0], colorType[show_elements_global.dark][RIVER][1], colorType[show_elements_global.dark][RIVER][2]);
            g -> fill_poly(features_river[i].feature_curve_points);
         }
      }
   } 


   // The buildings are displayed only depending on the zoom_scale
   if (zoom_scale > 0.5) { 
      for (int i = 0; i < features_building.size(); ++i) {
         contains_feature_curve_point = false;
         for (int p = 0; p < (features_building[i].feature_curve_points).size(); ++p) {
            if (g -> get_visible_world().contains((features_building[i].feature_curve_points)[p])) {
               contains_feature_curve_point = true;
               break;
            }
         }
         if ((features_building[i].feature_num_curve_points > 1) && (contains_feature_curve_point == true)) {
            g -> set_color(colorType[show_elements_global.dark][BUILDING][0], colorType[show_elements_global.dark][BUILDING][1], colorType[show_elements_global.dark][BUILDING][2]);
            g -> fill_poly(features_building[i].feature_curve_points);
         }
      }
   }

   // Draw streets
   g -> set_font_size(10);
   bool contains_segment_curve_point;

   if (zoom_scale > 1) {
      g -> set_line_width(zoom_scale * 5); // Narrower for small roads
      //g -> set_line_dash(ezgl::line_dash{4.0, 4.0}); // For small roads, make it dotted
      
      for (int i = 0; i < num_zoom_small; ++i) {
         g -> set_color(192, 192, 192); // Silver
         // Only load the segment if it is on the screen
         contains_segment_curve_point = false;
         // Check if any of the curve points are on the screen
         for (int p = 0; p < (segments_zoom_small[i].seg_curve_points).size(); ++p) {
            if (g -> get_visible_world().contains((segments_zoom_small[i].seg_curve_points)[p])) {
               contains_segment_curve_point = true;
               break;
            }
         }
         if ((g -> get_visible_world().contains(segments_zoom_small[i].from_xy)) || (g -> get_visible_world().contains(segments_zoom_small[i].to_xy)) || contains_segment_curve_point) {
            g -> set_color(128, 0, 0); // Maroon   
            if (segments_zoom_small[i].num_curve_p > 0) {
               g -> draw_line(segments_zoom_small[i].from_xy, (segments_zoom_small[i].seg_curve_points)[0]);
               g -> fill_arc((segments_zoom_small[i].seg_curve_points)[0], 3, 0, 360);
               
               for (int j = 0; j < (segments_zoom_small[i].num_curve_p - 1); ++j) {
                  g -> draw_line((segments_zoom_small[i].seg_curve_points)[j], (segments_zoom_small[i].seg_curve_points)[j + 1]);
                  g -> fill_arc((segments_zoom_small[i].seg_curve_points)[j + 1], 3, 0, 360);
               }

               g -> draw_line((segments_zoom_small[i].seg_curve_points)[(segments_zoom_small[i].num_curve_p) - 1], segments_zoom_small[i].to_xy);
               g -> fill_arc(segments_zoom_small[i].to_xy, 3, 0, 360);
            } else {
               g -> draw_line(segments_zoom_small[i].from_xy, segments_zoom_small[i].to_xy);
               g -> fill_arc(segments_zoom_small[i].to_xy, 3, 0, 360);
            }
         }
      } 
   } if (zoom_scale > 0.2) {
      g -> set_line_width(zoom_scale * 8); // Narrower for small roads

      for (int i = 0; i < num_zoom_0; ++i) { 
         g -> set_color(140, 140, 140); // Light gray
         contains_segment_curve_point = false;
         for (int p = 0; p < (segments_zoom_0[i].seg_curve_points).size(); ++p) {
            if (g -> get_visible_world().contains((segments_zoom_0[i].seg_curve_points)[p])) {
               contains_segment_curve_point = true;
               break;
            }
         }
         if ((g -> get_visible_world().contains(segments_zoom_0[i].from_xy)) || (g -> get_visible_world().contains(segments_zoom_0[i].to_xy)) || contains_segment_curve_point) {
            if (segments_zoom_0[i].num_curve_p > 0) {
               g -> draw_line(segments_zoom_0[i].from_xy, (segments_zoom_0[i].seg_curve_points)[0]);
               g -> fill_arc(segments_zoom_0[i].from_xy, 4, 0, 360);
               g -> fill_arc((segments_zoom_0[i].seg_curve_points)[0], 4, 0, 360);
               
               for (int j = 0; j < (segments_zoom_0[i].num_curve_p - 1); ++j) {
                  g -> draw_line((segments_zoom_0[i].seg_curve_points)[j], (segments_zoom_0[i].seg_curve_points)[j + 1]);
                  g -> fill_arc((segments_zoom_0[i].seg_curve_points)[j], 4, 0, 360);
                  g -> fill_arc((segments_zoom_0[i].seg_curve_points)[j + 1], 4, 0, 360);
               }

               g -> draw_line((segments_zoom_0[i].seg_curve_points)[(segments_zoom_0[i].num_curve_p) - 1], segments_zoom_0[i].to_xy);
               g -> fill_arc((segments_zoom_0[i].seg_curve_points)[(segments_zoom_0[i].num_curve_p) - 1], 4, 0, 360);
               g -> fill_arc(segments_zoom_0[i].to_xy, 4, 0, 360);
            } else {
               g -> draw_line(segments_zoom_0[i].from_xy, segments_zoom_0[i].to_xy);
               g -> fill_arc(segments_zoom_0[i].from_xy, 4, 0, 360);
               g -> fill_arc(segments_zoom_0[i].to_xy, 4, 0, 360);
            }
         }
      }      
   } if (zoom_scale > 0.08) {
      g -> set_line_width(zoom_scale * 10);

      for (int i = 0; i < num_zoom_1; ++i) { 
         g -> set_color(118, 118, 118); // Gray
         contains_segment_curve_point = false;
         for (int p = 0; p < (segments_zoom_1[i].seg_curve_points).size(); ++p) {
            if (g -> get_visible_world().contains((segments_zoom_1[i].seg_curve_points)[p])) {
               contains_segment_curve_point = true;
               break;
            }
         }
         if ((g -> get_visible_world().contains(segments_zoom_1[i].from_xy)) || (g -> get_visible_world().contains(segments_zoom_1[i].to_xy)) || contains_segment_curve_point) {
            if (segments_zoom_1[i].num_curve_p > 0) {
               g -> draw_line(segments_zoom_1[i].from_xy, (segments_zoom_1[i].seg_curve_points)[0]);
               g -> fill_arc(segments_zoom_1[i].from_xy, 5, 0, 360);
               g -> fill_arc((segments_zoom_1[i].seg_curve_points)[0], 5, 0, 360);
               
               for (int j = 0; j < (segments_zoom_1[i].num_curve_p - 1); ++j) {
                  g -> draw_line((segments_zoom_1[i].seg_curve_points)[j], (segments_zoom_1[i].seg_curve_points)[j + 1]);
                  g -> fill_arc((segments_zoom_1[i].seg_curve_points)[j], 5, 0, 360);
                  g -> fill_arc((segments_zoom_1[i].seg_curve_points)[j + 1], 5, 0, 360);
               }

               g -> draw_line((segments_zoom_1[i].seg_curve_points)[(segments_zoom_1[i].num_curve_p) - 1], segments_zoom_1[i].to_xy);
               g -> fill_arc((segments_zoom_1[i].seg_curve_points)[(segments_zoom_1[i].num_curve_p) - 1], 5, 0, 360);
               g -> fill_arc(segments_zoom_1[i].to_xy, 5, 0, 360);
            } else {
               g -> draw_line(segments_zoom_1[i].from_xy, segments_zoom_1[i].to_xy);
               g -> fill_arc(segments_zoom_1[i].from_xy, 5, 0, 360);
               g -> fill_arc(segments_zoom_1[i].to_xy, 5, 0, 360);
            }
         }
      }
   } if (zoom_scale > 0.03) {
      g -> set_line_width(zoom_scale * 12); // Wider for main roads

      for (int i = 0; i < num_zoom_2; ++i) {   
         g -> set_color(128, 128, 0); // Olive yellow
         contains_segment_curve_point = false;
         for (int p = 0; p < (segments_zoom_2[i].seg_curve_points).size(); ++p) {
            if (g -> get_visible_world().contains((segments_zoom_2[i].seg_curve_points)[p])) {
               contains_segment_curve_point = true;
               break;
            }
         }
         if ((g -> get_visible_world().contains(segments_zoom_2[i].from_xy)) || (g -> get_visible_world().contains(segments_zoom_2[i].to_xy)) || contains_segment_curve_point) { 
            if (segments_zoom_2[i].num_curve_p > 0) {
               g -> draw_line(segments_zoom_2[i].from_xy, (segments_zoom_2[i].seg_curve_points)[0]);
               g -> fill_arc(segments_zoom_2[i].from_xy, 6, 0, 360);
               g -> fill_arc((segments_zoom_2[i].seg_curve_points)[0], 6, 0, 360);
               
               for (int j = 0; j < (segments_zoom_2[i].num_curve_p - 1); ++j) {
                  g -> draw_line((segments_zoom_2[i].seg_curve_points)[j], (segments_zoom_2[i].seg_curve_points)[j + 1]);
                  g -> fill_arc((segments_zoom_2[i].seg_curve_points)[j], 6, 0, 360);
                  g -> fill_arc((segments_zoom_2[i].seg_curve_points)[j + 1], 6, 0, 360);
               }

               g -> draw_line((segments_zoom_2[i].seg_curve_points)[(segments_zoom_2[i].num_curve_p) - 1], segments_zoom_2[i].to_xy);
               g -> fill_arc((segments_zoom_2[i].seg_curve_points)[(segments_zoom_2[i].num_curve_p) - 1], 6, 0, 360);
               g -> fill_arc(segments_zoom_2[i].to_xy, 6, 0, 360);
            } else {
               g -> draw_line(segments_zoom_2[i].from_xy, segments_zoom_2[i].to_xy);
               g -> fill_arc(segments_zoom_2[i].from_xy, 6, 0, 360);
               g -> fill_arc(segments_zoom_2[i].to_xy, 6, 0, 360);
            }
         }
      }
   } if (zoom_scale > 0.01) {
      g -> set_line_width(zoom_scale * 15); // Wider for main roads

      for (int i = 0; i < num_zoom_3; ++i) {
         g -> set_color(215, 135, 95); // Light salmon
         contains_segment_curve_point = false;
         for (int p = 0; p < (segments_zoom_3[i].seg_curve_points).size(); ++p) {
            if (g -> get_visible_world().contains((segments_zoom_3[i].seg_curve_points)[p])) {
               contains_segment_curve_point = true;
               break;
            }
         }
         if ((g -> get_visible_world().contains(segments_zoom_3[i].from_xy)) || (g -> get_visible_world().contains(segments_zoom_3[i].to_xy)) || contains_segment_curve_point) {
            if (segments_zoom_3[i].num_curve_p > 0) {
               g -> draw_line(segments_zoom_3[i].from_xy, (segments_zoom_3[i].seg_curve_points)[0]);
               g -> fill_arc(segments_zoom_3[i].from_xy, 7.3, 0, 360);
               g -> fill_arc((segments_zoom_3[i].seg_curve_points)[0], 7.3, 0, 360);
               
               for (int j = 0; j < (segments_zoom_3[i].num_curve_p - 1); ++j) {
                  g -> draw_line((segments_zoom_3[i].seg_curve_points)[j], (segments_zoom_3[i].seg_curve_points)[j + 1]);
                  g -> fill_arc((segments_zoom_3[i].seg_curve_points)[j], 7.3, 0, 360);
                  g -> fill_arc((segments_zoom_3[i].seg_curve_points)[j + 1], 7.3, 0, 360);
               }

               g -> draw_line((segments_zoom_3[i].seg_curve_points)[(segments_zoom_3[i].num_curve_p) - 1], segments_zoom_3[i].to_xy);
               g -> fill_arc((segments_zoom_3[i].seg_curve_points)[(segments_zoom_3[i].num_curve_p) - 1], 7.3, 0, 360);
               g -> fill_arc(segments_zoom_3[i].to_xy, 7.3, 0, 360);
            } else {
               g -> draw_line(segments_zoom_3[i].from_xy, segments_zoom_3[i].to_xy);
               g -> fill_arc(segments_zoom_3[i].from_xy, 7.3, 0, 360);
               g -> fill_arc(segments_zoom_3[i].to_xy, 7.3, 0, 360);
            }
         }
      }
   } if (zoom_scale > 0.001) {
      g -> set_line_width(zoom_scale * 20); // Wider for main roads

      for (int i = 0; i < num_zoom_4; ++i) {
         g -> set_color(215, 0, 95); // Deep pinkish red
         contains_segment_curve_point = false;
         for (int p = 0; p < (segments_zoom_4[i].seg_curve_points).size(); ++p) {
            if (g -> get_visible_world().contains((segments_zoom_4[i].seg_curve_points)[p])) {
               contains_segment_curve_point = true;
               break;
            }
         }
         if ((g -> get_visible_world().contains(segments_zoom_4[i].from_xy)) || (g -> get_visible_world().contains(segments_zoom_4[i].to_xy)) || contains_segment_curve_point) {
            if (segments_zoom_4[i].num_curve_p > 0) {
               g -> draw_line(segments_zoom_4[i].from_xy, (segments_zoom_4[i].seg_curve_points)[0]);
               g -> fill_arc(segments_zoom_4[i].from_xy, 9.7, 0, 360);
               g -> fill_arc((segments_zoom_4[i].seg_curve_points)[0], 9.7, 0, 360);
               
               for (int j = 0; j < (segments_zoom_4[i].num_curve_p - 1); ++j) {
                  g -> draw_line((segments_zoom_4[i].seg_curve_points)[j], (segments_zoom_4[i].seg_curve_points)[j + 1]);
                  g -> fill_arc((segments_zoom_4[i].seg_curve_points)[j], 9.7, 0, 360);
                  g -> fill_arc((segments_zoom_4[i].seg_curve_points)[j + 1], 9.7, 0, 360);
               }

               g -> draw_line((segments_zoom_4[i].seg_curve_points)[(segments_zoom_4[i].num_curve_p) - 1], segments_zoom_4[i].to_xy);
               g -> fill_arc((segments_zoom_4[i].seg_curve_points)[(segments_zoom_4[i].num_curve_p) - 1], 9.7, 0, 360);
               g -> fill_arc(segments_zoom_4[i].to_xy, 9.7, 0, 360);
            } else { // No curves
               g -> draw_line(segments_zoom_4[i].from_xy, segments_zoom_4[i].to_xy);
               g -> fill_arc(segments_zoom_4[i].from_xy, 9.7, 0, 360);
               g -> fill_arc(segments_zoom_4[i].to_xy, 9.7, 0, 360);
            }
         }
      }
   } 


   // highlight the entire searched street
   // segments_of_streets[highlight_global.single_street_searched]
   // is the vector of all segment ids
   // std::unordered_map<StreetIdx, std::vector <StreetSegmentIdx>> segments_of_streets;
   if (highlight_global.single_street_searched > -1 && !highlight_global.from_highlighted && !highlight_global.to_highlighted) {
      g -> set_line_width(zoom_scale * 3);
      g -> set_color(255, 0, 0);

      // Draw each street segment
      // segments[highlight_global.single_street_searched][i]
      for (int i = 0; i < segments_of_streets[highlight_global.single_street_searched].size(); ++i) {
         if (segments[segments_of_streets[highlight_global.single_street_searched][i]].num_curve_p > 0) {
            g -> draw_line(segments[segments_of_streets[highlight_global.single_street_searched][i]].from_xy, 
                  (segments[segments_of_streets[highlight_global.single_street_searched][i]].seg_curve_points)[0]);
            
            for (int j = 0; j < (segments[segments_of_streets[highlight_global.single_street_searched][i]].num_curve_p - 1); ++j) {
               g -> draw_line((segments[segments_of_streets[highlight_global.single_street_searched][i]].seg_curve_points)[j], 
                           (segments[segments_of_streets[highlight_global.single_street_searched][i]].seg_curve_points)[j + 1]);
            }

            g -> draw_line(segments[segments_of_streets[highlight_global.single_street_searched][i]].seg_curve_points
                           [(segments[segments_of_streets[highlight_global.single_street_searched][i]].num_curve_p) - 1], 
                           segments[segments_of_streets[highlight_global.single_street_searched][i]].to_xy);
         } else { // No curves
            g -> draw_line(segments[segments_of_streets[highlight_global.single_street_searched][i]].from_xy, 
                           segments[segments_of_streets[highlight_global.single_street_searched][i]].to_xy);
         }
      }
   }

   // Draw found path
   if (found_path_global.draw_found_path == true) {
      g -> set_line_width(8);

      for (int i = 0; i < (found_path_global.found_path).size(); ++i) {
         g -> set_color(0, 191, 255); // Deep sky blue
         // contains_segment_curve_point = false;
         // for (int p = 0; p < ((found_path_global.found_path)[i].seg_curve_points).size(); ++p) {
         //    if (g -> get_visible_world().contains(((found_path_global.found_path)[i].seg_curve_points)[p])) {
         //       contains_segment_curve_point = true;
         //       break;
         //    }
         // }
         // if ((g -> get_visible_world().contains(segments_zoom_4[i].from_xy)) || (g -> get_visible_world().contains(segments_zoom_4[i].to_xy))) {
         StreetSegmentInfo segment_info = getStreetSegmentInfo((found_path_global.found_path)[i]);
         int num_curve = segment_info.numCurvePoints;
         IntersectionIdx segment_to = segment_info.to;
         IntersectionIdx segment_from = segment_info.from;

         std::vector<ezgl::point2d> curve_points;
         curve_points.resize(num_curve);
         for (int k = 0; k < num_curve; ++k) {
            LatLon temp_latlon = getStreetSegmentCurvePoint(found_path_global.found_path[i], k);
            float temp_x = x_from_lon(temp_latlon.longitude());
            float temp_y = y_from_lat(temp_latlon.latitude());
            ezgl::point2d temp_xy(temp_x, temp_y);
            curve_points[k] = temp_xy;
         }
         
         LatLon from_xy_latlon = getIntersectionPosition(segment_from);
         float from_x = x_from_lon(from_xy_latlon.longitude());
         float from_y = y_from_lat(from_xy_latlon.latitude());
         ezgl::point2d from_xy(from_x, from_y);

         LatLon to_xy_latlon = getIntersectionPosition(segment_to);
         float to_x = x_from_lon(to_xy_latlon.longitude());
         float to_y = y_from_lat(to_xy_latlon.latitude());
         ezgl::point2d to_xy(to_x, to_y);

         if (num_curve > 0) {
            g -> draw_line(from_xy, curve_points[0]);
            // g -> fill_arc(from_xy, 1 / zoom_scale, 0, 360);
            // g -> fill_arc(curve_points[0], 1 / zoom_scale, 0, 360);
            
            for (int j = 0; j < (num_curve - 1); ++j) {
               g -> draw_line(curve_points[j], curve_points[j + 1]);
               // g -> fill_arc(curve_points[j], 1 / zoom_scale, 0, 360);
               // g -> fill_arc(curve_points[j + 1], 1 / zoom_scale, 0, 360);
            }

            g -> draw_line(curve_points[num_curve - 1], to_xy);
            // g -> fill_arc(curve_points[num_curve - 1], 1 / zoom_scale, 0, 360);
            // g -> fill_arc(to_xy, 1 / zoom_scale, 0, 360);
         } else { // No curves
            g -> draw_line(from_xy, to_xy);
            // g -> fill_arc(from_xy, 1 / zoom_scale, 0, 360);
            // g -> fill_arc(to_xy, 1 / zoom_scale, 0, 360);
         }
         // }
      }
   }   

   // Draw street names and POI names
   if (zoom_scale > 0.001) {
      for (StreetSegmentIdx i = 0; i < num_zoom_4; ++i) {
         // Drawing street names, distancing of names based on zoom
         if ((g -> get_visible_world().contains(segments_zoom_4[i].from_xy)) || (g -> get_visible_world().contains(segments_zoom_4[i].to_xy))) {
            if (std::isnan((segments_zoom_4[i].angle) * (180.0 / M_PI)) == true) {
               continue;
            }
            g -> set_text_rotation((segments_zoom_4[i].angle) * (180.0 / M_PI));
            g -> set_color(240, 240, 240);       

            if (segments_zoom_4[i].street_name != "<unknown>") {
               if (segments_zoom_4[i].num_curve_p == 0) {
                  // Check if one way
                  std::string arrow;
                  if (segments_zoom_4[i].one_way == true) {
                     // Check direction of one way
                     if ((segments_zoom_4[i].from_xy).x < (segments_zoom_4[i].to_xy).x) {
                        arrow = " ===>";
                        g -> draw_text(segments_zoom_4[i].middle_xy, segments_zoom_4[i].street_name + arrow, segments_zoom_4[i].length, std::numeric_limits<double>::max());
                     } else if ((segments_zoom_4[i].from_xy).x > (segments_zoom_4[i].to_xy).x) {
                        arrow = "<=== ";
                        g -> draw_text(segments_zoom_4[i].middle_xy, arrow + segments_zoom_4[i].street_name, segments_zoom_4[i].length, std::numeric_limits<double>::max());   
                     }
                  } else {
                     g -> draw_text(segments_zoom_4[i].middle_xy, segments_zoom_4[i].street_name, segments_zoom_4[i].length, std::numeric_limits<double>::max());
                  }  
               } else {
                  // Check if one way
                  std::string arrow;
                  if (segments_zoom_4[i].one_way == true) {
                     // Check direction of one way
                     if ((segments_zoom_4[i].from_xy).x < (segments_zoom_4[i].to_xy).x) {
                        arrow = " ===>";
                        // Draw text at the middle curve point
                        g -> draw_text((segments_zoom_4[i].seg_curve_points)[segments_zoom_4[i].num_curve_p / 2], segments_zoom_4[i].street_name + arrow, segments_zoom_4[i].length, std::numeric_limits<double>::max());
                     } else if ((segments_zoom_4[i].from_xy).x > (segments_zoom_4[i].to_xy).x) {
                        arrow = "<=== ";
                        g -> draw_text((segments_zoom_4[i].seg_curve_points)[segments_zoom_4[i].num_curve_p / 2], arrow + segments_zoom_4[i].street_name, segments_zoom_4[i].length, std::numeric_limits<double>::max());   
                     }
                  } else {
                     g -> draw_text((segments_zoom_4[i].seg_curve_points)[segments_zoom_4[i].num_curve_p / 2], segments_zoom_4[i].street_name, segments_zoom_4[i].length, std::numeric_limits<double>::max());
                  } 
               }
            } 
         }
      }
   } if (zoom_scale > 0.01) {
      for (StreetSegmentIdx i = 0; i < num_zoom_3; ++i) {
         if ((g -> get_visible_world().contains(segments_zoom_3[i].from_xy)) || (g -> get_visible_world().contains(segments_zoom_3[i].to_xy))) {
            if (std::isnan((segments_zoom_3[i].angle) * (180.0 / M_PI)) == true) {
               continue;
            }
            g -> set_text_rotation((segments_zoom_3[i].angle) * (180.0 / M_PI));
            g -> set_color(240, 240, 240);       

            if (segments_zoom_3[i].street_name != "<unknown>") {
               if (segments_zoom_3[i].num_curve_p == 0) {
                  // Check if one way
                  std::string arrow;
                  if (segments_zoom_3[i].one_way == true) {
                     // Check direction of one way
                     if ((segments_zoom_3[i].from_xy).x < (segments_zoom_3[i].to_xy).x) {
                        arrow = " ===>";
                        g -> draw_text(segments_zoom_3[i].middle_xy, segments_zoom_3[i].street_name + arrow, segments_zoom_3[i].length, std::numeric_limits<double>::max());
                     } else if ((segments_zoom_3[i].from_xy).x > (segments_zoom_3[i].to_xy).x) {
                        arrow = "<=== ";
                        g -> draw_text(segments_zoom_3[i].middle_xy, arrow + segments_zoom_3[i].street_name, segments_zoom_3[i].length, std::numeric_limits<double>::max());   
                     }
                  } else {
                     g -> draw_text(segments_zoom_3[i].middle_xy, segments_zoom_3[i].street_name, segments_zoom_3[i].length, std::numeric_limits<double>::max());
                  }  
               } else {
                  // Check if one way
                  std::string arrow;
                  if (segments_zoom_3[i].one_way == true) {
                     // Check direction of one way
                     if ((segments_zoom_3[i].from_xy).x < (segments_zoom_3[i].to_xy).x) {
                        arrow = " ===>";
                        // Draw text at the middle curve point
                        g -> draw_text((segments_zoom_3[i].seg_curve_points)[segments_zoom_3[i].num_curve_p / 2], segments_zoom_3[i].street_name + arrow, segments_zoom_3[i].length, std::numeric_limits<double>::max());
                     } else if ((segments_zoom_3[i].from_xy).x > (segments_zoom_3[i].to_xy).x) {
                        arrow = "<=== ";
                        g -> draw_text((segments_zoom_3[i].seg_curve_points)[segments_zoom_3[i].num_curve_p / 2], arrow + segments_zoom_3[i].street_name, segments_zoom_3[i].length, std::numeric_limits<double>::max());   
                     }
                  } else {
                     g -> draw_text((segments_zoom_3[i].seg_curve_points)[segments_zoom_3[i].num_curve_p / 2], segments_zoom_3[i].street_name, segments_zoom_3[i].length, std::numeric_limits<double>::max());
                  } 
               }   
            } 
         }
      }

   } if (zoom_scale > 0.03) {
      for (StreetSegmentIdx i = 0; i < num_zoom_2; ++i) {   
         if ((g -> get_visible_world().contains(segments_zoom_2[i].from_xy)) || (g -> get_visible_world().contains(segments_zoom_2[i].to_xy))) {
            if (std::isnan((segments_zoom_2[i].angle) * (180.0 / M_PI)) == true) {
               continue;
            }
            g -> set_text_rotation((segments_zoom_2[i].angle) * (180.0 / M_PI));
            g -> set_color(240, 240, 240);       

            if (segments_zoom_2[i].street_name != "<unknown>") {
               if (segments_zoom_2[i].num_curve_p == 0) {
                  // Check if one way
                  std::string arrow;
                  if (segments_zoom_2[i].one_way == true) {
                     // Check direction of one way
                     if ((segments_zoom_2[i].from_xy).x < (segments_zoom_2[i].to_xy).x) {
                        arrow = " ===>";
                        g -> draw_text(segments_zoom_2[i].middle_xy, segments_zoom_2[i].street_name + arrow, segments_zoom_2[i].length, std::numeric_limits<double>::max());
                     } else if ((segments_zoom_2[i].from_xy).x > (segments_zoom_2[i].to_xy).x) {
                        arrow = "<=== ";
                        g -> draw_text(segments_zoom_2[i].middle_xy, arrow + segments_zoom_2[i].street_name, segments_zoom_2[i].length, std::numeric_limits<double>::max());   
                     }
                  } else {
                     g -> draw_text(segments_zoom_2[i].middle_xy, segments_zoom_2[i].street_name, segments_zoom_2[i].length, std::numeric_limits<double>::max());
                  }  
               } else {
                  // Check if one way
                  std::string arrow;
                  if (segments_zoom_2[i].one_way == true) {
                     // Check direction of one way
                     if ((segments_zoom_2[i].from_xy).x < (segments_zoom_2[i].to_xy).x) {
                        arrow = " ===>";
                        // Draw text at the middle curve point
                        g -> draw_text((segments_zoom_2[i].seg_curve_points)[segments_zoom_2[i].num_curve_p / 2], segments_zoom_2[i].street_name + arrow, segments_zoom_2[i].length, std::numeric_limits<double>::max());
                     } else if ((segments_zoom_2[i].from_xy).x > (segments_zoom_2[i].to_xy).x) {
                        arrow = "<=== ";
                        g -> draw_text((segments_zoom_2[i].seg_curve_points)[segments_zoom_2[i].num_curve_p / 2], arrow + segments_zoom_2[i].street_name, segments_zoom_2[i].length, std::numeric_limits<double>::max());   
                     }
                  } else {
                     g -> draw_text((segments_zoom_2[i].seg_curve_points)[segments_zoom_2[i].num_curve_p / 2], segments_zoom_2[i].street_name, segments_zoom_2[i].length, std::numeric_limits<double>::max());
                  } 
               }  
            } 
         }
      }


   } if (zoom_scale > 0.08) {
      for (int i = 0; i < num_zoom_1; ++i) { 
         if ((g -> get_visible_world().contains(segments_zoom_1[i].from_xy)) || (g -> get_visible_world().contains(segments_zoom_1[i].to_xy))) {
            if (std::isnan((segments_zoom_1[i].angle) * (180.0 / M_PI)) == true) {
               continue;
            }
            g -> set_text_rotation((segments_zoom_1[i].angle) * (180.0 / M_PI));
            g -> set_color(240, 240, 240);       

            if (segments_zoom_1[i].street_name != "<unknown>") {
               if (segments_zoom_1[i].num_curve_p == 0) {
                  // Check if one way
                  std::string arrow;
                  if (segments_zoom_1[i].one_way == true) {
                     // Check direction of one way
                     if ((segments_zoom_1[i].from_xy).x < (segments_zoom_1[i].to_xy).x) {
                        arrow = " ===>";
                        g -> draw_text(segments_zoom_1[i].middle_xy, segments_zoom_1[i].street_name + arrow, segments_zoom_1[i].length, std::numeric_limits<double>::max());
                     } else if ((segments_zoom_1[i].from_xy).x > (segments_zoom_1[i].to_xy).x) {
                        arrow = "<=== ";
                        g -> draw_text(segments_zoom_1[i].middle_xy, arrow + segments_zoom_1[i].street_name, segments_zoom_1[i].length, std::numeric_limits<double>::max());   
                     }
                  } else {
                     g -> draw_text(segments_zoom_1[i].middle_xy, segments_zoom_1[i].street_name, segments_zoom_1[i].length, std::numeric_limits<double>::max());
                  }  
               } else {
                  // Check if one way
                  std::string arrow;
                  if (segments_zoom_1[i].one_way == true) {
                     // Check direction of one way
                     if ((segments_zoom_1[i].from_xy).x < (segments_zoom_1[i].to_xy).x) {
                        arrow = " ===>";
                        // Draw text at the middle curve point
                        g -> draw_text((segments_zoom_1[i].seg_curve_points)[segments_zoom_1[i].num_curve_p / 2], segments_zoom_1[i].street_name + arrow, segments_zoom_1[i].length, std::numeric_limits<double>::max());
                     } else if ((segments_zoom_1[i].from_xy).x > (segments_zoom_1[i].to_xy).x) {
                        arrow = "<=== ";
                        g -> draw_text((segments_zoom_1[i].seg_curve_points)[segments_zoom_1[i].num_curve_p / 2], arrow + segments_zoom_1[i].street_name, segments_zoom_1[i].length, std::numeric_limits<double>::max());   
                     }
                  } else {
                     g -> draw_text((segments_zoom_1[i].seg_curve_points)[segments_zoom_1[i].num_curve_p / 2], segments_zoom_1[i].street_name, segments_zoom_1[i].length, std::numeric_limits<double>::max());
                  } 
               } 
            }
         }
      }  

      // Drawing POI markers and names
      if (show_elements_global.show_poi) {
         for (POIIdx id = 0; id < poi_zoom_1.size(); ++id) {
            g -> set_color(240, 240, 240);
            g -> set_text_rotation(0);

            std::string poi_type = poi_zoom_1[id].type;

            if (g -> get_visible_world().contains(poi_zoom_1[id].coord_xy)) {
               poi_image = g -> load_png(("libstreetmap/resources/poi_images/" + poi_type + ".png").c_str());
               g -> draw_surface(poi_image, {(poi_zoom_1[id].coord_xy).x, (poi_zoom_1[id].coord_xy).y + (20 / zoom_scale)}, 0.5);
               g -> draw_text(poi_zoom_1[id].coord_xy, poi_zoom_1[id].name);
            }
         }
      }
   } if (zoom_scale > 0.2) {
      for (StreetSegmentIdx i = 0; i < num_zoom_0; ++i) {
         if ((g -> get_visible_world().contains(segments_zoom_0[i].from_xy)) || (g -> get_visible_world().contains(segments_zoom_0[i].to_xy))) { 
            if (std::isnan((segments_zoom_0[i].angle) * (180.0 / M_PI)) == true) {
               continue;
            }
            g -> set_text_rotation((segments_zoom_0[i].angle) * (180.0 / M_PI));
            g -> set_color(240, 240, 240);       

            if (segments_zoom_0[i].street_name != "<unknown>") {
               if (segments_zoom_0[i].num_curve_p == 0) {
                  // Check if one way
                  std::string arrow;
                  if (segments_zoom_0[i].one_way == true) {
                     // Check direction of one way
                     if ((segments_zoom_0[i].from_xy).x < (segments_zoom_0[i].to_xy).x) {
                        arrow = " ===>";
                        g -> draw_text(segments_zoom_0[i].middle_xy, segments_zoom_0[i].street_name + arrow, segments_zoom_0[i].length, std::numeric_limits<double>::max());
                     } else if ((segments_zoom_0[i].from_xy).x > (segments_zoom_0[i].to_xy).x) {
                        arrow = "<=== ";
                        g -> draw_text(segments_zoom_0[i].middle_xy, arrow + segments_zoom_0[i].street_name, segments_zoom_0[i].length, std::numeric_limits<double>::max());   
                     }
                  } else {
                     g -> draw_text(segments_zoom_0[i].middle_xy, segments_zoom_0[i].street_name, segments_zoom_0[i].length, std::numeric_limits<double>::max());
                  }  
               } else {
                  // Check if one way
                  std::string arrow;
                  if (segments_zoom_0[i].one_way == true) {
                     // Check direction of one way
                     if ((segments_zoom_0[i].from_xy).x < (segments_zoom_0[i].to_xy).x) {
                        arrow = " ===>";
                        // Draw text at the middle curve point
                        g -> draw_text((segments_zoom_0[i].seg_curve_points)[segments_zoom_0[i].num_curve_p / 2], segments_zoom_0[i].street_name + arrow, segments_zoom_0[i].length, std::numeric_limits<double>::max());
                     } else if ((segments_zoom_0[i].from_xy).x > (segments_zoom_0[i].to_xy).x) {
                        arrow = "<=== ";
                        g -> draw_text((segments_zoom_0[i].seg_curve_points)[segments_zoom_0[i].num_curve_p / 2], arrow + segments_zoom_0[i].street_name, segments_zoom_0[i].length, std::numeric_limits<double>::max());   
                     }
                  } else {
                     g -> draw_text((segments_zoom_0[i].seg_curve_points)[segments_zoom_0[i].num_curve_p / 2], segments_zoom_0[i].street_name, segments_zoom_0[i].length, std::numeric_limits<double>::max());
                  } 
               }  
            } 
         }
      }      

      // Draw subway stations
      if (show_elements_global.subways) {
         for (int i = 0; i < subway_osm_nodes.size(); ++i) {
            g -> set_text_rotation(0);
            g -> set_color(102, 178, 255);
            
            subway_info temp_info = subway_node_info[subway_osm_nodes[i]];
            float x_coord = temp_info.xy_pos.x;
            float y_coord = temp_info.xy_pos.y;

            if (g -> get_visible_world().contains({x_coord, y_coord})) {
               subway_stations = g -> load_png("libstreetmap/resources/train_logo.png");
               g -> draw_surface(subway_stations, {x_coord, y_coord}, 0.06);
               g -> draw_text({x_coord, y_coord - 25 / zoom_scale}, temp_info.subway_name);
            }
         }          
      }

      // Drawing POI markers and names
      if (show_elements_global.show_poi) {
         for (POIIdx id = 0; id < poi_zoom_0.size(); ++id) {
            g -> set_color(240, 240, 240);
            g -> set_text_rotation(0);

            std::string poi_type = poi_zoom_0[id].type;

            if (g -> get_visible_world().contains(poi_zoom_0[id].coord_xy)) {
               poi_image = g -> load_png(("libstreetmap/resources/poi_images/" + poi_type + ".png").c_str());
               if (poi_type == "place_of_worship") {
                  g -> draw_surface(poi_image, {(poi_zoom_0[id].coord_xy).x, (poi_zoom_0[id].coord_xy).y + (20 / zoom_scale)}, 0.25);
               } else {
                  g -> draw_surface(poi_image, {(poi_zoom_0[id].coord_xy).x, (poi_zoom_0[id].coord_xy).y + (20 / zoom_scale)}, 0.5);
               }
            }
         }
      }
   } if (zoom_scale > 1) {
      for (StreetSegmentIdx i = 0; i < num_zoom_small; ++i) {     
         if ((g -> get_visible_world().contains(segments_zoom_small[i].from_xy)) || (g -> get_visible_world().contains(segments_zoom_small[i].to_xy))) { 
            if (std::isnan((segments_zoom_small[i].angle) * (180.0 / M_PI)) == true) {
               continue;
            }
            g -> set_text_rotation((segments_zoom_small[i].angle) * (180.0 / M_PI));
            g -> set_color(240, 240, 240);       

            if (segments_zoom_small[i].street_name != "<unknown>") {
               if (segments_zoom_small[i].num_curve_p == 0) {
                  // Check if one way
                  std::string arrow;
                  if (segments_zoom_small[i].one_way == true) {
                     // Check direction of one way
                     if ((segments_zoom_small[i].from_xy).x < (segments_zoom_small[i].to_xy).x) {
                        arrow = " ===>";
                        g -> draw_text(segments_zoom_small[i].middle_xy, segments_zoom_small[i].street_name + arrow, segments_zoom_small[i].length, std::numeric_limits<double>::max());
                     } else if ((segments_zoom_small[i].from_xy).x > (segments_zoom_small[i].to_xy).x) {
                        arrow = "<=== ";
                        g -> draw_text(segments_zoom_small[i].middle_xy, arrow + segments_zoom_small[i].street_name, segments_zoom_small[i].length, std::numeric_limits<double>::max());   
                     }
                  } else {
                     g -> draw_text(segments_zoom_small[i].middle_xy, segments_zoom_small[i].street_name, segments_zoom_small[i].length, std::numeric_limits<double>::max());
                  }  
               } else {
                  // Check if one way
                  std::string arrow;
                  if (segments_zoom_small[i].one_way == true) {
                     // Check direction of one way
                     if ((segments_zoom_small[i].from_xy).x < (segments_zoom_small[i].to_xy).x) {
                        arrow = " ===>";
                        // Draw text at the middle curve point
                        g -> draw_text((segments_zoom_small[i].seg_curve_points)[segments_zoom_small[i].num_curve_p / 2], segments_zoom_small[i].street_name + arrow, segments_zoom_small[i].length, std::numeric_limits<double>::max());
                     } else if ((segments_zoom_small[i].from_xy).x > (segments_zoom_small[i].to_xy).x) {
                        arrow = "<=== ";
                        g -> draw_text((segments_zoom_small[i].seg_curve_points)[segments_zoom_small[i].num_curve_p / 2], arrow + segments_zoom_small[i].street_name, segments_zoom_small[i].length, std::numeric_limits<double>::max());   
                     }
                  } else {
                     g -> draw_text((segments_zoom_small[i].seg_curve_points)[segments_zoom_small[i].num_curve_p / 2], segments_zoom_small[i].street_name, segments_zoom_small[i].length, std::numeric_limits<double>::max());
                  } 
               }   
            } 
         }
      }
      
      // Drawing POI markers and names
      if (show_elements_global.show_poi) {
         for (POIIdx id = 0; id < poi_zoom_small.size(); ++id) {
            g -> set_color(240, 240, 240);
            g -> set_text_rotation(0);

            std::string poi_type = poi_zoom_small[id].type;

            if (g -> get_visible_world().contains(poi_zoom_small[id].coord_xy)) {
               poi_image = g -> load_png(("libstreetmap/resources/poi_images/" + poi_type + ".png").c_str());
               g -> draw_surface(poi_image, {(poi_zoom_small[id].coord_xy).x, (poi_zoom_small[id].coord_xy).y + (20 / zoom_scale)}, 0.5);
            }
         }

         for (POIIdx id = 0; id < poi_zoom_0.size(); ++id) {
            g -> set_color(240, 240, 240);
            g -> set_text_rotation(0);

            std::string poi_type = poi_zoom_0[id].type;

            if (g -> get_visible_world().contains(poi_zoom_0[id].coord_xy)) {
               g -> draw_text(poi_zoom_0[id].coord_xy, poi_zoom_0[id].name);
            }
         }
      }
   } if (zoom_scale > 2) {
      if (show_elements_global.show_poi) {
         for (POIIdx id = 0; id < poi_zoom_small.size(); ++id) {
            g -> set_color(240, 240, 240);
            g -> set_text_rotation(0);

            std::string poi_type = poi_zoom_small[id].type;

            if (g -> get_visible_world().contains(poi_zoom_small[id].coord_xy)) {
               g -> draw_text(poi_zoom_small[id].coord_xy, poi_zoom_small[id].name);
            }
         }
      }
   }

   // ezgl::surface* subway_stations;
   // ezgl::surface* to_from_intersections;


   // Draw the intersection where the mouse was clicked
   if (highlight_global.from_highlighted) {
      if (highlight_global.from_id != -1){
         float x = intersections[highlight_global.from_id].xy_loc.x;
         float y = intersections[highlight_global.from_id].xy_loc.y;

         to_from_intersections = g -> load_png("libstreetmap/resources/blue_start.png");
         g -> draw_surface(to_from_intersections, {x, (y + (1 / (0.1 * zoom_scale))) + 1}, 0.03);      
      }
   }

   if (highlight_global.to_highlighted) {
      if (highlight_global.from_id != -1){
         float x = intersections[highlight_global.to_id].xy_loc.x;
         float y = intersections[highlight_global.to_id].xy_loc.y;

         to_from_intersections = g -> load_png("libstreetmap/resources/poi.png");
         g -> draw_surface(to_from_intersections, {x, (y + (1 / (0.1 * zoom_scale))) + 1}, 0.03); 
      }
   }


   if (highlight_global.all_intersections && !highlight_global.to_highlighted && highlight_global.from_highlighted) {
      for (int i = 0; i < highlight_global.two_streets_intersections.size(); ++i) {
         g -> set_color (255, 0, 0);
         float width = 10;
         float height = width;         
         float x = intersections[highlight_global.two_streets_intersections[i]].xy_loc.x;
         float y = intersections[highlight_global.two_streets_intersections[i]].xy_loc.y;
         g -> fill_rectangle({x - width / 2.0, y - height / 2.0}, {(x + width / 2.0), (y + height / 2.0)});         
      }
   }

}



// A helper function to process the search string
std::string process(std::string const& s, int type)
{
   std::string::size_type pos = s.find(" and ");
   if (type ==0){
      if (pos != std::string::npos)
      {
         return s.substr(0, (pos));
      }
      else
      {
         return s;
      }
   }
   else if (type == 1){
      if (pos != std::string::npos)
      {
         return s.substr((pos + 5));
      }
      else
      {
         return s;
      }
   }
   else if (type == 2){
      pos = s.find("from ");
      if (pos != std::string::npos)
      {
         return s.substr((pos + 5));
      }
      else
      {
         return s;
      }
   }
   else if (type == 3){
      pos = s.find("to ");
      if (pos != std::string::npos)
      {
         return s.substr((pos + 3));
      }
      else
      {
         return s;
      }
   }

   return "";
}


void drawMap() {
   // Set up the ezgl graphics window and hand control to it, as shown in the 
   // ezgl example program. 
   // This function will be called by both the unit tests (ece297exercise) 
   // and your main() function in main/src/main.cpp.
   // The unit tests always call loadMap() before calling this function
   // and call closeMap() after this function returns.

   ezgl::application::settings settings;

   settings.main_ui_resource = 
               "libstreetmap/resources/main.ui";
   settings.window_identifier = "MainWindow";
   settings.canvas_identifier = "MainCanvas";

   ezgl::application application(settings);
   

   // Draw testing
   ezgl::rectangle initial_world({x_from_lon(map_bounds[3]), y_from_lat(map_bounds[1])}, 
                                 {x_from_lon(map_bounds[2]), y_from_lat(map_bounds[0])});

   application.add_canvas("MainCanvas", draw_main_canvas, initial_world, {54,69,79});

   drawMapHelper(application, "");

}

// a helper function to draw a new map
// still using the same canvas
void drawMapHelper(ezgl::application &app, std::string city_name) {

   // Use the correct font for different languages
   std::string canada = "canada";
   std::string china = "china";
   std::string japan = "japan";
   std::string iran = "iran";
   std::string usa = "usa";

   if (city_name.find(china) != std::string::npos || city_name.find(japan) != std::string::npos) {
      ezgl::renderer *g = app.get_renderer();
      g -> format_font("Noto Sans CJK SC", ezgl::font_slant::normal, ezgl::font_weight::normal);
      std::cout << "Font changed" << std::endl;
   }

   if (city_name.find(iran) != std::string::npos ) {
      ezgl::renderer *g = app.get_renderer();
      g -> format_font("Noto Naskh Arabic", ezgl::font_slant::normal, ezgl::font_weight::normal);
      std::cout << "Font changed" << std::endl;         
   }

   if (city_name.find(usa) != std::string::npos || city_name.find(canada) != std::string::npos) {
      ezgl::renderer *g = app.get_renderer();
      g -> format_font("monospace", ezgl::font_slant::normal, ezgl::font_weight::normal);
      std::cout << "Font changed" << std::endl;             
   }

   if (!reload_map) {
      app.run(initial_setup, act_on_mouse_click,
                     nullptr, nullptr);
   }

   else {
      ezgl::rectangle initial_world({x_from_lon(map_bounds[3]), y_from_lat(map_bounds[1])}, 
                                    {x_from_lon(map_bounds[2]), y_from_lat(map_bounds[0])});

      app.get_canvas("MainCanvas") -> get_camera().set_world(initial_world);
      app.get_canvas("MainCanvas") -> get_camera().reset_world(initial_world);



      app.refresh_drawing();
      app.run(nullptr, act_on_mouse_click,
                     nullptr, nullptr);

   }

}