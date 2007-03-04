/***********************************************************************\
|									|
|	B+tree data structures & external interface	C++		|
|									|
|									|
|	Jan Jannink	created 5/27/94		revised 6/15/95		|
|									|
\***********************************************************************/

#ifndef BPLUSTREE
#define BPLUSTREE 1

/*~~~~~~~~~~~~~~~~	sample B+tree parametrization	~~~~~~~~~~~~~~~*/
				//  causes printing of node information
#define DEBUG 1
#undef DEBUG
				//  pointer vs. array representation of nodes
#define POINTER 1
				//  24 bytes is minimal size for 2-3 trees
#define NODE_SIZE 72
				//  16 bytes to store a data point
#define DATA_SIZE 16
				//  maximum number of available nodes
#define ARRAY_SIZE 24576


/*~~~~~~~	flag bits (5 of 16 used, 11 for magic value)	~~~~~~~*/
				//  bits set at node creation/split/merge
#define isLEAF	0x1
#define isROOT	0x2
				//  bits set at key insertion/deletion
#define isFULL	0x4
#define FEWEST	0x8
#define FLAGS	0xF
				//  identifies data as being a B+tree node
#define MAGIC	0xDEC0
#define MASK	0xFFF0


/*~~~~~~~~~~~~~~~~~~~~~~~~	constants	~~~~~~~~~~~~~~~~~~~~~~~*/
			//  ARRAY is a place holder value for:  fanout
#define ARRAY	1
			//  corresponds to a NULL node pointer value
#define NONODE	(nodearrayhead - 1)
			//  special node slot values used in key search
#define ERROR	-1
#define UPPER	-2
#define LOWER	-3


/*/*~~~~~~~~~~~~~~~~~~~~~~~~	node pointer and key type	~~~~~~~*/
#ifdef POINTER
typedef struct node	*Nptr;	//  streamlined pointer representation
#else
typedef int		Nptr;	//  more intuitive array representation
#endif
typedef int		keyT;	//  adapt key type to requirements
typedef char		*dataT;	//  adapt data type to requirements


/*~~~~~~~~~~~~~~~~~~~~~~~~	node status	~~~~~~~~~~~~~~~~~~~~~~~*/
typedef struct state {
  short	flags;
  short	pairs;
} State;			//  sizeof(State) must be <= sizeof(keyT)


/*~~~~~~~~~~~~~~	single node entry with key value	~~~~~~~*/
typedef struct entrY {
  keyT	key;			//  can be a hashed value
  Nptr	downNode;
} Entry;			//  WARNING: entry was a RESERVED word in C


/*~~~~~~~~~~~~~~~~~~~~	special header entry for internal node	~~~~~~~*/
typedef struct inner {
  State info;
  Nptr	firstNode;		//  node of smallest values
} Inner;


/*~~~~~~~~~~~~~~~~~~~~	special header entry for leaf node	~~~~~~~*/
typedef struct leaf {
  State info;
  Nptr	nextNode;		//  next leaf in sequential scan
} Leaf;


/*~~~~~~~~~~~~~~~~~~~~	unstructured data node	~~~~~~~~~~~~~~~~~~~~~~~*/
typedef struct data {
  char	value[NODE_SIZE];
} Data;


/*~~~~~~~~~~~~	data node header to handle duplicates	~~~~~~~~~~~~~~~*/
typedef struct dupdata {
  int	copy;			//  tallies the duplicate keys
  Nptr	next;			//  next node with same key value
  char	value[DATA_SIZE];
} DupData;


/*/*~~~~~~~~~~~~~~~~~~~~~~~~	structured tree node	~~~~~~~~~~~~~~~*\
|
|	The structures Entry, Inner and Leaf are all identical in size.
|	Each node is of size:  fanout * sizeof(Entry).  Through the
|	union X, it is possible to access the first space in any node
|	as: X.e[0], X.i, X.l, depending on the algorithms' needs.  The
|	value of the status flag isLEAF should always determine how the
|	first node space is used.  The node structure is defined below.
|
|	Internal node:			Leaf node:
|
|	+---------------+		+---------------+
|	|Inner: X.i	|		|Leaf: X.l	|
|	|  info		|		|  info		|
|	|  firstNode	|		|  nextNode	|
|	|---------------|		|---------------|
|	|Entry: X.e[1]	|		|Entry: X.e[1]	|
|	|  key		|		|  key		|
|	|  downNode	|		|  downNode	|
|	|---------------|		|---------------|
|	|	.	|		|	.	|
|	|	.	|		|	.	|
|	|	.	|		|	.	|
|	|---------------|		|---------------|
|	|Entry: X.e[n]	|		|Entry: X.e[n]	|
|	|  key		|		|  key		|
|	|  downNode	|		|  downNode	|
|	+---------------+		+---------------+
|
\*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
typedef struct node {
  union {
    Entry	e[ARRAY];	//  allows access to entry array
    Inner	i;
    Leaf	l;
    Data	d;
    DupData	dd;
  } X;
} Node;


/*~~~~~~~~~~~~~~~~~~~~~~~~	key comparison function type	~~~~~~~*/
typedef int (*KeyCmp)(keyT, keyT);

int compareKeys(keyT key1, keyT key2);


/*/*~~~~~~~~~~~~~~~~~~~~~~~~	tree class definition	~~~~~~~~~~~~~~~*/
class Tree {
private:
			//  private variables
  unsigned int	poolsize;	//  # of nodes allocated for tree
  Node		*tree;		//  pointer to array of nodes (NOT Nptr !)
  Nptr		root;		//  pointer to root node
  Nptr		leaf;		//  pointer to first leaf node in B+tree
  unsigned int	fanout;		//  # of pointers to other nodes
  unsigned int	minfanout;	//  usually minfanout == ceil(fanout/2)
  unsigned int	height;		//  nodes traversed from root to leaves
  Nptr		pool;		//  list of empty nodes
  keyT		theKey;		//  the key value used in tree operations
  dataT		theData;	//  data used for insertions/deletions
  union {			//  nodes to change in insert and delete
    Nptr	split;
    Nptr	merge;
  } branch;
  KeyCmp	keycmp;		//  pointer to function comparing two keys

			// private member functions
#ifdef DEBUG			//  print node/tree contents
  void		showNode(Nptr node);
  void		showBtree();
#endif
				//  initialization support functions
  void		initFreeNodePool(int quantity);
  Nptr		getFreeNode();
  void		putFreeNode(Nptr self);
				//  search support functions
  Nptr		descendToLeaf(Nptr curr);
  int		getSlot(Nptr curr);
  int		findKey(Nptr curr, int lo, int hi);
  int		bestMatch(Nptr curr, int slot);
				//  insert support functions
  Nptr		getDataNode(keyT key);
  Nptr		descendSplit(Nptr curr);
  void		insertEntry(Nptr node, int slot, Nptr sibling, Nptr downPtr);
  void		placeEntry(Nptr node, int slot, Nptr downPtr);
  Nptr		split(Nptr node);
  void		makeNewRoot(Nptr oldRoot, Nptr newNode);
				//  delete support functions
  Nptr		descendBalance(Nptr curr, Nptr left, Nptr right,
					Nptr lAnc, Nptr rAnc, Nptr parent);
  void		collapseRoot(Nptr oldRoot, Nptr newRoot);
  void		removeEntry(Nptr curr, int slot);
  Nptr		merge(Nptr left, Nptr right, Nptr anchor);
  Nptr		shift(Nptr left, Nptr right, Nptr anchor);

public:
//			// public member functions
		Tree(unsigned int poolsz, unsigned int fan, KeyCmp keyCmp);
		~Tree();
//Tree		*remakeBtree(int fillfactor);

  inline Nptr	Nonode() { return
#ifdef POINTER
tree
#endif
			 - 1; }

  Nptr		Search(keyT key);
  void		Insert(keyT key);
  void		Delete(keyT key);

  void		listBtreeValues(Nptr start, int count);
  void		listAllBtreeValues();
};

#endif
