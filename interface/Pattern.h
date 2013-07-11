#ifndef UserCode_L1RpcTriggerAnalysis_PatternCreator_H
#define UserCode_L1RpcTriggerAnalysis_PatternCreator_H

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include <vector>
#include <utility>
#include <ostream>
#include <map>


class  Pattern {
public:

  typedef std::map<uint32_t,  unsigned int >  DataType;

  Pattern() {}


  static uint32_t rotateDetId(uint32_t rawId, int step);
  Pattern getRotated(int step) const;


  //is not empty
  operator bool() const { return theData.size() > 0; }

  bool add(std::pair<uint32_t,  unsigned int > aData) { return !addOrCopy(aData); }

  static void add( std::vector<Pattern> & vpat, std::pair<uint32_t,  unsigned int > aData);

  unsigned int size() const { return theData.size(); }
  operator const DataType & () const {  return theData; }

  bool operator==(const Pattern& o) const;

private:

  // try to add data from raw id to this pattern. if the data from detUnit 
  // is already assigned to this patterns return a copy of this pattern with
  // modified (from aData) data attached to detUnit. Otherwise add data from detUnit
  // to this pattern and return an empty pattern;  
  Pattern  addOrCopy ( std::pair<uint32_t,  unsigned int > aData); 
  
private:
  DataType  theData;
  friend std::ostream & operator << (std::ostream &out, const Pattern &o);
};
#endif


