/* File: ast_expr.cc
 * -----------------
 * Implementation of expression node classes.
 */
#include "ast_expr.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "errors.h"
#include <string.h>
#include <typeinfo>
        

IntConstant::IntConstant(yyltype loc, int val) : Expr(loc) {
    value = val;
	Expr::type = Type::intType;
}

DoubleConstant::DoubleConstant(yyltype loc, double val) : Expr(loc) {
    value = val;
	Expr::type = Type::doubleType;
}

BoolConstant::BoolConstant(yyltype loc, bool val) : Expr(loc) {
    value = val;
	Expr::type = Type::boolType;
}

StringConstant::StringConstant(yyltype loc, const char *val) : Expr(loc) {
    Assert(val != NULL);
    value = strdup(val);
	Expr::type = Type::stringType;
}


Operator::Operator(yyltype loc, const char *tok) : Node(loc) {
    Assert(tok != NULL);
    strncpy(tokenString, tok, sizeof(tokenString));
}
CompoundExpr::CompoundExpr(Expr *l, Operator *o, Expr *r) 
  : Expr(Join(l->GetLocation(), r->GetLocation())) {
    Assert(l != NULL && o != NULL && r != NULL);
    (op=o)->SetParent(this);
    (left=l)->SetParent(this); 
    (right=r)->SetParent(this);
}

CompoundExpr::CompoundExpr(Operator *o, Expr *r) 
  : Expr(Join(o->GetLocation(), r->GetLocation())) {
    Assert(o != NULL && r != NULL);
    left = NULL; 
    (op=o)->SetParent(this);
    (right=r)->SetParent(this);
}
   
//TODO
void ArithmeticExpr::checkStmt() {
	const char *lt = NULL, *rt = NULL;
	if (left){
		left->checkStmt();
		lt = left->getTypeName();
	}

	right->checkStmt();
	rt = right->getTypeName();
	if (lt && rt){
		if ((strcmp(lt, "int") && strcmp(lt, "double")) ||
			(strcmp(rt, "int") && strcmp(rt, "double")) ||
				(strcmp(lt, rt)))
			ReportError::IncompatibleOperands(this->op, new Type(lt), new Type(rt));
	}
	else if (rt){
		if (strcmp(rt, "int") && strcmp(rt, "double"))
			ReportError::IncompatibleOperand(this->op, new Type(rt));
	}
}


void RelationalExpr::checkStmt() {
	left->checkStmt();
	const char *lt = left->getTypeName();
    
	right->checkStmt();
	const char *rt = right->getTypeName();
    
	if (lt && rt){
		if ((strcmp(lt, "int") && strcmp(lt, "double")) ||
			(strcmp(rt, "int") && strcmp(rt, "double")) ||
				(strcmp(lt, rt)))
		ReportError::IncompatibleOperands(this->op, new Type(lt), new Type(rt));
	}
}


void EqualityExpr::checkStmt() {
	left->checkStmt();
    	const char *lt = left->getTypeName();

	right->checkStmt();
	const char *rt = right->getTypeName();
	if (lt && rt){
		Decl *ld = Program::st->Lookup(lt);
		Decl *rd = Program::st->Lookup(rt);
        
		if (ld && rd){
			if (!strcmp(lt, rt))
				return;
			else if (typeid(*ld) == typeid(ClassDecl)){
				ClassDecl *lClsd = dynamic_cast<ClassDecl*>(ld);
				if (lClsd->IsCompatibleWith(rd))
					return;
			}
            		else if (typeid(*rd) == typeid(ClassDecl)){
				ClassDecl *rClsd = dynamic_cast<ClassDecl*>(rd);
				if (rClsd->IsCompatibleWith(ld))
					return;
			}
		}
		else if (ld && !strcmp(rt, "null"))
			return;
		else if (!strcmp(lt, rt))
			return;
	}
	ReportError::IncompatibleOperands(this->op, new Type(lt), new Type(rt));
}


void LogicalExpr::checkStmt() {
	const char *lt = NULL, *rt = NULL;
	if (left){
		left->checkStmt();
		lt = left->getTypeName();
	}

	right->checkStmt();
	rt = right->getTypeName();

	if (lt && rt){
		if (strcmp(lt, "bool") || strcmp(rt, "bool"))
			 ReportError::IncompatibleOperands(this->op, new Type(lt), new Type(rt));
	}
	else if (rt){
		if (strcmp(rt, "bool"))
			ReportError::IncompatibleOperand(this->op, new Type(rt));
	}    
}


void AssignExpr::checkStmt() {
	left->checkStmt();
	const char *lt = this->left->getTypeName();
	right->checkStmt();
	const char *rt = this->right->getTypeName();
    
	if (lt && rt){
		Decl *ld = Program::st->Lookup(lt);
		Decl *rd = Program::st->Lookup(rt);
		
		if (ld && rd){
			if (!strcmp(lt, rt))
				return;
			else if (typeid(*rd) == typeid(ClassDecl)){
				ClassDecl *rClsd = dynamic_cast<ClassDecl*>(rd);
				if (rClsd->IsCompatibleWith(ld))
					return;
			}
		}
		else if (ld && !strcmp(rt, "null"))
			return;
		else if (!strcmp(lt, rt))
			return;

	ReportError::IncompatibleOperands(this->op, new Type(lt), new Type(rt));
	}
}


void This::checkStmt() {
	Node *parent = this->GetParent();
	while (parent){
		if (typeid(*parent) == typeid(ClassDecl)){
			type = new NamedType(dynamic_cast<ClassDecl*>(parent)->getId());
			return;
		}
	parent = parent->GetParent();
	}

	ReportError::ThisOutsideClassScope(this);
}

ArrayAccess::ArrayAccess(yyltype loc, Expr *b, Expr *s) : LValue(loc) {
    (base=b)->SetParent(this); 
    (subscript=s)->SetParent(this);
}

Type *ArrayAccess::getType() {
  Type *type = base->getType();
  if (type)
    return type->getType();
  else
    return NULL;
}

const char *ArrayAccess::getTypeName() {
  Type *type = this->base->getType();
  if (type)
    return type->getType()->getTypeName();
  else
    return NULL;
}

void ArrayAccess::checkStmt(){
	base->checkStmt();
	if(typeid(this->type) != typeid(ArrayType))
		ReportError::BracketsOnNonArray(base);
    
	subscript->checkStmt();
	if (strcmp(subscript->getTypeName(), "int"))
		ReportError::SubscriptNotInteger(this->subscript);
}
     
FieldAccess::FieldAccess(Expr *b, Identifier *f) 
  : LValue(b? Join(b->GetLocation(), f->GetLocation()) : *f->GetLocation()) {
    Assert(f != NULL); // b can be be NULL (just means no explicit base)
    base = b; 
    if (base) base->SetParent(this); 
    (field=f)->SetParent(this);
}


void FieldAccess::checkStmt() {
	Decl *decl = NULL;
	if (base){
		base->checkStmt();        
		const char *basetype = base->getTypeName();
		
		if (basetype){
			Node *parent = this->GetParent();
			Decl *classLoc = NULL;
            
			while (parent){
				Hashtable<Decl*> *st = parent->getST();
				if (st){
					if ((classLoc = st->Lookup(basetype)) != NULL){
						decl = field->checkDeclIdByName(classLoc->getST(), field->getName());
						if ((decl == NULL) || (typeid(*decl) != typeid(VarDecl)))
							ReportError::FieldNotFoundInBase(field, new Type(basetype));
					}
				}
				parent = parent->GetParent();
			}

			if (classLoc == NULL){
				if ((classLoc = Program::st->Lookup(basetype)) != NULL){
					decl = field->checkDeclIdByName(classLoc->getST(), field->getName());
					if ((decl != NULL) && (typeid(*decl) == typeid(VarDecl)))
						ReportError::InaccessibleField(field, new Type(basetype));
					else
						ReportError::FieldNotFoundInBase(field, new Type(basetype));
				}
				else
					ReportError::FieldNotFoundInBase(field, new Type(basetype));
			}
		}
	}
	else{
		decl = field->checkDeclId();
		if (decl == NULL || typeid(*decl) != typeid(VarDecl)){
			ReportError::IdentifierNotDeclared(field, LookingForVariable);
			decl = NULL;
		}
	}
	if (decl != NULL)
		type = decl->getType();

}

Call::Call(yyltype loc, Expr *b, Identifier *f, List<Expr*> *a) : Expr(loc)  {
    Assert(f != NULL && a != NULL); // b can be be NULL (just means no explicit base)
    base = b;
    if (base) base->SetParent(this);
    (field=f)->SetParent(this);
    (actuals=a)->SetParentAll(this);
}


void Call::checkArgs(FnDecl *fndecl) {
	List<VarDecl*> *formals = fndecl->getFormals();
	int formalNum = formals->NumElements();
	int actualNum = actuals->NumElements();
	if (formalNum != actualNum){
		ReportError::NumArgsMismatch(field, formalNum, actualNum);
		return;
	}
	else{
		for (int i = 0; i < formalNum; ++i){
			VarDecl *vardecl = formals->Nth(i);
			const char *et = vardecl->getTypeName();
			Expr *expr = actuals->Nth(i);
			const char *ct = expr->getTypeName();
            
			if (et && ct){
				Decl *gd = Program::st->Lookup(ct);
				Decl *ed = Program::st->Lookup(et);
                
				if (gd && ed){
					if (strcmp(et, ct)){
						if (typeid(*gd) == typeid(ClassDecl)){
							ClassDecl *gClsd = dynamic_cast<ClassDecl*>(gd);
							if (!gClsd->IsCompatibleWith(ed))
                                				ReportError::ArgMismatch(expr, (i+1), new Type(ct), new Type(et));
                        			}
					}
				}
				else if (ed && strcmp(ct, "null"))
                    			ReportError::ArgMismatch(expr, (i+1), new Type(ct), new Type(et));
                		else if (gd == NULL && ed == NULL && strcmp(ct, et))
                    			ReportError::ArgMismatch(expr, (i+1), new Type(ct), new Type(et));
			}
		}
	}
}


void Call::checkStmt() {
	if (actuals){
		for (int i = 0; i < actuals->NumElements(); ++i)
			actuals->Nth(i)->checkStmt();
	}

	Decl *decl = NULL;

	if (base){
		base->checkStmt();
		const char *name = base->getTypeName();
		
		if (name){
			if ((decl = Program::st->Lookup(name)) != NULL){
				decl = field->checkDeclIdByName(decl->getST(), field->getName());
				if ((decl == NULL) || (typeid(*decl) != typeid(FnDecl)))
					ReportError::FieldNotFoundInBase(field, new Type(name));
				else
					checkArgs(dynamic_cast<FnDecl*>(decl));
			}
			else if ((typeid(*base->getType()) == typeid(ArrayType)) && 
				!strcmp(field->getName(), "length")){
				type = Type::intType;
			}
			else{
				ReportError::FieldNotFoundInBase( field, new Type(name));
			}
		}
	}
	else{
		decl =  field->checkDeclId();
		if ((decl == NULL) || (typeid(*decl) != typeid(FnDecl))){
			ReportError::IdentifierNotDeclared(field, LookingForFunction);
			decl = NULL; 
		}
		else
			checkArgs(dynamic_cast<FnDecl*>(decl));
	}
	if (decl != NULL)
		type = decl->getType(); // returnType
}


NewExpr::NewExpr(yyltype loc, NamedType *c) : Expr(loc) { 
  Assert(c != NULL);
  (cType=c)->SetParent(this);
}

//TODO
void NewExpr::checkStmt() {
	if (cType){
		const char *name = cType->getTypeName();
		if (name){
			Decl *decl = Program::st->Lookup(name);
			if ((decl == NULL) || (typeid(*decl) != typeid(ClassDecl)))
				ReportError::IdentifierNotDeclared(new Identifier(*cType->GetLocation(), name), LookingForClass);
		}
	}
}

NewArrayExpr::NewArrayExpr(yyltype loc, Expr *sz, Type *et) : Expr(loc) {
    Assert(sz != NULL && et != NULL);
    (size=sz)->SetParent(this); 
    (elemType=et)->SetParent(this);
}


const char *NewArrayExpr::getTypeName() {
	if (elemType){
		string delim = "[]";
		string str = elemType->getTypeName() + delim;
		return str.c_str();
	}
	else
		return NULL;
}

void NewArrayExpr::checkStmt() {
	size->checkStmt();
	if (strcmp(size->getTypeName(), "int"))
		ReportError::NewArraySizeNotInteger(size);
	
	elemType->checkTypeErr();
}

