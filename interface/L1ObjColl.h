#ifndef L1ObjColl_H
#define L1ObjColl_H

#include "UserCode/L1RpcTriggerAnalysis/interface/L1Obj.h"
#include <vector>

class L1ObjColl : public TObject {

public:
  L1ObjColl() {}
  virtual ~L1ObjColl(){}

  typedef L1Obj::TYPE TYPE;
  void set(const std::vector<L1Obj> & obj) { theL1Obj = obj; }
  void set(const std::vector<bool> & comp) { theL1Matching = comp; }
  void set(const std::vector<double> & dr) { theDeltaR = dr; }

  const std::vector<L1Obj> & getL1Objs() const { return theL1Obj; }
  const std::vector<bool> & getL1ObjsMatching() const { return theL1Matching; }
  const std::vector<double> & getL1ObjDeltaR() const { return theDeltaR; }

/*
  L1ObjColl selectByType( TYPE t1=L1Obj::NONE, TYPE t2=L1Obj::NONE, TYPE t3=L1Obj::NONE, TYPE t4=L1Obj::NONE);
  L1ObjColl selectByPtMin( double ptMin = 0.);
  L1ObjColl selectByEta( double etaMin = -2.5, double etaMax = 2.5);
  L1ObjColl selectByBx(  int bxMin = 0, int bxMax = 0);
  L1ObjColl selectByQuality( int qMin = 0, int qMax = 7);
  L1ObjColl operator+(const L1ObjColl& , const L1ObjColl&) const;
*/

  void print() const;

//tmp
  std::vector<L1Obj> getL1ObjsMatched(double ptMin = 0) const;
  std::vector<L1Obj> getL1ObjsSelected(
		  bool requireMatched = true, bool requireNonMatched = false, 
		  double ptMin = 0., double ptMax = 161.,
		  int bxMin = 0, int bxMax = 0,                  
		  double etaMin = -2.5, double etaMax = 2.5,
		  double phiMin = 0., double phiMax = 7.,
		  int qMin = 0, int qMax = 7) const;
  static  std::vector<L1Obj> typeSelector(const  std::vector<L1Obj> & col,  TYPE t1=L1Obj::NONE, TYPE t2=L1Obj::NONE, TYPE t3=L1Obj::NONE, TYPE t4=L1Obj::NONE);
   
  
private:
  std::vector<L1Obj> theL1Obj;
  std::vector<bool> theL1Matching;
  std::vector<double> theDeltaR;

public:
ClassDef(L1ObjColl,1)

};

#endif
