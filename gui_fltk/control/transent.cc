#include <math.h>

#include <FL/fl_draw.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Native_File_Chooser.H>
#include "xwintox.h"
#include "GFL_FILE.h"

#include "control/xwin.h"
#include "control/gtrnsfer.h"
#include "control/transent.h"
#include "util.h"
#include "misc.h"

void teSaveasPressed (Fl_Widget * w)
{
    TransfersEntry * te = (TransfersEntry *)w->parent ();
    Fl_Native_File_Chooser fnfc;
    fnfc.title ("Save as");
    fnfc.type (Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    int res = fnfc.show ();
    if (res == 1 | res == -1)
        return;
    else
    {
        free (te->transfer->localfilename_);
        te->transfer->localfilename_ = strdup (fnfc.filename ());
    }
}

TransfersEntry::TransfersEntry (int X, int Y, int W, int H, int S,
                                struct tm * Time, GFLTransfer * T, int I)
    : Fl_Group (X, Y, W, 50 * S), scale (S), transfer (T), inv (I),
      accept (0, 0, 1, 1, "Accept"), reject (0, 0, 1, 1, "Reject"),
      saveto (0, 0, 1, 1, "Save as"), progress (0, 0, 1, 1)
{
    strftime (date, 255, "%d/%b/%y %H:%M", Time);

    fl_font (FL_HELVETICA, 11 * scale);
    dl = fl_width (date) + 20;

    accept.color (fl_rgb_color (118, 202, 116));
    reject.color (fl_rgb_color (214, 78, 77));
    saveto.color (fl_rgb_color (160, 84, 160));
    progress.color (fl_rgb_color (209, 210, 214));

    accept.labelsize (11 * scale);
    reject.labelsize (11 * scale);
    saveto.labelsize (11 * scale);

    saveto.callback (teSaveasPressed);

    progress.selection_color (fl_rgb_color (118, 202, 116));
    progress.labelcolor (fl_rgb_color (25, 25, 50));
    progress.minimum (0);
    progress.maximum (1000);
    // progress.value((transfer->pos / transfer->size) * 1000);
    progress.labelsize (11.2 * scale);
    progress.labelfont (FL_HELVETICA_BOLD);
    progress.deactivate ();

    box (FL_FLAT_BOX);

    resize (X, Y, w (), h ());
    end ();
}

void TransfersEntry::resize (int X, int Y, int W, int H)
{
    Fl_Group::resize (X, Y, W, H);
    accept.resize (X + w () - (78 * scale), y () + (2 * scale), 54 * scale,
                   (h () / 2) - 2 * scale);
    reject.resize (X + w () - (78 * scale), y () + (h () / 2) + (2 * scale),
                   54 * scale, (h () / 2) - 4 * scale);
    saveto.resize (X + w () - (78 * scale) - (59 * scale), y () + (2 * scale),
                   54 * scale, (h () / 2) - 2 * scale);
    progress.resize (x () + dl, y () + (30 * scale), w () - dl - (83 * scale),
                     (16 * scale));
}

void TransfersEntry::draw ()
{
    int cut;
    char from[255] = {0};
    char proglabel[255] = {0};
    float percent = (((double)transfer->pos_) / transfer->size_) * 100;
    int val = (((double)transfer->pos_) / transfer->size_) * 1000;

    if (inv == 1)
        color (fl_rgb_color (198, 199, 214));
    else
        color (fl_rgb_color (239, 239, 239));

    sprintf (proglabel, "%.1f %%", percent);
    progress.value (val);
    progress.label (proglabel);
    progress.redraw ();

    Fl_Group::draw ();

    sprintf (from, "From: %s", GetDisplayName (transfer->contact_, 100));

    fl_font (FL_HELVETICA, 11 * scale);
    fl_color (fl_rgb_color (110, 110, 118));
    fl_draw (date, x () + (10 * scale), y () + (16 * scale));
    fl_draw (from, x () + (10 * scale), y () + (30 * scale));
    fl_draw (GetDisplaySize (transfer->size_), x () + (10 * scale),
             y () + (44 * scale));

    fl_color (fl_rgb_color (85, 85, 100));
    fl_font (FL_HELVETICA_BOLD, 12 * scale);

    for (cut = 255;
         cut > 0 && (fl_width ((GetShortenedText ((char *)transfer->filename_,
                                                  cut))) > w () - 140 - dl);
         cut = cut - 5)
        ;

    fl_draw (GetShortenedText ((char *)transfer->filename_, cut), x () + dl,
             y () + (16 * scale));
}
