#include <stdarg.h>

#include "compilium.h"

struct AST_LIST {
  ASTType type;
  int capacity;
  int size;
  ASTNode* nodes[];
};

const char* ASTTypeName[kNumOfASTType];

void InitASTTypeName() {
  ASTTypeName[kASTFuncDecl] = "FuncDecl";
  ASTTypeName[kASTFuncDef] = "FuncDef";
  ASTTypeName[kASTCompStmt] = "CompStmt";
  ASTTypeName[kASTExprBinOp] = "ExprBinOp";
  ASTTypeName[kASTConstant] = "Constant";
  ASTTypeName[kASTExprStmt] = "ExprStmt";
  ASTTypeName[kASTJumpStmt] = "JumpStmt";
  ASTTypeName[kASTForStmt] = "ForStmt";
  ASTTypeName[kASTILOp] = "ILOp";
  ASTTypeName[kASTList] = "List";
  ASTTypeName[kASTKeyword] = "Keyword";
  ASTTypeName[kASTDecltor] = "Decltor";
  ASTTypeName[kASTDirectDecltor] = "DirectDecltor";
  ASTTypeName[kASTIdent] = "Ident";
  ASTTypeName[kASTDecl] = "Decl";
  ASTTypeName[kASTParamDecl] = "ParamDecl";
}

const char* GetASTTypeName(ASTNode* node) {
  if (!node || kNumOfASTType <= node->type) return "?";
  return ASTTypeName[node->type];
}

ASTNode* ToASTNode(void* node) { return (ASTNode*)node; }

#define GenToAST(Type) \
  AST##Type* ToAST##Type(ASTNode* node) { \
    if (!node || node->type != kAST##Type) return NULL; \
    return (AST##Type*)node; \
  }

GenToAST(FuncDecl);
GenToAST(FuncDef);
GenToAST(CompStmt);
GenToAST(ExprBinOp);
GenToAST(Constant);
GenToAST(ExprStmt);
GenToAST(JumpStmt);
GenToAST(ForStmt);
GenToAST(ILOp);
GenToAST(List);
GenToAST(Keyword);
GenToAST(Decltor);
GenToAST(DirectDecltor);
GenToAST(Ident);
GenToAST(Decl);
GenToAST(ParamDecl);
GenToAST(Pointer);

#define GenAllocAST(Type) \
  AST##Type* AllocAST##Type() { \
    AST##Type* node = (AST##Type*)malloc(sizeof(AST##Type)); \
    node->type = kAST##Type; \
    return node; \
  }

GenAllocAST(FuncDecl);
GenAllocAST(FuncDef);
GenAllocAST(CompStmt);
GenAllocAST(ExprBinOp);
GenAllocAST(Constant);
GenAllocAST(ExprStmt);
GenAllocAST(JumpStmt);
GenAllocAST(ForStmt);
GenAllocAST(ILOp);
GenAllocAST(Keyword);
GenAllocAST(Decltor);
GenAllocAST(DirectDecltor);
GenAllocAST(Ident);
GenAllocAST(Decl);
GenAllocAST(ParamDecl);
GenAllocAST(Pointer);

ASTList* AllocASTList(int capacity) {
  ASTList* list = malloc(sizeof(ASTList) + sizeof(ASTNode*) * capacity);
  list->type = kASTList;
  list->capacity = capacity;
  list->size = 0;
  return list;
}

ASTNode* AllocAndInitASTConstant(const Token* token) {
  ASTConstant* node = AllocASTConstant();
  node->token = token;
  return ToASTNode(node);
}

ASTIdent* AllocAndInitASTIdent(const Token* token) {
  ASTIdent* node = AllocASTIdent();
  node->token = token;
  return node;
}

ASTKeyword* AllocAndInitASTKeyword(const Token* token) {
  ASTKeyword* node = AllocASTKeyword();
  node->token = token;
  return node;
}

ASTNode* AllocAndInitASTExprBinOp(const Token* op, ASTNode* left,
                                  ASTNode* right) {
  ASTExprBinOp* node = AllocASTExprBinOp();
  node->op = op;
  node->left = left;
  node->right = right;
  return ToASTNode(node);
}

ASTILOp* AllocAndInitASTILOp(ILOpType op, int dst_reg, int left_reg,
                             int right_reg, ASTNode* ast_node) {
  ASTILOp* node = AllocASTILOp();
  node->op = op;
  node->dst_reg = dst_reg;
  node->left_reg = left_reg;
  node->right_reg = right_reg;
  node->ast_node = ast_node;
  return node;
}

const char* GetIdentStrFromDirectDecltor(ASTDirectDecltor* direct_decltor) {
  if (!direct_decltor) return NULL;
  if (direct_decltor->direct_decltor)
    return GetIdentStrFromDirectDecltor(direct_decltor->direct_decltor);
  ASTIdent* ident = ToASTIdent(direct_decltor->data);
  if (!ident) return NULL;
  return ident->token->str;
}

const char* GetIdentStrFromDecltor(ASTDecltor* decltor) {
  if (!decltor) return NULL;
  return GetIdentStrFromDirectDecltor(decltor->direct_decltor);
}

const char* GetFuncNameStrFromFuncDef(ASTFuncDef* func_def) {
  if (!func_def) return NULL;
  return GetIdentStrFromDecltor(func_def->decltor);
}

void PrintASTNodePadding(int depth) {
  putchar('\n');
  for (int i = 0; i < depth; i++) putchar(' ');
}

void PrintfWithPadding(int depth, const char* fmt, ...) {
  PrintASTNodePadding(depth);
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stdout, fmt, ap);
  va_end(ap);
}

void PrintASTNodeWithName(int depth, const char* name, ASTNode* node) {
  PrintfWithPadding(depth, name);
  PrintASTNode(node, depth);
}
void PrintTokenWithName(int depth, const char* name, const Token* token) {
  PrintfWithPadding(depth + 1, name);
  PrintToken(token);
}
void PrintTokenListWithName(int depth, const char* name, TokenList* list) {
  PrintfWithPadding(depth + 1, name);
  PrintTokenList(list);
}
void PrintASTNode(ASTNode* node, int depth) {
  if (!node) {
    printf("(Null)");
    return;
  }
  if (node->type == kASTList) {
    ASTList* list = ToASTList(node);
    putchar('[');
    for (int i = 0; i < list->size; i++) {
      PrintASTNodePadding(depth + 1);
      PrintASTNode(list->nodes[i], depth + 1);
    }
    PrintASTNodePadding(depth);
    putchar(']');
    return;
  } else if (node->type < kNumOfASTType) {
    printf("(%s:", ASTTypeName[node->type]);
  } else {
    printf("(Unknown: %d)", node->type);
    return;
  }
  if (node->type == kASTFuncDecl) {
    ASTFuncDecl* func_decl = ToASTFuncDecl(node);
    PrintASTNodeWithName(depth + 1, "type_and_name=", func_decl->type_and_name);
    PrintASTNodeWithName(depth + 1,
                         "arg_list=", ToASTNode(func_decl->arg_list));
  } else if (node->type == kASTFuncDef) {
    ASTFuncDef* func_def = ToASTFuncDef(node);
    PrintASTNodeWithName(depth + 1,
                         "decl_specs=", ToASTNode(func_def->decl_specs));
    PrintASTNodeWithName(depth + 1, "decltor=", ToASTNode(func_def->decltor));
    PrintASTNodeWithName(depth + 1,
                         "comp_stmt=", ToASTNode(func_def->comp_stmt));
  } else if (node->type == kASTCompStmt) {
    ASTCompStmt* comp_stmt = ToASTCompStmt(node);
    PrintASTNodeWithName(depth + 1, "body=", ToASTNode(comp_stmt->stmt_list));
  } else if (node->type == kASTExprBinOp) {
    ASTExprBinOp* expr_bin_op = ToASTExprBinOp(node);
    PrintTokenWithName(depth + 1, "op=", expr_bin_op->op);
    PrintASTNodeWithName(depth + 1, "left=", expr_bin_op->left);
    PrintASTNodeWithName(depth + 1, "right=", expr_bin_op->right);
  } else if (node->type == kASTConstant) {
    ASTConstant* constant = ToASTConstant(node);
    PrintTokenWithName(depth + 1, "token=", constant->token);
  } else if (node->type == kASTExprStmt) {
    ASTExprStmt* expr_stmt = ToASTExprStmt(node);
    PrintASTNodeWithName(depth + 1, "expression=", expr_stmt->expr);
  } else if (node->type == kASTJumpStmt) {
    ASTJumpStmt* jump_stmt = ToASTJumpStmt(node);
    PrintASTNodeWithName(depth + 1, "kw=", ToASTNode(jump_stmt->kw));
    PrintASTNodeWithName(depth + 1, "param=", jump_stmt->param);
  } else if (node->type == kASTForStmt) {
    ASTForStmt* for_stmt = ToASTForStmt(node);
    PrintASTNodeWithName(depth + 1, "init_expr=", for_stmt->init_expr);
    PrintASTNodeWithName(depth + 1, "cond_expr=", for_stmt->cond_expr);
    PrintASTNodeWithName(depth + 1, "updt_expr=", for_stmt->updt_expr);
    PrintASTNodeWithName(depth + 1,
                         "body_comp_stmt=", for_stmt->body_comp_stmt);
  } else if (node->type == kASTILOp) {
    ASTILOp* il_op = ToASTILOp(node);
    PrintfWithPadding(depth + 1, "op=%s", GetILOpTypeName(il_op->op));
    PrintfWithPadding(depth + 1, "dst=%d", il_op->dst_reg);
    PrintfWithPadding(depth + 1, "left=%d", il_op->left_reg);
    PrintfWithPadding(depth + 1, "right=%d", il_op->right_reg);
    // PrintASTNodeWithName(depth + 1, "ast_node=", il_op->ast_node);
  } else if (node->type == kASTKeyword) {
    ASTKeyword* kw = ToASTKeyword(node);
    PrintTokenWithName(depth + 1, "token=", kw->token);
  } else if (node->type == kASTDecltor) {
    ASTDecltor* decltor = ToASTDecltor(node);
    PrintASTNodeWithName(depth + 1,
                         "direct_decltor=", ToASTNode(decltor->direct_decltor));
  } else if (node->type == kASTDirectDecltor) {
    ASTDirectDecltor* direct_decltor = ToASTDirectDecltor(node);
    PrintASTNodeWithName(depth + 1, "direct_decltor=",
                         ToASTNode(direct_decltor->direct_decltor));
    PrintASTNodeWithName(depth + 1, "data=", direct_decltor->data);
  } else if (node->type == kASTIdent) {
    ASTIdent* ident = ToASTIdent(node);
    PrintTokenWithName(depth + 1, "token=", ident->token);
  } else if (node->type == kASTDecl) {
    ASTDecl* decl = ToASTDecl(node);
    PrintASTNodeWithName(depth + 1, "decl_specs=", ToASTNode(decl->decl_specs));
    PrintASTNodeWithName(depth + 1,
                         "init_decltors=", ToASTNode(decl->init_decltors));
  } else if (node->type == kASTParamDecl) {
    ASTParamDecl* param_decl = ToASTParamDecl(node);
    PrintASTNodeWithName(depth + 1,
                         "decl_specs=", ToASTNode(param_decl->decl_specs));
    PrintASTNodeWithName(depth + 1, "decltor=", ToASTNode(param_decl->decltor));
  } else {
    Error("PrintASTNode not implemented for type %d (%s)", node->type,
          GetASTTypeName(node));
  }
  PrintfWithPadding(depth, ")");
}

void PushASTNodeToList(ASTList* list, ASTNode* node) {
  if (list->size >= list->capacity) {
    Error("No more space in ASTList");
  }
  list->nodes[list->size++] = node;
}

ASTNode* PopASTNodeFromList(ASTList* list) {
  if (list->size <= 0) {
    Error("Trying to pop empty ASTList");
  }
  return list->nodes[--list->size];
}

ASTNode* GetASTNodeAt(const ASTList* list, int index) {
  if (index < 0 || list->size <= index) {
    Error("ASTList: Trying to read index out of bound");
  }
  return list->nodes[index];
}

int GetSizeOfASTList(const ASTList* list) { return list->size; }

ASTNode* GetLastASTNode(const ASTList* list) {
  return GetASTNodeAt(list, GetSizeOfASTList(list) - 1);
}
