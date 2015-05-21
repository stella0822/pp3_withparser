/* File: ast_decl.cc
 * -----------------
 * Implementation of Decl node classes.
 */
#include "ast_decl.h"
#include "ast_type.h"
#include "ast_stmt.h"
#include "errors.h"

#include <stdio.h>
#include <string.h>
#include <typeinfo>
        
         
Decl::Decl(Identifier *n) : Node(*n->GetLocation()) {
    Assert(n != NULL);
    (id=n)->SetParent(this); 
}


VarDecl::VarDecl(Identifier *n, Type *t) : Decl(n) {
    Assert(n != NULL && t != NULL);
    (type=t)->SetParent(this);
}
  
//TODO
bool VarDecl::hasSameType(VarDecl *vd){
	if (type) return type->hasSameType(vd->getType());
	else return false;
}

void VarDecl::checkStmt(){}

void VarDecl::checkDeclErr(){
	if(type) type->checkTypeErr();
}

ClassDecl::ClassDecl(Identifier *n, NamedType *ex, List<NamedType*> *imp, List<Decl*> *m) : Decl(n) {
    // extends can be NULL, impl & mem may be empty lists but cannot be NULL
    Assert(n != NULL && imp != NULL && m != NULL);     
    extends = ex;
    if (extends) extends->SetParent(this);
    (implements=imp)->SetParentAll(this);
    (members=m)->SetParentAll(this);
//TODO
	st = new Hashtable<Decl*>;
}

//TODO
void ClassDecl::checkStmt(){
	if (members){
		for (int i = 0; i < members->NumElements(); ++i){
			members->Nth(i)->checkStmt();
		}
	}
}

void ClassDecl::checkDeclErr() {
	st->Enter(this->getId()->getName(), this);
	
	if(members){
		for (int i = 0; i < members->NumElements(); ++i){
			Decl *cur = members->Nth(i);
			Decl *prev;
			const char *name = cur->getId()->getName();
			if (name && ((prev = st->Lookup(name)) != NULL))
				ReportError::DeclConflict(cur, prev);
			else 	st->Enter(name, cur);
		}
	}


	NamedType *ext = extends;
	while(ext){
		const char *name = ext->getId()->getName();
		if (name){
			Node *node = Program::st->Lookup(name);
			if (node == NULL) {
				ReportError::IdentifierNotDeclared(ext->getId(), LookingForClass);
				break;
			}else if (typeid(*node) == typeid(ClassDecl)){
				ClassDecl *base = dynamic_cast<ClassDecl*>(node);
				List<Decl*> *baseMembers = base->members;
				List<Decl*> *inherited = new List<Decl*>;
				if (baseMembers){
					for (int i = 0; i < baseMembers->NumElements(); ++i){
						Decl *cur = baseMembers->Nth(i);
						Decl *prev;
						const char *name = cur->getId()->getName();
						if ((prev = st->Lookup(name)) != NULL){
							if (typeid(*cur) == typeid(VarDecl) || typeid(*cur) != typeid(*prev))
								ReportError::DeclConflict(prev, cur);
							else if (typeid(*cur) == typeid(FnDecl) && typeid(*cur) == typeid(*prev)){
								FnDecl *fdcur = dynamic_cast<FnDecl*>(cur);
								FnDecl *fdprev = dynamic_cast<FnDecl*>(prev);
								if (!fdcur->hasSameType(fdprev))
									ReportError::OverrideMismatch(fdprev);
							}
						}
						else inherited->Append(cur);
					}
					
					for (int i = 0; i < inherited->NumElements(); ++i){
						Decl *decl = inherited->Nth(i);
						st->Enter(decl->getId()->getName(), decl);
					}
				}
				ext = base->getExt();
			}
		}
	}
	
	if (implements){
		for (int i = 0; i < implements->NumElements(); ++i){
			NamedType *implement = implements->Nth(i);
			Identifier *id = implement->getId();
			if (id){
				Node *n = Program::st->Lookup(id->getName());
				if (n == NULL || (typeid(*n) != typeid(InterfaceDecl)))
					ReportError::IdentifierNotDeclared(id, LookingForInterface);
				else if (typeid(*n) == typeid(InterfaceDecl)){
					InterfaceDecl *interfd = dynamic_cast<InterfaceDecl*>(n);
					List<Decl*> *members = interfd->getMembers();
					for (int j = 0; j < members->NumElements(); ++j){
						FnDecl *cur = dynamic_cast<FnDecl*>(members->Nth(j));
						Decl *prev;
						const char *name = cur->getId()->getName();
						
						if ((prev = st->Lookup(name)) != NULL){
							if (typeid(*prev) != typeid(FnDecl))
								ReportError::DeclConflict(cur, prev);
							else if (!cur->hasSameType(dynamic_cast<FnDecl*>(prev)))
								ReportError::OverrideMismatch(prev);
						}
						else ReportError::InterfaceNotImplemented(this, implement);
					}
				}
			}
		}
	}

	if (members){
		for (int i = 0; i < members->NumElements(); ++i)
			members->Nth(i)->checkDeclErr();
	}
}


bool ClassDecl::IsCompatibleWith(Decl *decl){
	NamedType *ext = this->getExt();
  	List<NamedType*> *imp = this->getImp();

	if (typeid(*decl) == typeid(ClassDecl)){
		ClassDecl *clsd = dynamic_cast<ClassDecl*>(decl);

		if (ext){
			const char *name = ext->getTypeName();
			if (!strcmp(clsd->getId()->getName(), name))
				return true;
			else{
				if (name){
					Decl *extd = Program::st->Lookup(name);
					if (extd && typeid(*extd) == typeid(ClassDecl))
						return dynamic_cast<ClassDecl*>(extd)->IsCompatibleWith(decl);
				}
			}
		}
	}
	else if (typeid(*decl) == typeid(InterfaceDecl)){
		InterfaceDecl *interfd = dynamic_cast<InterfaceDecl*>(decl);
		
		if (imp){
			for (int i = 0; i < imp->NumElements(); ++i){
				NamedType *implement = imp->Nth(i);
				if (implement && !strcmp(interfd->getId()->getName(), implement->getTypeName()))
					return true;
			}
		}

		if(ext){
			const char *name = ext->getTypeName();
			if (name) {
				Decl *extd = Program::st->Lookup(name);
				if (extd && typeid(*extd) == typeid(ClassDecl))
					return dynamic_cast<ClassDecl*>(extd)->IsCompatibleWith(decl);
			}
		}
	}

	return false;
}	

InterfaceDecl::InterfaceDecl(Identifier *n, List<Decl*> *m) : Decl(n) {
    Assert(n != NULL && m != NULL);
    (members=m)->SetParentAll(this);
//TODO
	st = new Hashtable<Decl*>;
}

//TODO
void InterfaceDecl::checkDeclErr() {
	if (members){
		for (int i = 0; i < members->NumElements(); ++i){
			Decl *cur = members->Nth(i);
			Decl *prev;
			const char *name = cur->getId()->getName();	
			if (name){
				if ((prev = st->Lookup(name)) != NULL)
					ReportError::DeclConflict(cur, prev);
				else st->Enter(name, cur);
			}
		}
	}
}



void InterfaceDecl::checkStmt() {
    
}
	
FnDecl::FnDecl(Identifier *n, Type *r, List<VarDecl*> *d) : Decl(n) {
    Assert(n != NULL && r!= NULL && d != NULL);
    (returnType=r)->SetParent(this);
    (formals=d)->SetParentAll(this);
    body = NULL;
//TODO
	st = new Hashtable<Decl*>;
}

void FnDecl::SetFunctionBody(Stmt *b) { 
    (body=b)->SetParent(this);
}

//TODO
bool FnDecl::hasSameType(FnDecl *fd){
	if (!strcmp(id->getName(), fd->getId()->getName())){
		if (returnType->hasSameType(fd->getType())){
			List<VarDecl*> *fmList = formals;
			List<VarDecl*> *fdList = fd->getFormals();
			if (fmList && fdList){
				if (fmList->NumElements() == fdList->NumElements()){
					for (int i = 0; i < fmList->NumElements(); ++i){
						VarDecl *fmvd = fmList->Nth(i);
						VarDecl *fdvd = fdList->Nth(i);
						if (0!=fmvd->hasSameType(fdvd))
							return false;
					}
					return true;
				}
			}
		}
	}
	return false;
}

void FnDecl::checkDeclErr(){

}

void FnDecl::checkStmt(){

}



