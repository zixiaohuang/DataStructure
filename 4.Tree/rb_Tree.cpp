/*
二叉查找树/有序二叉树：指一棵空树或者具有下列性质的二叉树：
1.若任意节点的左子树不空，则左子树上所有结点的值均小于它的根节点的值
2.若任意节点的右子树不空，则右子树上所有结点的值均大于它的根节点的值
3.任意结点的左右树也分别为二叉查找树
4.没有键值相等的节点

因为一颗由n结点随机构造的二叉查找树的高度为lgn，一般操作的执行时间为O(lgn)，
但二叉查找树若退化成一棵具有n个结点的线性链后，则这些操作最坏情况运行时间为O(n)

红黑树：一种二叉查找树，但每个结点上增加一个存储位表示结点的颜色（Red/Black）

一棵n个结点的红黑树的高度如何保证始终保持在lgn？引出红黑树5个的性质
红黑树的性质：
1.红黑树是一棵平滑二叉搜索树，其中序遍历单调不减
2.每个节点不是红色就是黑色
3.根节点是黑色
4.每个叶子节点（叶子节点指树尾端NIL指针或NULL结点）是黑色
5.如果节点是红色，那么它的两个孩子节点都是黑色的
6.对于每个节点来说，从节点到叶子节点的路径包含相同数目的黑色节点（黑高度）


*/

#include <cstdio>
using namespace std;

//定义x兄弟的宏,父亲左分支是不是自己，如果是就是父亲的右分支
#define bro(x) (((x)->ftr->lc==(x))?((x)->ftr->rc):((x)->ftr->lc))

typedef bool RB_COLOR;//把布尔型定义为红和黑的颜色
#define RB_COLOR_RED true
#define RB_COLOR_BLACK false

template<typename T>
class redblacktree {
protected:
	struct RB_Node; //结点,维护信息、左右儿子、父亲、前驱后继函数、真后继函数

	RB_Node* _root;//根节点
	RB_Node* _hot;//维护父亲结点

	int _size;//树的大小

	void init(T) ;//初始化

	RB_Node* zig(RB_Node*) ;//左旋
	RB_Node* zag(RB_Node*) ;//右旋

	void SolveDoubleRed(RB_Node*) ;//双红修正
	void SolveLostBlack(RB_Node*) ;//失黑修正

	RB_Node* find(T);//寻找结点

	void removetree(RB_Node*) ;//删除整棵树，dfs遍历

public:
	struct iterator;//迭代器

	redblacktree() :_root(NULL), _hot(NULL), _size(0) {}//构造函数

	iterator insert(T) ;//插入，返回迭代器，结点的位置

	bool remove(T) ;//删除
	bool remove(iterator&) ;//删除迭代器

	iterator search(T) ;//寻找
	iterator lower_bound(T) ;//下届寻找，返回大于等于T的结点
	iterator upper_bound(T) ;//上届寻找

	void clear() ;//清理整棵树，dfs后序遍历入口

	int size() ;

	bool empty() ;

	iterator begin() ;
	static iterator end() ;
};

template<typename T>
struct redblacktree<T>::RB_Node {
	T val;//值
	RB_COLOR RBc;//颜色
	RB_Node* ftr;//父亲
	RB_Node* lc;//左儿子
	RB_Node* rc;//右儿子

	RB_Node(T v = T(), RB_COLOR RB = RB_COLOR_RED, RB_Node* f = NULL, RB_Node* lchild = NULL, RB_Node* rchild = NULL) :
		val(v), RBc(RB), ftr(f), lc(lchild), rc(rchild){}

	//真后继函数：树中大于t的最小的那个元素
	RB_Node* succ() {
		RB_Node* ptn = rc;
		while (ptn->lc) {//左儿子非空时，让ptn等于左儿子
			ptn = ptn->lc;
		}
		return ptn;
	}

	//前驱
	RB_Node* left_node() {//左边节点
		RB_Node* ptn = this;
		if (!lc) {//没有左子节点
			while (ptn->ftr&&ptn->ftr->lc == ptn) {//有父节点，不是根节点的时候，若是左子节点，则返回父节点
				ptn = ptn->ftr;
			}
			ptn = ptn->ftr;//再返回父节点的父节点，得到前继
		}
		else {//根节点，得到左儿子再一直往右走
			ptn = ptn->lc;
			while (ptn->rc) {
				ptn = ptn->rc;
			}
		}
		return ptn;
	}
	//后继
	RB_Node* right_node() {//右边节点
		RB_Node* ptn = this;
		if (!rc) {
			while (ptn->ftr&&ptn->ftr->rc == ptn) {
				ptn = ptn->ftr;
			}
			ptn = ptn->ftr;
		}
		else {
			ptn = ptn->rc;
			while (ptn->lc) {
				ptn = ptn->lc;
			}
		}
		return ptn;
	}
};

template<typename T>
struct redblacktree<T>::iterator {
protected:
	RB_Node* _real_node;

public://迭代器就是各种重载
	T operator*() {//重载* 返回真实值，指针就是一个迭代器
		return _real_node->val;
	}

	bool operator==(iterator const& itr) {
		return _real_node == itr._real_node;
	}

	bool operator!=(iterator const& itr) {
		return _real_node != itr._real_node;
	}

	bool operator!() {
		return !_real_node;//判断真实值是否为空
	}

	iterator& operator=(iterator const& itr) {
		_real_node = itr._real_node;
		return *this;
	}

	iterator& operator++() {
		_real_node = _real_node->right_node();
		return *this;
	}

	iterator& operator--() {
		_real_node = _real_node->left_node();
		return *this;
	}

	//构造函数
	iterator(RB_Node* node_nn=NULL):_real_node(node_nn){}//默认构造？
	iterator(T const& val_vv):_real_node(find(val_vv)) {}
	iterator(iterator const& itr):_real_node(itr._real_node){}//拷贝构造
};

//搜索
template<typename T>
	typename
redblacktree<T>::RB_Node* redblacktree<T>::find(T v) {
	RB_Node* ptn = _root;
	_hot = NULL;
	while (ptn&&ptn->val != v) {
		_hot = ptn;
		if (ptn->val > v) {
			ptn = ptn->lc;
		}
		else {
			ptn = ptn->rc;
		}
	}
	return ptn;
}

template<typename T>
typename
redblacktree<T>::iterator redblacktree<T>::search(T v) {
	return iterator(find(v));
}

template<typename T>
typename
redblacktree<T>::iterator redblacktree<T>::lower_bound(T v) {
	RB_Node* ptn = _root;
	_hot = NULL;
	while (ptn) {
		_hot = ptn;
		if (ptn->val >= v) {
			ptn = ptn->lc;
		}
		else {
			ptn = ptn->rc;
		}
	}
	if (_hot->val >= v) {
		ptn = _hot;
	}
	else {
		ptn = _hot->right_node();
	}
	return iterator(ptn);
}

template<typename T>
typename
redblacktree<T>::iterator redblacktree<T>::upper_bound(T v) {
	RB_Node* ptn = _root;
	_hot = NULL;
	while (ptn) {
		_hot = ptn;
		if (ptn->val > v) {
			ptn = ptn->lc;
		}
		else {
			ptn = ptn->rc;
		}
	}
	if (_hot->val > v) {
		ptn = _hot;
	}
	else {
		ptn = _hot->right_node();
	}
	return iterator(ptn);
}

template<typename T>
void redblacktree<T>::init(T v) {//初始化操作：直接作为根节点（黑节点）插入
	_root = new RB_Node(v,RB_COLOR_BLACK,NULL,NULL,NULL);
	_size = 1;
}

template<typename T>
typename
redblacktree<T>::RB_Node* redblacktree<T>::zig(RB_Node* ptn) {//右旋：右旋中的“右”，意味着“被旋转的节点将变成一个右节点”
	ptn->lc->ftr = ptn->ftr;//ptn父节点设为ptn左子节点的父节点
	if (ptn->ftr) {//ptn不是根节点
		if (ptn->ftr->lc == ptn) {//若ptn为它父节点的左子节点
			ptn->ftr->lc = ptn->lc;//则ptn的左子节点为ptn父节点的左子节点
		}
		else {//若ptn为它父节点的右子节点
			ptn->ftr->rc = ptn->lc;//则ptn的左子节点为ptn父节点的右子节点
		}
	}
	if (ptn->lc->rc) {//若ptn左子节点存在右子节点
		ptn->lc->rc->ftr = ptn;//则该右子节点的父节点设为ptn
	}
	ptn->ftr = ptn->lc;//ptn左子节点设为ptn的父节点
	ptn->lc = ptn->lc->rc;//ptn左子节点的右子节点设为ptn的左子节点
	ptn->ftr->rc = ptn;//将ptn设置为右子节点
	return ptn->ftr;
}

template<typename T>
typename
redblacktree<T>::RB_Node* redblacktree<T>::zag(RB_Node* ptn) {//左旋：左旋中的“左”，意味着“被旋转的节点将变成一个左节点”
	ptn->rc->ftr = ptn->ftr;
	if (ptn->ftr) {
		if (ptn->ftr->lc == ptn) {
			ptn->ftr->lc = ptn->rc;
		}
		else {
			ptn->ftr->rc = ptn->rc;
		}
	}
	if (ptn->rc->lc) {
		ptn->rc->lc->ftr = ptn;
	}
	ptn->ftr = ptn->rc;
	ptn->rc = ptn->rc->lc;
	ptn->ftr->lc = ptn;
	return ptn->ftr;
}


template<typename T>
typename
redblacktree<T>::iterator redblacktree<T>::insert(T v) {
	RB_Node* ptn = find(v);
	if (ptn) {//不希望元素重复，若存在则直接返回
		return iterator(ptn);
	}
	if (!_hot) {//父节点为空，直接插入作为根节点
		//assert(_size=0);
		init(v);
		return iterator(_root);
	}
	++_size;//不作为根插入
	ptn = new RB_Node(v, RB_COLOR_RED, _hot, NULL, NULL);//以红色插入
	if (_hot->val < v) {//v的值大于_hot的值，插入在右子节点
		_hot->rc = ptn;
	}
	else {//v的值小于_hot的值，插入在左子节点
		_hot->lc = ptn;
	}
	SolveDoubleRed(ptn);//双红修正
	return iterator(ptn);
}

/*
插入与双红现象
	（性质5比性质6好维护）为了尽可能维护性质6，每一次插入，都要将节点作为红色节点插入。
BST的插入应该比较容易做到，但问题在于，性质5可能因此被破坏，即如果被插入的节点的父亲是红色，就会出现双红现象。

双红缺陷是红黑树可能出现的两大缺陷之一。
处理红黑树的核心思想：将红色的节点移到根节点；然后，将根节点设为黑色。
既然是“将红色的节点移到根节点”，那就是说要不断的将破坏红黑树特性的红色节点上移（即通过左旋/右旋向根节点方向移到）

双红修正
分三种情况
RR-0（没有双红现象）：如果正在修正的节点的父亲是黑色，那么修正就已经结束了
RR-1(叔叔是黑色)（插入到等价B树的三节点中）：
如果正在修正的节点的父亲是红色（那么祖父一定是黑色），但是叔叔是黑色，那么只需要做1或2次选择，再进行2次染色就可以解决。
RR-2(叔叔是红色)（插入到等价B-树的四节点中）：
如果正在修正的节点的父亲是红色（那么祖父一定是黑色），而且叔叔也是红色，那么递归就开始了
*/

template<typename T>
void redblacktree<T>::SolveDoubleRed(RB_Node* nn) {
	while ((nn->ftr) && nn->ftr-> RBc == RB_COLOR_RED) {//递归到根和RR-0的情况
		RB_Node* pftr = nn->ftr;//父节点
		RB_Node* uncle = bro(pftr);//叔叔节点
		RB_Node* grdftr = pftr->ftr;//祖父节点
		if (uncle&&uncle->RBc == RB_COLOR_RED) {//RR-2:叔叔节点存在且位红色
			grdftr->RBc = RB_COLOR_RED;
			uncle->RBc = RB_COLOR_BLACK;
			pftr->RBc = RB_COLOR_BLACK;
			nn = grdftr;
		}
		else {//RR-1：叔叔节点是黑色
			if (grdftr->lc == pftr) {
				if (pftr->lc == nn) {//该当前节点为左子节点，旋转一次
					if (grdftr == _root) {//判断祖父节点是否为根节点
						_root = pftr;
					}
					zig(grdftr);//右旋,祖父节点变成父节点的子节点
					pftr->RBc = RB_COLOR_BLACK;
					grdftr->RBc = RB_COLOR_RED;
				}
				else {//该当前节点为右子节点，旋转两次
					if (grdftr == _root) {
						_root = nn;
					}
					zag(pftr); zig(grdftr);
					nn->RBc = RB_COLOR_BLACK;
					grdftr->RBc = RB_COLOR_RED;
				}
			}
			else {
				if (pftr->lc == nn) {
					if (grdftr == _root) {
						_root = nn;
					}
					zig(pftr); zag(grdftr);
					nn->RBc = RB_COLOR_BLACK;
					grdftr->RBc = RB_COLOR_RED;
					
				}
				else {
					if (grdftr == _root) {
						_root = pftr;
					}
					zag(grdftr);
					pftr->RBc = RB_COLOR_BLACK;
					grdftr->RBc = RB_COLOR_RED;
				}
			}
			return;
		}
	}
	if (nn == _root) {
		nn->RBc = RB_COLOR_BLACK;
	}
}

template<typename T>
typename
redblacktree<T>::iterator redblacktree<T>::begin() {
	RB_Node* ptn = _root;
	while (ptn->lc) {
		ptn = ptn->lc;
	}
	return iterator(ptn);

}

template<typename T>
typename
redblacktree<T>::iterator redblacktree<T>::end() {
	return iterator((RB_Node*)NULL);
}


template<typename T>
int redblacktree<T>::size() {
	return _size;
}


template<typename T>
bool redblacktree<T>::empty() {
	return !_size;
}

template<typename T>
void redblacktree<T>::clear() {
	removetree(_root);
	_size = 0;
	_root = NULL;
}


template<typename T>
void redblacktree<T>::removetree(RB_Node* ptn) {//后序遍历
	if (!ptn) {//若空直接返回
		return;
	}
	removetree(ptn->lc);
	removetree(ptn->rc);
	delete ptn;//初始化，插入new对应delete
}

/*
删除的思路：一路找到被删除节点的真后继（比该节点值大的第一个节点），然后回到覆盖原节点，如此反复。
若最后删除的是红节点，则没有涉及失黑修正。如果最后删除是黑节点，性质6被破坏
*/


/*
失黑修正
需要递归的情况：
LB-1（父亲为黑色，兄弟为红色）
不能直接解决，但可以利用一次旋转，触发递归，将它转化为不需要递归的LB-2R或者LB-3
LB-2B(没有红色侄子，且父亲为黑色，兄弟为黑色)
这时自己、父亲、兄弟都独占二节点。这是红黑树修正过程中唯一可能出现对数递归的情况。
这时染红兄弟，然后递归修正父亲。
不需要递归：
LB-2R（没有红色侄子，且父亲为红色，兄弟为黑色）
只需要对父亲和兄弟各做一次重染色
LB-3(有红色侄子)
修正方法是1或2次旋转，然后1或2次重染色
*/

template<typename T>
void redblacktree<T>::SolveLostBlack(RB_Node* nn) {
	while (nn != _root) {
		RB_Node* pftr = nn->ftr;
		RB_Node* bthr = bro(nn);
		if (bthr->RBc == RB_COLOR_RED) {  //LB-1
			bthr->RBc = RB_COLOR_BLACK;
			pftr->RBc = RB_COLOR_RED;
			if (_root == pftr) {
				_root = bthr;//维护根节点
			}
			if (pftr->lc == nn) {
				zag(pftr);
			}
			else {
				zig(pftr);
			}
			bthr = bro(nn);
			pftr = nn->ftr;
		}
		if (bthr->lc&&bthr->lc->RBc == RB_COLOR_RED) {//LB-3
			RB_COLOR oldRBc = pftr->RBc;
			pftr->RBc = RB_COLOR_BLACK;
			if (pftr->lc == nn) {
				if (_root == pftr) {
					_root = bthr->lc;
				}
				zig(bthr); zag(pftr);
			}
			else {
				bthr->lc->RBc = RB_COLOR_BLACK;
				if (_root == pftr) {
					_root = bthr;
				}
				zig(pftr);
			}
			pftr->ftr->RBc = oldRBc;
			return;
		}
		else if (bthr->rc&&bthr->rc->RBc == RB_COLOR_RED) {//LB-3
			RB_COLOR oldRBc = pftr->RBc;
			pftr->RBc = RB_COLOR_BLACK;
			if (pftr->lc == nn) {
				bthr->rc->RBc = RB_COLOR_BLACK;
				if (_root == pftr) {
					_root = bthr;//变成自己
				}
				zag(pftr);
			}
			else {
				if (_root == pftr) {
					_root = bthr->rc;
				}
				zag(bthr); zig(pftr);
			}
			pftr->ftr->RBc = oldRBc;
			return;
		}

		if (pftr->RBc == RB_COLOR_RED) {//LB-2
			pftr->RBc = RB_COLOR_BLACK;
			bthr->RBc = RB_COLOR_RED;
			return;
		}
		else {//LB-2B：父亲为黑色，兄弟为黑色
			bthr->RBc = RB_COLOR_RED;
			nn = pftr;
		}
	}
	
}


template<typename T>
bool redblacktree<T>::remove(T v) {
	RB_Node* ptn = find(v);
	RB_Node* node_suc;//后继
	if (!ptn) {//没有该元素，删除失败
		return false;
	}
	--_size;
	while (ptn->lc || ptn->rc) {
		if (!(ptn->lc)) {//没有左儿子，则只有右儿子，且阈值为1
			node_suc = ptn->rc;
		}
		else if (!(ptn->rc)) {//没有右儿子，只有左儿子
			node_suc = ptn->lc;
		}
		else {//两个儿子都没有
			node_suc = ptn->succ();
		}
		ptn->val = node_suc->val;
		ptn = node_suc;
	}
	if (ptn->RBc == RB_COLOR_BLACK) {//若删除的节点颜色为黑，需要失黑修正
		SolveLostBlack(ptn);
	}
	if (ptn->ftr) {//父亲存在,把父亲指向节点的指针消除
		if (ptn->ftr->lc == ptn)
		{
			ptn->ftr->lc = NULL;
		}
		else {
			ptn->ftr->rc = NULL;
		}
	}
	if (_root == ptn) {
		//assert(_size==0);
		_root = NULL;
	}
	delete ptn;
	return true;
}

template<typename T>
bool redblacktree<T>::remove(iterator& itr) {
	RB_Node* ptn = itr._real_node;
	itr._real_node = itr._real_node->right_node();
	if (!(itr._real_node)) {//没有该元素，删除失败
		itr._real_node = ptn->left_node();
	}
	RB_Node* node_suc;//后继
	--size;
	while (ptn->lc || ptn->rc) {
		if (!(ptn->lc)) {//没有左儿子，则只有右儿子，且阈值为1
			node_suc = ptn->rc;
		}
		else if (!(ptn->rc)) {
			node_suc = ptn->lc;
		}
		else {
			node_suc = ptn->succ();
		}
		ptn->val = node_suc->val;
		ptn = node_suc;
	}
	if (ptn->RBc == RB_COLOR_BLACK) {//若删除的节点颜色为黑，需要失黑修正
		SolveLostBlack(ptn);
	}
	if (ptn->ftr) {//父亲存在,把父亲指向节点的指针消除
		if (ptn->ftr->lc == ptn)
		{
			ptn->ftr->lc == NULL;
		}
		else {
			ptn->ftr->rc = NULL;
		}
	}
	if (_root == ptn) {
		//assert(_size==0);
		_root = NULL;
	}
	delete ptn;
	return true;
}

redblacktree<int> s;
#include <cstdlib>
#include <ctime>

int i;//i为全局变量，方便跟踪

int main()
{
	srand((unsigned)time(NULL));
	for (i=0; i < 256; ++i) {
		s.insert(i);//插入一个随机数0~1023
	}
	redblacktree<int>::iterator it;
	for (i=0; i < 256; ++i) {
		s.remove(i);
	}
	//for (it = s.begin(); it != s.end(); ++it) {
		//printf("%d\n", *it);
	//}
	

	return 0;
}

//总结
//不管是双红修正还是双黑修正，均涉及到重染色的操作，而其中RR-1,LB-1,LB-3均需注意维护根节点
//RR-2,LB-2B分别对应着B-树的上溢和下溢，均需要递归
//RR-2能递归到RR-0,RR-1,RR-2三种情况中的任何一种，LB-2B同样也会递归到LB-1,LB-2B,LB-2R,LB-3四种失黑修正情况中的任何一种
//而同样是递归，LB-1只能转移到LB-2R和LB-3

//红黑树稳定，应用广，重点是快