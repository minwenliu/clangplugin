//===- PrintFunctionNames.cpp ---------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Example clang plugin which simply prints the names of all the top-level decls
// in the input file.
//
//===----------------------------------------------------------------------===//

#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTImporter.h" 
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Sema/Sema.h"
#include "llvm/Support/raw_ostream.h"
#include "clang/Basic/SourceManager.h" 
#include "clang/Basic/FileManager.h" 
#include "clang/AST/Type.h" 
#include "log.h" 
#include "commonhelp.h" 
#include "SingletonHolder.h" 
#include "Struct_Record.h" 
#include <algorithm> 
#include <vector> 
using namespace std;
using namespace clang;


namespace {

    class PrintFunctionsConsumer : public ASTConsumer {
        CompilerInstance& Instance;
        str_commamdargs command; 
        SourceManager& sm;
        string mainfilename;
        public:
        PrintFunctionsConsumer(CompilerInstance &pinstance,str_commamdargs pargs): Instance(pinstance),command(pargs),sm(Instance.getSourceManager()),mainfilename("") {
            mainfilename=sm.getFileEntryForID(sm.getMainFileID())->getName();
            Help::PathConversion(mainfilename);
        }

#define FUNCTION_LIST()  \
        DEF_TRAVERSE_FUNCTION(CXXConstructor) DEF_TRAVERSE_FUNCTION(Function) DEF_TRAVERSE_FUNCTION(CXXMethod)

        void HandleTranslationUnit(ASTContext& context) override {

            struct Visitor : public RecursiveASTVisitor<Visitor> {
                Visitor(CompilerInstance& ins,SourceManager &psm,const str_commamdargs &pargs,string pmainfilename)
                    : Instance(ins),sm(psm),command(pargs),functionstring(""),enterassignleft(false),functionkey(-1),mainfilename(pmainfilename) {
                    }

                //木有办法，基类的TraverseFunctionHelper(D)为私有属性，不然重载一个这个函数就可以
                //现在好了，所有的函数都要重载才能记录函数名，普通类函数，构造，析构，c的函数等等。
                //先重写这三种吧,代码都一样，用宏定义吧
#define DEF_TRAVERSE_FUNCTION(D) \
    bool Traverse##D##Decl(D##Decl* FD) \
    {\
        if(sm.isInMainFile(FD->getLocation())&&sm.isWrittenInMainFile(FD->getLocation()))\
        { \
            if(FD->hasBody()&&FD->isThisDeclarationADefinition())\
            {\
                functionstring=FD->getNameAsString();\
                functionkey=static_cast<int>(getline(FD->getLocation()));\
                st_funcall fun(static_cast<unsigned>(functionkey),functionstring,mainfilename);\
                SingletonHolder<MGRFunctionCall>::Instance()->insertfuncall(functionkey,fun);\
            }\
        }\
        RecursiveASTVisitor<Visitor>::Traverse##D##Decl(FD); \
        functionstring="";\
        functionkey=-1;\
        return true; \
    }
                FUNCTION_LIST()
                
#undef DEF_TRAVERSE_FUNCTION
#undef FUNCTION_LIST
                //if you use clang version 3.6,the second parameter must be  deleted
                bool TraverseBinAssign(BinaryOperator *S,DataRecursionQueue *Queue = nullptr)
                {
                    if(sm.isWrittenInMainFile(S->getExprLoc()))
                    {
                        WalkUpFromBinAssign(S);
                        enterassignleft=true;
                        TraverseStmt(static_cast<Stmt*>(S->getLHS()));
                        enterassignleft=false;
                        TraverseStmt(static_cast<Stmt*>(S->getRHS()));
                    }
                    return true;
                }
                bool VisitCallExpr(CallExpr* expr)
                {
                    if(sm.isWrittenInMainFile(expr->getExprLoc()))
                    {
                        if(functionkey>0)
                        {
                            st_funcall* pfun=SingletonHolder<MGRFunctionCall>::Instance()->getByKey(functionkey);
                            if(pfun)
                            {
                                pfun->addcallee(expr->getDirectCallee()->getNameAsString());
                            }
                        }
                    }   
                    return true;
                }
                bool VisitValDecl(VarDecl* vdecl)
                {
                    return true;
                }
                //修正，宏的使用
                bool VisitMemberExpr(MemberExpr* memexpr)
                {
                    SourceLocation loc=memexpr->getExprLoc();
                    while(loc.isMacroID())
                        loc=sm.getImmediateMacroCallerLoc(loc);
                    if(sm.isWrittenInMainFile(loc))
                    {
                        const FieldDecl* fielddecl=dyn_cast<FieldDecl>(memexpr->getMemberDecl());
                        const CXXMethodDecl* methoddecl=dyn_cast<CXXMethodDecl>(memexpr->getMemberDecl());
                        if(fielddecl)
                        {
                            if(!fielddecl->getType()->isBuiltinType()) //just watch builtintype
                                return true;
                            const RecordDecl* recode=fielddecl->getParent();
                            SourceLocation parentloc=recode->getLocStart();
                            unsigned int line=getline(loc);
                            string exprname=memexpr->getMemberDecl()->getNameAsString();
                            string tmppath=sm.getFilename(parentloc).data();
                            Help::PathConversion(tmppath);
                            const char* refilename=tmppath.c_str();
                            if(!Help::IsSameSvr(mainfilename,refilename)) //only output the record where define at the same  path
                                return true;
                            unsigned int reline=getline(parentloc);
                            string rcname=recode->getNameAsString();//不知道为什么如果这里传.c_str()后的const char× 会乱码。
                            //string tmpstring=refilename+rcname; //以下注释都是去掉过滤功能
                            //auto itbg=SingletonHolder<str_filter>::Instance()->filters.begin();
                            //auto itend=SingletonHolder<str_filter>::Instance()->filters.end();
                            //auto fit=find(itbg,itend,tmpstring);
                            if(1)//fit!=itend)
                            {
                                //ERROR("File:%s|Line:%d|Function:%s|MemberExpr Find,name is:%s|RecordType Name-FileName-DefineLine|%s-%s-%d|IsinAssign:%d", \
                                //        sm.getFilename(loc).data(),getline(loc),functionstring.c_str(), \
                                //        memexpr->getMemberDecl()->getNameAsString().c_str(),recode->getNameAsString().c_str(),sm.getFilename(parentloc).data(),getline(parentloc),enterassignleft?1:0);
                                RecordKey rk=RecordKey(reline,refilename); 
                                OutRecord rc=OutRecord(mainfilename.c_str(),functionstring,line,exprname,refilename,reline,rcname,enterassignleft);
                                SingletonHolder<MGRRECORD>::Instance()->addrecord(rk,rc);
                            }
                        }
                        if(methoddecl)
                        {
                            if(functionkey>0)
                            {
                                st_funcall* pfun=SingletonHolder<MGRFunctionCall>::Instance()->getByKey(functionkey);
                                if(pfun)
                                {
                                    pfun->addcallee(methoddecl->getNameAsString());
                                }
                            }
                        }
                    }
                    return true;
                }
                bool VisitCXXMemberCallExpr(CXXMemberCallExpr* cxxmemcall)
                {
                    return true;
                }
                bool VisitCXXMethodDecl(CXXMethodDecl* MD){
                    return true;
                }
                //识别除法的使用,忽略隐式转换
                bool VisitBinDiv(BinaryOperator* bo)
                {
                    if(sm.isWrittenInMainFile(bo->getExprLoc()))
                    {
                        Expr* rhs=bo->getRHS()->IgnoreImplicit();
                        if(!isa<IntegerLiteral>(rhs))
                        {
                            ERROR("find a div binaryoperator,at %u|%s",getline(bo->getExprLoc()),mainfilename.c_str());
                        }
                    }
                    return true;
                }

                //获取代码的行号
                unsigned getline(SourceLocation loc)
                {
                    std::pair<FileID,unsigned> V=sm.getDecomposedLoc(loc);
                    unsigned linenum=sm.getLineNumber(V.first,V.second);
                    return linenum; 
                }
                CompilerInstance & Instance;
                SourceManager& sm;
                const str_commamdargs &command;
                string functionstring;
                bool enterassignleft;
                int functionkey;
                string mainfilename;
            } v(Instance,Instance.getSourceManager(),command,mainfilename);
            SingletonHolder<str_filter>::Instance()->load(command.filterfile());
            //SingletonHolder<str_filter>::Instance()->DebugString();
            v.TraverseDecl(context.getTranslationUnitDecl());
            SingletonHolder<MGRRECORD>::Instance()->PrintPoolContent(mainfilename+".data");
            SingletonHolder<MGRFunctionCall>::Instance()->DebugString();
        }
    };

    class PrintFunctionNamesAction : public PluginASTAction {
        str_commamdargs command;        
        protected:
        std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                llvm::StringRef) override {
            return llvm::make_unique<PrintFunctionsConsumer>(CI, command);
        }

        bool ParseArgs(const CompilerInstance &CI,const std::vector<std::string> &args) override {
            for (unsigned i = 0, e = static_cast<unsigned>(args.size()); i != e; ++i) {
                if(i==0)
                    command.set_filterfile(args[i]);
                if(i==1)
                    command.set_datafilename(args[i]);
            }
            if (!args.empty() && args[0] == "help")
                PrintHelp(llvm::errs());

            return true;
        }
        void PrintHelp(llvm::raw_ostream& ros) {
            ros << "Fisrt arg:Filter filename,Second arg:Data Store filename\n";
        }
    };
}

static FrontendPluginRegistry::Add<PrintFunctionNamesAction>
X("data-ref", "find all user define struct or class memberexpr ");
