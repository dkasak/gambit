//#
//# FILE: lexicon.cc -- Efg<->Nfg correspondence computations
//#
//# $Id$
//#

#include "efg.h"
#include "normal.h"
#include "glist.h"

typedef gArray<int> Correspondence;

class Lexicon   {
  public:
    BaseNormalForm *N;
    gArray<gList<Correspondence *> > strategies;

    Lexicon(const BaseEfg &);
    ~Lexicon();

    void MakeStrategy(EFPlayer *p);
    void MakeReducedStrats(EFPlayer *, Node *, Node *);
};

Lexicon::Lexicon(const BaseEfg &E)
  : N(0), strategies(E.NumPlayers())
{ }

Lexicon::~Lexicon()
{
  for (int i = 1; i <= strategies.Length(); i++)
    while (strategies[i].Length())  delete strategies[i].Remove(1);
}

void Lexicon::MakeStrategy(EFPlayer *p)
{
  Correspondence *c = new Correspondence(p->NumInfosets());
  
  for (int i = 1; i <= p->NumInfosets(); i++)  {
    if (p->InfosetList()[i]->flag == 1)
      (*c)[i] = p->InfosetList()[i]->whichbranch;
    else
      (*c)[i] = 0;
  }
  strategies[p->GetNumber()].Append(c);
}

void Lexicon::MakeReducedStrats(EFPlayer *p, Node *n, Node *nn)
{
  int i;
  Node *m, *mm;

  if (n->NumChildren() > 0)  {
    if (n->GetPlayer() == p)  {
      if (n->GetInfoset()->flag == 0)  {
	// we haven't visited this infoset before
	n->GetInfoset()->flag = 1;
	for (i = 1; i <= n->NumChildren(); i++)   {
	  Node *m = n->GetChild(i);
	  n->whichbranch = m;
	  n->GetInfoset()->whichbranch = i;
	  MakeReducedStrats(p, m, nn);
	}
	n->GetInfoset()->flag = 0;
      }
      else  {
	// we have visited this infoset, take same action
	MakeReducedStrats(p, n->GetChild(n->GetInfoset()->whichbranch),
			  nn);
      }
    }
    else  {
      n->ptr = NULL;
      if (nn != NULL)
	n->ptr = nn->GetParent();
      n->whichbranch = n->GetChild(1);
      if (n->GetInfoset())
	n->GetInfoset()->whichbranch = 0;
      MakeReducedStrats(p, n->GetChild(1), n->GetChild(1));
    }
  }
  else if (nn)  {
    for (m = NULL; ; nn = nn->GetParent()->ptr->whichbranch)  {
      m = nn->NextSibling();
      if (m || nn->GetParent()->ptr == NULL)   break;
    }
    if (m)  {
      mm = m->GetParent()->whichbranch;
      m->GetParent()->whichbranch = m;
      MakeReducedStrats(p, m, m);
      m->GetParent()->whichbranch = mm;
    }
    else 
      MakeStrategy(p);
  }
  else
    MakeStrategy(p);
}


#include "contiter.h"

template <class T> NormalForm<T> *MakeReducedNfg(Efg<T> &E)
{
  int i;

  Lexicon *L = new Lexicon(E);
  for (i = 1; i <= E.NumPlayers(); i++)  {
    E.RootNode()->ptr = NULL;
    L->MakeReducedStrats(E.PlayerList()[i], E.RootNode(), NULL);
  }
  gArray<int> dim(E.NumPlayers());
  for (i = 1; i <= E.NumPlayers(); i++)
    dim[i] = L->strategies[i].Length();

  L->N = new NormalForm<T>(dim);
  L->N->SetTitle(E.GetTitle());
  
  for (i = 1; i <= E.NumPlayers(); i++)   {
    for (int j = 1; j <= L->strategies[i].Length(); j++)   {
      gString name;
      for (int k = 1; k <= L->strategies[i][j]->Length(); k++)
	if ((*L->strategies[i][j])[k] > 0)
	  name += ToString((*L->strategies[i][j])[k]);
        else
	  name += "*";
      L->N->SetStratName(i, j, name);
    }
  }
  
  ContIter<T> iter((NormalForm<T> *) L->N);
  gArray<gArray<int> *> corr(E.NumPlayers());
  gArray<gListIter<Correspondence *> *> corrs(E.NumPlayers());
  for (i = 1; i <= E.NumPlayers(); i++)  {
    corrs[i] = new gListIter<Correspondence *>(L->strategies[i]);
    corr[i] = corrs[i]->GetValue();
  }

  gVector<T> value(E.NumPlayers());

  int pl = E.NumPlayers();
  while (1)  {
    E.Payoff(corr, value);
    for (int j = 1; j <= E.NumPlayers(); j++)
      iter.Payoff(j) = value[j];
    
    iter.NextContingency();
    while (pl > 0)   {
      (*corrs[pl])++;
      if (!corrs[pl]->PastEnd())  {
	corr[pl] = corrs[pl]->GetValue();
	break;
      }
      corrs[pl]->GoFirst();
      corr[pl] = corrs[pl]->GetValue();
      pl--;
    }

    if (pl == 0)  break;
    pl = E.NumPlayers();
  }

  for (i = 1; i <= E.NumPlayers(); i++)
    delete corrs[i];

  E.lexicon = L;
  return ((NormalForm<T> *) E.lexicon->N);
}

void BaseEfg::DeleteLexicon(void)
{
  if (lexicon)   delete lexicon;
  lexicon = 0;
}

template <class T>
void RealizationProbs(const NormalForm<T> &N, const gPVector<T> &mp,
		      const Efg<T> &E, gDPVector<T> &bp,
		      int pl, const gArray<int> *const actions, Node *n)
{
  static const T tremble = (T) 0.0;
  T prob;

  for (int i = 1; i <= n->NumChildren(); i++)   {
    if (n->GetPlayer() && !n->GetPlayer()->IsChance())   {
      if (n->GetPlayer()->GetNumber() == pl)  {
	if ((*actions)[n->GetInfoset()->GetNumber()] == i)
	  prob = (T) 1.0 - tremble + tremble / (T) n->NumChildren();
	else
	  prob = tremble / (T) n->NumChildren();
      }
      else
	prob = (T) 1.0 / (T) n->NumChildren();
    }
    else  {   // n.GetPlayer() == 0
      prob = (T) ((ChanceInfoset<T> *) n->GetInfoset())->GetActionProb(i);
    }

    Node *child = n->GetChild(i);
    child->bval = prob * n->bval;
    child->nval += child->bval;    

    RealizationProbs(N, mp, E, bp, pl, actions, child);
  }    
	
}

template <class T>
void BehaviorStrat(const Efg<T> &E, gDPVector<T> &bp, int pl, Node *n)
{
  for (int i = 1; i <= n->NumChildren(); i++)   {
    Node *child = n->GetChild(i);
    if (n->GetPlayer() && n->GetPlayer()->GetNumber() == pl)
      if (n->nval > (T) 0.0)  {
	bp(n->GetPlayer()->GetNumber(), n->GetInfoset()->GetNumber(), i) =
	  child->nval / n->nval;
      }
    BehaviorStrat(E, bp, pl, child);
  }
}

void ClearNodeProbs(Node *n)
{
  n->nval = 0.0;
  for (int i = 1; i <= n->NumChildren(); i++)
    ClearNodeProbs(n->GetChild(i));
}

template <class T>
void MixedToBehav(const NormalForm<T> &N, const gPVector<T> &mp,
		  const Efg<T> &E, gDPVector<T> &bp)
{
  if (!E.lexicon || E.lexicon->N != &N)   return;

  int sset = N.NumStratSets();
  while (N.Dimensionality(sset) != mp.Lengths())  {
    sset--;
    assert(sset > 0);
  }

  Node *n = E.RootNode();

  for (int pl = 1; pl <= N.NumPlayers(); pl++)   {
    ClearNodeProbs(n);

    for (int st = 1; st <= N.NumStrats(pl, sset); st++)  {
      if (mp(pl, st) > (T) 0.0)  {
//	const gArray<int> *const actions = N.GetActions(pl, st, sset);
	const gArray<int> *const actions = E.lexicon->strategies[pl][sset];

	n->bval = mp(pl, st);

	RealizationProbs(N, mp, E, bp, pl, actions, E.RootNode());
      }
    }
    
    E.RootNode()->nval = (T) 1.0;
    BehaviorStrat(E, bp, pl, n);
  }
}


#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

#include "rational.h"

TEMPLATE void MixedToBehav(const NormalForm<double> &,const gPVector<double> &,
			   const Efg<double> &, gDPVector<double> &);
TEMPLATE void MixedToBehav(const NormalForm<gRational> &,
			   const gPVector<gRational> &,
			   const Efg<gRational> &, gDPVector<gRational> &);

TEMPLATE void RealizationProbs(const NormalForm<double> &N, const gPVector<double> &mp,
			       const Efg<double> &E, gDPVector<double> &bp,
			       int pl, const gArray<int> *const actions, Node *);
TEMPLATE void RealizationProbs(const NormalForm<gRational> &N, const gPVector<gRational> &mp,
			       const Efg<gRational> &E, gDPVector<gRational> &bp,
			       int pl, const gArray<int> *const actions, Node *);


TEMPLATE void BehaviorStrat(const Efg<double> &E, gDPVector<double> &bp, int pl, Node *n);
TEMPLATE void BehaviorStrat(const Efg<gRational> &E, gDPVector<gRational> &bp, int pl, Node *n);



template NormalForm<double> *MakeReducedNfg(Efg<double> &);
template NormalForm<gRational> *MakeReducedNfg(Efg<gRational> &);

#include "glist.imp"
#include "garray.imp"

template class gList<Correspondence *>;
template class gNode<Correspondence *>;
template class gListIter<Correspondence *>;
template gOutput &operator<<(gOutput &, const gList<Correspondence *> &);
template class gArray<gList<Correspondence *> >;
template class gArray<gListIter<Correspondence *> *>;
