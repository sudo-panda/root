/*****************************************************************************
 * Project: BaBar detector at the SLAC PEP-II B-factory
 * Package: RooFitCore
 *    File: $Id: RooAbsCategory.cc,v 1.14 2001/05/03 02:15:53 verkerke Exp $
 * Authors:
 *   DK, David Kirkby, Stanford University, kirkby@hep.stanford.edu
 *   WV, Wouter Verkerke, UC Santa Barbara, verkerke@slac.stanford.edu
 * History:
 *   07-Mar-2001 WV Created initial version
 *
 * Copyright (C) 2001 University of California
 *****************************************************************************/

#include <iostream.h>
#include <stdlib.h>
#include "TString.h"
#include "TH1.h"
#include "RooFitCore/RooAbsCategory.hh"
#include "RooFitCore/RooArgSet.hh"
#include "RooFitCore/Roo1DTable.hh"

ClassImp(RooAbsCategory) 
;

RooAbsCategory::RooAbsCategory(const char *name, const char *title) : 
  RooAbsArg(name,title)
{
  setValueDirty(kTRUE) ;  
  setShapeDirty(kTRUE) ;  
}

RooAbsCategory::RooAbsCategory(const RooAbsCategory& other,const char* name) :
  RooAbsArg(other,name), _value(other._value) 
{
  TIterator* iter=other._types.MakeIterator() ;
  TObject* obj ;
  while (obj=iter->Next()) {
    _types.Add(obj->Clone()) ;
  }
  delete iter ;

  setValueDirty(kTRUE) ;
  setShapeDirty(kTRUE) ;
}


RooAbsCategory::~RooAbsCategory()
{
  // We own the contents of _types 
  _types.Delete() ;
}

RooAbsCategory& RooAbsCategory::operator=(const RooAbsCategory& other)
{
  RooAbsArg::operator=(other) ;
  
  if (!lookupType(other._value) && !TString(other._value.GetName()).IsNull()) {
    cout << "RooAbsCategory::operator=(" << GetName() << "): index " 
	 << other._value.getVal() << " not defined for this category" << endl ;
  } else {
    _value = other._value ;
    setValueDirty(kTRUE) ;
  }
  return *this ;
}

RooAbsArg& RooAbsCategory::operator=(const RooAbsArg& aother)
{
  return operator=((const RooAbsCategory&)aother) ;
}


Int_t RooAbsCategory::getIndex() const
{
  if (isValueDirty() || isShapeDirty()) {
    _value = traceEval() ;

    setValueDirty(false) ;
    setShapeDirty(false) ;
  } 

  return _value.getVal() ;
}


const char* RooAbsCategory::getLabel() const
{
  if (isValueDirty() || isShapeDirty()) {
    _value = traceEval() ;

    setValueDirty(false) ;
    setShapeDirty(false) ;
  } 

  return _value.GetName() ;
}




RooCatType RooAbsCategory::traceEval() const
{
  RooCatType value = evaluate() ;
  
  //Standard tracing code goes here
  if (!isValid(value)) {
  }

  //Call optional subclass tracing code
  traceEvalHook(value) ;

  return value ;
}



TIterator* RooAbsCategory::typeIterator() const
{
  return _types.MakeIterator() ;
}


Bool_t RooAbsCategory::operator==(Int_t index) const
{
  return (index==getIndex()) ;
}

Bool_t RooAbsCategory::operator==(const char* label) const
{
  return !TString(label).CompareTo(getLabel()) ;
}

Bool_t RooAbsCategory::isValidIndex(Int_t index) const
{
  return lookupType(index)?kTRUE:kFALSE ;
}

Bool_t RooAbsCategory::isValidLabel(const char* label) const
{
  return lookupType(label)?kTRUE:kFALSE ;
}



const RooCatType* RooAbsCategory::defineType(const char* label)
{
  // Find lowest unused index
  Int_t index(-1) ;
  while(lookupType(++index,kFALSE)) ;
  
  // Assign this index to given label 
  return defineType(label,index) ;
}



const RooCatType* RooAbsCategory::defineType(const char* label, Int_t index) 
{
  if (isValidIndex(index)) {
    cout << "RooAbsCategory::defineType(" << GetName() << "): index " 
	 << index << " already assigned" << endl ;
    return 0 ;
  }

  if (isValidLabel(label)) {
    cout << "RooAbsCategory::defineType(" << GetName() << "): label " 
	 << label << " already assigned" << endl ;
    return 0 ;
  }

  Bool_t first = _types.GetEntries()?kFALSE:kTRUE ;
  RooCatType *newType = new RooCatType(label,index) ;
  _types.Add(newType) ;

  if (first) _value = RooCatType(label,index) ;
  setShapeDirty(kTRUE) ;

  return newType ;
}



void RooAbsCategory::clearTypes() 
{
  _types.Delete() ;
  _value = RooCatType("",0) ;
  setShapeDirty(kTRUE) ;
}



const RooCatType* RooAbsCategory::lookupType(const RooCatType &other, Bool_t printError) const {
  // Find our type that matches the specified type, or return 0 for no match.

  RooCatType* type ;
  Int_t n= _types.GetEntries();
  for (int i=0 ; i < n; i++) {
    type = (RooCatType*)_types.At(i) ;
    if((*type) == other) return type; // delegate comparison to RooCatType
  }
  if (printError) {
    cout << ClassName() << "::" << GetName() << ":lookupType: no match for ";
    other.printToStream(cout,OneLine);
  }
  return 0 ;
}

const RooCatType* RooAbsCategory::lookupType(Int_t index, Bool_t printError) const
{
  // Find our type corresponding to the specified index, or return 0 for no match.

  RooCatType* type ;
  Int_t n= _types.GetEntries();
  for (int i=0 ; i < n; i++) {
    type = (RooCatType*)_types.At(i) ;
    if((*type) == index) return type; // delegate comparison to RooCatType
  }
  if (printError) {
    cout << ClassName() << "::" << GetName() << ":lookupType: no match for index "
	 << index << endl;
  }
  return 0 ;
}

const RooCatType* RooAbsCategory::lookupType(const char* label, Bool_t printError) const 
{
  // Find our type corresponding to the specified label, or return 0 for no match.

  RooCatType* type ;
  Int_t n= _types.GetEntries();
  for (int i=0 ; i < n; i++) {
    type = (RooCatType*)_types.At(i) ;
    if((*type) == label) return type; // delegate comparison to RooCatType
  }
  if (printError) {
    cout << ClassName() << "::" << GetName() << ":lookupType: no match for label "
	 << label << endl;
  }
  return 0 ;
}

Bool_t RooAbsCategory::isValid() const
{
  return isValid(_value) ;
}

Bool_t RooAbsCategory::isValid(RooCatType value)  const
{
  return isValidIndex(value.getVal()) ;
}

Roo1DTable* RooAbsCategory::createTable(const char *label)  const
{
  return new Roo1DTable(GetName(),label,*this) ;
}

Bool_t RooAbsCategory::readFromStream(istream& is, Bool_t compact, Bool_t verbose) 
{
  //Read object contents from stream (dummy for now)
  return kFALSE ;
} 

void RooAbsCategory::writeToStream(ostream& os, Bool_t compact) const
{
  //Write object contents to stream 

  if (compact) {
    os << getLabel() ;
  } else {
    os << getLabel() ;
  }
}

void RooAbsCategory::printToStream(ostream& os, PrintOption opt, TString indent) const
{
  // Print info about this object to the specified stream. In addition to the info
  // from RooAbsArg::printToStream() we add:
  //
  //     Shape : label, index, defined types

  RooAbsArg::printToStream(os,opt,indent);
  if(opt >= Shape) {
    os << indent << "--- RooAbsCategory ---" << endl;
    if (_types.GetEntries()==0) {
      os << indent << "  ** No values defined **" << endl;
      return;
    }
    os << indent << "  Value is \"" << getLabel() << "\" (" << getIndex() << ")" << endl;
    os << indent << "  Has the following possible values:" << endl;
    indent.Append("    ");
    opt= lessVerbose(opt);
    RooCatType *type;
    Int_t n= _types.GetEntries();
    for (int i=0 ; i < n ; i++) {
      type= (RooCatType*)_types.At(i);
      os << indent;
      type->printToStream(os,opt,indent);
    }
  }
}


