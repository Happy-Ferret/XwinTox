#include <algorithm>
#include <vector>
#include <stdio.h>

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/fl_draw.H>

#include "contacts.h"
#include "xwintox_win.h"
#include "svgs.h"
#include "nanosvg/nsvgwrap.h"

#include "control/gui.h"
#include "control/msgarea.h"

extern int CGUIUPDFLAG;

using namespace std;

ContactsEntry::ContactsEntry(int X, int Y, int S, Contact_t *C, Groupchat_t *G,
                             short T)
	: Fl_Box(X, Y, 224 * S, 50 * S)
{
	scale =S;
	contact =C;
	groupchat =G;
	type =T;
	selected =0;

	if(!T)
	{
		icon =new SVGBox(X+ (4 * S), Y+ (2 * S), 46 * S, 46 *S, S,
		                 default_av, 0.35);
		invicon =new SVGBox(X+ (4 * S), Y+ (2 * S), 46 * S, 46 *S, S,
		                    default_av, 0.35);
		invicon->hide();
		groupchat =new Groupchat_t;
		groupchat->num =65535;
	}
	else
	{
		icon =new SVGBox(X- (8 * S), Y+ (2 * S), 46 * S, 46 *S, S,
		                 groupsvg, 0.63);
		invicon =new SVGBox(X+ (14 * S), Y+ (12 * S), 46 * S, 46 *S, S,
		                    groupsvg2, 0.63);
		invicon->hide();
		contact =new Contact_t;
		contact->num =65535;
	}

	box(FL_FLAT_BOX);
	color(fl_rgb_color(65, 65, 65));
}

void ContactsEntry::draw()
{
	int txt_color =255;
	char *name, *status;

	if(selected)
	{
		color(255);
		txt_color =0;
		icon->hide();
		invicon->show();
	}
	else
	{
		color(fl_rgb_color(65, 65, 65));
		txt_color=255;
		invicon->hide();
		icon->show();
	}

	Fl_Box::draw();

	if(!type) /* todo: resize() calls to the svgbox, make it work */
	{
		name =GetDisplayName(contact, 16);
		status =GetDisplayStatus(contact, 25);

		if(contact->connected) fl_color(2);
		else fl_color(FL_RED);

		fl_pie(x() + (185 * scale), this->y() + (20 * scale), 10 * scale,
		       10 * scale, 0, 360);
	}
	else
	{
		name =groupchat->name;
		status="";
	}

	fl_color(txt_color);
	fl_font(FL_HELVETICA, 12 * scale);
	fl_draw(name, x() + (50 * scale), y() + (22 * scale));
	fl_font(FL_HELVETICA, 10 * scale);
	fl_draw(status, x() + (50 * scale), y() + (36 * scale));
	icon->draw(); /* make it account for scrollbar::value(); */
	invicon->draw();
}

int ContactsEntry::handle(int event)
{
	switch(event)
	{
	case FL_PUSH:
		if(Fl::event_button() == FL_LEFT_MOUSE)
		{
			for(const auto entry : ((ContactsList*) parent())->entries)
			{
				entry->selected =0;
				entry->redraw();
				entry->icon->redraw();
				entry->invicon->redraw();
				((Sidebar*)(parent()->parent()))->bottom_area->deselect_all();
			}

			selected =1;
			((ContactsList*) parent())->seltype =type;
			redraw();
			icon->redraw();
			invicon->redraw();

			if(!type)
			{
				((ContactsList*) parent())->selected =contact->num;
			}
			else
			{
				((ContactsList*) parent())->selected =groupchat->num;
			}

			icon->hide();
			invicon->show();

			if(!type)
			{
				Xw->contents->NewCurrentArea(FindContactMArea(contact));
			}
			else
			{
				Xw->contents->NewCurrentArea(FindGroupchatMArea(groupchat));
			}

			return 1;
		}
		else if(Fl::event_button() == FL_RIGHT_MOUSE)
		{
			Fl_Menu_Item contmenu[] = { { "Delete contact" }, { 0 } };
			contmenu->labelsize(12 * scale);
			const Fl_Menu_Item *m = contmenu->popup(Fl::event_x(), Fl::event_y(),
			                                        0, 0, 0);

			if(!m) return 0;
			else if(strcmp(m->label(), "Delete contact") == 0)
			{
				vector <ContactsEntry*> *ref =&
				                              ((ContactsList*) parent())->entries;
				ref->erase(std::remove(ref->begin(), ref->end(), this),
				           ref->end());
				DeleteContact(contact->num);
				parent()->remove(this);
				Fl::delete_widget(this);
				Fl::delete_widget(FindContactMArea(contact));
				CGUIUPDFLAG =1;
				return 0;
			}
		}
	}

	return 0;
}

ContactsList::ContactsList(int X, int Y, int W, int H, int S)
	: Fl_Scroll(X, Y, W, H)
{
	scale =S;
	selected =-1;
	color(fl_rgb_color(65, 65, 65));
	type(6);
}

void ContactsList::draw()
{
	Fl_Scroll::draw();

	for(const auto entry : entries)
	{
		entry->icon->redraw();
	}
}

void ContactsList::resize(int X, int Y, int W, int H)
{
	Fl_Scroll::resize(X, Y, W, H);

	for(const auto entry : entries)
	{
		entry->icon->draw();
	}
}

int ContactsList::handle(int event)
{
	switch(event)
	{
	case FL_PUSH:
		if(Fl::event_button() == FL_LEFT_MOUSE)
		{
			for(const auto entry : entries)
			{
				entry->redraw();
				entry->icon->redraw();
			}
		}
	}

	Fl_Scroll::handle(event);
	return 0;
}

void ContactsList::clear_all()
{
	this->clear();
	entries.clear();
	this->redraw();
	parent()->redraw();
}

void ContactsList::deselect_all()
{
	for(const auto entry : entries)
	{
		entry->selected =0;
		entry->icon->show();
		entry->invicon->hide();
		entry->redraw();
		entry->icon->redraw();
	}
}

StatusBox::StatusBox(int X, int Y, int W, int H, int S) : Fl_Box(X, Y, W, H)
{
	scale =S;
	box(FL_FLAT_BOX);
	color(fl_rgb_color(65, 65, 65));
}

void StatusBox::draw()
{
	Fl_Box::draw();
	fl_color(2);
	fl_pie(this->x() + (5 * scale), this->y() + (15 * scale), 10 * scale,
	       10 * scale, 0, 360);
}


SVGBox::SVGBox(int X, int Y, int W, int H, int S, const char* pic,
               double factor) : Fl_Box(X, Y, W, H)
{
	img_r =svgrast(pic, W, H, S, factor);
	img =new Fl_RGB_Image(img_r, W, H, 4);
}

void SVGBox::draw()
{
	Fl_Box::draw();
}

int SVGBox::handle(int event)
{
	switch(event)
	{
	case FL_PUSH:
		do_callback();
		return 1;
	}

	return 0;
}

void SVGBox::hide()
{
	Fl_Box::hide();
	image(0);
}

void SVGBox::show()
{
	Fl_Box::show();
	image(img);
}

Sidebar_Top_Area::Sidebar_Top_Area(int S) : Fl_Group(Xw->basex * S,
	        Xw->basey * S, (Xw->sblength * S), 60 * S)
{
	scale =S;

	box(FL_FLAT_BOX);
	color(fl_rgb_color(28, 28, 28));

	statusbox =new StatusBox(x() + (192 * S), y() + (10 * S), 20 * S, 40 * S, S);
	avbox =new SVGBox(x() + (10 * S), y() + (10 * S), 40 * S , 40 * S, S,
	                  default_av, 0.3);
	name =new Fl_Input(x() + (62 * S), y() + (18 * S), 128 *S, 11 * S);
	status =new Fl_Input(x() + (62 * S), y() + (30 * S), 128 *S, 11 * S);
	name->textsize(12 * S);
	status->textsize(10 * S);
	name->box(FL_FLAT_BOX);
	name->color(fl_rgb_color(28, 28, 28));
	status->box(FL_FLAT_BOX);
	status->color(fl_rgb_color(28, 28, 28));
	name->textcolor(255);
	status->textcolor(54);
	name->value("XwinTox User");
	status->value("Toxing on XwinTox");

	avbox->show();

	end();
}

Sidebar_Bottom_Area::Sidebar_Bottom_Area(int S) : Fl_Group(Xw->basex * S,
	        Xw->h() - (36 * S) - (Xw->basey * S), (Xw->sblength * S),
	        Xw->h() - (Xw->basey * S))
{
	scale =S;

	box(FL_FLAT_BOX);
	color(fl_rgb_color(28, 28, 28));

	addfriend =new SVGBox(0, 0, 56 * scale, 36 * S, S, addfriendsvg, 0.5);
	newgroup =new SVGBox(0, 0, 112 * S , 36 * S, S, groupsvg, 0.5);
	transfers =new SVGBox(0, 0, 168 * S , 36 * S, S, transfersvg, 0.5);
	settings =new SVGBox(0, 0, 212 * S , 36 * S, S, settingssvg, 0.5);
	addfriend->show(); newgroup->show(); transfers->show(); settings->show();

	end();
}

void Sidebar_Bottom_Area::resize(int X, int Y, int W, int H)
{
	Fl_Group::resize(X, Y, W, H);
	addfriend->position(x(), Xw->h() - (36 * scale));
	newgroup->position(x() + (56 * scale), Xw->h() - (36 * scale));
	transfers->position(x() + (112 * scale), Xw->h() - (36 * scale));
	settings->position(x() + (168 * scale), Xw->h() - (36 * scale));
}

void Sidebar_Bottom_Area::deselect_all()
{
	addfriend->box(FL_NO_BOX);
	newgroup->box(FL_NO_BOX);
	transfers->box(FL_NO_BOX);
	settings->box(FL_NO_BOX);
}


Sidebar::Sidebar(int S) : Fl_Group(Xw->basex * S,Xw->basey * S,
	                                   (Xw->sblength * S),
	                                   Xw->h() - (Xw->basey * S))
{
	scale =S;

	box(FL_FLAT_BOX);
	color(fl_rgb_color(65, 65, 65));
	top_area =new Sidebar_Top_Area(S);
	bottom_area =new Sidebar_Bottom_Area(S);
	contacts =new ContactsList(x(), y() + (60 * S),(Xw->sblength * S),
	                           h() - (36 * S) - (60 * S), S);

	resize(x(), y(), w(), h());
	end();
}

void Sidebar::resize(int X, int Y, int W, int H)
{
	Fl_Group::resize(X, Y, W, H);
	top_area->resize(Xw->basex * scale, Xw->basey * scale,
	                 Xw->sblength * scale, 60 * scale);
	bottom_area->resize(Xw->basex * scale,
	                    Xw->h() - (36 * scale) - (Xw->basey * scale),
	                    (Xw->sblength * scale),
	                    Xw->h() - (Xw->basey * scale));
	contacts->resize(x(), y() + (60 * scale), (Xw->sblength * scale),
	                 h() - (36 * scale) - (60 * scale));
}


GArea::GArea(int S, const char *C) : Fl_Group(Xw->sblength * S,
	        Xw->basey * S,
	        Xw->w() - (Xw->sblength * S),
	        Xw->h()- (Xw->basey * S))
{
	caption =C;
	scale =S;

	box(FL_FLAT_BOX);
	color(255);
}

void GArea::draw()
{
	Fl_Group::draw();

	fl_color(0);
	fl_font(FL_HELVETICA_BOLD, 12 * scale);
	fl_draw(caption, x() + (10 * scale), y() + (24 * scale));

	fl_color(fl_rgb_color(192, 192, 192));
	fl_line(x(), y() + (60 * scale), Fl::w(), y() + (60 * scale));
	fl_color(0);
}


const char *default_av ="<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?> <!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.0//EN\" \"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\"> <!-- Created with Inkscape (http://www.inkscape.org/) --><svg height=\"100.00000pt\" id=\"svg2801\" inkscape:export-filename=\"/home/Krug/Projekte/Icon-Sets/set2/png/8a.png\" inkscape:export-xdpi=\"72.000000\" inkscape:export-ydpi=\"72.000000\" inkscape:version=\"0.39\" sodipodi:docbase=\"/home/Krug/Projekte/Icon-Sets/set2/svg\" sodipodi:docname=\"8a.svg\" sodipodi:version=\"0.32\" width=\"100.00000pt\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:cc=\"http://web.resource.org/cc/\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\" xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">   <metadata>     <rdf:RDF xmlns:cc=\"http://web.resource.org/cc/\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">       <cc:Work rdf:about=\"\">         <dc:title>people</dc:title>         <dc:description></dc:description>         <dc:subject>           <rdf:Bag>             <rdf:li>icon</rdf:li>             <rdf:li>people</rdf:li>           </rdf:Bag>         </dc:subject>         <dc:publisher>           <cc:Agent rdf:about=\"http://www.openclipart.org/\">             <dc:title>Open Clip Art Library</dc:title>           </cc:Agent>         </dc:publisher>         <dc:creator>           <cc:Agent>             <dc:title>Juliane Krug</dc:title>           </cc:Agent>         </dc:creator>         <dc:rights>           <cc:Agent>             <dc:title>Juliane Krug</dc:title>           </cc:Agent>         </dc:rights>         <dc:date></dc:date>         <dc:format>image/svg+xml</dc:format>         <dc:type rdf:resource=\"http://purl.org/dc/dcmitype/StillImage\"/>         <cc:license rdf:resource=\"http://web.resource.org/cc/PublicDomain\"/>         <dc:language>en</dc:language>       </cc:Work>       <cc:License rdf:about=\"http://web.resource.org/cc/PublicDomain\">         <cc:permits rdf:resource=\"http://web.resource.org/cc/Reproduction\"/>         <cc:permits rdf:resource=\"http://web.resource.org/cc/Distribution\"/>         <cc:permits rdf:resource=\"http://web.resource.org/cc/DerivativeWorks\"/>       </cc:License>     </rdf:RDF>   </metadata>   <defs id=\"defs2803\">     <linearGradient id=\"linearGradient4384\">       <stop id=\"stop4385\" offset=\"0.0000000\" style=\"stop-color:#ffffff;stop-opacity:0.77513230;\"/>       <stop id=\"stop4386\" offset=\"1.0000000\" style=\"stop-color:#ffffff;stop-opacity:0.0000000;\"/>     </linearGradient>     <linearGradient id=\"linearGradient4376\">       <stop id=\"stop4377\" offset=\"0.0000000\" style=\"stop-color:#000000;stop-opacity:0.52645504;\"/>       <stop id=\"stop4378\" offset=\"1.0000000\" style=\"stop-color:#000000;stop-opacity:0.0000000;\"/>     </linearGradient>     <linearGradient id=\"linearGradient4362\">       <stop id=\"stop4363\" offset=\"0.0000000\" style=\"stop-color:#ffffff;stop-opacity:0.66666669;\"/>       <stop id=\"stop4364\" offset=\"1.0000000\" style=\"stop-color:#ffffff;stop-opacity:0.0000000;\"/>     </linearGradient>     <linearGradient id=\"linearGradient4358\">       <stop id=\"stop4359\" offset=\"0\" style=\"stop-color:#000;stop-opacity:1;\"/>       <stop id=\"stop4360\" offset=\"1\" style=\"stop-color:#fff;stop-opacity:1;\"/>     </linearGradient>     <linearGradient id=\"linearGradient2860\" inkscape:collect=\"always\" x1=\"0.41025642\" x2=\"0.39743590\" xlink:href=\"#linearGradient4376\" y1=\"0.24218750\" y2=\"0.50000000\"/>     <radialGradient cx=\"0.32142857\" cy=\"0.50000000\" fx=\"0.32653064\" fy=\"0.49218750\" id=\"radialGradient2862\" inkscape:collect=\"always\" r=\"0.54614192\" xlink:href=\"#linearGradient4384\"/>     <radialGradient cx=\"0.18666667\" cy=\"0.25781250\" fx=\"0.18000001\" fy=\"0.24218750\" id=\"radialGradient2865\" inkscape:collect=\"always\" r=\"0.50000000\" xlink:href=\"#linearGradient4384\"/>   </defs>   <sodipodi:namedview bordercolor=\"#666666\" borderopacity=\"1.0\" id=\"base\" inkscape:cx=\"124.39392\" inkscape:cy=\"50.000000\" inkscape:pageopacity=\"0.0\" inkscape:pageshadow=\"2\" inkscape:window-height=\"937\" inkscape:window-width=\"1272\" inkscape:window-x=\"0\" inkscape:window-y=\"0\" inkscape:zoom=\"3.6100000\" pagecolor=\"#ffffff\"/>   <path d=\"M 65.305390,72.630083 C 70.054791,77.813645 73.077138,87.100859 79.769477,87.100860 C 86.245934,85.301011 87.109462,69.894314 87.972989,66.582595 C 80.633005,71.622168 73.077139,74.501924 65.305390,72.630083 z \" id=\"path9383\" sodipodi:nodetypes=\"cccc\" style=\"fill:#ffffff;fill-opacity:1.0000000;fill-rule:evenodd;stroke:#000000;stroke-width:3.7500000;stroke-linecap:round;stroke-linejoin:round;stroke-miterlimit:4.0000000;stroke-opacity:1.0000000;\" transform=\"translate(-6.997474,1.874351)\"/>   <path d=\"M 211.21884 70.290855 A 60.941830 57.825485 0 1 0 89.335178,70.290855 A 60.941830 57.825485 0 1 0 211.21884 70.290855 z\" id=\"path3713\" sodipodi:cx=\"150.27701\" sodipodi:cy=\"70.290855\" sodipodi:rx=\"60.941830\" sodipodi:ry=\"57.825485\" sodipodi:type=\"arc\" style=\"fill:#f0cfa7;fill-opacity:1.0000000;fill-rule:evenodd;stroke:#000000;stroke-width:8.5041637;stroke-linecap:round;stroke-linejoin:round;stroke-miterlimit:4.0000000;stroke-opacity:1.0000000;\" transform=\"matrix(0.440859,0.000000,0.000000,0.441062,-3.749320,18.08246)\"/>   <path d=\"M 36.307019,99.436753 C 35.916378,124.63301 100.54578,127.51437 100.38948,99.436753 C 101.93426,77.108006 90.829033,64.412306 88.069362,66.570776 C 86.668121,67.666753 86.147486,92.006539 76.679554,86.407897 C 72.076674,83.686098 66.928216,73.231034 64.675609,72.884169 C 61.276534,72.360765 54.767786,69.608667 48.790072,64.457958 C 47.221779,63.106638 36.836468,76.996947 36.307019,99.436753 z \" id=\"path4369\" sodipodi:nodetypes=\"ccssssc\" style=\"fill:#8ccb1b;fill-opacity:1.0000000;fill-rule:evenodd;stroke:#000000;stroke-width:3.7500000;stroke-linecap:round;stroke-linejoin:round;stroke-miterlimit:4.0000000;stroke-opacity:1.0000000;\" transform=\"translate(-6.997474,1.874351)\"/>   <path d=\"M 65.035626,3.0232057 C 26.028598,10.685788 38.042879,64.332841 50.681294,34.996115 C 53.270470,43.901949 86.265063,48.577091 86.374475,44.899124 C 87.525583,5.9590044 86.899837,46.395264 98.325360,46.661357 C 110.80586,27.543402 92.683595,0.51975556 65.035626,3.0232057 z \" id=\"path14564\" sodipodi:nodetypes=\"ccscc\" style=\"fill:#000000;fill-opacity:1.0000000;fill-rule:evenodd;stroke:#000000;stroke-width:3.7500000;stroke-linecap:round;stroke-linejoin:round;stroke-miterlimit:4.0000000;stroke-opacity:1.0000000;\" transform=\"translate(-6.997474,1.874351)\"/>   <path d=\"M 79.871103,30.629242 C 79.497403,31.473103 79.330593,35.970366 78.996103,47.285492 C 78.886693,50.963459 45.866533,46.285077 43.277353,37.379242 C 40.499083,43.828264 37.803683,46.080437 35.621103,45.879242 C 35.432313,47.098642 35.246103,48.329072 35.246103,49.597992 C 35.246103,63.676545 47.290653,75.097992 62.121103,75.097992 C 76.951553,75.097992 88.964853,63.676544 88.964853,49.597992 C 88.964853,49.213715 88.857603,48.853138 88.839853,48.472992 C 83.442393,45.728949 81.354693,33.550071 80.339853,30.972992 C 80.195063,30.845126 80.018843,30.754086 79.871103,30.629242 z \" id=\"path2856\" style=\"fill:url(#linearGradient2860);fill-opacity:1.0000000;fill-rule:evenodd;stroke:none;stroke-width:8.5041637;stroke-linecap:round;stroke-linejoin:round;stroke-miterlimit:4.0000000;stroke-opacity:1.0000000;\"/>   <path d=\"M 58.031250,4.5000000 C 42.265939,6.7101142 29.113656,23.778100 34.322214,39.456927 C 38.680063,44.983223 39.878843,31.046285 44.133421,32.992013 C 51.645992,41.582640 64.778778,43.222731 75.745485,43.490689 C 78.917518,40.561904 76.516331,32.408412 77.843750,27.531250 C 83.286031,21.742121 83.392372,34.879782 85.761139,38.197214 C 87.694231,45.380219 92.227211,43.728839 92.478393,37.013856 C 96.068059,17.807544 76.065306,2.2342371 58.031250,4.5000000 z \" id=\"path2861\" style=\"fill:url(#radialGradient2862);fill-opacity:1.0000000;fill-rule:evenodd;stroke:none;stroke-width:3.7500000;stroke-linecap:round;stroke-linejoin:round;stroke-miterlimit:4.0000000;stroke-opacity:1.0000000;\"/>   <path d=\"M 41.778740,71.033760 C 34.704477,81.347952 31.468760,94.523755 32.778740,106.90876 C 39.153561,119.92530 56.193390,121.72527 69.112784,120.76977 C 79.836799,120.15313 93.554901,113.69487 92.309990,101.03376 C 92.491596,91.083929 90.928641,79.745663 83.466240,72.533760 C 81.326533,79.440575 82.102068,90.464421 73.622490,92.877510 C 63.667135,92.067752 62.667012,77.948265 52.880237,76.803107 C 48.894005,75.886267 44.951472,71.087318 41.778740,71.033760 z \" id=\"path2864\" style=\"fill:url(#radialGradient2865);fill-opacity:1.0000000;fill-rule:evenodd;stroke:none;stroke-width:3.7500000;stroke-linecap:round;stroke-linejoin:round;stroke-miterlimit:4.0000000;stroke-opacity:1.0000000;\"/> </svg> ";

const char* addfriendsvg ="<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?> <!-- Created with Inkscape (http://www.inkscape.org/) -->  <svg    xmlns:dc=\"http://purl.org/dc/elements/1.1/\"    xmlns:cc=\"http://creativecommons.org/ns#\"    xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"    xmlns:svg=\"http://www.w3.org/2000/svg\"    xmlns=\"http://www.w3.org/2000/svg\"    xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\"    xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\"    width=\"112\"    height=\"72\"    viewBox=\"0 0 112 72.000001\"    id=\"svg2\"    version=\"1.1\"    inkscape:version=\"0.91 r13725\"    sodipodi:docname=\"plus.svg\">   <defs      id=\"defs4\" />   <sodipodi:namedview      id=\"base\"      pagecolor=\"#ffffff\"      bordercolor=\"#666666\"      borderopacity=\"0.5\"      inkscape:pageopacity=\"0\"      inkscape:pageshadow=\"2\"      inkscape:zoom=\"3.959798\"      inkscape:cx=\"12.62558\"      inkscape:cy=\"59.968413\"      inkscape:document-units=\"px\"      inkscape:current-layer=\"layer1\"      showgrid=\"false\"      units=\"px\"      inkscape:window-width=\"1920\"      inkscape:window-height=\"1022\"      inkscape:window-x=\"0\"      inkscape:window-y=\"0\"      inkscape:window-maximized=\"1\"      showborder=\"true\" />   <metadata      id=\"metadata7\">     <rdf:RDF>       <cc:Work          rdf:about=\"\">         <dc:format>image/svg+xml</dc:format>         <dc:type            rdf:resource=\"http://purl.org/dc/dcmitype/StillImage\" />         <dc:title></dc:title>       </cc:Work>     </rdf:RDF>   </metadata>   <g      inkscape:label=\"Layer 1\"      inkscape:groupmode=\"layer\"      id=\"layer1\"      transform=\"translate(0,-980.36218)\">     <rect        style=\"opacity:0.1;fill:#1c1c1c;fill-opacity:0.5;stroke:none;stroke-width:3;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:0.1\"        id=\"rect4241\"        x=\"0\"        y=\"979.88373\"        width=\"112\"        height=\"72\" />     <path        style=\"opacity:1;fill:#ffffff;fill-opacity:1;stroke:#b6b5bb;stroke-width:2.5;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1\"        d=\"m 72.615234,1012.9774 -11.861328,0 0,-13.23045 -8.138672,0 0,13.47465 -13.230468,0 0,7.0508 13.230468,0 0,12.705 7.361328,0 0,-12.8554 12.638672,0 z\"        id=\"rect4136\"        inkscape:connector-curvature=\"0\"        sodipodi:nodetypes=\"ccccccccccccc\" />   </g> </svg>";

const char* groupsvg ="<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?> <!-- Created with Inkscape (http://www.inkscape.org/) -->  <svg    xmlns:dc=\"http://purl.org/dc/elements/1.1/\"    xmlns:cc=\"http://creativecommons.org/ns#\"    xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"    xmlns:svg=\"http://www.w3.org/2000/svg\"    xmlns=\"http://www.w3.org/2000/svg\"    xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\"    xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\"    width=\"112\"    height=\"72\"    viewBox=\"0 0 112 72.000001\"    id=\"svg2\"    version=\"1.1\"    inkscape:version=\"0.91 r13725\"    sodipodi:docname=\"group.svg\">   <defs      id=\"defs4\" />   <sodipodi:namedview      id=\"base\"      pagecolor=\"#ffffff\"      bordercolor=\"#666666\"      borderopacity=\"1.0\"      inkscape:pageopacity=\"0\"      inkscape:pageshadow=\"2\"      inkscape:zoom=\"3.959798\"      inkscape:cx=\"12.62558\"      inkscape:cy=\"59.968413\"      inkscape:document-units=\"px\"      inkscape:current-layer=\"layer1\"      showgrid=\"false\"      units=\"px\"      inkscape:window-width=\"1920\"      inkscape:window-height=\"1022\"      inkscape:window-x=\"0\"      inkscape:window-y=\"0\"      inkscape:window-maximized=\"1\"      showborder=\"true\" />   <metadata      id=\"metadata7\">     <rdf:RDF>       <cc:Work          rdf:about=\"\">         <dc:format>image/svg+xml</dc:format>         <dc:type            rdf:resource=\"http://purl.org/dc/dcmitype/StillImage\" />         <dc:title></dc:title>       </cc:Work>     </rdf:RDF>   </metadata>   <g      inkscape:label=\"Layer 1\"      inkscape:groupmode=\"layer\"      id=\"layer1\"      transform=\"translate(0,-980.36218)\">     <rect        style=\"opacity:1;fill:#1c1c1c;fill-opacity:0;stroke:none;stroke-width:3;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1\"        id=\"rect4241\"        x=\"0\"        y=\"979.88373\"        width=\"112\"        height=\"72\" />     <g        id=\"g3\"        style=\"fill:#ffffff\"        transform=\"matrix(2.3372461,0,0,2.2615907,38.000449,998.36218)\">       <path          d=\"m 9.429,11.411 c 4.71,1.681 3.195,4.507 3.195,4.507 l -9.958,0 c 0,0 -0.773,-3.161 3.465,-4.507 0,0 1.682,1.412 3.298,0 z\"          id=\"path5\"          style=\"fill:#ffffff\"          inkscape:connector-curvature=\"0\" />       <path          d=\"m 4.92,6.682 c 0,0 0.27,3.938 2.793,3.938 2.523,0 2.826,-3.668 2.826,-3.668 0,0 0.101,-3.194 -2.826,-3.194 -2.927,0 -2.793,2.924 -2.793,2.924 z\"          id=\"path7\"          style=\"fill:#ffffff;stroke:none\"          inkscape:connector-curvature=\"0\" />       <path          d=\"M 3.359,7.276 C 3.354,7.184 3.343,6.756 3.452,6.194 2.931,6.094 2.611,5.858 2.611,5.858 -0.539,6.903 0.042,9.088 0.042,9.088 l 3.651,0 C 3.428,8.178 3.368,7.413 3.361,7.313 L 3.359,7.276 Z\"          id=\"path9\"          style=\"fill:#ffffff\"          inkscape:connector-curvature=\"0\" />       <path          d=\"m 12.1,7.522 0,0.041 -0.004,0.04 c -0.035,0.427 -0.127,0.944 -0.29,1.485 l 3.554,0 c 0,0 0.583,-2.186 -2.567,-3.23 0,0 -0.319,0.234 -0.835,0.335 0.153,0.684 0.146,1.221 0.142,1.329 z\"          id=\"path11\"          style=\"fill:#ffffff\"          inkscape:connector-curvature=\"0\" />       <path          d=\"M 5.904,2.525 C 5.923,2.403 5.93,2.328 5.93,2.328 5.93,2.328 6.002,0 3.88,0 1.758,0 1.855,2.131 1.855,2.131 c 0,0 0.18,2.606 1.781,2.85 C 3.812,4.468 4.097,3.918 4.558,3.436 4.859,3.121 5.297,2.774 5.904,2.525 Z\"          id=\"path13\"          style=\"fill:#ffffff\"          inkscape:connector-curvature=\"0\" />       <path          d=\"M 11.758,4.99 C 13.422,4.82 13.627,2.328 13.627,2.328 13.627,2.328 13.699,0 11.576,0 9.455,0 9.553,2.131 9.553,2.131 c 0,0 0.013,0.183 0.061,0.454 0.519,0.238 0.905,0.548 1.18,0.832 0.468,0.484 0.77,1.042 0.964,1.573 z\"          id=\"path15\"          style=\"fill:#ffffff\"          inkscape:connector-curvature=\"0\" />     </g>   </g> </svg>";

const char *groupsvg2 ="<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?> <!-- Created with Inkscape (http://www.inkscape.org/) --> <svg xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:cc=\"http://creativecommons.org/ns#\" xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\" xmlns:svg=\"http://www.w3.org/2000/svg\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\" xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" width=\"40\" height=\"40\" viewBox=\"0 0 40 40.000001\" id=\"svg2\" version=\"1.1\" inkscape:version=\"0.91 r13725\" sodipodi:docname=\"tmp.svg\"> <defs id=\"defs4\" /> <sodipodi:namedview id=\"base\" pagecolor=\"#ffffff\" bordercolor=\"#666666\" borderopacity=\"1.0\" inkscape:pageopacity=\"0\" inkscape:pageshadow=\"2\" inkscape:zoom=\"3.959798\" inkscape:cx=\"-34.346513\" inkscape:cy=\"59.968413\" inkscape:document-units=\"px\" inkscape:current-layer=\"layer1\" showgrid=\"false\" units=\"px\" inkscape:window-width=\"1920\" inkscape:window-height=\"1022\" inkscape:window-x=\"0\" inkscape:window-y=\"0\" inkscape:window-maximized=\"1\" showborder=\"true\" /> <metadata id=\"metadata7\"> <rdf:RDF> <cc:Work rdf:about=\"\"> <dc:format>image/svg+xml</dc:format> <dc:type rdf:resource=\"http://purl.org/dc/dcmitype/StillImage\" /> <dc:title></dc:title> </cc:Work> </rdf:RDF> </metadata> <g inkscape:label=\"Layer 1\" inkscape:groupmode=\"layer\" id=\"layer1\" transform=\"translate(0,-1012.3622)\"> <rect style=\"opacity:1;fill:#1c1c1c;fill-opacity:0;stroke:none;stroke-width:3;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1\" id=\"rect4241\" x=\"0\" y=\"979.88373\" width=\"112\" height=\"72\" /> <g id=\"g3\" style=\"fill:#333333\" transform=\"matrix(2.5969401,0,0,2.5128785,4.9880721e-4,1012.3622)\"> <path d=\"m 9.429,11.411 c 4.71,1.681 3.195,4.507 3.195,4.507 l -9.958,0 c 0,0 -0.773,-3.161 3.465,-4.507 0,0 1.682,1.412 3.298,0 z\" id=\"path5\" style=\"fill:#333333\" inkscape:connector-curvature=\"0\" /> <path d=\"m 4.92,6.682 c 0,0 0.27,3.938 2.793,3.938 2.523,0 2.826,-3.668 2.826,-3.668 0,0 0.101,-3.194 -2.826,-3.194 -2.927,0 -2.793,2.924 -2.793,2.924 z\" id=\"path7\" style=\"fill:#333333;stroke:none\" inkscape:connector-curvature=\"0\" /> <path d=\"M 3.359,7.276 C 3.354,7.184 3.343,6.756 3.452,6.194 2.931,6.094 2.611,5.858 2.611,5.858 -0.539,6.903 0.042,9.088 0.042,9.088 l 3.651,0 C 3.428,8.178 3.368,7.413 3.361,7.313 L 3.359,7.276 Z\" id=\"path9\" style=\"fill:#333333\" inkscape:connector-curvature=\"0\" /> <path d=\"m 12.1,7.522 0,0.041 -0.004,0.04 c -0.035,0.427 -0.127,0.944 -0.29,1.485 l 3.554,0 c 0,0 0.583,-2.186 -2.567,-3.23 0,0 -0.319,0.234 -0.835,0.335 0.153,0.684 0.146,1.221 0.142,1.329 z\" id=\"path11\" style=\"fill:#333333\" inkscape:connector-curvature=\"0\" /> <path d=\"M 5.904,2.525 C 5.923,2.403 5.93,2.328 5.93,2.328 5.93,2.328 6.002,0 3.88,0 1.758,0 1.855,2.131 1.855,2.131 c 0,0 0.18,2.606 1.781,2.85 C 3.812,4.468 4.097,3.918 4.558,3.436 4.859,3.121 5.297,2.774 5.904,2.525 Z\" id=\"path13\" style=\"fill:#333333\" inkscape:connector-curvature=\"0\" /> <path d=\"M 11.758,4.99 C 13.422,4.82 13.627,2.328 13.627,2.328 13.627,2.328 13.699,0 11.576,0 9.455,0 9.553,2.131 9.553,2.131 c 0,0 0.013,0.183 0.061,0.454 0.519,0.238 0.905,0.548 1.18,0.832 0.468,0.484 0.77,1.042 0.964,1.573 z\" id=\"path15\" style=\"fill:#333333\" inkscape:connector-curvature=\"0\" /> </g> </g> </svg>";

const char* transfersvg ="<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?> <!-- Created with Inkscape (http://www.inkscape.org/) -->  <svg xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:cc=\"http://creativecommons.org/ns#\" xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\" xmlns:svg=\"http://www.w3.org/2000/svg\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\" xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" width=\"112\" height=\"72\" viewBox=\"0 0 112 72.000001\" id=\"svg2\" version=\"1.1\" inkscape:version=\"0.91 r13725\" sodipodi:docname=\"transfer.svg\">   <defs   id=\"defs4\" />   <sodipodi:namedview   id=\"base\"   pagecolor=\"#ffffff\"   bordercolor=\"#666666\"   borderopacity=\"1.0\"   inkscape:pageopacity=\"0\"   inkscape:pageshadow=\"2\"   inkscape:zoom=\"3.959798\"   inkscape:cx=\"12.62558\"   inkscape:cy=\"59.968413\"   inkscape:document-units=\"px\"   inkscape:current-layer=\"layer1\"   showgrid=\"false\"   units=\"px\"   inkscape:window-width=\"1920\"   inkscape:window-height=\"1022\"   inkscape:window-x=\"0\"   inkscape:window-y=\"0\"   inkscape:window-maximized=\"1\"   showborder=\"true\" />   <metadata   id=\"metadata7\">  <rdf:RDF>    <cc:Work    rdf:about=\"\">   <dc:format>image/svg+xml</dc:format>   <dc:type   rdf:resource=\"http://purl.org/dc/dcmitype/StillImage\" />   <dc:title></dc:title>    </cc:Work>  </rdf:RDF>   </metadata>   <g   inkscape:label=\"Layer 1\"   inkscape:groupmode=\"layer\"   id=\"layer1\"   transform=\"translate(0,-980.36218)\">  <rect  style=\"opacity:1;fill:#1c1c1c;fill-opacity:0;stroke:none;stroke-width:3;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1\"  id=\"rect4241\"  x=\"0\"  y=\"979.88373\"  width=\"112\"  height=\"72\" />  <g  transform=\"matrix(2.3073119,0,0,2.3071848,37.998502,998.36232)\"  id=\"g3\">    <path    style=\"fill:#ffffff\"    inkscape:connector-curvature=\"0\"    d=\"M 6.303,15.588 C 6.199,15.57 6.063,15.531 5.986,15.438 5.909,15.345 5.837,15.137 5.867,14.834 L 6.482,8.78 C 6.505,8.55 6.597,8.288 6.779,8.319 6.912,8.342 7.119,8.483 7.331,8.694 l 1.4,1.4 4.144,-4.145 c 0.076,-0.076 0.191,-0.105 0.321,-0.084 0.131,0.023 0.263,0.096 0.37,0.203 l 1.828,1.828 c 0.223,0.223 0.277,0.533 0.117,0.691 l -4.143,4.143 1.4,1.4 c 0.293,0.293 0.387,0.508 0.377,0.602 -0.01,0.094 -0.138,0.215 -0.463,0.248 L 6.627,15.595 C 6.513,15.608 6.402,15.606 6.303,15.588 l 0,0 z\"    id=\"path5\" />    <path    style=\"fill:#ffffff\"    inkscape:connector-curvature=\"0\"    d=\"M 9.303,0.015 C 9.405,0.033 9.543,0.072 9.619,0.165 9.696,0.259 9.77,0.467 9.738,0.77 L 9.122,6.825 C 9.1,7.055 9.006,7.315 8.825,7.284 8.691,7.26 8.484,7.122 8.273,6.911 l -1.4,-1.4 -4.144,4.144 C 2.652,9.731 2.539,9.76 2.408,9.737 2.277,9.715 2.146,9.645 2.037,9.536 L 0.209,7.708 C -0.014,7.485 -0.066,7.175 0.092,7.015 l 4.143,-4.143 -1.4,-1.4 C 2.543,1.181 2.45,0.967 2.46,0.872 2.47,0.778 2.597,0.657 2.922,0.625 L 8.978,0.008 c 0.114,-0.013 0.223,-0.01 0.325,0.007 l 0,0 z\"    id=\"path7\" />  </g>   </g> </svg>";

const char* settingssvg ="<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?> <!-- Created with Inkscape (http://www.inkscape.org/) --> <svg xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:cc=\"http://creativecommons.org/ns#\" xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\" xmlns:svg=\"http://www.w3.org/2000/svg\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\" xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" width=\"112\" height=\"72\" viewBox=\"0 0 112 72.000001\" id=\"svg2\" version=\"1.1\" inkscape:version=\"0.91 r13725\" sodipodi:docname=\"transfer.svg\"> <defs id=\"defs4\" /> <sodipodi:namedview id=\"base\" pagecolor=\"#ffffff\" bordercolor=\"#666666\" borderopacity=\"1.0\" inkscape:pageopacity=\"0\" inkscape:pageshadow=\"2\" inkscape:zoom=\"3.959798\" inkscape:cx=\"12.62558\" inkscape:cy=\"59.968413\" inkscape:document-units=\"px\" inkscape:current-layer=\"layer1\" showgrid=\"false\" units=\"px\" inkscape:window-width=\"1920\" inkscape:window-height=\"1022\" inkscape:window-x=\"0\" inkscape:window-y=\"0\" inkscape:window-maximized=\"1\" showborder=\"true\" /> <metadata id=\"metadata7\"> <rdf:RDF> <cc:Work rdf:about=\"\"> <dc:format>image/svg+xml</dc:format> <dc:type rdf:resource=\"http://purl.org/dc/dcmitype/StillImage\" /> <dc:title></dc:title> </cc:Work> </rdf:RDF> </metadata> <g inkscape:label=\"Layer 1\" inkscape:groupmode=\"layer\" id=\"layer1\" transform=\"translate(0,-980.36218)\"> <rect style=\"opacity:1;fill:#1c1c1c;fill-opacity:0;stroke:none;stroke-width:3;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1\" id=\"rect4241\" x=\"0\" y=\"979.88373\" width=\"112\" height=\"72\" /> <path inkscape:connector-curvature=\"0\" style=\"fill:#ffffff\" d=\"m 74,1014.5779 c 0,-0.4542 -0.375793,-0.8253 -0.82536,-0.8253 l -1.521614,0 c -0.45879,0 -0.938329,-0.3551 -1.06513,-0.7954 l -2.065706,-4.8227 c -0.21902,-0.3942 -0.14755,-0.9798 0.172911,-1.3072 l 1.028242,-1.0258 c 0.322767,-0.3228 0.322767,-0.8461 0,-1.1688 l -2.473775,-2.476 c -0.320461,-0.3227 -0.84611,-0.3227 -1.175793,0 l -1.113544,1.1158 c -0.327378,0.3274 -0.919885,0.415 -1.323343,0.2075 l -4.195966,-1.6829 c -0.43804,-0.1222 -0.802305,-0.5971 -0.802305,-1.0558 l 0,-1.55612 c 0,-0.44954 -0.371182,-0.823 -0.82075,-0.823 l -3.504322,0 c -0.454179,0 -0.827666,0.37346 -0.827666,0.823 l 0,1.55612 c 0,0.4587 -0.359654,0.9359 -0.793084,1.072 l -4.864553,2.0817 c -0.391931,0.2282 -0.972911,0.1521 -1.297983,-0.166 l -1.0951,-1.0927 c -0.318156,-0.3205 -0.84611,-0.3205 -1.166571,0 l -2.476081,2.4782 c -0.325072,0.3227 -0.325072,0.846 0,1.1688 l 1.18732,1.1965 c 0.329683,0.3227 0.417291,0.9129 0.205187,1.314 l -1.655331,4.1611 c -0.12219,0.4403 -0.597118,0.7977 -1.055908,0.7977 l -1.65072,0 c -0.454179,0 -0.823055,0.3711 -0.823055,0.8253 l 0,3.5018 c 0,0.4564 0.368876,0.8299 0.823055,0.8299 l 1.65072,0 c 0.45879,0 0.938329,0.355 1.069741,0.793 l 2.033429,4.7859 c 0.232853,0.3942 0.149856,0.9797 -0.165994,1.3002 l -1.138905,1.1365 c -0.320461,0.3228 -0.320461,0.8461 0,1.1688 l 2.480692,2.4759 c 0.322766,0.3228 0.848415,0.3228 1.164265,0 l 1.217291,-1.2103 c 0.325072,-0.3227 0.908357,-0.408 1.314121,-0.1913 l 4.24438,1.6921 c 0.43343,0.1268 0.793084,0.604 0.793084,1.0558 l 0,1.6184 c 0,0.4564 0.373487,0.8276 0.827666,0.8276 l 3.504322,0 c 0.449568,0 0.82075,-0.3712 0.82075,-0.8276 l 0,-1.6184 c 0,-0.4518 0.359654,-0.929 0.802305,-1.0627 l 4.746974,-2.0149 c 0.396542,-0.2236 0.972911,-0.1452 1.300288,0.1729 l 1.072047,1.0789 c 0.320461,0.3205 0.848415,0.3205 1.168876,0 l 2.480691,-2.4805 c 0.320461,-0.3204 0.320461,-0.8484 0,-1.1665 l -1.145821,-1.1411 c -0.31585,-0.3228 -0.405764,-0.9129 -0.189049,-1.3141 l 1.719885,-4.281 c 0.119884,-0.438 0.599423,-0.7953 1.058213,-0.7999 l 1.521614,0 c 0.449567,0 0.82536,-0.3712 0.82536,-0.8253 l 0,-3.5041 z m -17.998847,7.6675 c -3.253026,0 -5.883574,-2.635 -5.883574,-5.8855 0,-3.2505 2.630548,-5.8832 5.883574,-5.8832 3.248415,0 5.888184,2.6327 5.888184,5.8832 0,3.2505 -2.639769,5.8855 -5.888184,5.8855 z\" id=\"path3\" /> </g> </svg>";

