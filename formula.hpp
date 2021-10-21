#ifndef FORMULA_HPP
#define FORMULA_HPP
#include <vector>
#include <set>
#include "clause.hpp"

class Formula {
public:
	Formula (const Formula& formula) 
	: lit2clause(formula.lit2clause), 
	  clauses(formula.clauses), single_clauses(formula.single_clauses), contain_false(formula.contain_false){		
		for (auto it = clauses.begin(); it != clauses.end(); it++)
			if (*it)
				(*it)->ref();
		for (auto it = lit2clause.begin(); it != lit2clause.end(); it++)
			if (*it)
				(*it)->ref();
	}

	Formula (std::set<Clause *>& set_clauses, size_t num_literals) 
	: clauses(std::vector<Clause *>(set_clauses.begin(),set_clauses.end())), lit2clause(Literal2Clause(num_literals)), 
	  single_clauses(std::set<size_t>()), contain_false(false){
		for (size_t i = 0; i<num_literals; i++){
			lit2clause[i] = nullptr;
		}
		for (size_t i = 0; i<clauses.size(); i++){
			Clause* curr_clause = clauses[i];
			for (auto it2 = curr_clause->begin(); it2 != curr_clause->end(); it2++){
				Literal* literal = *it2;
				if (not lit2clause.find(literal))
					lit2clause[literal] = new ClauseSet();
				lit2clause[literal]->insert(i);
			}
			if (curr_clause->size() == 1)
				single_clauses.insert(i);
		}
	}
	
	Literal* chooseLiteral(){
		Clause* min_clause = nullptr;
		for (size_t i = 0; i < clauses.size(); i++)
			if (clauses[i])
				if (not (min_clause)){
					min_clause = clauses[i];
				} else {
					if (clauses[i]->size() < min_clause->size())
						min_clause = clauses[i];
				}

		return *(min_clause->begin());
	}
	
	void removeSingleClauses(){
		while(not single_clauses.empty()){
			Clause *curr_clause = clauses[*single_clauses.begin()];
			this->removeLiteral(*(curr_clause->begin()));
		}
	}

	int removeLiteral(Literal *literal){
		if (not lit2clause.find(literal)){
			return -1;
		}
		Literal *con_lit = literal->con_lit;
		this->removeClausesWithLiteral(literal);
		if (lit2clause.find(con_lit)){
			this->removeLiteralFromClauses(con_lit);
		}
		return 0;
	}
		
	bool isEmpty(){
		for (auto it = clauses.begin(); it != clauses.end(); it++)
			if (*it)
				return false;
		return true;
	}
	
	bool containFalse(){
		return contain_false;
	}
	
	~Formula() {
		for (auto it = lit2clause.begin(); it != lit2clause.end(); it++)
			if (*it)
				(*it)->unref();
		for (auto it = clauses.begin(); it != clauses.end(); it++)
			if (*it)
				(*it)->unref();
	}
	
private:
	Literal2Clause lit2clause;
	std::vector<Clause *> clauses;
	std::set<size_t> single_clauses;
	bool contain_false;
	
	void removeClausesWithLiteral(Literal *literal){
		ClauseSet * map_literal = lit2clause[literal];
		for (auto it = map_literal->begin();it!=map_literal->end();++it){
			size_t clause_num = *it;
			Clause * curr_clause = clauses[clause_num];
			clauses[clause_num] = nullptr;
			if (curr_clause->size() == 1)
				single_clauses.erase(clause_num);
			for (auto it2 = (curr_clause)->begin(); it2 != (curr_clause)->end(); it2++){
				Literal* curr_lit = *it2;
				ClauseSet *lit_set = lit2clause[curr_lit];
				if (curr_lit != literal) {
					if (lit_set->size() == 1){
						lit_set->unref();
						lit2clause.erase(curr_lit);
					} else{
						if (not lit_set->lastRef()){
							ClauseSet* new_set = new ClauseSet(*lit_set);
							lit_set->unref();
							lit2clause[curr_lit] = new_set;
							lit_set = new_set;
						}
						lit_set->erase(clause_num);
					}
				}
			}
			curr_clause->unref();
		}
		map_literal->unref();
		lit2clause.erase(literal);
	}

	void removeLiteralFromClauses(Literal *literal){
		for (auto it = lit2clause[literal]->begin();it!=lit2clause[literal]->end();++it){
			size_t clause_num = *it;
			Clause *curr_clause = clauses[clause_num];
			if (not curr_clause->lastRef()){
				Clause* new_clause = new Clause(*curr_clause);
				clauses[clause_num] = new_clause;
				curr_clause->unref();
				new_clause->erase(literal);
				curr_clause = new_clause;
			} else{
				curr_clause->erase(literal);
			}
			if (curr_clause->size() == 1)
				single_clauses.insert(clause_num);
			else if (curr_clause->empty()){
				single_clauses.erase(clause_num);
				contain_false = true;
			}
		}
		lit2clause[literal]->unref();
		lit2clause.erase(literal);
	}
};
#endif
