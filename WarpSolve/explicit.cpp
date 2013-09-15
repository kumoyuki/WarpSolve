#include "explicit.h"
#include "solution.h"
#include "satsolver.h"
#include "SExp.h"
#include "sat3.h"


#include <stdio.h>
#include <memory>
#include <algorithm>

using namespace Heureka;
using namespace sat3;

bool add_clause(cnfExpression& exp, const List& l) { return false; }
bool add_clause(cnfExpression& exp, const Num& l) { return false; }

void solveExplicit(const char* v) {
    std::string cnf(v);
    auto r = Datum::Reader::Read(cnf);
    if(r->Errors.size() != 0) {
        fprintf(stderr, "could not parse clause: %s\n", v);
        std::for_each(std::begin(r->Errors), std::end(r->Errors),
                      [] (const Datum::Reader::Error& e) {
                          fprintf(stderr, "(%d, %d) - %s\n", e.Line, e.Column, e.Message.c_str()); });
        return; }

    // format is a list of 3-lists
    // each 3-list is a disjunction of vars named
    // a var named ~var is considered a negation of var
    const Datum& sexp = *r->Red;

    if(!sexp.IsList()) {
        fprintf(stderr, "invalid set of clauses: %s\n", sexp.AsString().c_str());
        return; }
        
    const List& clauses = dynamic_cast<const List&>(sexp);
    cnfExpression exp;
    auto err = std::find_if(
      std::begin(clauses), std::end(clauses),
      [&] (const Datum* d) -> bool {
          if(d->IsList()) {
              auto conjunct = dynamic_cast<const List&>(*d);
              if(conjunct.Length() > 3) {
                  fprintf(stderr, "not in 3CNF, %d vars in clause: %s\n",
                          conjunct.Length(),
                          conjunct.AsString().c_str());
                  return true; }
              
              return add_clause(exp, conjunct); }

          else if(d->IsNumber())
              return add_clause(exp, dynamic_cast<const Num&>(*d)); });

    if(err != std::end(clauses)) {
        fprintf(stderr, "error in clause %d\n", err - std::begin(clauses));
        return; }

    return; }
