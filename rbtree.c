#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include "page.h"
#include "memory.h"

void *MEM_START;
#define max(a,b) ((a)>(b)?(a):(b))
enum {RED,BLACK};
typedef struct _tree_node_t
{
	int data;
	int color;
	struct _tree_node_t *lchild,*rchild,*parent;
}tree_node_t;

typedef struct _tree_root_t
{
	tree_node_t *node;
}tree_root_t;

void insert_fixup(tree_root_t *root,tree_node_t *new_node);
void delete_fixup(tree_root_t *root,tree_node_t *node);
int height(tree_node_t *root)
{
	if(root == NULL)
	  return 0;
	return max(height(root->lchild),height(root->rchild))+1;
}
//初始化树结点
tree_node_t *init_node(int elem)
{
	tree_node_t *node = (tree_node_t*)mm_malloc(sizeof(tree_node_t));
	if(node)
	{
		node->data = elem;
		node->color = RED;
		node->lchild = NULL;
		node->rchild = NULL;
		node->parent = NULL;
	}
	return node;
}
void right_rotate(tree_root_t *root,tree_node_t *base_node)
{
	tree_node_t *tmp = base_node->lchild;
	base_node->lchild = tmp->rchild;
	tmp->rchild = base_node;

	tmp->parent = base_node->parent;
	base_node->parent = tmp;
	if(base_node->lchild)
	  base_node->lchild->parent = base_node;

	if(tmp->parent == NULL)
	{
		root->node = tmp;
	}
	else
	{
		if(base_node == tmp->parent->lchild)
		  tmp->parent->lchild = tmp;
		else
		  tmp->parent->rchild = tmp;
	}
}
//左旋，
void left_rotate(tree_root_t *root,tree_node_t *base_node)
{
	//tmp为中间变量，用于存储中间结点指针
	//先交换孩子指针域
	tree_node_t *tmp = base_node->rchild;
	base_node->rchild = tmp->lchild;
	tmp->lchild = base_node;

	//修改双亲指针域，要考虑上级为根结点，下级为空的情况
	tmp->parent = base_node->parent;
	base_node->parent = tmp;
	//base_node->rchild即为交换前的右孙结点，不空，则要修改其双亲指针
	if(base_node->rchild)
	  base_node->rchild->parent = base_node;
	//上级为根结点，
	if(tmp->parent == NULL)
	{
		root->node = tmp;
	}
	else
	{
		//上级不为根结点，则判断基点是左孩子还是右孩子，据此修改父结点对应孩子指针域
		if(base_node == tmp->parent->lchild)
		  tmp->parent->lchild = tmp;
		else
		  tmp->parent->rchild = tmp;
	}
}

void insert_node(tree_root_t *root,tree_node_t *new_node)
{
	tree_node_t *p_node = root->node;
	tree_node_t *tmp = NULL;
	//一直找到对应叶子结点去插入tmp用于取前一个指针，p_node最后一定为NULL
	while(p_node)
	{
		tmp = p_node;
		if(p_node->data > new_node->data)
		  p_node = p_node->lchild;
		else if(p_node->data < new_node->data)
		  p_node = p_node->rchild;
		else
		{
			printf("%d already exsit!\n",new_node->data);
			return ;
		}
	}
	//tmp为空则说循环没有执行，为空树
	if(tmp == NULL)
	{
		root->node = new_node;
		new_node->parent = NULL;
		//init node
		//这里没有初始化其它成员
	}
	//这个else不能少，不然会访问到非法内存
	else
	{
		assert(tmp->data != new_node->data);
		if(tmp->data > new_node->data)
		  tmp->lchild = new_node;
		else
		  tmp->rchild = new_node;

		//这里没有初始化其它成员
		new_node->parent = tmp;
	}
	//初始化新结点成员，但没有对数据域处理，也可以在外面初始化，这里就可以不用再次赋值
	new_node->color = RED;
	new_node->lchild = NULL;
	new_node->rchild = NULL;
	//调整
	insert_fixup(root,new_node);
}
//new_node应该为红色
void insert_fixup(tree_root_t *root,tree_node_t *new_node)
{
	tree_node_t *parent,*gparent;
	parent = new_node->parent;
	//插入的结点即为根结点，将其颜色涂黑即可
	if(parent == NULL)
	  new_node->color = BLACK;
	//红父（黑父不用调整），gparent一定为黑色
	else if(parent->color != BLACK)
	{
		gparent = parent->parent;
		//红父不能为根结点，gparent不可能为空	
		//父结点为左结点
		if(parent == gparent->lchild)
		{
			//叔为黑（由于需要递归调用，叔可能不为空）
			if(gparent->rchild == NULL || gparent->rchild->color == BLACK)
			{
				//new_node为左孩子，则右旋并交换gparent和parent的颜色
				if(new_node == parent->lchild)
				{
					right_rotate(root,gparent);
					parent->color = BLACK;
					gparent->color = RED;
				}
				//new_node为右孩子，则先左旋，转化为上面的情形，递归调用，也可直接处理--再右旋gparent，交换颜色gparent->color = RED;new_node->color = BLACK;
				else
				{
					left_rotate(root,parent);
					insert_fixup(root,parent);
				}
			}
			//叔为红，则令叔父为黑，祖父为红，对祖父递归调整
			else
			{
				parent->color = BLACK;
				gparent->color = RED;
				gparent->rchild->color = BLACK;
				insert_fixup(root,gparent);
			}
		}
		//父结点为右孩子
		else
		{
			//叔为黑（由于需要递归调用，叔可能不为空）
			if(gparent->lchild == NULL || gparent->lchild->color == BLACK)
			{
				//new_node为左孩子，则先右旋，转化为下面的情形，递归调用，也可直接处理--再左旋gparent，交换颜色gparent->color = RED;new_node->color = BLACK;
				if(new_node == parent->lchild)
				{
					right_rotate(root,parent);
					insert_fixup(root,parent);
				}
				//new_node为右孩子，直接左旋，交换颜色
				else
				{
					left_rotate(root,gparent);
					parent->color = BLACK;
					gparent->color = RED;
				}
			}
			//叔为红，直接变色，递归gparent
			else
			{
				parent->color = BLACK;
				gparent->color = RED;
				gparent->lchild->color = BLACK;
				insert_fixup(root,gparent);
			}
		}
	}
	//开头有判断,但在gparent刚好为根结点，且没有递归时，可能出现根结点为红的情况。
	//assert(root->node->color == BLACK);
	root->node->color = BLACK;
}
void delete_node(tree_root_t *root,int elem)
{
	//递归删除也可以，最终只会删除叶子结点，效果不变，但效率较低，可能的话，可以把删除不用递归来完成，在只有一个分支的时候可以依情况直接处理。
	tree_node_t *p_node = root->node;
	//tree_node_t *tmp = NULL;
	while(p_node)
	{
		//根据2叉排序树的特性分支查找指定结点
		if(p_node->data > elem)
			p_node = p_node->lchild;
		else if(p_node->data < elem)
			p_node = p_node->rchild;
		//找到指定结点
		else
		{
			tree_node_t *ptmp;
			int tmp;
			if(p_node->lchild != NULL)
			{
				//在左子树中寻找替代结点
				ptmp = p_node->lchild;
				while(ptmp->rchild)
					ptmp = ptmp->rchild;
				//找到替代结点，先保存其值，递归删除后，将原本要删除的结点值替换
				tmp = ptmp->data;
				delete_node(root,tmp);
				p_node->data = tmp;
			}
			else if(p_node->rchild != NULL)
			{
				//在右子树中寻找替代结点
				ptmp = p_node->rchild;
				while(ptmp->lchild)
					ptmp = ptmp->rchild;
				//找到替代结点，先保存其值，递归删除后，将原本要删除的结点值替换
				tmp = ptmp->data;
				delete_node(root,tmp);
				p_node->data = tmp;
			}
			//为叶子结点
			else
			{			
				//根结点即为要删除结点，且无左右子树，直接删除
				if(root->node == p_node)
					root->node = NULL;
				//要删结点为红色结点，直接删除，后面统一释放
				else if(p_node->color == RED)
				{
					if(p_node == p_node->parent->lchild)
						p_node->parent->lchild = NULL;
					else
						p_node->parent->rchild = NULL;
				}
				//要删结点为黑色结点，先调整后删除，后面统一释放
				else
				{								
					delete_fixup(root,p_node);
					if(p_node == p_node->parent->lchild)
					{
						p_node->parent->lchild = NULL;
					}
					else
						p_node->parent->rchild = NULL;
				}
				//释放资源，但不要将p_node置空，后面要依据p_node的值来判定，
				mm_free(p_node);
				
				
			}
			//不要忘了此处的break
			break;
		}
	}
	//找完后p_node为空，说明没找到所给的元素
	if(p_node == NULL)
	{
		printf("%d not exsit!\n",elem);
	}
}
//node应该为黑色
void delete_fixup(tree_root_t *root,tree_node_t *node)
{
	assert(node);
	tree_node_t *parent = node->parent;
	tree_node_t *sibling = NULL;
	//递归至根结点
	if(parent == NULL)
		return ;
	
	//本结点为左孩子
	if(parent->lchild == node)
	{
		sibling = parent->rchild;
		//本结点为黑
		assert(node->color == BLACK);
		//红父
		if(parent->color == RED)
		{
			//兄弟一定为黑
			assert(sibling != NULL && sibling->color == BLACK);
			//左侄为黑
			if(sibling->lchild == NULL || sibling->lchild->color == BLACK)
			{
				//右侄为黑,双黑侄，交换父兄颜色即可
				if(sibling->rchild == NULL || sibling->rchild->color == BLACK)
				{
					parent->color = BLACK;
					sibling->color = RED;
				}
				else
					left_rotate(root,parent);	
			}
			//左侄为红
			else
			{
				//旋转后左侄为红（父结点之前的颜色），父为黑
				right_rotate(root,sibling);
				left_rotate(root,parent);
				parent->color = BLACK;
			}
		}
		//黑父
		else
		{
			//红兄
			if(sibling->color == RED)
			{
				//一定是双黑侄，旋转后交换父兄颜色后再次对node调整，红兄转化为黑兄来处理
				left_rotate(root,parent);
				sibling->color = BLACK;
				parent->color = RED;
				delete_fixup(root,node);	
			}
			//黑兄
			else
			{
				//左侄为红
				if(sibling->lchild != NULL && sibling->lchild->color == RED)
				{
					sibling->lchild->color = BLACK;
					right_rotate(root,sibling);
					left_rotate(root,parent);						
				}
				//右侄为红，则交换父兄颜色（本层内不用处理），右侄变为黑。
				else if(sibling->rchild != NULL && sibling->rchild->color == RED)
				{
					sibling->rchild->color = BLACK;
					left_rotate(root,parent);
				}
				//双黑侄,将兄弟结点涂黑，递归调整父结点
				else 
				{
					sibling->color = RED;
					delete_fixup(root,parent);
				}					
			}
		}		
	}
	//本结点为右孩子
	else 
	{
		sibling = parent->lchild;
		assert(node->color == BLACK);
		//红父
		if(parent->color == RED)
		{
			//一定有黑兄
			assert(sibling != NULL && sibling->color == BLACK);
			//右侄为黑
			if(sibling->rchild == NULL || sibling->rchild->color == BLACK)
			{
				//双黑侄，交换父兄颜色即可
				if(sibling->lchild == NULL || sibling->lchild->color == BLACK)
				{
					parent->color = BLACK;
					sibling->color = RED;
				}
				//左侄为红，右侄为黑，右旋即可
				else
				{
					right_rotate(root,parent);					
				}				
			}
			//右侄为红，则右侄替换父结点，颜色换成父结点颜色，父结点涂黑
			else
			{
				parent->color = BLACK;
				left_rotate(root,sibling);
				right_rotate(root,parent);
			}
		}
		//黑父
		else
		{
			assert(sibling != NULL);
			//红兄
			if(sibling->color == RED)
			{
				//一定有双黑侄 旋转后交换父兄颜色再次对node调整，红兄转化为黑兄处理
				right_rotate(root,parent);
				parent->color = RED;
				sibling->color = BLACK;
				delete_fixup(root,node);
			}
			//黑兄
			else
			{
				//右侄为红，旋转后，右侄顶替父结点，颜色要变更为父结点原来颜色，父结点为黑
				if(sibling->rchild != NULL && sibling->rchild->color == RED)
				{					
					sibling->rchild->color = BLACK;
					right_rotate(root,sibling);
					left_rotate(root,parent);
				}
				//左侄为红，右侄为黑，右旋后变色,父变黑，兄变为父原来颜色，左侄变黑
				else if(sibling->lchild != NULL && sibling->lchild->color == RED)
				{
					sibling->lchild->color = BLACK;
					right_rotate(root,parent);
				}
				//双黑侄，兄变红，递归调用父结点
				else
				{
					sibling->color = RED;
					delete_fixup(root,parent);
				}
			}			
		}
	}
}
void mid_order(tree_node_t *root,void (*pfun)(tree_node_t *))
{
	if(root == NULL)
	  return ;
	mid_order(root->lchild,pfun);
	pfun(root);
	mid_order(root->rchild,pfun);
}
void _level_order(tree_node_t *root,void (*pfun)(tree_node_t *),int level)
{
	assert(level >= 0);
	if(root == NULL ||  level == 0)
	  return ;
	if(level == 1)
	{
	  pfun(root);
	  return ;
	}
	_level_order(root->lchild,pfun,level-1);
	_level_order(root->rchild,pfun,level-1);
}
void level_order(tree_node_t *root, void (*pfun)(tree_node_t *))
{
	int level = height(root);
	int i;
	for(i = 1; i <= level; ++i)
	{
		printf("level %d: ",i);
		_level_order(root,pfun,i);
		printf("\n");
	}
}
void show(tree_node_t *node)
{
	printf("%d(%c)  ",node->data,node->color==RED?'R':'B');
}
tree_root_t *init_root()
{
	tree_root_t *root = (tree_root_t*)mm_malloc(sizeof(tree_root_t));
	if(root)
	{
		root->node = NULL;
	}
	return root;
}
//root满足红黑树特性就返回其根结点到叶子结点黑色结点数，若不满足红黑树特性就返回0
int check_rbtree(tree_node_t *root)
{
	//根结点为空，则返回1
	if(root == NULL)
		return 1;	
	//根结点为红（递归调用时可能为红），则需判断其子结点不能为红，且递归调用左右孩子的返回值相同。满足两个条件则返回其值
	if(root->color == RED)
	{
		if(root->lchild == NULL && root->rchild == NULL)
			return 1;
		else if(root->lchild == NULL || root->rchild == NULL)
			return 0;
		else if(root->lchild->color == RED || root->rchild->color == RED)
			return 0;
		else if(check_rbtree(root->lchild) > 0 && check_rbtree(root->lchild) == check_rbtree(root->rchild))
			return check_rbtree(root->lchild);
		else
			return 0;
	}
	//根结点为黑，则只需判断递归调用左右孩子的返回值相同。满足两个条件则返回其值+1
	else if(check_rbtree(root->lchild) > 0 && check_rbtree(root->lchild) == check_rbtree(root->rchild))
		return check_rbtree(root->lchild)+1;
	else
		return 0;	
	
}

int main()
{
	MEM_START = malloc(MEM_SIZE);
	if(MEM_START == NULL)
	{
		perror("malloc failed");
		exit(1);
	}
	printf("输入正数插入节点，输入负数删除指定节点\n");
	tree_root_t *root = init_root();
	int number;
	while(1)
	{
		if(scanf("%d",&number) == 1)
		{
			//正数插入
			if(number > 0)
			  insert_node(root,init_node(number));
			//负数删除相应正数
			else if(number < 0)
			  delete_node(root,0-number);
			//输入0退出
			else
			  break;
			printf("mid_order:\n");
			//中序遍历
			mid_order(root->node,show);
			printf("\nlevel_order:\n");
			//层序遍历
			level_order(root->node,show);
			//检查有无破坏红黑树特性
			printf("check_rbtree:%d\n",check_rbtree(root->node));
		}
		else
		  while(getchar() != '\n')
			;
	}
	free(MEM_START);
	return 0;
}
