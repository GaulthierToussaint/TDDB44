#include "optimize.hh"

/*** This file contains all code pertaining to AST optimisation. It currently
     implements a simple optimisation called "constant folding". Most of the
     methods in this file are empty, or just relay optimize calls downward
     in the AST. If a more powerful AST optimization scheme were to be
     implemented, only methods in this file should need to be changed. ***/


ast_optimizer *optimizer = new ast_optimizer();


/* The optimizer's interface method. Starts a recursive optimize call down
   the AST nodes, searching for binary operators with constant children. */
void ast_optimizer::do_optimize(ast_stmt_list *body)
{
    if (body != NULL) {
        body->optimize();
    }
}


/* Returns 1 if an AST expression is a subclass of ast_binaryoperation,
   ie, eligible for constant folding. */
bool ast_optimizer::is_binop(ast_expression *node)
{
    switch (node->tag) {
    case AST_ADD:
    case AST_SUB:
    case AST_OR:
    case AST_AND:
    case AST_MULT:
    case AST_DIVIDE:
    case AST_IDIV:
    case AST_MOD:
        return true;
    default:
        return false;
    }
}

bool ast_optimizer::is_constant(ast_expression* left, ast_expression* right){
    if(left->tag == AST_INTEGER || left->tag == AST_REAL){
            if (right->tag == AST_INTEGER || right->tag == AST_REAL){
                return true;
            }else{
                return false;
            }
    }

    if(left->tag == AST_ID && (right->tag == AST_INTEGER || right->tag == AST_REAL)){
        sym_index left_index = left->get_ast_id()->sym_p;
        symbol* left_symbol = sym_tab->get_symbol(left_index);
        if(left_symbol->tag == SYM_CONST){
            return true;
        }
    }
    return false;
}

ast_expression* ast_optimizer::optimize_binaryop(ast_binaryoperation* node){
    ast_expression* left = fold_constants(node->left);
    ast_expression* right = fold_constants(node->right);

    if(!is_constant(left,right)){
        return node;
    }

    if (left->type == integer_type && right->type == integer_type){
        long l_value;
        if (left->tag == AST_ID){
            sym_index left_index = left->get_ast_id()->sym_p;
            constant_symbol* left_symbol = sym_tab->get_symbol(left_index)->get_constant_symbol();
            l_value = left_symbol->const_value.ival;
        }else {
            l_value = left->get_ast_integer()->value;
        }
        long r_value = right -> get_ast_integer()->value;

        switch (node->tag){
            case AST_ADD:
                right = new ast_integer(node->pos,l_value + r_value);
                break;
            case AST_OR:
                right = new ast_integer(node->pos, l_value || r_value);
                break;
            case AST_SUB:
                right = new ast_integer(node->pos,l_value - r_value);
                break;
            case AST_AND:
                right = new ast_integer(node->pos,l_value && r_value);
                break;
            case AST_MULT:
                right = new ast_integer(node->pos,l_value * r_value);
                break;
            case AST_IDIV:
                right = new ast_integer(node->pos,l_value / r_value);
                break;
            case AST_DIVIDE:
                right = new ast_real(node->pos,l_value / r_value);
                break;
            case AST_MOD:
                right = new ast_integer(node->pos,l_value % r_value);
                break;
            default:
                return node;
        }
    }

    else if (left->type == real_type && right->type == real_type){
        double l_value;
        if (left->tag == AST_ID){
            sym_index left_index = left->get_ast_id()->sym_p;
            constant_symbol* left_symbol = sym_tab->get_symbol(left_index)->get_constant_symbol();
            l_value = left_symbol->const_value.rval;
        }else {
            l_value = left->get_ast_real()->value;
        }
        double r_value = right -> get_ast_real()->value;

        switch (node->tag){
            case AST_ADD:
                right = new ast_real(node->pos,l_value + r_value);
                break;
            case AST_OR:
                right = new ast_real(node->pos, l_value || r_value);
                break;
            case AST_SUB:
                right = new ast_real(node->pos,l_value - r_value);
                break;
            case AST_AND:
                right = new ast_real(node->pos,l_value && r_value);
                break;
            case AST_MULT:
                right = new ast_real(node->pos,l_value * r_value);
                break;
            case AST_DIVIDE:
                right = new ast_real(node->pos,l_value / r_value);
                break;
            default:
                return node;
        }
    }
    return right;
}

/* We overload this method for the various ast_node subclasses that can
   appear in the AST. By use of virtual (dynamic) methods, we ensure that
   the correct method is invoked even if the pointers in the AST refer to
   one of the abstract classes such as ast_expression or ast_statement. */
void ast_node::optimize()
{
    fatal("Trying to optimize abstract class ast_node.");
}

void ast_statement::optimize()
{
    fatal("Trying to optimize abstract class ast_statement.");
}

void ast_expression::optimize()
{
    fatal("Trying to optimize abstract class ast_expression.");
}

void ast_lvalue::optimize()
{
    fatal("Trying to optimize abstract class ast_lvalue.");
}

void ast_binaryoperation::optimize()
{
    fatal("Trying to optimize abstract class ast_binaryoperation.");
}

void ast_binaryrelation::optimize()
{
    fatal("Trying to optimize abstract class ast_binaryrelation.");
}



/*** The optimize methods for the concrete AST classes. ***/

/* Optimize a statement list. */
void ast_stmt_list::optimize()
{
    if (preceding != NULL) {
        preceding->optimize();
    }
    if (last_stmt != NULL) {
        last_stmt->optimize();
    }
}


/* Optimize a list of expressions. */
void ast_expr_list::optimize()
{
    /* Your code here */
    if (preceding != NULL) {
        preceding->optimize();
    }
    if (last_expr != NULL) {
        last_expr = optimizer->fold_constants(last_expr);
    }
}


/* Optimize an elsif list. */
void ast_elsif_list::optimize()
{
    /* Your code here */
    if (preceding != NULL)
    {
        preceding->optimize();
    }
    if (last_elsif != NULL)
    {
        last_elsif->optimize();
    }
}


/* An identifier's value can change at run-time, so we can't perform
   constant folding optimization on it unless it is a constant.
   Thus we just do nothing here. It can be treated in the fold_constants()
   method, however. */
void ast_id::optimize()
{
}

void ast_indexed::optimize()
{
    /* Your code here */
    if (index != NULL){
        index -> optimize();
    }
}



/* This convenience method is used to apply constant folding to all
   binary operations. It returns either the resulting optimized node or the
   original node if no optimization could be performed. */
ast_expression *ast_optimizer::fold_constants(ast_expression *node)
{
    /* Your code here */
    node->optimize();

    if (is_binop(node)){
        node = optimize_binaryop(node->get_ast_binaryoperation());
    }

    return node;
}

/* All the binary operations should already have been detected in their parent
   nodes, so we don't need to do anything at all here. */
void ast_add::optimize()
{
    /* Your code here */
    left = optimizer->fold_constants(left);
    right = optimizer->fold_constants(right);
}

void ast_sub::optimize()
{
    /* Your code here */
    left = optimizer->fold_constants(left);
    right = optimizer->fold_constants(right);
}

void ast_mult::optimize()
{
    /* Your code here */
    left = optimizer->fold_constants(left);
    right = optimizer->fold_constants(right);
}

void ast_divide::optimize()
{
    /* Your code here */
    left = optimizer->fold_constants(left);
    right = optimizer->fold_constants(right);
}

void ast_or::optimize()
{
    /* Your code here */
    left = optimizer->fold_constants(left);
    right = optimizer->fold_constants(right);
}

void ast_and::optimize()
{
    /* Your code here */
    left = optimizer->fold_constants(left);
    right = optimizer->fold_constants(right);
}

void ast_idiv::optimize()
{
    /* Your code here */
    left = optimizer->fold_constants(left);
    right = optimizer->fold_constants(right);
}

void ast_mod::optimize()
{
    /* Your code here */
    left = optimizer->fold_constants(left);
    right = optimizer->fold_constants(right);
}



/* We can apply constant folding to binary relations as well. */
void ast_equal::optimize()
{
    /* Your code here */
    left = optimizer->fold_constants(left);
    right = optimizer->fold_constants(right);
}

void ast_notequal::optimize()
{
    /* Your code here */
    left = optimizer->fold_constants(left);
    right = optimizer->fold_constants(right);
}

void ast_lessthan::optimize()
{
    /* Your code here */
    left = optimizer->fold_constants(left);
    right = optimizer->fold_constants(right);
}

void ast_greaterthan::optimize()
{
    /* Your code here */
    left = optimizer->fold_constants(left);
    right = optimizer->fold_constants(right);
}



/*** The various classes derived from ast_statement. ***/

void ast_procedurecall::optimize()
{
    /* Your code here */
    if (parameter_list != NULL){
        parameter_list ->optimize();
    }
}


void ast_assign::optimize()
{
    /* Your code here */
    if (rhs != NULL){
        rhs = optimizer->fold_constants(rhs);
    }
}


void ast_while::optimize()
{
    /* Your code here */
    if (condition != NULL){
        condition = optimizer->fold_constants(condition);
    }

    if (body != NULL){
       body->optimize();
    }
}


void ast_if::optimize()
{
    /* Your code here */
    if (condition != NULL){
        condition = optimizer->fold_constants(condition);
    }

    if (body != NULL){
        body->optimize();
    }

    if (elsif_list != NULL){
        elsif_list->optimize();
    }

    if (else_body != NULL){
        else_body->optimize();
    }
}


void ast_return::optimize()
{
    /* Your code here */
    if (value != NULL){
        value = optimizer->fold_constants(value);
    }
}


void ast_functioncall::optimize()
{
    /* Your code here */
    if (parameter_list != NULL){
        parameter_list ->optimize();
    }
}

void ast_uminus::optimize()
{
    /* Your code here */
    if (expr != NULL){
        expr = optimizer->fold_constants(expr);
    }
}

void ast_not::optimize()
{
    /* Your code here */
    if (expr != NULL){
        expr = optimizer->fold_constants(expr);
    }
}


void ast_elsif::optimize()
{
    /* Your code here */
    if (condition != NULL){
        condition = optimizer->fold_constants(condition);
    }

    if(body != NULL){
        body -> optimize();
    }
}



void ast_integer::optimize()
{
    /* Your code here */
}

void ast_real::optimize()
{
    /* Your code here */
}

/* Note: See the comment in fold_constants() about casts and folding. */
void ast_cast::optimize()
{
    /* Your code here */
}



void ast_procedurehead::optimize()
{
    fatal("Trying to call ast_procedurehead::optimize()");
}


void ast_functionhead::optimize()
{
    fatal("Trying to call ast_functionhead::optimize()");
}
