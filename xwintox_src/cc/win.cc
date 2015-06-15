#include <stdio.h>

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/fl_draw.H>

#include "contacts.h"
#include "xwintox_win.h"
#include "svgs.h"
#include "nanosvg/nsvgwrap.h"

extern class XwinTox *XwinTox;

ContactsEntry::ContactsEntry(int X, int Y, int S, Contact_t *C) : Fl_Box (X, Y, 224 * S, 50 * S)
{
	scale =S;
	contact =C;
	selected =0;

	icon = new SVGBox(X+(4 * S), Y+(2 * S), 46 * S, 46 *S, S, default_av, 0.35);

	box(FL_FLAT_BOX);
	color(fl_rgb_color(65, 65, 65));
}

void ContactsEntry::draw()
{
	int txt_color =255;
	char name[255] = { 0 }, status[255] = { 0 };
	if (selected) { color(255); txt_color =0; }
	else {color (fl_rgb_color(65, 65, 65)); txt_color=255; }

	if (strlen(contact->name) == 0)
	{ 
		strncpy(name, contact->pubkey, 14);
		name[14] ='.'; name[15] ='.'; name[16] ='.';
		if (strlen(contact->statusm) == 0) strcpy (status, "Unknown");
	}
	else if (strlen(contact->name) >= 15)
	{
		strncpy(name, contact->name, 15);
		name[15] ='.'; name[16] ='.'; name[17] ='.';
	}
	else
	{
		strcpy(name, contact->name);
	}

	if (strlen(contact->statusm) >= 20)
	{
		strncpy(status, contact->statusm, 20);
		status[20] ='.'; status[21] ='.'; status[22] ='.';
	}
	else if (strlen (contact->statusm) > 0)
	{
		strcpy(status, contact->statusm);
	}
	
	Fl_Box::draw();
	fl_color(txt_color);
	fl_font(FL_HELVETICA, 12 * scale);
	fl_draw(name, x() + (50 * scale), y() + (22 * scale));
	fl_font(FL_HELVETICA, 10 * scale);
	fl_draw(status, x() + (50 * scale), y() + (36 * scale));

	fl_color(2);
	fl_pie(x() + (185 * scale), this->y() + (20 * scale), 10 * scale,
		  10 * scale, 0, 360);
	icon->draw(); /* make it account for scrollbar::value(); */
}

int ContactsEntry::handle(int event) 
{
	switch(event) 
	{
	case FL_PUSH:
		selected =1;
		redraw(); icon->redraw();
		XwinTox->contents->NewCurrentArea(FindContactMArea(contact));
		return 1;
	}
	return 0;
}

ContactsList::ContactsList(int X, int Y, int W, int H, int S) : Fl_Scroll (X, Y, W, H)
{
	scale =S;
	color(fl_rgb_color(65, 65, 65));
	type(6);

}

void ContactsList::draw() 
{
	Fl_Scroll::draw();
	for (const auto entry : entries)
	{
		entry->icon->redraw();
	}
}
void ContactsList::resize(int X, int Y, int W, int H) 
{
	Fl_Scroll::resize(X, Y, W, H);
	for (const auto entry : entries)
	{
		entry->icon->draw();
	}
}

int ContactsList::handle(int event) 
{
	switch(event) 
	{
	case FL_PUSH:
		for (const auto entry : entries)
		{
		entry->selected =0;
		entry->redraw(); entry->icon->redraw();
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

StatusBox::StatusBox(int X, int Y, int W, int H, int S) : Fl_Box (X, Y, W, H)
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


SVGBox::SVGBox(int X, int Y, int W, int H, int S, const char* pic, double factor) : Fl_Box (X, Y, W, H)
{
	img_r =svgrast(pic, W, H, S, factor);
	img =new Fl_RGB_Image(img_r, W, H, 4);
	image(img);
}

void SVGBox::draw()
{
	Fl_Box::draw();
}

Sidebar_Top_Area::Sidebar_Top_Area(int S) : Fl_Group (0, 0, 224 * S, 60 * S)
{
	scale =S;

	box(FL_FLAT_BOX);
	color(fl_rgb_color(28, 28, 28));

	statusbox =new StatusBox(192 * S, 10 * S, 20 * S, 40 * S, S);
	avbox =new SVGBox(10 * S, 10 * S, 40 * S , 40 * S, S, default_av, 0.3);
	name =new Fl_Input(62 * S, 18 * S, 128 *S, 11 * S);
	status =new Fl_Input(62 * S, 30 * S, 128 *S, 11 * S);
	name->textsize(12 * S);
	status->textsize(10 * S);
	name->box(FL_FLAT_BOX); name->color(fl_rgb_color(28, 28, 28));
	status->box(FL_FLAT_BOX); status->color(fl_rgb_color(28, 28, 28));
	name->textcolor(255);
	status->textcolor(54);
	name->value("XwinTox User"); status->value("Toxing on XwinTox");

	end();
}

Sidebar_Bottom_Area::Sidebar_Bottom_Area(int S) : Fl_Group (0, 
										(480 * S) - (36 * S), 224 * S, 480 * S)
{
	scale =S;

	box(FL_FLAT_BOX);
	color(fl_rgb_color(28, 28, 28));

	addfriend =new SVGBox(0, (480 * S) - (36 * S), 56 * S , 36 * S, S, addfriendsvg, 0.5);
	newgroup =new SVGBox(56 * S, (480 * S) - (36 * S), 112 * S , 36 * S, S, groupsvg, 0.5);
	transfers =new SVGBox(112 * S, (480 * S) - (36 * S), 168 * S , 36 * S, S, transfersvg, 0.5);
	settings =new SVGBox(168 * S, (480 * S) - (36 * S), 212 * S , 36 * S, S, settingssvg, 0.5);

	end();
}


Sidebar::Sidebar(int S) : Fl_Group (0, 0, 224 * S, 480 * S)
{
	scale =S;

	box(FL_FLAT_BOX);
	color(fl_rgb_color(65, 65, 65));
	top_area =new Sidebar_Top_Area(S);
	bottom_area =new Sidebar_Bottom_Area(S);
	contacts =new ContactsList(0, 60 * S, (224 * S), h() - (36 * S) - (60 * S), S);
	end();
}

void Sidebar::resize(int X, int Y, int W, int H)
{
	Fl_Group::resize(X, Y, W, H);
	top_area->resize(0, 0, 224 * scale, 60 * scale);
	bottom_area->resize(0, H - (36 * scale), 224 * scale, 480 * scale);
	contacts->resize(0, 60 * scale, (224 * scale),
				    h() - (36 * scale) - (60 * scale));
} 


GArea::GArea(int S, const char *C) : Fl_Group (224 * S, 0, 416 * S, 480 * S)
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

GAddFriend::GAddFriend(int S) : GArea (S, "Add Friends")
{
	id =new Fl_Input(x() + (10 * S), y() + (90 * S), 
					(x() + w() - (224 * S) - (20 * S)), 
					24 * S);
	message =new Fl_Multiline_Input(x() + (10 * S), y() + (140 * S), 
									(x() + w() - (224 * S) - (20 * S)), 
					84 * S);
	printf("%d\n", Fl::w());

	send =new Fl_Button(parent()->w() - 62 * scale, y() + 234 * scale, 52 * scale,
						20 * scale, "Add");/*Fl::w() - 62 * scale, y() + 234 * scale, 52 * scale,
						20 * scale, "Add");*/

	id->textsize (12 * S); message->textsize (12 * S);
	message->value("Please accept this friend request.");
	send->color(fl_rgb_color(107, 194, 96));
	send->labelcolor(255);
	send->labelsize(14 * S);

	end();
}

void GAddFriend::resize(int X, int Y, int W, int H)
{
	Fl_Group::resize(X, Y, W, H);
	id->resize (x() + (10 * scale), y() + (90 * scale), 
				(x() + w() - (224 * scale) - (20 * scale)), 
				24 * scale);
	message->resize (x() + (10 * scale), y() + (140 * scale), 
					(x() + w() - (224 * scale) - (20 * scale)), 
					84 * scale);
	send->resize(parent()->w() - 62 * scale, y() + 234 * scale, 52 * scale,
						20 * scale);
}

void GAddFriend::draw()
{
	GArea::draw();

	fl_color(0);
	fl_font(FL_HELVETICA, 12 * scale);
	fl_draw("Tox ID", x() + (10 * scale), y() + 84 * scale);
	fl_draw("Message", x() + (10 * scale), y() + 134 * scale);
}

GMessageArea::GMessageArea(int S, Contact_t *C) : Fl_Group (224 * S, 0, 416 * S, 480 * S)
{
	contact =C;
	scale =S;

	box(FL_FLAT_BOX);
	color(255);

	icon = new SVGBox(x()+(12 * S), y()+(9 * S), 40 * S, 40 *S, S, default_av, 0.32);

	end();
}

void GMessageArea::draw()
{
	Fl_Group::draw();

	fl_color(0);
	fl_font(FL_HELVETICA_BOLD, 12 * scale);
	fl_draw(contact->name, x() + (60 * scale), y() + (26 * scale));

	fl_color(fl_rgb_color(192, 192, 192));
	fl_line(x(), y() + (60 * scale), Fl::w(), y() + (60 * scale));
	fl_color(0);
}

XWContents::XWContents(int S) : Fl_Box (224 * S, 0, 416 * S, 480 * S)
{
	scale =S;
	//resizable(0);
	box(FL_FLAT_BOX);
	color(4);

	addfriend =new GAddFriend(S);
	currentarea =addfriend;
}

void XWContents::NewCurrentArea(Fl_Group *G)
{
	currentarea->hide();
	this->newcurrentarea =G;
	currentarea =newcurrentarea;
	currentarea->show();
	currentarea->redraw();
}

void XWContents::draw()
{
	//if (currentarea != newcurrentarea)
//	currentarea->hide();
	//currentarea =newcurrentarea;
	//currentarea->show();
	//Fl_Box::draw();
}

XwinTox::XwinTox(int w, int h, const char* c, int S) : Fl_Double_Window(w, h, c) 
{
	scale =S;

	box(FL_FLAT_BOX);
	color(255);

	contents =new XWContents(S);
	sidebar =new Sidebar(S);
	resizable(contents);
}

void XwinTox::resize (int X, int Y, int W, int H)
{
	Fl_Double_Window::resize (X, Y, W, H);
	contents->resize(224 * scale, 0, W - (224 * scale), H);
}

