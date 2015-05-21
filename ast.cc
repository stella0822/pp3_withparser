/* File: ast.cc
 * ------------
 */

#include "ast.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "ast_stmt.h"
#include "errors.h"
#include <string.h> // strdup
#include <stdio.h>  // printf

Node::Node(yyltype loc) {
    location = new yyltype(loc);
    parent = NULL;
}

Node::Node() {
    location = NULL;
    parent = NULL;
}
	 
Identifier::Identifier(yyltype loc, const char *n) : Node(loc) {
    name = strdup(n);
} 

//TODO
//Lookup decl from local-most to global scope
Decl *Identifier::checkDeclId() {
	Decl *decl = NULL;
	Node *parent = this->GetParent();
	while (parent){
		Hashtable<Decl*> *st = parent->getST();
		if ((st != NULL) && ((decl = st->Lookup(this->name)) != NULL)) return decl;
		
		parent = parent->GetParent();
	}
	
	return Program::st->Lookup(this->name);	
}

//Lookup decl in the fixed scope with a given name
Decl *Identifier::checkDeclIdByName(Hashtable<Decl*> *st, const char *name){
	return (st) ? (st->Lookup(name)) : NULL;
}
