#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <set>
#include <map>

using namespace std;
struct Literal {
public:
	int idx;
	Literal *con_lit;
};

class Clause : public set<Literal *>{
private:
	int refs = 1;
public:
	Clause()
		: set<Literal *>(), refs(1){}

	Clause(const Clause &clause)
		: set<Literal *>(clause), refs(1){}

	void ref(){
		refs++;
	}
	void unref(){
		refs--;
		if (refs == 0)
			delete this;
	}
	bool lastRef(){
		return refs == 1;
	}
};
class ClauseSet : public set<Clause *>{
private:
	int refs = 1;
public:
	ClauseSet()
	: set<Clause *>(), refs(1){}
	ClauseSet(const ClauseSet &clause_set)
		: set<Clause *>(clause_set), refs(1){}
	void ref(){
		refs++;
	}
	void unref(){
		refs--;
		if (refs == 0)
			delete this;
	}
	bool lastRef(){
		return refs == 1;
	}
};

class Literal2Clause : public vector<ClauseSet *>{
public:
	Literal2Clause(int num_literals) : vector<ClauseSet *>(num_literals){}
	ClauseSet*& operator[] (const Literal* literal);
	ClauseSet*& operator[] (const int idx);
	bool find(Literal* literal){
		return operator[](literal) != NULL;
	}
	void erase(Literal* literal){
		operator[](literal) = NULL;
	}
};

ClauseSet*& Literal2Clause::operator[] (const Literal* literal){
	return vector<ClauseSet *>::operator[](literal->idx);
}
ClauseSet*& Literal2Clause::operator[] (const int idx){
	return vector<ClauseSet *>::operator[](idx);
}

class Formula {
public:
	Formula (const Formula& formula) 
	: clear_literals(formula.clear_literals), lit2clause(formula.lit2clause), 
	  clauses(formula.clauses), single_clauses(formula.single_clauses), contain_false(formula.contain_false){		
		for (auto it = clauses.begin(); it != clauses.end(); it++)
			(*it)->ref();
		for (auto it = lit2clause.begin(); it != lit2clause.end(); it++)
			if (*it)
				(*it)->ref();
	}

	Formula (set<Clause *>& clauses, int num_literals) 
	: clauses(clauses), lit2clause(Literal2Clause(num_literals)), 
	  single_clauses(set<Clause *>()), contain_false(false){
		for (int i = 0; i<num_literals; i++){
			lit2clause[i] = NULL;
		}
		for (auto it = clauses.begin(); it!= clauses.end(); it++){
			Clause* curr_clause = *it;
			for (auto it2 = curr_clause->begin(); it2 != curr_clause->end(); it2++){
				Literal* literal = *it2;
				if (not lit2clause.find(literal))
					lit2clause[literal] = new ClauseSet();
				lit2clause[literal]->insert(curr_clause);
			}
			if (curr_clause->size() == 1)
				single_clauses.insert(curr_clause);
		}
	}
	
	Literal* chooseLiteral(){
		Clause* min_clause = *clauses.begin();
		for (auto it = clauses.begin();it!=clauses.end();it++)
			if ((*it)->size() < min_clause->size())
				min_clause = *it;
		return *(min_clause->begin());
	}
	
	void removeSingleClauses(){
		// checkConsistency("removeSingleClauses1");
		while(not single_clauses.empty()){
			Clause *curr_clause = *single_clauses.begin();
			this->removeLiteral(*(curr_clause->begin()));
		}
		// checkConsistency("removeSingleClauses2");
	}

	void removeDefinitelyClearLiterals(){
		while (not clear_literals.empty())
		{
			Literal *curr_lit = clear_literals.back();
			clear_literals.pop_back();
			if (lit2clause.find(curr_lit))
				this->removeClausesWithLiteral(curr_lit);
		}


		// bool have_clear_literals = true;
		// while(have_clear_literals){
		// 	// cout << "still removing" << endl;
		// 	have_clear_literals = false;
		// 	for (auto it = lit2clause.begin(); it!= lit2clause.end(); it++){
		// 		Literal *literal = it->first;
		// 		if (lit2clause.find(literal->con_lit) == lit2clause.end())
		// 		{
		// 			have_clear_literals = true;
		// 			this->removeClausesWithLiteral(literal);
		// 			break;
		// 		}
		// 	}
		// }
	}
	
	int removeLiteral(Literal *literal){
		// formula_copy->checkConsistency("copy1");
		if (not lit2clause.find(literal)){
			// cout << "problem";
			return -1;}
		Literal *con_lit = literal->con_lit;
		// checkConsistency("removeLiteral1");
		this->removeClausesWithLiteral(literal);
		// cout << "Here" << endl;
		// formula_copy->checkConsistency("copy2");
		// cout << "Here0" << endl;
		if (lit2clause.find(con_lit)){
			this->removeLiteralFromClauses(con_lit);
			// formula_copy->checkConsistency("copy2.5");
		}
		// formula_copy->checkConsistency("copy3");
		// formula_copy->checkConsistency("copy4");
		// checkConsistency("removeLiteral2");
		return 0;
	}
		
	bool isEmpty(){
		return clauses.empty();
	}
	
	bool containFalse(){
		return contain_false;
	}
	
	~Formula() {
		for (auto it = lit2clause.begin(); it != lit2clause.end(); it++)
			if (*it)
				(*it)->unref();
		for (auto it = clauses.begin(); it != clauses.end(); it++)
			(*it)->unref();
	}
private:
	vector<Literal *> clear_literals;
	Literal2Clause lit2clause;
	set<Clause *> clauses;
	set<Clause *> single_clauses;
	bool contain_false;
	
	void removeClausesWithLiteral(Literal *literal){
		// cout << lit2clause[literal]->size() << endl;
		// checkConsistency("removeClausesWithLiteral1");
		ClauseSet * map_literal = lit2clause[literal];
		for (auto it = map_literal->begin();it!=map_literal->end();++it){
			Clause * curr_clause = *it;
			// cout << "erase1\n";
			clauses.erase(curr_clause);
			if (curr_clause->size() == 1)
				single_clauses.erase(curr_clause);
			// cout << "erase1 end\n";
			for (auto it2 = (curr_clause)->begin(); it2 != (curr_clause)->end(); it2++){
				Literal* curr_lit = *it2;
				ClauseSet *lit_set = lit2clause[curr_lit];
				if (curr_lit != literal) {
					if (lit_set->size() == 1){
						lit_set->unref();
						lit2clause.erase(curr_lit);
						clear_literals.push_back(curr_lit->con_lit);
					} else{
						if (not lit_set->lastRef()){
							// cout << "in not last ref" << endl;
							ClauseSet* new_set = new ClauseSet(*lit_set);
							// cout << new_set->refs << endl;
							// cout << "unref\n";
							lit_set->unref();
							// cout << "unref end\n";
							lit2clause[curr_lit] = new_set;
							lit_set = new_set;
						}
						// cout << "erase\n";
						lit_set->erase(curr_clause);
					}
				}
			}
			// cout << "unref2\n";
			curr_clause->unref();
			// cout << "unref2 end\n";
		}
		map_literal->unref();
		lit2clause.erase(literal);
		// cout << "out of for" << endl;
		// checkConsistency("removeClausesWithLiteral2");
		// cout << "out of for2" << endl;
	}

	// void checkConsistency(string str){
	// 	bool consistent = true;
	// 	bool clause_cons = true;
	// 	bool map_cons = true;
	// 	// cout <<"for1" << endl;
	// 	for (auto it = clauses->begin(); it!= clauses->end(); it++){
	// 		Clause* curr_clause = *it;
	// 		for (auto it2 = curr_clause->begin(); it2!= curr_clause->end(); it2++){
	// 			if (lit2clause[*it2]->find(curr_clause) == lit2clause[*it2]->end()){
	// 				consistent = false;
	// 				clause_cons = false;
	// 			}

	// 		}
	// 	}
	// 	// cout << "out of for1" << endl;
	// 	for (auto it = lit2clause.begin(); it!= lit2clause.end(); it++){
	// 		Literal *literal = it->first;
	// 		ClauseSet *clause_set = it->second;
	// 		// cout << "for3" << endl;
	// 		// cout << clause_set->size() << endl;
	// 		for (auto it2 = clause_set->begin(); it2!= clause_set->end(); it2++){
	// 			// cout << (*it2)->size() << endl;
	// 			if ((*it2)->find(literal) == (*it2)->end()){
	// 				consistent = false;
	// 				map_cons = false;
	// 			}
	// 		}
	// 		// cout << "out of for3" << endl;
	// 	}
	// 	// cout << "out of for2" << endl;
	// 	if (not consistent){
	// 		cout << "not consistent "<< clause_cons << map_cons << str << endl;
	// 	}
	// 	else
	// 		cout << clause_cons << map_cons << str << endl;
	// }

	void removeLiteralFromClauses(Literal *literal){
		for (auto it = lit2clause[literal]->begin();it!=lit2clause[literal]->end();++it){
			Clause *curr_clause = *it;
			if (not curr_clause->lastRef()){
				// cout << "not lastRef" << endl;
				Clause* new_clause = new Clause(*curr_clause);
				// formula_copy->checkConsistency("copy3.2100");
				// cout << formula_copy->clauses->size() <<endl;
				clauses.erase(curr_clause);
				// formula_copy->checkConsistency("copy3.2200");
				// cout << formula_copy->clauses->size() <<endl;
				clauses.insert(new_clause);
				// cout << formula_copy->clauses->size() <<endl;
				// formula_copy->checkConsistency("copy3.2300");
				curr_clause->unref();
				// formula_copy->checkConsistency("copy3.2400");
				new_clause->erase(literal);
				// formula_copy->checkConsistency("copy3.2500");
				
				// formula_copy->checkConsistency("copy3.200");
				for (auto it2 = new_clause->begin(); it2 != new_clause->end(); it2++){
					Literal *curr_lit = *it2;
					ClauseSet *map_lit = lit2clause[curr_lit];
					if (not map_lit->lastRef()){
						ClauseSet* new_set = new ClauseSet(*map_lit);
						map_lit->unref();
						lit2clause[curr_lit] = new_set;
						map_lit = new_set;
					}
					map_lit->erase(curr_clause);
					map_lit->insert(new_clause);
				}
				curr_clause = new_clause;
				// formula_copy->checkConsistency("copy3.100");

			} else{
				curr_clause->erase(literal);
				// formula_copy->checkConsistency("copy3.000");
			}
			if (curr_clause->empty()){
				single_clauses.erase(curr_clause);
				contain_false = true;
			}
			if (curr_clause->size() == 1)
				single_clauses.insert(curr_clause);
		}
		lit2clause[literal]->unref();
		lit2clause.erase(literal);
		// checkConsistency("removeLiteralFromClauses");
	}
};

/*class Formula {
public:
	Formula (int var_num, int clause_num, int ** clause2var): 
		     var_num(var_num), clause_num(clause_num), clause2var(clause2var) {
		vector<int> * tmp_array = new vector<int>[var_num + 1];
		for (int i = 0; i < clause_num; i++) {
			int * curr_clause = clause2var[i];
			int clause_vars = curr_clause[0];
			for (int j = 1; j <= clause_vars; j++){
				int curr_lit = curr_clause[j];
				if (curr_lit > 0)
					tmp_array[curr_lit].push_back(i);
				else
					tmp_array[-curr_lit].push_back(-i);
			}
		}
		
		var2clause = new int* [var_num + 1];
		for (int i = 1; i <= var_num; i++){
			vector<int> buffer = tmp_array[i];
			int * new_array = new int [buffer.size() + 1];
				new_array[0] = buffer.size();
				copy (buffer.begin(), buffer.end(), new_array + 1);
				var2clause[i] = new_array;
			
		}
		for (auto it = var2clause[1] + 1; it != var2clause[1] + 1 + var2clause[1][0]; ++it){
			cout << *it << " ";
		}
		delete [] tmp_array;
	}
	
	Formula remove_lit(int lit){
		
	}
	
private:
	int var_num;
	int clause_num;
	int ** clause2var;
	int ** var2clause;
	
};
*/
int main(int argc, char *argv[]) {
	string file_name = argv[1];
	ifstream input(file_name);
	string line;
	string dump;
	int var_num;
	int clause_num;
	Literal *literals;
	set<Clause *> clauses = set<Clause *>();
	Clause *curr_clause = new Clause;
	int curr_clause_num = 0;
	int curr_num;
	while(getline(input, line)){
		if (line[0] == 'c')
			continue;
		if (line[0] == '%')
			break;
		if (line[0] == 'p') {
			istringstream iss(line);
			iss >> dump >> dump >> var_num >> clause_num;
			literals = new Literal[2 * var_num];
			for (int i = 0; i < var_num; i++){
				literals[i].idx = i;
				literals[i].con_lit = literals + i + var_num;
				literals[i + var_num].idx = i + var_num;
				literals[i + var_num].con_lit = literals + i;
			}
			continue;
		}
		istringstream iss(line);
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
	bool res = false;
	Formula *formula = new Formula(clauses, 2 * var_num);

	vector<Formula *> stack;
	while (true){
	// for (int i = 0; i < 1; i++) {
		// cout << stack->size() << endl;
		// cout << "removeSingleClauses" << endl;
		formula->removeDefinitelyClearLiterals();
		formula->removeSingleClauses();
		// cout << "end removeSingleClauses" << endl;

		if (formula->isEmpty()){
			// cout << "isEmpty\n";
			res = true;
			break;
		}
		if (formula->containFalse()){
			delete formula;
			if(formula = stack.back()){
				stack.pop_back();
				// cout << stack->size();
				// cout << "continue\n";
				continue;
			}
			else {
			// cout << "break\n";
				break;}
		}
		// cout << "choose Literal" << endl;
		Literal *removed_literal = formula->chooseLiteral();
		// cout << "make copy" << endl;
		Formula *formula_copy = new Formula(*formula);
		// formula->checkConsistency("formula");
		// formula_copy->checkConsistency("formula2");
		// cout << "remove literal" << endl;
		formula->removeLiteral(removed_literal);
		// formula->checkConsistency("formula_remove");
		// formula_copy->checkConsistency("formula2_remove");
		// cout << "remove literal2" << endl;
		if ((formula_copy->removeLiteral(removed_literal->con_lit)) == 0)
			stack.push_back(formula_copy);
		// cout << "end" << endl;

	}
	delete formula;
	for (auto it = stack.begin(); it != stack.end();it++)
		delete *it;
	// delete formula_copy;
	cout << res;
	delete[] literals;
}

