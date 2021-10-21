#ifndef CLAUSE_HPP
#define CLAUSE_HPP
#include <vector>
#include <set>

struct Literal {
public:
	size_t idx;
	Literal *con_lit;
};

class Clause : public std::set<Literal *>{
private:
	size_t refs = 1;
public:
	Clause()
		: std::set<Literal *>(), refs(1){}

	Clause(const Clause &clause)
		: std::set<Literal *>(clause), refs(1){}

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

class ClauseSet : public std::set<size_t>{
private:
	size_t refs = 1;
public:
	ClauseSet()
	: std::set<size_t>(), refs(1){}
	ClauseSet(const ClauseSet &clause_set)
		: std::set<size_t>(clause_set), refs(1){}
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

class Literal2Clause : public std::vector<ClauseSet *>{
public:
	Literal2Clause(size_t num_literals) : std::vector<ClauseSet *>(num_literals){}
	ClauseSet*& operator[] (const Literal* literal);
	ClauseSet*& operator[] (const size_t idx);
	bool find(Literal* literal){
		return operator[](literal) != nullptr;
	}
	void erase(Literal* literal){
		operator[](literal) = nullptr;
	}
};

ClauseSet*& Literal2Clause::operator[] (const Literal* literal){
	return std::vector<ClauseSet *>::operator[](literal->idx);
}
ClauseSet*& Literal2Clause::operator[] (const size_t idx){
	return std::vector<ClauseSet *>::operator[](idx);
}
#endif
