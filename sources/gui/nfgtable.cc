//
// FILE: nfgtable.cc -- Implementation of normal form table class
//
// $Id$
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/config.h"
#include "nfgshow.h"
#include "nfgtable.h"
#include "nfgconst.h"

class NfgGridTable : public wxGridTableBase {
private:
  Nfg *m_nfg;
  NfgTable *m_table;

public:
  NfgGridTable(NfgTable *p_table, Nfg *p_nfg);
  virtual ~NfgGridTable() { }

  int GetNumberRows(void);
  int GetNumberCols(void);
  wxString GetValue(int row, int col);
  wxString GetRowLabelValue(int);
  wxString GetColLabelValue(int);
  void SetValue(int, int, const wxString &) { /* ignore */ }
  bool IsEmptyCell(int, int) { return false; }

  bool InsertRows(size_t pos = 0, size_t numRows = 1);
  bool AppendRows(size_t numRows = 1);
  bool DeleteRows(size_t pos = 0, size_t numRows = 1);
  bool InsertCols(size_t pos = 0, size_t numCols = 1);
  bool AppendCols(size_t numCols = 1);
  bool DeleteCols(size_t pos = 0, size_t numCols = 1);

  wxGridCellAttr *GetAttr(int row, int col);
};

NfgGridTable::NfgGridTable(NfgTable *p_table, Nfg *p_nfg)
  : m_nfg(p_nfg), m_table(p_table)
{ }

int NfgGridTable::GetNumberRows(void)
{
  return (m_table->GetSupport().NumStrats(m_table->GetRowPlayer()) +
	  m_table->ShowProbs() + m_table->ShowDominance() +
	  m_table->ShowValues());
}

int NfgGridTable::GetNumberCols(void)
{
  return (m_table->GetSupport().NumStrats(m_table->GetColPlayer()) +
	  m_table->ShowProbs() + m_table->ShowDominance() + 
	  m_table->ShowValues());
}

wxString NfgGridTable::GetRowLabelValue(int p_row)
{
  int numStrats = m_table->GetSupport().NumStrats(m_table->GetRowPlayer());
  if (p_row + 1 <= numStrats) {
    return (char *) m_table->GetSupport().Strategies(m_table->GetRowPlayer())[p_row+1]->Name();
  }
  else if (p_row + 1 == numStrats + m_table->ShowDominance()) {
    return "Dom";
  }
  else if (p_row + 1 == 
	   numStrats + m_table->ShowDominance() + m_table->ShowProbs()) {
    return "Prob";
  }
  else {
    return "Val";
  }
}

wxString NfgGridTable::GetColLabelValue(int p_col)
{
  int numStrats = m_table->GetSupport().NumStrats(m_table->GetColPlayer());
  if (p_col + 1 <= numStrats) {
    return (char *) m_table->GetSupport().Strategies(m_table->GetColPlayer())[p_col+1]->Name();
  }
  else if (p_col + 1 == numStrats + m_table->ShowDominance()) {
    return "Dom";
  }
  else if (p_col + 1 == 
	   numStrats + m_table->ShowDominance() + m_table->ShowProbs()) {
    return "Prob";
  }
  else {
    return "Val";
  }
}

wxString NfgGridTable::GetValue(int row, int col)
{
  int rowPlayer = m_table->GetRowPlayer();
  int colPlayer = m_table->GetColPlayer();
  const NFSupport &support = m_table->GetSupport();
  int numRowStrats = support.NumStrats(rowPlayer);
  int numColStrats = support.NumStrats(colPlayer);

  if (row < numRowStrats && col < numColStrats) {
    gArray<int> strategy(m_table->GetProfile());
    strategy[m_table->GetRowPlayer()] = row + 1;
    strategy[m_table->GetColPlayer()] = col + 1;
    
    StrategyProfile profile(*m_nfg);
    for (int pl = 1; pl <= strategy.Length(); pl++) {
      profile.Set(pl, support.GetStrategy(pl, strategy[pl]));
    }

    NFOutcome *outcome = m_nfg->GetOutcome(profile);
    if (m_table->OutcomeValues()) {
      wxString ret = "(";
      for (int pl = 1; pl <= strategy.Length(); pl++) {
	ret += wxString::Format("%s",
				(char *) ToText(m_nfg->Payoff(outcome, pl),
						m_table->GetDecimals()));
	if (pl < strategy.Length()) {
	  ret += wxString(",");
	}
      }
      ret += ")";
      return ret;
    }
    else {
      if (outcome) {
	wxString ret = (char *) outcome->GetName();
	if (ret == "") {
	  ret = (char *) (gText("Outcome") + ToText(outcome->GetNumber()));
	}
	return ret;
      }
      else {
	return "Null";
      }
    }
  }
  else if (row < numRowStrats &&
	   col == numColStrats + m_table->ShowDominance() - 1) {
    Strategy *strategy = support.GetStrategy(rowPlayer, row + 1);
    if (support.IsDominated(strategy, true)) {
      return "S";
    }
    else if (support.IsDominated(strategy, false)) {
      return "W";
    }
    else {
      return "N";
    }
  }
  else if (row == numRowStrats + m_table->ShowDominance() - 1 &&
	   col < numColStrats) {
    Strategy *strategy = support.GetStrategy(colPlayer, col + 1);
    if (support.IsDominated(strategy, true)) {
      return "S";
    }
    else if (support.IsDominated(strategy, false)) {
      return "W";
    }
    else {
      return "N";
    }
  }
  else if (row < numRowStrats && 
	   col == numColStrats + m_table->ShowDominance() + m_table->ShowProbs() - 1) {
    Strategy *strategy = support.GetStrategy(rowPlayer, row + 1);
    return ((char *) ToText(m_table->GetSolution()(strategy)));
  }
  else if (row == numRowStrats + m_table->ShowDominance() + m_table->ShowProbs() - 1 && 
	   col < numColStrats) {
    Strategy *strategy = support.GetStrategy(colPlayer, col + 1);
    return ((char *) ToText(m_table->GetSolution()(strategy)));
  }
  else if (row < numRowStrats && 
	   col == numColStrats + m_table->ShowDominance() + m_table->ShowProbs() + m_table->ShowValues() - 1) {
    Strategy *strategy = support.GetStrategy(rowPlayer, row + 1);
    return ((char *) ToText(m_table->GetSolution().Payoff(strategy->Player(), strategy)));
  }
  else if (row == numRowStrats + m_table->ShowDominance() + m_table->ShowProbs() + m_table->ShowValues() - 1 && 
	   col < numColStrats) {
    Strategy *strategy = support.GetStrategy(colPlayer, col + 1);
    return ((char *) ToText(m_table->GetSolution().Payoff(strategy->Player(), strategy)));
  }

  return "";
}

bool NfgGridTable::InsertRows(size_t pos, size_t numRows)
{
  wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_INSERTED,
			 pos, numRows);
  GetView()->ProcessTableMessage(msg);
  return true;
}

bool NfgGridTable::AppendRows(size_t numRows)
{
  wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_APPENDED, numRows);
  GetView()->ProcessTableMessage(msg);
  return true;
}

bool NfgGridTable::DeleteRows(size_t pos, size_t numRows)
{
  wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_DELETED,
			 pos, numRows);
  GetView()->ProcessTableMessage(msg);
  return true;
}

bool NfgGridTable::InsertCols(size_t pos, size_t numCols)
{
  wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_COLS_INSERTED,
			 pos, numCols);
  GetView()->ProcessTableMessage(msg);
  return true;
}

bool NfgGridTable::AppendCols(size_t numCols)
{
  wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_COLS_APPENDED, numCols);
  GetView()->ProcessTableMessage(msg);
  return true;
}

bool NfgGridTable::DeleteCols(size_t pos, size_t numCols)
{
  wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_COLS_DELETED,
			 pos, numCols);
  GetView()->ProcessTableMessage(msg);
  return true;
}

wxGridCellAttr *NfgGridTable::GetAttr(int row, int col)
{
  wxGridCellAttr *attr = new wxGridCellAttr;

  if (row >= m_table->GetSupport().NumStrats(m_table->GetRowPlayer()) &&
      col >= m_table->GetSupport().NumStrats(m_table->GetColPlayer())) {
    attr->SetBackgroundColour(*wxBLACK);
  }
  else if (row >= m_table->GetSupport().NumStrats(m_table->GetRowPlayer()) ||
	   col >= m_table->GetSupport().NumStrats(m_table->GetColPlayer())) {
    attr->SetBackgroundColour(*wxLIGHT_GREY);
  }
  else {
    attr->SetBackgroundColour(*wxWHITE);
  }

  attr->SetAlignment(wxCENTER, wxCENTER);

  return attr;
}

class ColoredStringRenderer : public wxGridCellRenderer {
public:
  // draw the string
  virtual void Draw(wxGrid& grid,
		    wxGridCellAttr& attr,
		    wxDC& dc,
		    const wxRect& rect,
		    int row, int col,
		    bool isSelected);

  // return the string extent
  virtual wxSize GetBestSize(wxGrid& grid,
			     wxGridCellAttr& attr,
			     wxDC& dc,
			     int row, int col);

  virtual wxGridCellRenderer *Clone() const
    { return new ColoredStringRenderer; }

protected:
  // set the text colours before drawing
  void SetTextColoursAndFont(wxGrid& grid,
			     wxGridCellAttr& attr,
			     wxDC& dc,
			     bool isSelected);

  // calc the string extent for given string/font
  wxSize DoGetBestSize(wxGridCellAttr& attr,
		       wxDC& dc,
		       const wxString& text);
};

void ColoredStringRenderer::SetTextColoursAndFont(wxGrid& grid,
                                                     wxGridCellAttr& attr,
                                                     wxDC& dc,
                                                     bool isSelected)
{
    dc.SetBackgroundMode( wxTRANSPARENT );

    // TODO some special colours for attr.IsReadOnly() case?

    if ( isSelected )
    {
        dc.SetTextBackground( grid.GetSelectionBackground() );
        dc.SetTextForeground( grid.GetSelectionForeground() );
    }
    else
    {
        dc.SetTextBackground( attr.GetBackgroundColour() );
        dc.SetTextForeground( attr.GetTextColour() );
    }

    dc.SetFont( attr.GetFont() );
}

wxSize ColoredStringRenderer::DoGetBestSize(wxGridCellAttr& attr,
					    wxDC& dc,
					    const wxString& text)
{
  wxCoord x = 0, y = 0;
  dc.SetFont(attr.GetFont());
  dc.GetTextExtent(text, &x, &y);
  return wxSize(x, y);
}

wxSize ColoredStringRenderer::GetBestSize(wxGrid& grid,
                                             wxGridCellAttr& attr,
                                             wxDC& dc,
                                             int row, int col)
{
  return DoGetBestSize(attr, dc, grid.GetCellValue(row, col));
}

void ColoredStringRenderer::Draw(wxGrid& grid,
                                    wxGridCellAttr& attr,
                                    wxDC& dc,
                                    const wxRect& rectCell,
                                    int row, int col,
                                    bool isSelected)
{
  wxGridCellRenderer::Draw(grid, attr, dc, rectCell, row, col, isSelected);

  // now we only have to draw the text
  SetTextColoursAndFont(grid, attr, dc, isSelected);

  wxRect rect = rectCell;
  rect.Inflate(-1);

  wxCoord x, y;
  grid.DrawTextRectangle(dc, "(", rect);
  dc.GetTextExtent("(", &x, &y);
  rect.x += x;

  wxString text = grid.GetCellValue(row, col);
  dc.SetTextForeground(*wxRED);
  for (unsigned int i = 0; i < text.Length(); i++) {
    if (text[i] == ',') {
      wxColour color = dc.GetTextForeground();
      dc.SetTextForeground(*wxBLACK);
      grid.DrawTextRectangle(dc, ",", rect);
      dc.GetTextExtent(",", &x, &y);
      rect.x += x;

      if (color == *wxRED) {
	dc.SetTextForeground(*wxBLUE);
      }
      else {
	dc.SetTextForeground(*wxRED);
      }
    }
    else {
      grid.DrawTextRectangle(dc, text[i], rect);
      dc.GetTextExtent(text[i], &x, &y);
      rect.x += x;
    }
  }
  
  dc.SetTextForeground(*wxBLACK);
  grid.DrawTextRectangle(dc, ")", rect); 
}

//======================================================================
//                   class NfgTable: Member functions
//======================================================================

BEGIN_EVENT_TABLE(NfgTable, wxPanel)
  EVT_GRID_SELECT_CELL(NfgTable::OnCellSelect)
  EVT_GRID_CELL_LEFT_DCLICK(NfgTable::OnLeftDoubleClick)
  EVT_GRID_LABEL_LEFT_CLICK(NfgTable::OnLabelLeftClick)
END_EVENT_TABLE()

NfgTable::NfgTable(Nfg &p_nfg, wxWindow *p_parent)
  : wxPanel(p_parent, -1), m_nfg(p_nfg), m_parent(p_parent), 
    m_editable(true), m_cursorMoving(false), m_rowPlayer(1), m_colPlayer(2),
    m_support(m_nfg), m_solution(0),
    m_showProb(0), m_showDom(0), m_showValue(0)
{
  SetAutoLayout(true);

  m_grid = new wxGrid(this, -1, wxDefaultPosition, wxDefaultSize);
  m_grid->SetTable(new NfgGridTable(this, &m_nfg), true);
  m_grid->SetGridCursor(0, 0);
  m_grid->SetEditable(false);
  m_grid->DisableDragRowSize();
  m_grid->AdjustScrollbars();
  //  m_grid->SetDefaultRenderer(new ColoredStringRenderer);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
  topSizer->Add(m_grid, 1, wxALL | wxEXPAND | wxALIGN_RIGHT, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
  Show(true);
}

void NfgTable::SetProfile(const gArray<int> &p_profile)
{
  m_grid->SetGridCursor(p_profile[GetRowPlayer()] - 1,
			p_profile[GetColPlayer()] - 1);
  m_grid->Refresh();
}

gArray<int> NfgTable::GetProfile(void) const
{
  return ((NfgShow *) m_parent)->GetProfile();
}

void NfgTable::SetPlayers(int p_rowPlayer, int p_colPlayer)
{ 
  m_grid->BeginBatch();
  m_rowPlayer = p_rowPlayer;
  m_colPlayer = p_colPlayer;
  int stratRows = m_grid->GetRows() - m_showProb - m_showDom - m_showValue;
  int stratCols = m_grid->GetCols() - m_showProb - m_showDom - m_showValue;

  if (m_support.NumStrats(p_rowPlayer) < stratRows) {
    m_grid->DeleteRows(0, stratRows - m_support.NumStrats(p_rowPlayer));
  }
  else if (m_support.NumStrats(p_rowPlayer) > stratRows) {
    m_grid->InsertRows(0, m_support.NumStrats(p_rowPlayer) - stratRows); 
  }

  if (m_support.NumStrats(p_colPlayer) < stratCols) {
    m_grid->DeleteCols(0, stratCols - m_support.NumStrats(p_colPlayer));
  }
  else if (m_support.NumStrats(p_colPlayer) > stratCols) {
    m_grid->InsertCols(0, m_support.NumStrats(p_colPlayer) - stratCols);
  }

  ((NfgShow *) m_parent)->SetStrategy(m_rowPlayer, 1);
  ((NfgShow *) m_parent)->SetStrategy(m_colPlayer, 1);
  m_grid->EndBatch();
  m_grid->AdjustScrollbars();
  m_grid->Refresh();
}

void NfgTable::SetStrategy(int p_player, int p_strategy)
{
  if (!m_cursorMoving) {
    // prevents reentry
    if (p_player == GetRowPlayer()) {
      m_grid->SetGridCursor(p_strategy - 1, m_grid->GetCursorColumn());
    }
    else if (p_player == GetColPlayer()) {
      m_grid->SetGridCursor(m_grid->GetCursorRow(), p_strategy - 1);
    }
  }
}

void NfgTable::ToggleProbs(void)
{
  m_showProb = 1 - m_showProb;
  if (m_showProb) {
    m_grid->AppendCols();
    m_grid->AppendRows();
  }
  else {
    m_grid->DeleteCols();
    m_grid->DeleteRows();
  }
  m_grid->AdjustScrollbars();
  m_grid->Refresh();
}

void NfgTable::ToggleDominance(void)
{
  m_showDom = 1 - m_showDom;
  if (m_showDom) {
    m_grid->AppendCols();
    m_grid->AppendRows();
  }
  else {
    m_grid->DeleteCols();
    m_grid->DeleteRows();
  }
  m_grid->AdjustScrollbars();
  m_grid->Refresh();
}

void NfgTable::ToggleValues(void)
{
  m_showValue = 1 - m_showValue;
  if (m_showValue) {
    m_grid->AppendCols();
    m_grid->AppendRows();
  }
  else {
    m_grid->DeleteCols();
    m_grid->DeleteRows();
  }
  m_grid->AdjustScrollbars();
  m_grid->Refresh();
}

void NfgTable::OnCellSelect(wxGridEvent &p_event)
{
  if (p_event.GetRow() >= m_support.NumStrats(GetRowPlayer()) ||
      p_event.GetCol() >= m_support.NumStrats(GetColPlayer())) {
    p_event.Veto();
  }
  else {
    m_cursorMoving = true;  // this prevents re-entry
    ((NfgShow *) m_parent)->SetStrategy(GetRowPlayer(), p_event.GetRow() + 1);
    ((NfgShow *) m_parent)->SetStrategy(GetColPlayer(), p_event.GetCol() + 1);
    m_cursorMoving = false;
    // now continue with the default behavior (i.e., highlight the new cell)
    p_event.Skip(); 
  }
}

void NfgTable::OnLeftDoubleClick(wxGridEvent &p_event)
{
  if (m_editable &&
      p_event.GetRow() < m_support.NumStrats(GetRowPlayer()) &&
      p_event.GetCol() < m_support.NumStrats(GetColPlayer())) {
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED,
			 NFG_EDIT_OUTCOMES_PAYOFFS);
    m_parent->AddPendingEvent(event);
  }
}

void NfgTable::OnLabelLeftClick(wxGridEvent &p_event)
{
  // for the moment, just veto it
  p_event.Veto();
}

void NfgTable::SetSupport(const NFSupport &p_support)
{
  m_support = p_support;
  SetPlayers(m_rowPlayer, m_colPlayer);
  m_grid->Refresh();
}

void NfgTable::SetSolution(const MixedSolution &p_solution)
{
  if (m_solution) {
    delete m_solution;
  }
  m_solution = new MixedSolution(p_solution);
  m_grid->Refresh();
}

void NfgTable::ClearSolution(void)
{
  if (m_solution) {
    delete m_solution;
    m_solution = 0;
  }
}

//-----------------------------------------------------------------------
//               class NfgTable::Settings: Member functions
//-----------------------------------------------------------------------

NfgTable::Settings::Settings(void)
  : m_decimals(2)
{
  LoadSettings();
}

NfgTable::Settings::~Settings()
{ }

void NfgTable::Settings::LoadSettings(void)
{
  wxConfig config("Gambit");
  config.Read("NfgDisplay/Display-Precision", &m_decimals, 2);
  config.Read("NfgDisplay/Outcome-Values", &m_outcomeValues, 1);
}

void NfgTable::Settings::SaveSettings(void) const
{
  wxConfig config("Gambit");
  config.Write("NfgDisplay/Display-Precision", (long) m_decimals);
  config.Write("NfgDisplay/Outcome-Values", (long) m_outcomeValues);
}
