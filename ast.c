#include "compilium.h"

const ASTDataRoot *GetDataAsRoot(const ASTNode *node) {
  if (!node || node->type != kRoot) return NULL;
  return &node->data.root;
}

const ASTDataVarDef *GetDataAsVarDef(const ASTNode *node) {
  if (!node || node->type != kVarDef) return NULL;
  return &node->data.var_def;
}

const ASTDataFuncDecl *GetDataAsFuncDecl(const ASTNode *node) {
  if (!node || node->type != kFuncDecl) return NULL;
  return &node->data.func_decl;
}

const ASTDataFuncDef *GetDataAsFuncDef(const ASTNode *node) {
  if (!node || node->type != kFuncDef) return NULL;
  return &node->data.func_def;
}

const ASTDataCompStmt *GetDataAsCompStmt(const ASTNode *node) {
  if (!node || node->type != kCompStmt) return NULL;
  return &node->data.comp_stmt;
}

const ASTDataExprBinOp *GetDataAsExprBinOp(const ASTNode *node) {
  if (!node || node->type != kExprBinOp) return NULL;
  return &node->data.expr_bin_op;
}

const ASTDataExprVal *GetDataAsExprVal(const ASTNode *node) {
  if (!node || node->type != kExprVal) return NULL;
  return &node->data.expr_val;
}

const ASTDataExprStmt *GetDataAsExprStmt(const ASTNode *node) {
  if (!node || node->type != kExprStmt) return NULL;
  return &node->data.expr_stmt;
}

const ASTDataReturnStmt *GetDataAsReturnStmt(const ASTNode *node) {
  if (!node || node->type != kReturnStmt) return NULL;
  return &node->data.return_stmt;
}

const ASTDataILOp *GetDataAsILOpOfType(const ASTNode *node, ILOpType type) {
  if (!node || node->type != kILOp || node->data.il_op.op != type) return NULL;
  return &node->data.il_op;
}

ASTNode *AllocateASTNode(ASTType type) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = type;
  return node;
}

ASTNode *AllocateASTNodeAsExprVal(const Token *token) {
  ASTNode *node = AllocateASTNode(kExprVal);
  node->data.expr_val.token = token;
  return node;
}

ASTNode *AllocateASTNodeAsExprBinOp(ASTExprBinOpType op_type) {
  ASTNode *node = AllocateASTNode(kExprBinOp);
  node->data.expr_bin_op.op_type = op_type;
  node->data.expr_bin_op.left = NULL;
  node->data.expr_bin_op.right = NULL;
  return node;
}
void SetOperandOfExprBinOp(ASTNode *node, ASTNode *left, ASTNode *right) {
  node->data.expr_bin_op.left = left;
  node->data.expr_bin_op.right = right;
}

ASTNode *AllocateASTNodeAsILOp(ILOpType op, int dst_reg, int left_reg,
                               int right_reg, const ASTNode *ast_node) {
  ASTNode *node = AllocateASTNode(kILOp);
  node->data.il_op.op = op;
  node->data.il_op.dst_reg = dst_reg;
  node->data.il_op.left_reg = left_reg;
  node->data.il_op.right_reg = right_reg;
  node->data.il_op.ast_node = ast_node;
  return node;
}

void PrintASTNodePadding(int depth) {
  putchar('\n');
  for (int i = 0; i < depth; i++) putchar(' ');
}

const char *ILOpTypeStr[kNumOfILOpFunc];

const char *InternalGetILOpTypeStr(ILOpType type) {
  if(!ILOpTypeStr[0]){
    ILOpTypeStr[kILOpAdd] = "Add";
    ILOpTypeStr[kILOpSub] = "Sub";
    ILOpTypeStr[kILOpMul] = "Mul";
    ILOpTypeStr[kILOpLoadImm] = "LoadImm";
    ILOpTypeStr[kILOpFuncBegin] = "FuncBegin";
    ILOpTypeStr[kILOpFuncEnd] = "FuncEnd";
    ILOpTypeStr[kILOpReturn] = "Return";
  }
  if (kNumOfILOpFunc <= type) return "?";
  return ILOpTypeStr[type];
}

void PrintASTNodeList(ASTNodeList *list, int depth);
void PrintASTNode(const ASTNode *node, int depth) {
  if (!node) {
    printf("(Null)");
    return;
  }
  if (node->type == kRoot) {
    printf("(Root:");
    PrintASTNodePadding(depth);
    printf("root_list=");
    PrintASTNodeList(node->data.root.root_list, depth + 1);
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kVarDef) {
    printf("(VarDef:");
    PrintASTNodePadding(depth);
    printf("type=");
    PrintTokenList(node->data.var_def.type_tokens);
    PrintASTNodePadding(depth);
    printf("name=%s", node->data.var_def.name->str);
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kFuncDecl) {
    printf("(FuncDecl:");
    PrintASTNodePadding(depth);
    printf("type_and_name=");
    PrintASTNode(node->data.func_decl.type_and_name, depth + 1);
    PrintASTNodePadding(depth);
    printf("arg_list=");
    PrintASTNodeList(node->data.func_decl.arg_list, depth + 1);
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kFuncDef) {
    printf("(FuncDef:");
    //
    PrintASTNodePadding(depth);
    printf("func_decl=");
    PrintASTNode(node->data.func_def.func_decl, depth + 1);
    //
    PrintASTNodePadding(depth);
    printf("comp_stmt=");
    PrintASTNode(node->data.func_def.comp_stmt, depth + 1);
    //
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kCompStmt) {
    printf("(CompStatement:");
    //
    PrintASTNodePadding(depth);
    printf("(body=");
    PrintASTNodeList(node->data.comp_stmt.stmt_list, depth + 1);
    //
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kExprBinOp) {
    printf("(ExprBinOp:");
    //
    PrintASTNodePadding(depth);
    printf("op_type=%d", GetDataAsExprBinOp(node)->op_type);
    //
    PrintASTNodePadding(depth);
    printf("left=");
    PrintASTNode(GetDataAsExprBinOp(node)->left, depth + 1);
    //
    PrintASTNodePadding(depth);
    printf("right=");
    PrintASTNode(GetDataAsExprBinOp(node)->right, depth + 1);
    //
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kExprVal) {
    printf("(ExprVal:");
    PrintASTNodePadding(depth);
    printf("token=");
    PrintToken(GetDataAsExprVal(node)->token);
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kExprStmt) {
    printf("(ExprStmt:");
    PrintASTNodePadding(depth);
    printf("expression=");
    PrintASTNode(node->data.expr_stmt.expr, depth + 1);
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kReturnStmt) {
    printf("(ReturnStmt:");
    PrintASTNodePadding(depth);
    printf("expr_stmt=");
    PrintASTNode(node->data.return_stmt.expr_stmt, depth + 1);
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kForStatement) {
    printf("(ForStatement:");
    PrintASTNodePadding(depth);
    printf("init_expr=");
    PrintASTNode(node->data.for_stmt.init_expr, depth + 1);
    PrintASTNodePadding(depth);
    printf("cond_expr=");
    PrintASTNode(node->data.for_stmt.cond_expr, depth + 1);
    PrintASTNodePadding(depth);
    printf("updt_expr=");
    PrintASTNode(node->data.for_stmt.updt_expr, depth + 1);
    PrintASTNodePadding(depth);
    printf("body_comp_stmt=");
    PrintASTNode(node->data.for_stmt.body_comp_stmt, depth + 1);
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kILOp) {
    printf("(ILOp:");
    printf(" op=%s", InternalGetILOpTypeStr(node->data.il_op.op));
    printf(" dst=%d", node->data.il_op.dst_reg);
    printf(" left=%d", node->data.il_op.left_reg);
    printf(" right=%d", node->data.il_op.right_reg);
    printf(")");

  } else {
    Error("PrintASTNode not implemented for type %d", node->type);
  }
}

struct AST_NODE_LIST {
  int capacity;
  int size;
  ASTNode *nodes[];
};

ASTNodeList *AllocateASTNodeList(int capacity) {
  ASTNodeList *list =
      malloc(sizeof(ASTNodeList) + sizeof(ASTNode *) * capacity);
  list->capacity = capacity;
  list->size = 0;
  return list;
}

void PushASTNodeToList(ASTNodeList *list, ASTNode *node) {
  if (list->size >= list->capacity) {
    Error("No more space in ASTNodeList");
  }
  list->nodes[list->size++] = node;
}

ASTNode *PopASTNodeFromList(ASTNodeList *list) {
  if (list->size <= 0) {
    Error("Trying to pop empty ASTNodeList");
  }
  return list->nodes[--list->size];
}

ASTNode *GetASTNodeAt(const ASTNodeList *list, int index) {
  if (index < 0 || list->size <= index) {
    Error("ASTNodeList: Trying to read index out of bound");
  }
  return list->nodes[index];
}

int GetSizeOfASTNodeList(const ASTNodeList *list) { return list->size; }

ASTNode *GetLastASTNode(const ASTNodeList *list) {
  return GetASTNodeAt(list, GetSizeOfASTNodeList(list) - 1);
}

void PrintASTNodeList(ASTNodeList *list, int depth) {
  putchar('[');
  PrintASTNodePadding(depth);
  for (int i = 0; i < list->size; i++) {
    PrintASTNode(list->nodes[i], depth + 1);
    PrintASTNodePadding(depth);
  }
  putchar(']');
}
