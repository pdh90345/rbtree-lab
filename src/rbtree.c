#include "rbtree.h"

#include <stdlib.h>

void delete_nodes(rbtree *t, node_t *node);
void rbtree_insert_fixup(rbtree *t, node_t *new_node);
void left_rotate(rbtree *t, node_t *x);
void right_rotate(rbtree *t, node_t *x);
void rb_delete_fixup(rbtree *t, node_t *x);

rbtree *new_rbtree(void)
{
  rbtree *p = (rbtree *)calloc(1, sizeof(rbtree));
  if (!p)
  {
    return NULL;
  }
  node_t *nil_node = (node_t *)calloc(1, sizeof(node_t));
  if (!nil_node)
  {
    free(p);
    return NULL;
  }

  // 자기 참조를 통해 리프 노드의 역할을 하고 메모리 관리의 단순화, 안정성, 효율성 향상
  nil_node->left = nil_node; // 모든 리프노드가 nil노드를 가리키게
  nil_node->right = nil_node;
  nil_node->parent = nil_node;
  nil_node->color = RBTREE_BLACK;
  // 구조체는 빈 껍데기 -> key는 나중에

  p->nil = nil_node;
  p->root = nil_node; // 초기 상태에서는 root는 nil노드 가리킴

  return p;
}

void delete_rbtree(rbtree *t)
{
  if (t->root != t->nil)
  {
    delete_nodes(t, t->root);
  }

  free(t->nil); // nli노드 해제
  free(t);      // 트리 구조체 해제
}

void delete_nodes(rbtree *t, node_t *node) // 후위 순회 방식
{
  if (node != t->nil)
  {
    delete_nodes(t, node->left);
    delete_nodes(t, node->right);
    free(node);
  }
}

node_t *rbtree_insert(rbtree *t, const key_t key)
{
  node_t *new_node = (node_t *)calloc(1, sizeof(node_t));
  if (!new_node)
  {
    return NULL;
  }

  new_node->left = t->nil;
  new_node->right = t->nil;
  new_node->key = key;
  new_node->color = RBTREE_RED;
  new_node->parent = t->nil;

  node_t *current = t->root;
  // current는 t->root의 값을 복사하여 로컬로 저장한 복사본이므로
  // t->root가 실제로 새 노드를 가리키게 해야한다.
  if (current == t->nil)
  {
    t->root = new_node;
  }

  while (current != t->nil)
  {
    // 반복적으로 업데이트 하여 정확성을 보장
    new_node->parent = current;
    if (new_node->key < current->key)
    {
      if (current->left == t->nil)
      {
        current->left = new_node;
        break;
      }
      current = current->left;
    }
    else
    {
      if (current->right == t->nil)
      {
        current->right = new_node;
        break;
      }
      current = current->right;
    }
  }

  rbtree_insert_fixup(t, new_node);

  return t->root;
}

void rbtree_insert_fixup(rbtree *t, node_t *new_node)
{
  while (new_node->parent->color == RBTREE_RED)
  {
    node_t *grandp = new_node->parent->parent; // 할아버지
    if (!grandp)
    {
      break;
    }

    node_t *uncle;
    if (new_node->parent == grandp->left) // new의 부모가 할아버지의 왼쪽 자식
    {
      uncle = grandp->right;
      if (uncle->color == RBTREE_RED) // case 1 : 삼촌 : red
      {
        new_node->parent->color = RBTREE_BLACK;
        uncle->color = RBTREE_BLACK;
        grandp->color = RBTREE_RED;
        new_node = grandp; // 할아버지 자리에서 판단
      }
      else
      {
        if (new_node == new_node->parent->right) // new가 부모의 오른쪽 자식 (case 2)
        {
          new_node = new_node->parent; // 부모랑 바꾼후
          left_rotate(t, new_node);
        }
        // case 3
        grandp->color = RBTREE_RED;             // 할아버지 red
        new_node->parent->color = RBTREE_BLACK; // 부모 black
        right_rotate(t, grandp);                // 할아버지 위치에서 회전
      }
    }
    else // new의 부모가 할아버지의 오른쪽 자식
    {
      uncle = grandp->left;
      if (uncle->color == RBTREE_RED)
      {
        new_node->parent->color = RBTREE_BLACK;
        uncle->color = RBTREE_BLACK;
        grandp->color = RBTREE_RED;
        new_node = grandp;
      }
      else
      {
        if (new_node == new_node->parent->left)
        {
          new_node = new_node->parent;
          right_rotate(t, new_node);
        }
        grandp->color = RBTREE_RED;
        new_node->parent->color = RBTREE_BLACK;
        left_rotate(t, grandp);
      }
    }
  }
  t->root->color = RBTREE_BLACK;
}

void left_rotate(rbtree *t, node_t *x)
{
  node_t *y = x->right; // x의 오른쪽 자식을 y로 설정 -> y는 회전 후 x 위치로
  x->right = y->left;   // y의 왼쪽 자식을 x의 오른쪽 자식으로

  if (y->left != t->nil) // y의 왼쪽 자식이 있을 경우
  {
    y->left->parent = x; // y의 왼쪽 자식을 x에 붙인다
  }
  y->parent = x->parent; // y의 부모를 x의 부모로

  if (x->parent == t->nil) // x의 부모가 없으면 y가 루트
  {
    t->root = y;
  }
  else if (x == x->parent->left) // x가 부모의 왼쪽 자식이면
  {
    x->parent->left = y;
  }
  else // 오른쪽 자식이면
  {
    x->parent->right = y;
  }

  y->left = x;   // y의 왼쪽 자식은 x
  x->parent = y; // x의 부모는 y
}
void right_rotate(rbtree *t, node_t *x)
{
  node_t *y = x->left; // x의 왼쪽 자식을 y로 설정 -> y는 회전 후 x 위치로
  x->left = y->right;  // y의 오른쪽 자식을 x의 왼쪽 자식으로

  if (y->right != t->nil) // y의 오른쪽 자식이 있을 경우
  {
    y->right->parent = x; // y의 오른쪽 자식을 x에 붙인다
  }
  y->parent = x->parent; // y의 부모를 x의 부모로

  if (x->parent == t->nil) // x의 부모가 없으면 y가 루트
  {
    t->root = y;
  }
  else if (x == x->parent->right) // x가 부모의 오른쪽 자식이면
  {
    x->parent->right = y;
  }
  else // 왼쪽 자식이면
  {
    x->parent->left = y;
  }

  y->right = x;  // y의 오른쪽 자식은 x
  x->parent = y; // x의 부모는 y
}

node_t *rbtree_find(const rbtree *t, const key_t key)
{
  // 없으면 NULL 작으면 왼쪽 크면 오른쪽
  node_t *current = t->root;
  while (current != t->nil)
  {
    if (key == current->key)
    {
      return current;
    }
    current = (key < current->key) ? current->left : current->right;
  }

  return NULL;
}

node_t *rbtree_min(const rbtree *t)
{
  node_t *current = t->root;
  while (current->left != t->nil) // current의 자식이 nil일 때 빠져나와야 한다
  {
    current = current->left;
  }
  return current;
}

node_t *rbtree_max(const rbtree *t)
{
  node_t *current = t->root;
  while (current->right != t->nil)
  {
    current = current->right;
  }
  return current;
}

void rb_transplant(rbtree *t, node_t *u, node_t *v)
{
  // u를 v로 바꾸기
  // 부모와의 연결을 여기서 다 처리한다
  if (u->parent == t->nil)
  {
    t->root = v;
  }
  else if (u == u->parent->left)
  {
    u->parent->left = v;
  }
  else
  {
    u->parent->right = v;
  }
  v->parent = u->parent; // v의 부모가 u의 부모를 가리킨다
}

node_t *find_successor(rbtree *t, node_t *p)
{
  node_t *current = p;
  while (current->left != t->nil)
  {
    current = current->left;
  }
  return current;
}

int rbtree_erase(rbtree *t, node_t *p)
{
  if (!p)
  {
    return 0;
  }
  node_t *y = p, *x;
  color_t y_og_color = y->color;
  if (p->left == t->nil) // p의 왼쪽 자식이 없으면 오른쪽 자식을 p 자리로
  {
    x = p->right;
    rb_transplant(t, p, p->right);
  }
  else if (p->right == t->nil) // 오른쪽 없으면 왼쪽 자식을 p 자리로
  {
    x = p->left;
    rb_transplant(t, p, p->left);
  }
  else // 둘다 있으면
  {
    y = find_successor(t, p->right); // successor 찾음
    y_og_color = y->color;
    x = y->right; // x에 y의 오른쪽 자식 저장
    if (y->parent == p)
    {
      x->parent = y;
    }
    else // y가 p의 직접적인 자식이 아닐때
    {
      rb_transplant(t, y, y->right); // y자리에 y오른쪽
      y->right = p->right;           // y를 p위치로 이동, p의 오른쪽 자식을 y의 오른쪽 자식으로
      y->right->parent = y;          // y의 새 오른쪽 자식의 부모를 y로
    }
    rb_transplant(t, p, y); // p의 왼쪽 자식들을 y의 왼쪽 자식으로
    y->left = p->left;
    y->left->parent = y;
    y->color = p->color; // y의 색을 p의 색으로 바꾼다
  }
  if (y_og_color == RBTREE_BLACK)
  {
    rb_delete_fixup(t, x); // x는 y의 위치를 대신 -> y의 전 색에 따라 fixup 실행
  }
  return 1;
}
void rb_delete_fixup(rbtree *t, node_t *x)
{
  while (x != t->root && x->color == RBTREE_BLACK) //
  {
    node_t *bro;
    if (x == x->parent->left) // x가 왼쪽 자식일 때
    {
      bro = x->parent->right;       // 형제는 오른쪽 자식
      if (bro->color == RBTREE_RED) // 형제가 red일때
      {
        bro->color = RBTREE_BLACK; // case 1
        x->parent->color = RBTREE_RED;
        left_rotate(t, x->parent);
        bro = x->parent->right;
      }
      if (bro->left->color == RBTREE_BLACK && bro->right->color == RBTREE_BLACK)
      {                          // 형제의 왼쪽 자식이 black and 오른쪽 자식도 black
        bro->color = RBTREE_RED; // case 2
        x = x->parent;
      }
      else
      {
        if (bro->right->color == RBTREE_BLACK) // 형제의 오른쪽 자식이 black 일때
        {
          bro->left->color = RBTREE_BLACK; // case 3
          bro->color = RBTREE_RED;
          right_rotate(t, bro);
          bro = x->parent->right;
        }
        bro->color = x->parent->color; // case 4
        x->parent->color = RBTREE_BLACK;
        bro->right->color = RBTREE_BLACK;
        left_rotate(t, x->parent);
        x = t->root;
      }
    }
    else
    {
      bro = x->parent->left;
      if (bro->color == RBTREE_RED)
      {
        bro->color = RBTREE_BLACK;
        x->parent->color = RBTREE_RED;
        right_rotate(t, x->parent);
        bro = x->parent->left;
      }
      if (bro->right->color == RBTREE_BLACK && bro->left->color == RBTREE_BLACK)
      {
        bro->color = RBTREE_RED;
        x = x->parent;
      }
      else
      {
        if (bro->left->color == RBTREE_BLACK)
        {
          bro->right->color = RBTREE_BLACK;
          bro->color = RBTREE_RED;
          left_rotate(t, bro);
          bro = x->parent->left;
        }
        bro->color = x->parent->color;
        x->parent->color = RBTREE_BLACK;
        bro->left->color = RBTREE_BLACK;
        right_rotate(t, x->parent);
        x = t->root;
      }
    }
  }
  x->color = RBTREE_BLACK;
}

// 중위 순회를 사용하여 트리의 키를 배열에 저장하는 도우미 함수
void inorder_traverse(node_t *node, key_t *arr, size_t *index, size_t n, node_t *nil)
{
  if (node == nil || *index >= n)
  {
    return; // 베이스 케이스: nil 노드에 도달하거나 배열 크기를 초과한 경우
  }
  // 왼쪽 서브트리를 먼저 방문
  inorder_traverse(node->left, arr, index, n, nil);
  if (*index < n)
  {
    arr[*index] = node->key; // 현재 노드의 키를 배열에 저장
    (*index)++;              // 배열 인덱스 증가
  }
  // 오른쪽 서브트리 방문
  inorder_traverse(node->right, arr, index, n, nil);
}

// RB 트리를 배열로 변환하는 함수
int rbtree_to_array(const rbtree *t, key_t *arr, size_t n)
{
  if (t->root == t->nil)
  {
    return 0; // 트리가 비어있는 경우
  }
  size_t index = 0;                                  // 배열 인덱스 초기화
  inorder_traverse(t->root, arr, &index, n, t->nil); // 중위 순회 시작
  return (int)index;                                 // 변환된 요소의 수 반환
}