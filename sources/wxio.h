// File: wxio.h--defines an input/output class based on gio for wxwindows
// This creates a text window that will accept output and provide input
// using the << and >> redirection operators.  One instance of this class
// will always be created and called gdebug, (see also gerror,gin,gout).
// $Id$
#ifndef 	WXIO_H
#define WXIO_H

#include <stdio.h>
#include <assert.h>
#include "wx.h"
#include "gambitio.h"

#ifdef __GNUG__
#pragma interface
#endif   // __GNUG__


class gWxInput: public gInput, public wxFrame
{
private:
	wxTextWindow	*f;
	char					line_buffer[100];
	bool						ok,shown;
	gWxInput(const gFileInput &);
	gWxInput &operator=(const gFileInput &);
public:
	// Constructors
	gWxInput(void):wxFrame(0,"gWxInput",0,0,200,200)
	{
	f=new wxTextWindow(this,-1,-1,-1,-1,wxNATIVE_IMPL);
	ok=(f) ? true:false;
	Show(TRUE);
	}

	gWxInput(const char *in):wxFrame(0,(char *)in,0,0,200,200)
	{
	f=new wxTextWindow(this,-1,-1,-1,-1,wxNATIVE_IMPL);
	ok=(f) ? true:false;
	Show(TRUE);
	}
// You can close the window in the sense that it is no longer visible, but it
// is never deleted.  In the GUI (windows) the printf function is overloaded
// to output to wout.  wout is created at startup.

	Bool OnClose(void) {f->Clear();Show(FALSE);return FALSE;}
	Bool Show(Bool show) {shown=show;wxWindow::Show(show);return shown;}

//
// Close the window pointed to, if any.
//
	~gWxInput()   {f=0; }

// Load a text file into the input window (this is unique to WxInput)
	void Load(const char *file) {f->LoadFile((char *)file);}

//
// Input primitives for the builtin types.

		gInput& operator>>(int &x)
			{ assert(f);  f->GetLineText(f->GetNumberOfLines(),line_buffer); ok=sscanf(line_buffer, "%d", &x);   return *this; }
		gInput& operator>>(unsigned int &x)
			{ assert(f);  f->GetLineText(f->GetNumberOfLines(),line_buffer); ok=sscanf(line_buffer, "%d", &x);   return *this; }
		gInput& operator>>(long &x)
			{ assert(f);  f->GetLineText(f->GetNumberOfLines(),line_buffer); ok=sscanf(line_buffer, "%ld", &x);   return *this; }
		gInput& operator>>(char &x)
			{ assert(f);  f->GetLineText(f->GetNumberOfLines(),line_buffer); ok=sscanf(line_buffer, "%c", &x);   return *this; }
		gInput& operator>>(double &x)
			{ assert(f);  f->GetLineText(f->GetNumberOfLines(),line_buffer); ok=sscanf(line_buffer, "%lf", &x);  return *this; }
		gInput& operator>>(float &x)
			{ assert(f);  f->GetLineText(f->GetNumberOfLines(),line_buffer); ok=sscanf(line_buffer, "%f", &x);   return *this; }
		gInput& operator>>(char *x)
			{ assert(f);  f->GetLineText(f->GetNumberOfLines()-1,line_buffer); ok=sscanf(line_buffer, "%s", x);
				return *this;
			}
//-grp

//
// Unget the character given.  Only one character of pushback is guaranteed.
//
		int get(char &)	{return 1;}
		void unget(char c) {;}
		bool eof(void) const {return 0;};

//
// Returns nonzero if the end-of-file marker has been reached.
//
		bool IsValid(void) const	{return ok;}
		void seekp(long ) const {;}
};

//
// This class is a (simple) implementation of an output stream.  It is built
// around the C stdio library in preference to the C++ iostream library
// since some compilers (notable GNU C++) suffer from executable bloat
// when iostreams are used.
//
class gWxOutput: public gOutput, public wxFrame
{
	private:
	wxTextWindow	*f;
	char					buffer[100];
	bool					ok,shown;
	int Width,Prec;
	char Represent;

	//
// The copy constructor and assignment operator are declared private to
// override the default meanings of these functions.  They are never
// implemented.
//+grp
		gWxOutput(const gFileOutput &);
		gWxOutput &operator=(const gFileOutput &);
//-grp

	public:
//
// The default constructor, initializing the instance to point to no stream
//
		gWxOutput(void):wxFrame(NULL,"gWxOutput",0,0,200,200)
		{
		f=new wxTextWindow(this,-1,-1,-1,-1,wxREADONLY);
		ok=(f) ? true:false;
		Width=0;
		Prec=6;
		Represent='f';
		Show(TRUE);
		}

		gWxOutput(const char *out):wxFrame(NULL,(char *)out,0,0,200,200)
		{
		f=new wxTextWindow(this,-1,-1,-1,-1,wxREADONLY);
		ok=(f) ? true:false;
		Width=0;
		Prec=6;
		Represent='f';
		Show(TRUE);
		}
// You can close the window in the sense that it is no longer visible, but it
// is never deleted.  In the GUI (windows) the printf function is overloaded
// to output to wout.  wout is created at startup.
	Bool OnClose(void) {f->Clear();Show(FALSE);return FALSE;}
	Bool Show(Bool show) {shown=show;wxWindow::Show(show);return shown;}
//
// Close the window pointed to, if any.
//
		~gWxOutput()   { }

//
// Output primitives for the basic types

		gOutput& operator<<(int x)
			{ assert(f);  if (!shown) Show(TRUE); (*f)<<x; return *this; }
		gOutput& operator<<(unsigned int x)
			{ assert(f);  if (!shown) Show(TRUE); (*f)<<(int)x; return *this; }
		gOutput& operator<<(long x)
			{ assert(f);  if (!shown) Show(TRUE); (*f)<<x; return *this; }
		gOutput& operator<<(char x)
			{ assert(f);  if (!shown) Show(TRUE); (*f)<<x; return *this; }
		gOutput& operator<<(double x)
			{ assert(f);  if (!shown) Show(TRUE); (*f)<<x; return *this; }
		gOutput& operator<<(float x)
			{ assert(f);  if (!shown) Show(TRUE); (*f)<<x; return *this; }
		gOutput& operator<<(const char *x)
			{ assert(f);  if (!shown) Show(TRUE); (*f)<<(char *)x;  return *this; }
		gOutput& operator<<(const void *x)
			{ assert(f);  if (!shown) Show(TRUE); sprintf(buffer, "%p", x); (*f)<<buffer; return *this; }

		bool IsValid(void) const {return ok;}

		// Functions to control the appearance of the output
		int GetWidth(void) {return Width;}
		gOutput &SetWidth(int w) {Width=w;return (*this);}
		int GetPrec(void) {return Prec;}
		gOutput &SetPrec(int p) {Prec=p;return (*this);}
		gOutput &SetExpMode(void) {Represent = 'e';return *this;}
		gOutput &SetFloatMode(void){Represent = 'f';return *this;}
		char GetRepMode(void) {return Represent;}

};

#define gWXOUT		"wout"
#define gWXERR		"werr"
#define gWXIN			"win"

extern gWxOutput *wout,*werr;
#endif   // WXIO_H
