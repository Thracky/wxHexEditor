/***********************************(GPL)********************************
*   wxHexEditor is a hex edit tool for editing massive files in Linux   *
*   Copyright (C) 2006  Erdem U. Altinyurt                              *
*                                                                       *
*   This program is free software; you can redistribute it and/or       *
*   modify it under the terms of the GNU General Public License         *
*   as published by the Free Software Foundation; either version 2      *
*   of the License, or any later version.                               *
*                                                                       *
*   This program is distributed in the hope that it will be useful,     *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of      *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the       *
*   GNU General Public License for more details.                        *
*                                                                       *
*   You should have received a copy of the GNU General Public License   *
*   along with this program;                                            *
*   if not, write to the Free Software	Foundation, Inc.,                *
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA        *
*                                                                       *
*               home  : wxhexeditor.sourceforge.net                     *
*               email : death_knight at gamebox.net                     *
*************************************************************************/

#ifndef __wxHexEditor__
#define __wxHexEditor__

#include <wx/filename.h>

#include "FileDifference.h"
#include "DataInterpreter.h"
#include "InfoPanel.h"
#include "HexEditorGui.h"
#include "HexEditorCtrl/HexEditorCtrl.h"

#define MB 1024*1024	//this utility uses old ECS format.

class scrollthread;
class HexEditor: public HexEditorCtrl {
	public:
	    HexEditor(wxWindow* parent,
					int id,
					wxStatusBar *statusbar=NULL,
					DataInterpreter *interpreter=NULL,
					InfoPanel *infopanel=NULL,
					wxFileName* myfile=NULL,
					const wxPoint& pos=wxDefaultPosition,
					const wxSize& size=wxDefaultSize,
					long style=0);
		~HexEditor( void );
		friend class scrollthread;

		void OnOffsetScroll(wxScrollEvent &event);
		void LoadFromOffset(int64_t position, bool cursor_reset = false, bool paint = true );	//loads file from position

		bool FileOpen( wxFileName& filename  );
		bool FileSave( bool question = true );
		bool FileClose( void );
		bool FileUndo( void );
		bool FileRedo( void );
		void GoTo( int64_t goto_offset=-1 );
		void OnResize( wxSizeEvent &event );


		int64_t FileLenght( void ){ return myfile->Length();};
		wxFileName GetFileName( void ){ return myfile->GetFileName();};
		bool SetFileAccessMode( FileDifference::FileAccessMode fam ){ return myfile->SetAccessMode( fam ); };
		int GetFileAccessMode( void ){ return myfile->GetAccessMode();};
		wxString GetFileAccessModeString( void ){ return myfile->GetAccessModeString();};
		int GetFD( void ){ return myfile->fd(); };
//		void redo( void );
//		void undo( void );
//		void finddlg( void );
//		int64_t findbin( const char *target, int size, int64_t start_from );
//		int64_t find( wxString *target, int size, int64_t start_from, bool as_hex = false );
		void SetHexInsertionPoint ( int local_hex_location );

protected:
		bool Selector( bool=true );
		void UpdateCursorLocation( bool force=false );
		void OnKeyboardChar(wxKeyEvent& event);
		void OnKeyboardInput(wxKeyEvent& event);
		void OnKeyboardSelector( wxKeyEvent& event );
		void OnKeyboardSelectionEnd( wxKeyEvent& event );
		void OnMouseTest( wxMouseEvent& event );
		//void RefreshCursor(int64_t cursor_location = -1 );
		void OnMouseLeft( wxMouseEvent& event );
		void OnMouseSelectionEnd( wxMouseEvent& event );
		void OnMouseMove( wxMouseEvent& event );
		void OnMouseWhell( wxMouseEvent& event );
		void OnOffsetMouseFocus( wxMouseEvent& event );

//=====================
/*

	bool copy( bool as_hex = false );
	bool paste( bool as_hex = false );
	bool replace ( bool as_hex = false );

private:

    void init_hex_editor( void );
    wxWindow* parent;
	int id;
	int search_at_buffer( const char *bfr, int bfr_size, const char* search, int search_size );

protected:
   	friend class FindDialog;
	copy_maker *copy_mark;


*/
//=======================
	protected:
		wxStatusBar* statusbar;
		FileDifference *myfile;
		scrollthread *myscroll;
		DataInterpreter *interpreter;
		InfoPanel *infopanel;

	private:
	    void Dynamic_Connector( void );
	    void Dynamic_Disconnector( void );
};

class scrollthread:wxThreadHelper{
	private:
		HexEditor *parent;
		int speed, sleeper,cursor;
	public:
	scrollthread(int initial_speed, HexEditor *parent ):parent(parent){
		sleeper = 25;
		cursor = 0;
		speed = initial_speed;
		Create();
		GetThread()->Run();
		if(speed == 0)
			GetThread()->Pause();
		}

	void *Entry(){
		while( !(GetThread()->TestDestroy()) ){
			if(speed == 0)
				continue;	// loop to "while" for init of class and wait for GetThread()->Pause();
			int64_t FileLenght = parent->FileLenght();
			parent->page_offset += ( parent->hex_ctrl->BytePerLine() )*speed;
			if( parent->page_offset < 0 )
				parent->page_offset = 0;
			else if( parent->page_offset + parent->hex_ctrl->ByteCapacity() >= FileLenght ){
				parent->page_offset = FileLenght - parent->hex_ctrl->ByteCapacity();
				parent->page_offset += parent->hex_ctrl->BytePerLine() - (parent->page_offset % parent->hex_ctrl->BytePerLine()) ; //cosmetic
				}
			wxMutexGuiEnter();
		//	parent->MyFreeze();
			parent->LoadFromOffset( parent->page_offset, false, false );

				parent->SetHexInsertionPoint(cursor);
				parent->Selector();
				parent->PaintSelection();
				parent->UpdateCursorLocation( true );

			if( parent->offset_scroll->GetThumbPosition() != parent->page_offset / parent->hex_ctrl->ByteCapacity() )
				parent->offset_scroll->SetThumbPosition( parent->page_offset / parent->hex_ctrl->ByteCapacity() );
		//	parent->MyThaw();

		//	wxYieldIfNeeded();
			wxMutexGuiLeave();
			GetThread()->Sleep(sleeper);
			if( parent->page_offset == 0 ||
				parent->page_offset + parent->hex_ctrl->ByteCapacity() >= FileLenght )
				GetThread()->Pause();
			}
		return NULL;
		}
	void UpdateSpeed(int new_speed, int sleeptime = 25){
		if(new_speed == 0)
			GetThread()->Pause();
		else
			GetThread()->Resume();
		speed = new_speed;
		sleeper = sleeptime;
		cursor = parent->GetLocalHexInsertionPoint();
		}
	};

#endif
