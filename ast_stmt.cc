/* File: ast_stmt.cc
 * -----------------
 * Implementation of statement node classes.
 */
#include "ast_stmt.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "ast_expr.h"
#include "errors.h"
#include <typeinfo>

//TODO
Hashtable<Decl*> *Program::st  = new Hashtable<Decl*>;

Program::Program(List<Decl*> *d) {
    Assert(d != NULL);
    (decls=d)->SetParentAll(this);
}

//TODO
void Program::checkStmt() {
	for (int i = 0; i < decls->NumElements(); ++i)
		decls->Nth(i)->checkStmt();
}

void Program::checkDeclErr() {
	if (decls){
		for (int i = 0; i < decls->NumElements(); ++i){
			Decl *cur = decls->Nth(i);
			Decl *prev;
			const char *name = cur->getId()->getName();
			if (name){
				if ((prev = Program::st->Lookup(name)) != NULL)
					ReportError::DeclConflict(cur, prev);
				else st->Enter(name, cur);
			}
		}

		for (int i = 0; i < decls->NumElements(); ++i)
			decls->Nth(i)->checkDeclErr();
	}
}

void Program::Check() {
    /* pp3: here is where the semantic analyzer is kicked off.
     *      The general idea is perform a tree traversal of the
     *      entire program, examining all constructs for compliance
     *      with the semantic rules.  Each node can have its own way of
     *      checking itself, which makes for a great use of inheritance
     *      and polymorphism in the node classes.
     */
	this->checkStmt();
	this->checkDeclErr();
}

StmtBlock::StmtBlock(List<VarDecl*> *d, List<Stmt*> *s) {
    Assert(d != NULL && s != NULL);
    (decls=d)->SetParentAll(this);
    (stmts=s)->SetParentAll(this);
//TODO
	st = new Hashtable<Decl*>;
}

//TODO
void StmtBlock::checkStmt() {
	if (stmts){
		for (int i = 0; i < stmts->NumElements(); ++i){
			Stmt *stmt = stmts->Nth(i);
			stmt->checkStmt();
		}
	}
}

void StmtBlock::checkDeclErr() {
	if (decls){
		for (int i = 0; i < decls->NumElements(); ++i){
			VarDecl *cur = decls->Nth(i);
			Decl *prev;
			const char *name = cur->getId()->getName();

			if (name){
				if ((prev = st->Lookup(name)) != NULL)
					ReportError::DeclConflict(cur, prev);
				else{
					st->Enter(name, cur);
					cur->checkDeclErr();
				}
			}
		}
	}

	if (stmts){
		for (int i = 0; i < stmts->NumElements(); ++i){
			Stmt *stmt = stmts->Nth(i);
			stmt->checkDeclErr();
		}
	}
}


ConditionalStmt::ConditionalStmt(Expr *t, Stmt *b) { 
    Assert(t != NULL && b != NULL);
    (test=t)->SetParent(this); 
    (body=b)->SetParent(this);
}

//TODO
void ConditionalStmt::checkStmt(){
	test->checkStmt();
	if (strcmp(test->getTypeName(), "bool"))
		ReportError::TestNotBoolean(test);

	body->checkStmt();
}

void ConditionalStmt::checkDeclErr(){

}

ForStmt::ForStmt(Expr *i, Expr *t, Expr *s, Stmt *b): LoopStmt(t, b) { 
    Assert(i != NULL && t != NULL && s != NULL && b != NULL);
    (init=i)->SetParent(this);
    (step=s)->SetParent(this);
}

//TODO
void ForStmt::checkStmt() {
	if (init)
		init->checkStmt();
	if (step)
		step->checkStmt();
  	
	ConditionalStmt::checkStmt();
}

void WhileStmt::checkStmt() {
	ConditionalStmt::checkStmt();
}

IfStmt::IfStmt(Expr *t, Stmt *tb, Stmt *eb): ConditionalStmt(t, tb) { 
    Assert(t != NULL && tb != NULL); // else can be NULL
    elseBody = eb;
    if (elseBody) elseBody->SetParent(this);
}

//TODO
void IfStmt::checkDeclErr() {
	ConditionalStmt::checkDeclErr();
	if (elseBody)
		elseBody->checkDeclErr();
}

void IfStmt::checkStmt() {
	ConditionalStmt::checkStmt();
	if (elseBody)
		elseBody->checkStmt();
}

void BreakStmt::checkStmt() {
	Node *parent = this->GetParent();
	while (parent){
		if ((typeid(*parent) == typeid(WhileStmt)) ||
			(typeid(*parent) == typeid(ForStmt)))
			return;
	
	parent = parent->GetParent();
	}
	
	ReportError::BreakOutsideLoop(this);
}


ReturnStmt::ReturnStmt(yyltype loc, Expr *e) : Stmt(loc) { 
    Assert(e != NULL);
    (expr=e)->SetParent(this);
}

//TODO

void ReturnStmt::checkStmt() {
	const char *correctLoc;
	Node *parent = this->GetParent();
	
	while (parent){
		if (typeid(*parent) == typeid(FnDecl))
			correctLoc = dynamic_cast<FnDecl*>(parent)->getTypeName();

		parent = parent->GetParent();
	}
	if (expr){
		expr->checkStmt();
		const char *curType = expr->getTypeName();

		if (curType && correctLoc){
			Decl *gd = Program::st->Lookup(curType);
          		Decl *cd = Program::st->Lookup(correctLoc);

			if (gd && cd){
				if (!strcmp(curType, correctLoc))
					return;
				else if (typeid(*gd) == typeid(ClassDecl)){
					ClassDecl *gClsd = dynamic_cast<ClassDecl*>(gd);

					if (gClsd->IsCompatibleWith(cd))
					return;
				}
			}
			else if (cd && !strcmp(curType, "null"))
				return;
			else if (!strcmp(curType, correctLoc))
				return;

			ReportError::ReturnMismatch(this, new Type(curType), new Type(correctLoc));
		}
	}
	else if (strcmp("void", correctLoc))
		ReportError::ReturnMismatch(this, new Type("void"), new Type(correctLoc));
}

PrintStmt::PrintStmt(List<Expr*> *a) {    
    Assert(a != NULL);
    (args=a)->SetParentAll(this);
}

//TODO

void PrintStmt::checkStmt() {
	if (args){
		for (int i = 0; i < args->NumElements(); ++i){
			Expr *expr = args->Nth(i);
			expr->checkStmt();
			const char *typeName = expr->getTypeName();
		if (typeName && strcmp(typeName, "string") && strcmp(typeName, "int") && strcmp(typeName, "bool"))
			ReportError::PrintArgMismatch(expr, (i+1), new Type(typeName));
		}
	}
}


