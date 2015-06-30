#include <ctype.h>

#include <FL/Fl.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/fl_draw.H>

#include "resolv/dresolv.h"

#include "control/gui.h"
#include "control/gtrnsfer.h"

GTransfers::GTransfers(int S) : GArea(S, "File Transfers")
{
	scale = S;

	list =new TransfersList(0, 0, 1, 1, scale);

	resize(x(), y(), w(), h());
	end();
}

void GTransfers::resize(int X, int Y, int W, int H)
{
	Fl_Group::resize(Xw->sblength * scale,
	                 Xw->basey * scale,
	                 Xw->w() - (Xw->sblength * scale),
	                 Xw->h()- (Xw->basey * scale));
	list->resize(x(), y() + (61 * scale), w(), h() - (61 * scale));
}

void GTransfers::draw()
{
	GArea::draw();
}
