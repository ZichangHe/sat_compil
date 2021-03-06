/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set expandtab tabstop=2 softtabstop=2 shiftwidth=2: */

#ifndef _SIS_SPARSE_H_
#define _SIS_SPARSE_H_

#include <stdio.h>

namespace SYN_SIS
{

/* hack to fix conflict with libX11.a */
#define sm_alloc sm_allocate
#define sm_free sm_free_space

/*
 *  sparse.h -- sparse matrix package header file
 */

typedef struct sm_element_struct sm_element;
typedef struct sm_row_struct sm_row;
typedef struct sm_col_struct sm_col;
typedef struct sm_matrix_struct sm_matrix;


/*
 *  sparse matrix element
 */
struct sm_element_struct {
    int row_num;		/* row number of this element */
    int col_num;		/* column number of this element */
    sm_element *next_row;	/* next row in this column */
    sm_element *prev_row;	/* previous row in this column */
    sm_element *next_col;	/* next column in this row */
    sm_element *prev_col;	/* previous column in this row */
    char *user_word;		/* user-defined word */
};


/*
 *  row header
 */
struct sm_row_struct {
    int row_num;		/* the row number */
    int length;			/* number of elements in this row */
    int flag;			/* user-defined word */
    sm_element *first_col;	/* first element in this row */
    sm_element *last_col;	/* last element in this row */
    sm_row *next_row;		/* next row (in sm_matrix linked list) */
    sm_row *prev_row;		/* previous row (in sm_matrix linked list) */
    char *user_word;		/* user-defined word */
};


/*
 *  column header
 */
struct sm_col_struct {
    int col_num;		/* the column number */
    int length;			/* number of elements in this column */
    int flag;			/* user-defined word */
    sm_element *first_row;	/* first element in this column */
    sm_element *last_row;	/* last element in this column */
    sm_col *next_col;		/* next column (in sm_matrix linked list) */
    sm_col *prev_col;		/* prev column (in sm_matrix linked list) */
    char *user_word;		/* user-defined word */
};


/*
 *  A sparse matrix
 */
struct sm_matrix_struct {
    sm_row **rows;		/* pointer to row headers (by row #) */
    int rows_size;		/* alloc'ed size of above array */
    sm_col **cols;		/* pointer to column headers (by col #) */
    int cols_size;		/* alloc'ed size of above array */
    sm_row *first_row;		/* first row (linked list of all rows) */
    sm_row *last_row;		/* last row (linked list of all rows) */
    int nrows;			/* number of rows */
    sm_col *first_col;		/* first column (linked list of columns) */
    sm_col *last_col;		/* last column (linked list of columns) */
    int ncols;			/* number of columns */
    char *user_word;		/* user-defined word */
};


#define sm_get_col(A, colnum)	\
    (((colnum) >= 0 && (colnum) < (A)->cols_size) ? \
	(A)->cols[colnum] : (sm_col *) 0)

#define sm_get_row(A, rownum)	\
    (((rownum) >= 0 && (rownum) < (A)->rows_size) ? \
	(A)->rows[rownum] : (sm_row *) 0)

#define sm_foreach_row(A, prow)	\
	for(prow = A->first_row; prow != 0; prow = prow->next_row)

#define sm_foreach_col(A, pcol)	\
	for(pcol = A->first_col; pcol != 0; pcol = pcol->next_col)

#define sm_foreach_row_element(prow, p)	\
	for(p = (prow == 0) ? 0 : prow->first_col; p != 0; p = p->next_col)

#define sm_foreach_col_element(pcol, p) \
	for(p = (pcol == 0) ? 0 : pcol->first_row; p != 0; p = p->next_row)

#define sm_put(x, val) \
	(x->user_word = (char *) val)

#define sm_get(type, x) \
	((type) (x->user_word))

sm_matrix* sm_allocate();
void sm_free_space( sm_matrix* );
sm_element* sm_insert( sm_matrix* A, int row, int col );
void sm_remove( sm_matrix* A, int row, int col );
void sm_delrow( sm_matrix* A, int row );
void sm_delcol( sm_matrix* A, int col );
int sm_num_elements( sm_matrix* A );
void sm_copy_row( register sm_matrix *dest,
							 int dest_row,
							 sm_row *prow );
sm_row *sm_row_and( sm_row* p1,
							   sm_row* p2 );
void sm_row_free( register sm_row* prow );

sm_element *
sm_col_insert(register sm_col *pcol, register int row);

void 
sm_resize(register sm_matrix *A, int row, int col);

sm_element *sm_find(sm_matrix *A, int rownum, int colnum);
sm_matrix *sm_alloc_size(int row, int col);
sm_matrix *sm_alloc();
sm_col *sm_col_dup(register sm_col *pcol);
sm_row *sm_row_dup(register sm_row *prow);
sm_element *sm_row_insert(register sm_row *prow, register int col);
void sm_col_free(register sm_col *pcol);
sm_col *sm_col_alloc();
sm_row *sm_row_alloc();
sm_element *sm_row_find(sm_row *prow, int col);
sm_element *sm_col_find(sm_col *pcol, int row);
sm_col *sm_col_and(sm_col *p1, sm_col *p2);
sm_row *sm_longest_row(sm_matrix *A);
sm_col *sm_longest_col(sm_matrix *A);
void sm_remove_element(register sm_matrix *A, register sm_element *p);
void sm_col_free(register sm_col *pcol);
sm_row *sm_row_alloc();
sm_col *sm_col_alloc();
void sm_col_remove_element(register sm_col *pcol, register sm_element *p);
void sm_row_remove_element(register sm_row *prow, register sm_element *p);
sm_element *sm_row_insert(register sm_row *prow, register int col);
int sm_row_compare(sm_row *p1, sm_row *p2);
int sm_row_hash(sm_row *prow, int modulus);
void sm_row_remove(register sm_row *prow, register int col);
void sm_print(FILE *fp, sm_matrix *A);
sm_matrix *sm_dup(sm_matrix *A);
int sm_row_intersects(sm_row *p1, sm_row *p2);

}

#endif //_SIS_SPARSE_H_


