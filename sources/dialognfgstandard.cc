//
// FILE: dialognfgstandard.cc -- Standard solution selection for normal form
//
// $Id$
//

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#include "wx/mdi.h"
#endif

#include "nfg.h"
#include "dialogauto.h"
#include "dialognfgstandard.h"

//========================================================================
//                dialogNfgStandard: Member functions
//========================================================================

const int idSTANDARD_TYPE = 1000;
const int idSTANDARD_NUM = 1001;

BEGIN_EVENT_TABLE(dialogNfgStandard, wxDialog)
  EVT_RADIOBOX(idSTANDARD_TYPE, dialogNfgStandard::OnChange)
  EVT_RADIOBOX(idSTANDARD_NUM, dialogNfgStandard::OnChange)
END_EVENT_TABLE()

dialogNfgStandard::dialogNfgStandard(wxWindow *p_parent, const Nfg &p_nfg)
  : guiAutoDialog(p_parent, "Standard Solution"), m_nfg(p_nfg)
{
  int standardType = 0, standardNum = 0, precision = 0;
  /*
  gText defaultsFile(gambitApp.ResourceFile());
  wxGetResource(SOLN_SECT, "Nfg-Standard-Type", &standardType,
		defaultsFile);
  wxGetResource(SOLN_SECT, "Nfg-Standard-Num", &standardNum,
		defaultsFile);
  wxGetResource(SOLN_SECT, "Nfg-Standard-Precision", &precision, defaultsFile);
  */

  wxString typeChoices[] = { "Nash", "Perfect" };
  m_standardType = new wxRadioBox(this, idSTANDARD_TYPE, "Type",
				  wxDefaultPosition, wxDefaultSize,
				  (m_nfg.NumPlayers() == 2) ? 2 : 1,
				  typeChoices, 0, wxRA_SPECIFY_ROWS);
  m_standardType->SetSelection(standardType);
  m_standardType->SetConstraints(new wxLayoutConstraints);
  m_standardType->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_standardType->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_standardType->GetConstraints()->width.AsIs();
  m_standardType->GetConstraints()->height.AsIs();

  wxString numChoices[] = { "One", "Two", "All" };
  m_standardNum = new wxRadioBox(this, idSTANDARD_NUM, "Number",
				 wxDefaultPosition, wxDefaultSize,
				 3, numChoices, 0, wxRA_SPECIFY_ROWS);
  m_standardNum->SetSelection(standardNum);
  m_standardNum->SetConstraints(new wxLayoutConstraints);
  m_standardNum->GetConstraints()->left.SameAs(m_standardType, wxRight, 10);
  m_standardNum->GetConstraints()->top.SameAs(m_standardType, wxTop);
  m_standardNum->GetConstraints()->width.AsIs();
  m_standardNum->GetConstraints()->height.AsIs();

  wxString precisionChoices[] = { "Float", "Rational" };
  m_precision = new wxRadioBox(this, -1, "Precision",
			       wxDefaultPosition, wxDefaultSize,
			       2, precisionChoices, 0, wxRA_SPECIFY_ROWS);
  m_precision->SetSelection(precision);
  m_precision->SetConstraints(new wxLayoutConstraints);
  m_precision->GetConstraints()->left.SameAs(m_standardNum, wxRight, 10);
  m_precision->GetConstraints()->top.SameAs(m_standardType, wxTop);
  m_precision->GetConstraints()->width.AsIs();
  m_precision->GetConstraints()->height.AsIs();
  
  m_description = new wxTextCtrl(this, -1);
  m_description->Enable(FALSE);
  m_description->SetConstraints(new wxLayoutConstraints);
  m_description->GetConstraints()->left.SameAs(m_standardType, wxLeft);
  m_description->GetConstraints()->right.SameAs(m_precision, wxRight);
  m_description->GetConstraints()->height.AsIs();
  m_description->GetConstraints()->top.SameAs(m_standardNum, wxBottom, 5);

  m_okButton->GetConstraints()->top.SameAs(m_description, wxBottom, 10);
  m_okButton->GetConstraints()->right.SameAs(m_cancelButton, wxLeft, 10);
  m_okButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_okButton->GetConstraints()->height.AsIs();

  m_cancelButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_cancelButton->GetConstraints()->centreX.SameAs(m_description, wxCentreX);
  m_cancelButton->GetConstraints()->width.AsIs();
  m_cancelButton->GetConstraints()->height.AsIs();

  m_helpButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_helpButton->GetConstraints()->left.SameAs(m_cancelButton, wxRight, 10);
  m_helpButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_helpButton->GetConstraints()->height.AsIs();

  OnChange();

  AutoSize();
}

dialogNfgStandard::~dialogNfgStandard()
{
  /*
  if (Completed() == wxOK) {
    gText defaultsFile(gambitApp.ResourceFile());
    wxWriteResource(SOLN_SECT, "Nfg-Standard-Type",
		    m_standardType->GetSelection(), defaultsFile);
    wxWriteResource(SOLN_SECT, "Nfg-Standard-Num",
		    m_standardNum->GetSelection(), defaultsFile);
    wxWriteResource(SOLN_SECT, "Nfg-Standard-Precision",
		    m_precision->GetSelection(), defaultsFile);
  }
  */
}

void dialogNfgStandard::OnChange(void)
{
  switch (m_standardType->GetSelection()) {
  case 0:
    switch (m_standardNum->GetSelection()) {
    case 0:
      if (m_nfg.NumPlayers() == 2 && IsConstSum(m_nfg)) {
	m_description->SetValue("LpSolve");
	m_precision->Enable(TRUE);
      }
      else if (m_nfg.NumPlayers() == 2) {
	m_description->SetValue("LcpSolve");
	m_precision->Enable(TRUE);
      }
      else {
	m_description->SetValue("SimpdivSolve");
	m_precision->Enable(FALSE);
	m_precision->SetSelection(0);
      }
      break;
    case 1:
    case 2:
      if (m_nfg.NumPlayers() == 2) {
	m_description->SetValue("EnumMixedSolve");
	m_precision->Enable(TRUE);
      }
      else {
	m_description->SetValue("LiapSolve");
	m_precision->SetSelection(0);
	m_precision->Enable(FALSE);
      }
      break;
    }
    break;

  case 1:
    switch (m_standardNum->GetSelection()) {
    case 0:
      if (m_nfg.NumPlayers() == 2) {
	m_description->SetValue("EnumMixedSolve");
	m_precision->Enable(TRUE);
      }
      else 
	m_description->SetValue("NOT IMPLEMENTED");
      break;
    case 1:
      if (m_nfg.NumPlayers() == 2) {
	m_description->SetValue("EnumMixedSolve");
	m_precision->Enable(TRUE);
      }
      else
	m_description->SetValue("NOT IMPLEMENTED");
      break;
    case 2:
      if (m_nfg.NumPlayers() == 2) {
	m_description->SetValue("EnumMixedSolve");
	m_precision->Enable(TRUE);
      }
      else
	m_description->SetValue("NOT IMPLEMENTED");
      break;
    }
    break;
  }
}

nfgStandardType dialogNfgStandard::Type(void) const
{
  switch (m_standardType->GetSelection()) {
  case 0:
    return nfgSTANDARD_NASH;
  case 1:
    return nfgSTANDARD_PERFECT;
  default:
    return nfgSTANDARD_NASH;
  }
}

nfgStandardNum dialogNfgStandard::Number(void) const
{
  switch (m_standardNum->GetSelection()) {
  case 0:
    return nfgSTANDARD_ONE;
  case 1:
    return nfgSTANDARD_TWO;
  case 2:
    return nfgSTANDARD_ALL;
  default:
    return nfgSTANDARD_ALL;
  }
}

