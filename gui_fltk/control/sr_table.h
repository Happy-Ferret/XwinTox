#ifndef SR_TABLE_H__
#define SR_TABLE_H__

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Table_Row.H>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <vector>
#include <algorithm> // STL sort

#define MARGIN (18 * scale)

// Font face/sizes for header and rows
#define HEADER_FONTFACE FL_HELVETICA_BOLD
#define HEADER_FONTSIZE 13.5 * scale
#define ROW_FONTFACE FL_HELVETICA
#define ROW_FONTSIZE 11.5 * scale

// A single row of columns
class Row
{
public:
	void *userdata;
	std::vector<char*> cols;
};

// Sort class to handle sorting column using std::sort
class SortColumn
{
	int _col, _reverse;
public:
	SortColumn(int col, int reverse)
	{
		_col = col;
		_reverse = reverse;
	}
	bool operator()(const Row &a, const Row &b)
	{
		const char *ap = (_col < (int)a.cols.size()) ? a.cols[_col] : "",
		            *bp = (_col < (int)b.cols.size()) ? b.cols[_col] : "";

		if(isdigit(*ap) && isdigit(*bp))      // cheezy detection of numeric data
		{
// Numeric sort
			int av=0;
			sscanf(ap, "%d", &av);
			int bv=0;
			sscanf(bp, "%d", &bv);
			return(_reverse ? av < bv : bv < av);
		}
		else
		{
// Alphabetic sort
			return(_reverse ? strcmp(ap, bp) > 0 : strcmp(ap, bp) < 0);
		}
	}
};

class MyTable;

typedef void (*fnCellCB) (MyTable *w, void* rowdata, void* userdata);

// Derive a custom class from Fl_Table_Row
class MyTable : public Fl_Table_Row
{
private:
	std::vector<Row> _rowdata; // data in each row
	int _sort_reverse;
	int _sort_lastcol;
	static void event_callback(Fl_Widget*, void*);
	void event_callback2(); // callback for table events
	void *userdatacellcallback;
	fnCellCB cellcallback;
protected:
	void draw_cell(TableContext context, int R=0, int C=0, // table cell drawing
	               int X=0, int Y=0, int W=0, int H=0);
	void sort_column(int col, int reverse=0); // sort table by a column
	void draw_sort_arrow(int X,int Y,int W,int H);
	int scale;
public:
// Ctor
	MyTable(int x, int y, int w, int h, int s, const char **Gh, const char *l=0)
		: Fl_Table_Row(x,y,w,h,l)
	{
		G_header =(Gh);
		scale =s;
		_sort_reverse = 0;
		_sort_lastcol = -1;
		end();
		callback(event_callback, (void*)this);
	}
	~MyTable() { } // Dtor
	void load_command(void *userdata, std::vector <const char*> columns);
	void autowidth(int pad); // Automatically set column widths to data
	void resize_window(); // Resize parent window to size of table
	void setcellcallback(fnCellCB cb, void* userdata)
	{
		cellcallback =cb;
		userdatacellcallback =userdata;
	}
	void clearall()
	{
		for (const auto row : _rowdata)
		{
			for (const auto col : row.cols)
			{
				free(col);
			}
		}
		_rowdata.clear();
		cols(0);
		rows(0);
		clear();
	}
	const char **G_header;
};

#endif