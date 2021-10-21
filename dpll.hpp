#ifndef DPLL_HPP
#define DPLL_HPP
#include <fstream>
#include <vector>
#include <sstream>
#include <set>
#include <string>
#include "formula.hpp"
#include "clause.hpp"

class DPLL{
public:
	void readDIMACSFile(std::string file_name){
		std::ifstream input(file_name);
		std::string line;
		std::string dump;
		size_t var_num;
		size_t clause_num;
		std::set<Clause *> clauses = std::set<Clause *>();
		Clause *curr_clause = new Clause;
		size_t curr_clause_num = 0;
		int curr_num;
		while(std::getline(input, line)){
			if (line[0] == 'c')
				continue;
			if (line[0] == '%')
				break;
			if (line[0] == 'p') {
				std::istringstream iss(line);
				iss >> dump >> dump >> var_num >> clause_num;
				literals = new Literal[2 * var_num];
				for (size_t i = 0; i < var_num; i++){
					literals[i].idx = i;
					literals[i].con_lit = literals + i + var_num;
					literals[i + var_num].idx = i + var_num;
					literals[i + var_num].con_lit = literals + i;
				}
				continue;
			}
			std::istringstream iss(line);
			while (iss >> curr_num) {
				if (curr_num == 0){
					clauses.insert(curr_clause);
					curr_clause_num++;
					if (curr_clause_num < clause_num)
						curr_clause = new Clause;
				} else {
					if (curr_num > 0)
						curr_clause->insert(literals + curr_num - 1);
					else
						curr_clause->insert(literals - curr_num + var_num - 1);
				}
			}
		}
		formula = new Formula(clauses, 2 * var_num);
	}

	bool run(){
		bool res = false;
		std::vector<Formula *> stack;
		while (true){
			formula->removeSingleClauses();
			if (formula->isEmpty()){
				res = true;
				break;
			}
			if (formula->containFalse()){
				if(stack.empty()){
					break;
				}
				delete formula;
				formula = stack.back();
				stack.pop_back();
				continue;
			}
			Literal *removed_literal = formula->chooseLiteral();
			Formula *formula_copy = new Formula(*formula);
			formula->removeLiteral(removed_literal);
			if ((formula_copy->removeLiteral(removed_literal->con_lit)) == 0)
				stack.push_back(formula_copy);
			else
				delete formula_copy;
		}
		delete formula;
		formula = nullptr;
		for (auto it = stack.begin(); it != stack.end();it++)
			delete *it;
		delete[] literals;
		literals = nullptr;
		return res;
	}

	~DPLL(){
		if (formula)
			delete formula;
		if (literals)
			delete[] literals;
	}
	
private:
	Formula *formula = nullptr;
	Literal *literals = nullptr;
};

#endif