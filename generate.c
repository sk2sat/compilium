#include "compilium.h"

// https://wiki.osdev.org/System_V_ABI
// registers should be preserved: rbx, rsp, rbp, r12, r13, r14, and r15
// scratch registers: rax, rdi, rsi, rdx, rcx, r8, r9, r10, r11
// return value: rax

#define NUM_OF_SCRATCH_REGS 9

#define REAL_REG_RAX 1
#define REAL_REG_RDI 2
#define REAL_REG_RSI 3
#define REAL_REG_RDX 4
#define REAL_REG_RCX 5
#define REAL_REG_R8 6
#define REAL_REG_R9 7
#define REAL_REG_R10 8
#define REAL_REG_R11 9

const char *ScratchRegNames[NUM_OF_SCRATCH_REGS + 1] = {
    "NULL", "rax", "rdi", "rsi", "rdx", "rcx", "r8", "r9", "r10", "r11"};

int GetLabelNumber() {
  static int num = 1;
  return num++;
}

typedef struct {
  int save_label_num;
  int real_reg;
} RegAssignInfo;

#define NUM_OF_ASSIGN_INFOS 128
RegAssignInfo reg_assign_infos[NUM_OF_ASSIGN_INFOS];

int RealRegAssignTable[NUM_OF_SCRATCH_REGS + 1];
int RealRegRefOrder[NUM_OF_SCRATCH_REGS + 1];
int order_count = 1;

void GenerateSpillData(FILE *fp) {
  fprintf(fp, ".data\n");
  for (int i = 0; i < NUM_OF_ASSIGN_INFOS; i++) {
    if (reg_assign_infos[i].save_label_num) {
      fprintf(fp, "L%d: .quad 0\n", reg_assign_infos[i].save_label_num);
    }
  }
}

void PrintRegisterAssignment() {
  puts("==== ASSIGNMENT ====");
  for (int i = 1; i < NUM_OF_SCRATCH_REGS + 1; i++) {
    if (RealRegAssignTable[i]) {
      printf("\treg[%d] => %s (%d)\n", RealRegAssignTable[i],
             ScratchRegNames[i], RealRegRefOrder[i]);
    } else {
      printf("\tnone => %s (%d)\n", ScratchRegNames[i], RealRegRefOrder[i]);
    }
  }
  puts("==== END OF ASSIGNMENT ====");
}

int SelectVirtualRegisterToSpill() {
  for (int i = 1; i < NUM_OF_SCRATCH_REGS + 1; i++) {
    if (RealRegAssignTable[i]) {
      printf("\treg[%d] => %s (%d)\n", RealRegAssignTable[i],
             ScratchRegNames[i], RealRegRefOrder[i]);
    }
    if (RealRegAssignTable[i] &&
        RealRegRefOrder[i] <= order_count - NUM_OF_SCRATCH_REGS)
      return RealRegAssignTable[i];
  }
  Error("SelectVirtualRegisterToSpill: NOT_REACHED");
  return 0;
}

void SpillVirtualRegister(FILE *fp, int virtual_reg) {
  RegAssignInfo *info = &reg_assign_infos[virtual_reg];
  if (!info->save_label_num) info->save_label_num = GetLabelNumber();
  //
  fprintf(fp, "mov [rip + L%d], %s\n", info->save_label_num,
          ScratchRegNames[info->real_reg]);
  //
  RealRegAssignTable[info->real_reg] = 0;
  info->real_reg = 0;
  printf("\tvirtual_reg[%d] is spilled to label %d\n", virtual_reg,
         info->save_label_num);
}

void SpillRealRegister(FILE *fp, int rreg) {
  if (RealRegAssignTable[rreg]) {
    SpillVirtualRegister(fp, RealRegAssignTable[rreg]);
  }
}

int FindFreeRealReg(FILE *fp) {
  for (int i = 1; i < NUM_OF_SCRATCH_REGS + 1; i++) {
    if (!RealRegAssignTable[i]) return i;
  }
  PrintRegisterAssignment();
  SpillVirtualRegister(fp, SelectVirtualRegisterToSpill());
  PrintRegisterAssignment();
  for (int i = 1; i < NUM_OF_SCRATCH_REGS + 1; i++) {
    if (!RealRegAssignTable[i]) return i;
  }
  Error("FindFreeRealReg: NOT_REACHED");
  return 0;
}

void AssignVirtualRegToRealReg(FILE *fp, int virtual_reg, int real_reg) {
  RegAssignInfo *info = &reg_assign_infos[virtual_reg];
  if (info->real_reg == real_reg) {
    // already satisfied.
    RealRegRefOrder[real_reg] = order_count++;
    return;
  }
  // first, free target real reg.
  SpillRealRegister(fp, real_reg);
  // next, assign virtual reg to real reg.
  if (info->real_reg) {
    printf("\tvirtual_reg[%d] is stored on %s, moving...\n", virtual_reg,
           ScratchRegNames[info->real_reg]);
    fprintf(fp, "mov %s, %s\n", ScratchRegNames[real_reg],
            ScratchRegNames[info->real_reg]);
    RealRegAssignTable[info->real_reg] = 0;
  } else if (info->save_label_num) {
    printf("\tvirtual_reg[%d] is stored at label %d, restoring...\n",
           virtual_reg, info->save_label_num);
    fprintf(fp, "mov %s, [rip + L%d]\n", ScratchRegNames[real_reg],
            info->save_label_num);
  }
  RealRegAssignTable[real_reg] = virtual_reg;
  RealRegRefOrder[real_reg] = order_count++;
  info->real_reg = real_reg;
  if (info->save_label_num) {
  }
  printf("\tvirtual_reg[%d] => %s\n", virtual_reg, ScratchRegNames[real_reg]);
}

const char *AssignRegister(FILE *fp, int reg_id) {
  printf("requested reg_id = %d\n", reg_id);
  if (reg_id < 1 || NUM_OF_ASSIGN_INFOS <= reg_id) {
    Error("reg_id out of range (%d)", reg_id);
  }
  RegAssignInfo *info = &reg_assign_infos[reg_id];
  if (info->real_reg) {
    printf("\texisted on %s\n", ScratchRegNames[info->real_reg]);
    RealRegRefOrder[info->real_reg] = order_count++;
    return ScratchRegNames[info->real_reg];
  }
  int real_reg = FindFreeRealReg(fp);
  AssignVirtualRegToRealReg(fp, reg_id, real_reg);
  return ScratchRegNames[real_reg];
}

const char *GetParamRegister(int param_index) {
  // param-index: 1-based
  if (param_index < 1 || NUM_OF_SCRATCH_REGS <= param_index) {
    Error("param_index exceeded (%d)", param_index);
  }
  return ScratchRegNames[param_index];
}

void GenerateCode(FILE *fp, ASTList *il) {
  fputs(".intel_syntax noprefix\n", fp);
  // generate func symbol
  for (int i = 0; i < GetSizeOfASTList(il); i++) {
    ASTNode *node = GetASTNodeAt(il, i);
    ASTILOp *op = ToASTILOp(node);
    if (op->op == kILOpFuncBegin) {
      const char *func_name =
          GetFuncNameStrFromFuncDef(ToASTFuncDef(op->ast_node));
      if (!func_name) {
        Error("func_name is null");
      }
      fprintf(fp, ".global %s%s\n", kernel_type == kKernelDarwin ? "_" : "",
              func_name);
    }
  }
  // generate code
  for (int i = 0; i < GetSizeOfASTList(il); i++) {
    ASTNode *node = GetASTNodeAt(il, i);
    ASTILOp *op = ToASTILOp(node);
    if (!op) {
      Error("op is null!");
    }
    switch (op->op) {
      case kILOpFuncBegin: {
        const char *func_name =
            GetFuncNameStrFromFuncDef(ToASTFuncDef(op->ast_node));
        if (!func_name) {
          Error("func_name is null");
        }
        fprintf(fp, "%s%s:\n", kernel_type == kKernelDarwin ? "_" : "",
                func_name);
        fprintf(fp, "push    rbp\n");
        fprintf(fp, "mov     rbp, rsp\n");
      } break;
      case kILOpFuncEnd:
        fprintf(fp, "mov     dword ptr [rbp - 4], 0\n");
        fprintf(fp, "pop     rbp\n");
        fprintf(fp, "ret\n");
        break;
      case kILOpLoadImm: {
        const char *dst_name = AssignRegister(fp, op->dst_reg);
        //
        ASTConstant *val = ToASTConstant(op->ast_node);
        switch (val->token->type) {
          case kInteger: {
            char *p;
            const char *s = val->token->str;
            int n = strtol(s, &p, 0);
            if (!(s[0] != 0 && *p == 0)) {
              Error("%s is not valid as integer.", s);
            }
            fprintf(fp, "mov %s, %d\n", dst_name, n);

          } break;
          case kStringLiteral: {
            int label_for_skip = GetLabelNumber();
            int label_str = GetLabelNumber();
            fprintf(fp, "jmp L%d\n", label_for_skip);
            fprintf(fp, "L%d:\n", label_str);
            fprintf(fp, ".asciz  \"%s\"\n", val->token->str);
            fprintf(fp, "L%d:\n", label_for_skip);
            fprintf(fp, "lea     %s, [rip + L%d]\n", dst_name, label_str);
          } break;
          default:
            Error("kILOpLoadImm: not implemented for token type %d",
                  val->token->type);
        }
      } break;
      case kILOpLoadIdent: {
        const char *dst_name = AssignRegister(fp, op->dst_reg);
        //
        ASTIdent *ident = ToASTIdent(op->ast_node);
        switch (ident->token->type) {
          case kIdentifier: {
            fprintf(fp, "lea     %s, [rip + %s%s]\n", dst_name,
                    kernel_type == kKernelDarwin ? "_" : "", ident->token->str);
          } break;
          default:
            Error("kILOpLoadIdent: not implemented for token type %d",
                  ident->token->type);
        }
      } break;
      case kILOpAdd:
      case kILOpSub:
      case kILOpMul: {
        const char *dst = AssignRegister(fp, op->dst_reg);
        const char *left = AssignRegister(fp, op->left_reg);
        const char *right = AssignRegister(fp, op->right_reg);
        //
        if (op->op == kILOpAdd) {
          fprintf(fp, "add %s, %s\n", left, right);
          fprintf(fp, "mov %s, %s\n", dst, left);
        } else if (op->op == kILOpSub) {
          fprintf(fp, "sub %s, %s\n", left, right);
          fprintf(fp, "mov %s, %s\n", dst, left);
        } else if (op->op == kILOpMul) {
          // eDX: eAX <- eAX * r/m
          fprintf(fp, "push rax\n");
          fprintf(fp, "mov rax, %s\n", left);
          fprintf(fp, "push rdx\n");
          fprintf(fp, "imul %s\n", right);
          fprintf(fp, "pop rdx\n");
          fprintf(fp, "mov %s, rax\n", dst);
          fprintf(fp, "pop rax\n");
        }
      } break;
      case kILOpReturn: {
        AssignVirtualRegToRealReg(fp, op->left_reg, REAL_REG_RAX);
      } break;
      case kILOpCall: {
        ASTList *call_params = ToASTList(op->ast_node);
        if (!call_params) Error("call_params is not an ASTList");
        for (int i = 1; i < GetSizeOfASTList(call_params); i++) {
          AssignVirtualRegToRealReg(
              fp, ToASTILOp(GetASTNodeAt(call_params, i))->dst_reg, i + 1);
        }
        ASTIdent *func_ident = ToASTIdent(GetASTNodeAt(call_params, 0));
        if (!func_ident) Error("call_params[0] is not an ASTIdent");
        fprintf(fp, ".global %s%s\n", kernel_type == kKernelDarwin ? "_" : "",
                func_ident->token->str);
        fprintf(fp, "call %s%s\n", kernel_type == kKernelDarwin ? "_" : "",
                func_ident->token->str);
      } break;
      default:
        Error("Not implemented code generation for ILOp%s",
              GetILOpTypeName(op->op));
    }
  }
  GenerateSpillData(fp);
}

#define MAX_IL_NODES 2048
void Generate(FILE *fp, ASTNode *root) {
  ASTList *intermediate_code = AllocASTList(MAX_IL_NODES);

  GenerateIL(intermediate_code, root);
  PrintASTNode(ToASTNode(intermediate_code), 0);
  putchar('\n');

  GenerateCode(fp, intermediate_code);
}
