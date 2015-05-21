/* File: ast_decl.h
 * ----------------
 * In our parse tree, Decl nodes are used to represent and
 * manage declarations. There are 4 subclasses of the base class,
 * specialized for declarations of variables, functions, classes,
 * and interfaces.
 *
 * pp3: You will need to extend the Decl classes to implement 
 * semantic processing including detection of declaration conflicts 
 * and managing scoping issues.
 */

#ifndef _H_ast_decl
#define _H_ast_decl

#include "ast.h"
#include "list.h"
#include "ast_type.h"
#include "hashtable.h"

class Type;
class NamedType;
class Identifier;
class Stmt;

class Decl : public Node 
{
  protected:
    Identifier *id;
  
  public:
    Decl(Identifier *name);
    friend std::ostream& operator<<(std::ostream& out, Decl *d) { return out << d->id; }

	Identifier *getId() {return id;}
	virtual const char *getTypeName() {return NULL;}
	virtual Type *getType() { return NULL; }
};

class VarDecl : public Decl 
{
  protected:
    Type *type;
    
  public:
    VarDecl(Identifier *name, Type *type);
//TODO
	Type *getType() {return type;}
	const char *getTypeName() {return type->getTypeName();}
	bool hasSameType(VarDecl *vd);
	void checkDeclErr();
	void checkStmt();
};

class ClassDecl : public Decl 
{
  protected:
    List<Decl*> *members;
    NamedType *extends;
    List<NamedType*> *implements;

	Hashtable<Decl*> *st;

  public:
    ClassDecl(Identifier *name, NamedType *extends, 
              List<NamedType*> *implements, List<Decl*> *members);
//TODO
	NamedType *getExt() {return extends;}
	List<NamedType*> *getImp() {return implements;}
	Hashtable<Decl*> *getST () {return st;}
	bool IsCompatibleWith(Decl *decl);
	void checkDeclErr();
	void checkStmt();
};

class InterfaceDecl : public Decl 
{
  protected:
    List<Decl*> *members;
//TODO
	Hashtable<Decl*> *st;
    
  public:
    InterfaceDecl(Identifier *name, List<Decl*> *members);
//TODO
	List<Decl*> *getMembers() {return members;}
	Hashtable<Decl*> *getST() {return st;}
	void checkDeclErr();
	void checkStmt();	
};

class FnDecl : public Decl 
{
  protected:
    List<VarDecl*> *formals;
    Type *returnType;
    Stmt *body;
//TODO
	Hashtable<Decl*> *st;
    
  public:
    FnDecl(Identifier *name, Type *returnType, List<VarDecl*> *formals);
    void SetFunctionBody(Stmt *b);
//TODO
	Type *getType() {return returnType;}
	const char *getTypeName() {return returnType->getTypeName();}
	List<VarDecl*> *getFormals() {return formals;}
	Hashtable<Decl*> *getST() {return st;}
	bool hasSameType(FnDecl *fd);
	void checkDeclErr();
	void checkStmt();	
	
};

#endif
