//
// FILE: efgshow.h -- Declarations of both type dependent and independent
//                    classes for extensive form display code.
// $Id$
//

#ifndef EXTSHOW_H
#define EXTSHOW_H

#include "efgnfgi.h"
#include "gambit.h"
#include "accels.h"
#include "efgsolng.h"
#include "bsolnsf.h"
#include "paramsd.h"

class BaseTreeWindow;
class EfgSolnShow;
class EfgShowToolBar;
class EFSupportInspectDialog;
class NodeSolnShow;
class TreeWindow;

template <class T> class SolutionList: public gSortList<T>
{
private:
	unsigned int max_id;
public:
	SolutionList(void):gSortList<T>(),max_id(1) {}
	SolutionList(const gList<T> &l): gSortList<T>(l),max_id(1) { }
	virtual int Append(const T &a)
	{(*this)[gSortList<T>::Append(a)].SetId(max_id++);return Length();}
};

typedef BehavSolution<gNumber> BehavSolutionT;
typedef SolutionList<BehavSolutionT> BehavSolutionList;

class EfgShow: public wxFrame, public EfgNfgInterface, public EfgShowInterface,
               public ParametrizedGame
{
private:
	Efg &ef;
	gList<EFSupport *> supports;
	// Solution routines
	BehavSolutionList solns;
	struct StartingPoints
	{
		BehavSolutionList profiles;
		int last;
		StartingPoints() : last(-1) { }
	} starting_points;
  	int cur_soln;
   ParameterSetList param_sets;
	// we can display EF for one support, while working on a different support
	// disp_sup always corresponds to the support currently displayed.  cur_sup
	// corresponds to the support that will be operated upon by solution algs.
	EFSupport *cur_sup,*disp_sup;
	wxFrame *parent;
	EFSupportInspectDialog *support_dialog;
	// all_nodes must at all times refer to the prefix traversal order of the tree.
	// It is TreeWindow's job to call RemoveSolutions every time the nodes are
	// changes.  all_nodes is maintained in RemoveSolutions.
	gList<Node *> all_nodes;
   EfgSolnShow *soln_show;
	BSolnSortFilterOptions sf_options;
	gList<Accel>	accelerators;
	gString	filename;
	int log_item,zoom_win_item;	// menu items of checkable menus
	EfgShowToolBar *toolbar;
   // Private functions
	gArray<AccelEvent> MakeEventNames(void);
	void SetOptions(void);
	NodeSolnShow *node_inspect;
	bool SolveNormal(void);
	void SubgamesSetup(void);
	void NodeInspect(bool insp);
	struct es_features
	{
		Bool node_inspect;
		Bool iset_hilight;
		es_features(void):node_inspect(FALSE),iset_hilight(FALSE) { }
		es_features(const es_features &s):node_inspect(s.node_inspect),iset_hilight(s.iset_hilight) { }
	} features;
   void MakeMenus(void);
public:
	TreeWindow *tw;
	// Constructor.  You need only suply the Efg
	EfgShow(Efg &ef,EfgNfgInterface *nfg=0,int subgame=1,wxFrame *frame=0,
								const char *title=0,int x=-1,int y=-1,int w=600,
								int h=400,int type=wxDEFAULT_FRAME);
	// Destructor
	~EfgShow();
	// Event handlers
	Bool 		OnClose(void);
	void		OnMenuCommand(int id);
	void		OnSize(int w,int h);
	void		OnSetFocus(void);
	// Solution routines
	void		Solve(void);
	void		SolveSetup(int what);
	void 		InspectSolutions(void);
	void		RemoveSolutions(void);
	void		ChangeSolution(int soln);
   void		SetParameters(void);
	void 		OnSelectedMoved(const Node *n);
	BehavSolutionT CreateSolution(void);
	// Solution interface to the algorithms
	void 		PickSolutions(const Efg &,gList<BehavSolutionT> &);
	BehavProfileT CreateStartProfile(int how);
	void		RemoveStartProfiles(void);
	void 		SetPickSubgame(const Node *n);
	// Solution interface to normal form
	void 		SolutionToEfg(const BehavProfileT &s,bool set=false);
	const 	Efg *InterfaceObjectEfg(void) {return &ef;}
	wxFrame *Frame(void);
	// Solution access for TreeWindow
  	gNumber	BranchProb(const Node *n,int br);
	// Reset the supports.
	void GameChanged(void);
	// Inteface for infoset hilighting between the tree and solution display
	void HilightInfoset(int pl,int iset,int who);
	// Accelerators allow for platform-indep handling of hotkeys
	int			CheckAccelerators(wxKeyEvent &ev);
	// Need this to let the parent know that a solution window has been closed
	void		SolnShowDied(void);
	// EFSupport support
	void SupportInspect(int what=0);
	EFSupport *MakeSupport(void);
	void			SolveElimDom(void);
	// Used by TreeWindow
	virtual gString AsString(TypedSolnValues what,const Node *n,int br=0) const;
	// Display some inherent game properties
	void ShowGameInfo(void);
	// Currently used support
	const EFSupport *GetSupport(int which);
	// File name
   void SetFileName(void);
	void SetFileName(const gString &s);
  	const gString &Filename(void) const;

   void UpdateSpace(void);
};

// Solution constants
typedef enum {EFG_NO_SOLUTION=-1,EFG_GOBIT_SOLUTION,EFG_LIAP_SOLUTION,
							EFG_LCP_SOLUTION,EFG_PURENASH_SOLUTION,EFG_CSUM_SOLUTION,
							EFG_NUM_SOLUTIONS} EfgSolutionT;

#endif