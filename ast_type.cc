/* File: ast_type.cc
 * -----------------
 * Implementation of type node classes.
 */
#include "ast_type.h"
#include "ast_decl.h"
#include "ast_stmt.h"
#include "errors.h"
#include <string.h>
#include <typeinfo>
 
/* Class constants
 * ---------------
 * These are public constants for the built-in base types (int, double, etc.)
 * They can be accessed with the syntax Type::intType. This allows you to
 * directly access them and share the built-in types where needed rather that
 * creates lots of copies.
 */

Type *Type::intType    = new Type("int");
Type *Type::doubleType = new Type("double");
Type *Type::voidType   = new Type("void");
Type *Type::boolType   = new Type("bool");
Type *Type::nullType   = new Type("null");
Type *Type::stringType = new Type("string");
Type *Type::errorType  = new Type("error"); 

Type::Type(const char *n) {
    Assert(n);
    typeName = strdup(n);
}
	
NamedType::NamedType(Identifier *i) : Type(*i->GetLocation()) {
    Assert(i != NULL);
    (id=i)->SetParent(this);
}

//TODO
bool NamedType::hasSameType(Type *nt) {
	if (typeid(*nt) == typeid(NamedType))
		return !strcmp(this->getTypeName(), nt->getTypeName());

	return false;
}


void NamedType::checkTypeErr() {
	const char *name = id->getName();
	Decl *decl = Program::st->Lookup(name);
	if ((decl == NULL) || 
		(((typeid(*decl) != typeid(ClassDecl))) && 
			((typeid(*decl) != typeid(InterfaceDecl))))){
		ReportError::IdentifierNotDeclared(id, LookingForType);
		id = NULL;
	}
}

ArrayType::ArrayType(yyltype loc, Type *et) : Type(loc) {
    Assert(et != NULL);
    (elemType=et)->SetParent(this);
}

//TODO
const char *ArrayType::getTypeName() { 
	if (elemType){
		string delim = "[]";
		string str = elemType->getTypeName() + delim;
		return str.c_str();
	}
	else return NULL;
}


bool ArrayType::hasSameType(Type *at) {
  return elemType->hasSameType(at->getType());
}

void ArrayType::checkTypeErr() {
   elemType->checkTypeErr();
}

